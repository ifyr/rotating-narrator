#include "SubtitleAnimatorWidget.h"

#include <QtWidgets>

SubtitleAnimatorWidget::SubtitleAnimatorWidget(QWidget * parent)
  : QGraphicsView(parent)
  , m_Playing(false)
{
    setScene(&m_Scene);
    setRenderHint(QPainter::Antialiasing);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    SetBackgroundColor(QColor(5, 5, 5));
    setStyleSheet(QString("QGraphicsView { padding: -5;}"));

    Subtitle oriSbutitle;
    m_OriItem = new AnimatedTextItem(oriSbutitle);
    m_Scene.addItem(m_OriItem);

    installEventFilter(this);
}

void SubtitleAnimatorWidget::SetSubtitles(const QList<Subtitle> & subtitles)
{
    m_Subtitles = subtitles;
    SetCurrentIndex(0);
}

void SubtitleAnimatorWidget::SetBackgroundColor(const QColor & color)
{
    setBackgroundBrush(QBrush(color));
}

void SubtitleAnimatorWidget::SetBackgroundTransparent(const bool & transparent)
{
    m_IsTransparent = transparent;
}

void SubtitleAnimatorWidget::UpdateScreenStyle(
  const Style & style, const ScreenRatio & ratio,
  const int & resolutionType, const double & textHorizontalRatio)
{
    m_Style = style;

    int screenXR = 16;
    int screenYR = 9;

    switch (ratio) {
    case R_16_9: {
        screenXR = 16;
        screenYR = 9;
        break;
    }
    case R_9_16: {

        screenXR = 9;
        screenYR = 16;
        break;
    }
    }

    int resolution = 60;
    if (m_Style == Recording) {
        if (resolutionType == 0) {
            resolution = 80;
            m_OriItem->setScale(0.66);
            m_NormalScale = 0.66;
        } else if (resolutionType == 1) {
            resolution = 120;
            m_OriItem->setScale(1.0);
            m_NormalScale = 1.0;
        } else if (resolutionType == 2) {
            resolution = 120 * 2;
            m_OriItem->setScale(2.0);
            m_NormalScale = 2.0;
        }
    } else if (m_Style == Normal) {
        resolution = 60;
        m_OriItem->setScale(0.5);
        m_NormalScale = 0.5;
    } else if (m_Style == Small) {
        resolution = 30;
        m_OriItem->setScale(0.25);
        m_NormalScale = 0.25;
    }

    m_Scene.setSceneRect(0, 0, screenXR * (resolution + 1), screenYR * (resolution + 1));
    setFixedSize(screenXR * resolution, screenYR * resolution);

    QPointF viewCenter(screenXR * 0.5 * resolution, screenYR * 0.5 * resolution);
    QPointF textLeftBottom(screenXR * textHorizontalRatio * resolution, screenYR * 0.5 * resolution);
    centerOn(viewCenter);
    m_OriItem->setPos(textLeftBottom);
}

qint64 SubtitleAnimatorWidget::Elapsed() const
{
    return m_ElapsedTimer.elapsed();
}

void SubtitleAnimatorWidget::SetAvoidOverlap(const bool & avoidOverlap)
{
    m_AvoidOverlap = avoidOverlap;
}

void SubtitleAnimatorWidget::SetSameScreenPartCount(const int & value)
{
    m_SameScreenPartCount = value;
}

QColor SubtitleAnimatorWidget::GetBackgroundColor() const
{
    return backgroundBrush().color();
}

void SubtitleAnimatorWidget::SetCurrentIndex(const int & index)
{
    if (m_Playing || m_Subtitles.isEmpty()) {
        return;
    }
    foreach (auto * item, m_DisplayedItems) {
        item->TerminateAnimation();
        item->setParent(nullptr);
        m_Scene.removeItem(item);
        item->deleteLater();
    }
    m_DisplayedItems.clear();

    m_CurrentIndex = index - 1;

    if (m_CurrentIndex > m_Subtitles.size() || m_CurrentIndex < 0) {
        return;
    }

    int firstPartIndex = 0;
    if (m_SameScreenPartCount > 0) {
        firstPartIndex = m_Subtitles[m_CurrentIndex].part - m_SameScreenPartCount + 1;
    }

    for (int i = 0; i <= m_CurrentIndex; i++) {
        Subtitle subtitle = m_Subtitles.at(i);
        AnimatedTextItem * proxyItem = new AnimatedTextItem(subtitle, m_OriItem);

        if (subtitle.part < firstPartIndex) {
            proxyItem->setVisible(false);
        }

        proxyItem->SetAvoidOverlap(m_AvoidOverlap);
        if (m_DisplayedItems.size() > 0) {
            m_DisplayedItems.last()->SetReferenceItem(proxyItem);
        }
        m_DisplayedItems.push_back(proxyItem);

        double targetScale = m_NormalScale * subtitle.cameraZoom;
        m_OriItem->setScale(targetScale);
    }
}

