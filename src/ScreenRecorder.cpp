#include "ScreenRecorder.h"
#include "SubtitleAnimatorWidget.h"

#include <QtWidgets>
#include <QtConcurrent>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

static const double cropRatio = 0.8;

void logError(int errnum)
{
    char errbuf[AV_ERROR_MAX_STRING_SIZE];
    av_make_error_string(errbuf, AV_ERROR_MAX_STRING_SIZE, errnum);
    qWarning("Error occurred: %s", errbuf);
}

ScreenRecorder::ScreenRecorder(SubtitleAnimatorWidget * view, QObject * parent)
  : QObject(parent)
  , m_View(view)
{
    QDir dir;
    if (!dir.exists("./out")) {
        if (dir.mkpath("./out")) {
        }
    }
}

ScreenRecorder::~ScreenRecorder()
{
}

void ScreenRecorder::StartRecording(const bool & transparent)
{
    m_IsTransparent = transparent;
    m_Width = m_View->width();
    m_Height = m_View->height();

    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QRegularExpression regex("[\u4e00-\u9fa5]");
    if (desktopPath.contains(regex)) {
        qInfo() << "detected, changing to default path.";
        desktopPath = "./out";
    }
    if (m_IsTransparent) {
        m_FilePath = desktopPath + "/" + QDateTime::currentDateTime().toString("dd-hh-mm-ss") + ".mkv";
    } else {
        m_FilePath = desktopPath + "/" + QDateTime::currentDateTime().toString("dd-hh-mm-ss") + ".ts";
    }

    InitFFmpeg();

    //while (!QThread::currentThread()->isInterruptionRequested()) {
    while (m_Manager->GetIsGenerating()) {
        QThread::msleep(5);
        QApplication::processEvents();
        qint64 currentTime = m_View->Elapsed();
        if (currentTime < 0 || (m_LastCaptureTime > 0 && currentTime - m_LastCaptureTime < m_CaptureInterval)) {
            continue;
        }
        m_LastCaptureTime = currentTime;
        QImage img;
        QMetaObject::invokeMethod(
          m_View, [this, &img]() {
              QPixmap pixmap(m_View->size());
              QPainter painter(&pixmap);
              painter.setRenderHint(QPainter::Antialiasing);
              painter.setRenderHint(QPainter::TextAntialiasing);
              m_View->render(&painter);
              painter.end();
              img = pixmap.toImage();
              img = img.copy(0, 0, m_Width, m_Height * cropRatio);
          },
          Qt::BlockingQueuedConnection);
        const uint8_t * srcData[1] = { img.bits() };
        int srcLinesize[1] = { static_cast<int>(img.bytesPerLine()) };
        if (m_IsTransparent) {
            sws_scale(swsCtx, srcData, srcLinesize, 0, m_Height * cropRatio, avFrame->data, avFrame->linesize);
        } else {
            sws_scale(swsCtx, srcData, srcLinesize, 0, m_Height * cropRatio, avFrame->data, avFrame->linesize);
        }
        avFrame->pts = m_View->Elapsed();
        EncodeFrame();
    }

    if (!m_IsTransparent) {
        if (formatCtx) {
            av_write_trailer(formatCtx);
        }

        if (codecCtx) {
            avcodec_free_context(&codecCtx);
            codecCtx = nullptr;
        }

        if (formatCtx) {
            if (!(formatCtx->oformat->flags & AVFMT_NOFILE)) {
                avio_closep(&formatCtx->pb);
            }
            avformat_free_context(formatCtx);
            formatCtx = nullptr;
        }

        if (avFrame) {
            av_frame_free(&avFrame);
            avFrame = nullptr;
        }

        if (swsCtx) {
            sws_freeContext(swsCtx);
            swsCtx = nullptr;
        }
    }
    

    emit SgnRecordingEnd();
}

void ScreenRecorder::StopRecording()
{
    QThread::currentThread()->requestInterruption();
}

void ScreenRecorder::SetScreenRecorderManager(ScreenRecorderManager * manager)
{
    m_Manager = manager;
}

