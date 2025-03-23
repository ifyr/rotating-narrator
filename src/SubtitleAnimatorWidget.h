#ifndef SubtitleAnimatorWidget_H_
#define SubtitleAnimatorWidget_H_

#include <QGraphicsView>
#include <QElapsedTimer>

#include "Subtitle.h"
#include "AnimatedTextItem.h"

class AdjustableTimer
{
public:
    AdjustableTimer()
      : offset(0)
    {
    }

    void invalidate()
    {
        timer.invalidate();
        offset = 0;
    }

    void restart()
    {
        timer.start();
        offset = 0;
    }

    void addMilliseconds(qint64 ms)
    {
        offset += ms;
    }

    qint64 elapsed() const
    {
        return timer.elapsed() + offset;
    }

private:
    QElapsedTimer timer;
    qint64 offset;
};

class SubtitleAnimatorWidget : public QGraphicsView
{
    Q_OBJECT

public:
    enum Style
    {
        Normal,
        Small,
        Recording
    };

    enum ScreenRatio
    {
        R_16_9,
        R_9_16,
    };

public:
    SubtitleAnimatorWidget(QWidget * parent = nullptr);

    qint64 Elapsed() const;
    QColor GetBackgroundColor() const;

    void SetSubtitles(const QList<Subtitle> & subtitles);
    void SetBackgroundColor(const QColor & color);
    void SetBackgroundTransparent(const bool & transparent);

    void SetCurrentIndex(const int & index);
    void SetAutoPlay(const bool & play);

    void SetAvoidOverlap(const bool & avoidOverlap);
    void SetSameScreenPartCount(const int & value);

    void UpdateScreenStyle(
      const Style & style, const ScreenRatio & ratio,
      const int & resolution, const double & textHorizontalRatio);

signals:

    void SgnIndexChange(int index);
    void SgnAnimationFinished(const bool & generate);

protected:
    bool eventFilter(QObject * obj, QEvent * event) override;

private:
    void ShowNextSubtitle();
    void DisplaySubtitle(const Subtitle & subtitle);
    void StartScreenShake(int durationMs, int amplitude);

    void PlayCameraAnimation(const double & zoom);

private:
    QGraphicsScene m_Scene;
    AnimatedTextItem * m_OriItem;

    QList<Subtitle> m_Subtitles;
    QList<AnimatedTextItem *> m_DisplayedItems;

    AdjustableTimer m_ElapsedTimer;
    qint64 m_TimestampBenchmark = 0;

    int m_CurrentIndex = 0;
    bool m_Playing;

    int m_SameScreenPartCount = -1;
    bool m_AvoidOverlap = false;

    Style m_Style = Normal;
    double m_NormalScale = 1.0;

    bool m_IsTransparent = false;
};

#endif // ifndef SoundManager_H_