void SubtitleAnimatorWidget::SetAutoPlay(const bool & play)
{
    m_Playing = play;
    if (m_Playing) {
        if (m_Subtitles.size() > 1 && m_CurrentIndex < m_Subtitles.size() - 1) {
            if (m_CurrentIndex <= 0) {
                m_TimestampBenchmark = 0;
            } else {
                m_TimestampBenchmark = m_Subtitles[m_CurrentIndex + 1].startTimeMs - 10;
            }
            m_ElapsedTimer.invalidate();
            m_ElapsedTimer.restart();
            ShowNextSubtitle();
        } else {
            m_Playing = false;
            m_ElapsedTimer.invalidate();
            emit SgnAnimationFinished(true);
        }
    } else {
        m_ElapsedTimer.invalidate();
    }
}

bool SubtitleAnimatorWidget::eventFilter(QObject * obj, QEvent * event)
{
    if (event->type() == QEvent::Wheel) {
        return true;
    }
    return QGraphicsView::eventFilter(obj, event);
}

void SubtitleAnimatorWidget::ShowNextSubtitle()
{
    if (!m_Playing) {
        return;
    }

    if (m_CurrentIndex >= m_Subtitles.size()) {
        m_Playing = false;
        emit SgnAnimationFinished(true);
        m_ElapsedTimer.invalidate();
        return;
    }

    int witeMilSec = 10;

    if (m_CurrentIndex == m_Subtitles.size() - 1) {
        const auto & subtitle = m_Subtitles[m_CurrentIndex];
        int timeOut = subtitle.startTimeMs + subtitle.durationMs - m_TimestampBenchmark;
        if (m_ElapsedTimer.elapsed() >= timeOut + 400) {
            m_CurrentIndex++;
        }
    } else {
        const auto & nextSubtitle = m_Subtitles[m_CurrentIndex + 1];
        qint64 startTime = nextSubtitle.startTimeMs - m_TimestampBenchmark;
        qint64 elapsed = m_ElapsedTimer.elapsed();

        if (elapsed >= startTime) {
            if (m_DisplayedItems.size() > 1) {
                m_DisplayedItems[m_DisplayedItems.size() - 2]->ReferenceToParent();
                if (m_SameScreenPartCount > 0) {
                    int firstPartIndex = nextSubtitle.part - m_SameScreenPartCount + 1;
                    m_DisplayedItems.last()->HidePastPart(firstPartIndex);
                }
            }
            m_CurrentIndex++;
            emit SgnIndexChange(m_CurrentIndex + 1);
            DisplaySubtitle(nextSubtitle);
        } else {
            const auto & curSubtitle = m_Subtitles[m_CurrentIndex];

            if (elapsed > curSubtitle.startTimeMs + 800 && elapsed <= startTime - 900) {
                m_ElapsedTimer.addMilliseconds(startTime - 850 - elapsed);
            }
        }
    }

    if (m_Playing) {
        QTimer::singleShot(witeMilSec, this, &SubtitleAnimatorWidget::ShowNextSubtitle);
    }
}

void SubtitleAnimatorWidget::DisplaySubtitle(const Subtitle & subtitle)
{
    AnimatedTextItem * proxyItem = new AnimatedTextItem(subtitle, m_OriItem);
    proxyItem->SetAvoidOverlap(m_AvoidOverlap);

    if (m_DisplayedItems.size() > 0) {
        m_DisplayedItems.last()->SetReferenceItem(proxyItem, true);
    }
    if (subtitle.durationMs > 650) {
        proxyItem->PlayEnterAnimation(subtitle.enterAnimation);
    }
    PlayCameraAnimation(subtitle.cameraZoom);
    m_DisplayedItems.push_back(proxyItem);
}

void SubtitleAnimatorWidget::StartScreenShake(int durationMs, int amplitude)
{
    QTimer * timer = new QTimer(this);
    const QPointF originalPos = this->pos();
    bool isShaken = false;

    connect(timer, &QTimer::timeout, this, [=]() mutable {
        if (isShaken) {
            this->move(originalPos.toPoint());
            timer->stop();
            timer->deleteLater();
        } else {
            int offsetX = QRandomGenerator::global()->bounded(-amplitude, amplitude + 1);
            int offsetY = QRandomGenerator::global()->bounded(-amplitude, amplitude + 1);
            this->move(originalPos.x() + offsetX, originalPos.y() + offsetY);
            isShaken = true;
        }
    });

    timer->start(durationMs / 2);
}

void SubtitleAnimatorWidget::PlayCameraAnimation(const double & zoom)
{
    double curScale = m_OriItem->scale();
    double targetScale = m_NormalScale * zoom;
    QPropertyAnimation * anim = new QPropertyAnimation(m_OriItem, "scale");
    anim->setDuration(300);
    anim->setStartValue(curScale);
    anim->setEndValue(targetScale);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}
