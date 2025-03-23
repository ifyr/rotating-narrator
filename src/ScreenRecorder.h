#ifndef ScreenRecorder_H_
#define ScreenRecorder_H_

#include <QObject>
#include <QImage>
#include <QPainter>
#include <QMutex>
#include <QDateTime>
#include <QString>
#include <QQueue>
#include <QWaitCondition>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>

class SubtitleAnimatorWidget;
class AVFormatContext;
class AVCodecContext;
class AVStream;
class AVFrame;
class SwsContext;
class ScreenRecorderManager;

class ScreenRecorder : public QObject
{
    Q_OBJECT

public:
    ScreenRecorder(SubtitleAnimatorWidget * view, QObject * parent = nullptr);
    ~ScreenRecorder();

    void StartRecording(const bool & transparent);
    void StopRecording();

    void SetScreenRecorderManager(ScreenRecorderManager * manager);

signals:
    void SgnRecordingEnd();

private:
    void InitFFmpeg();
    void EncodeFrame();

private:
    struct Frame
    {
        QImage image;
        qint64 timestamp;

        Frame(const QImage & img, qint64 ts)
          : image(img)
          , timestamp(ts)
        {
        }
    };

private:
    int m_Width;
    int m_Height;
    QString m_FilePath;

    AVFormatContext * formatCtx = nullptr;
    AVCodecContext * codecCtx = nullptr;
    AVStream * videoStream = nullptr;
    AVFrame * avFrame = nullptr;
    SwsContext * swsCtx = nullptr;

    int64_t startTime = 0;

    qint64 m_LastCaptureTime = 0;
    int m_CaptureInterval = 10;
    SubtitleAnimatorWidget * m_View;

    bool m_IsTransparent = false;

    ScreenRecorderManager * m_Manager = nullptr;
};

class ScreenRecorderManager : public QObject
{
    Q_OBJECT

public:
    ScreenRecorderManager(SubtitleAnimatorWidget * view, QObject * parent = nullptr);
    ~ScreenRecorderManager();

    void StartRecording(bool isTransparent);
    void StopRecording();

    bool GetIsGenerating() const;

signals:
    void SgnCleanupFinished();
    void SgnStartRecording(bool isTransparent);
    void SgnStopRecording();

private:
    QThread * m_Thread;
    ScreenRecorder * m_Recorder;
    SubtitleAnimatorWidget * m_View;

    bool m_IsGenerating = false;

     mutable QMutex m_Mutex;
};

#endif // ScreenRecorder_H_