void ScreenRecorder::InitFFmpeg()
{
    if (m_IsTransparent) {
        avformat_alloc_output_context2(&formatCtx, nullptr, "matroska", m_FilePath.toLocal8Bit().data());
    } else {
        avformat_alloc_output_context2(&formatCtx, nullptr, nullptr, m_FilePath.toLocal8Bit().data());
    }

    if (!formatCtx) {
        qWarning("Could not create output context");
        return;
    }

    const AVCodec * codec;
    if (m_IsTransparent) {
        codec = avcodec_find_encoder_by_name("prores_ks");
    } else {
        codec = avcodec_find_encoder_by_name("libx264");
    }

    videoStream = avformat_new_stream(formatCtx, nullptr);
    codecCtx = avcodec_alloc_context3(codec);

    codecCtx->codec_id = codec->id;

    if (m_IsTransparent) {
        codecCtx->width = m_Width;
        codecCtx->height = m_Height * cropRatio;
        codecCtx->gop_size = 10;
        codecCtx->max_b_frames = 0;
        codecCtx->time_base = AVRational { 1, 1000 };
        codecCtx->bit_rate = 100000;
        codecCtx->pix_fmt = AV_PIX_FMT_YUVA444P10LE;
    } else {
        codecCtx->width = m_Width;
        codecCtx->height = m_Height * cropRatio;
        codecCtx->gop_size = 30;
        codecCtx->max_b_frames = 0;
        codecCtx->time_base = AVRational { 1, 1000 };
        codecCtx->bit_rate = 50000000;
        codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    }

    if (formatCtx->oformat->flags & AVFMT_GLOBALHEADER) {
        codecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    avcodec_open2(codecCtx, codec, nullptr);

    avcodec_parameters_from_context(videoStream->codecpar, codecCtx);
    videoStream->time_base = AVRational { 1, 1000 };

    if (!(formatCtx->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&formatCtx->pb, m_FilePath.toLocal8Bit().data(), AVIO_FLAG_WRITE) < 0) {
            qWarning("Could not open output file");
            return;
        }
    }

    int ret = avformat_write_header(formatCtx, nullptr);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        qWarning("Error occurred when opening output file: %s", errbuf);
        return;
    }

    avFrame = av_frame_alloc();
    if (!avFrame) {
        qWarning("Could not allocate video frame");
        return;
    }
    avFrame->format = codecCtx->pix_fmt;
    avFrame->width = codecCtx->width;
    avFrame->height = codecCtx->height;

    if (av_frame_get_buffer(avFrame, 32) < 0) {
        qWarning("Could not allocate the video frame data");
        return;
    }

    if (m_IsTransparent) {
        swsCtx = sws_getContext(m_Width, m_Height * cropRatio, AV_PIX_FMT_BGRA,
                                m_Width, m_Height * cropRatio, AV_PIX_FMT_YUVA444P10LE,
                                SWS_POINT, nullptr, nullptr, nullptr);
    } else {
        swsCtx = sws_getContext(m_Width, m_Height * cropRatio, AV_PIX_FMT_RGB32,
                                m_Width, m_Height * cropRatio, AV_PIX_FMT_YUV420P,
                                SWS_BICUBIC, nullptr, nullptr, nullptr);
    }
}

void ScreenRecorder::EncodeFrame()
{
    if (av_frame_make_writable(avFrame) < 0) {
        qWarning("Frame is not writable");
        return;
    }
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = nullptr;
    pkt.size = 0;
    int ret = avcodec_send_frame(codecCtx, avFrame);
    if (ret < 0) {
        logError(ret);
        return;
    }
    while (ret >= 0) {
        ret = avcodec_receive_packet(codecCtx, &pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            logError(ret);
            return;
        }
        pkt.stream_index = videoStream->index;
        av_packet_rescale_ts(&pkt, codecCtx->time_base, videoStream->time_base);
        ret = av_interleaved_write_frame(formatCtx, &pkt);
        if (ret < 0) {
            logError(ret);
            av_packet_unref(&pkt);
            return;
        }
        av_packet_unref(&pkt);
    }
}

ScreenRecorderManager::ScreenRecorderManager(SubtitleAnimatorWidget * view, QObject * parent)
  : QObject(parent)
  , m_View(view)
  , m_Thread(new QThread(this))
  , m_Recorder(nullptr)
{
    m_Recorder = new ScreenRecorder(view);
    m_Recorder->moveToThread(m_Thread);
    m_Recorder->SetScreenRecorderManager(this);
    m_Thread->start();

    connect(this, &ScreenRecorderManager::SgnStartRecording, m_Recorder, &ScreenRecorder::StartRecording);
    connect(this, &ScreenRecorderManager::SgnStopRecording, m_Recorder, &ScreenRecorder::StopRecording);
    connect(m_Recorder, &ScreenRecorder::SgnRecordingEnd, this, &ScreenRecorderManager::SgnCleanupFinished);
}

ScreenRecorderManager::~ScreenRecorderManager()
{
    if (m_Thread) {
        m_Thread->quit();
        m_Thread->wait();
        m_Thread->deleteLater();
        m_Thread = nullptr;
    }

    if (m_Recorder) {
        m_Recorder->setParent(nullptr);
        m_Recorder->deleteLater();
        m_Recorder = nullptr;
    }
}

void ScreenRecorderManager::StartRecording(bool isTransparent)
{
    m_IsGenerating = true;
    emit SgnStartRecording(isTransparent);
}

void ScreenRecorderManager::StopRecording()
{
    QTimer::singleShot(500, this, [&]() {
        m_IsGenerating = false;
    });
}

bool ScreenRecorderManager::GetIsGenerating() const
{
    QMutexLocker locker(&m_Mutex); // 自动加锁和解锁
    return m_IsGenerating;
}
