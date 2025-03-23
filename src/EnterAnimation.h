#ifndef EnterAnimation_H_
#define EnterAnimation_H_

#include <QGraphicsTextItem>

#include "Subtitle.h"

class QParallelAnimationGroup;

class EnterAnimation : public QObject
{
    Q_OBJECT
public:
    virtual void Play(QGraphicsTextItem * item) = 0;
};

class EnterAnimationFactory
{
public:
    static EnterAnimation * CreateAnimation(EnterAnimationType type);
    static quint32 GetAnimationDuration(EnterAnimationType type);
};

class TypewriterAnimation : public EnterAnimation
{
    Q_OBJECT
public:
    void Play(QGraphicsTextItem * item) override;
};

class ZoomInAnimation : public EnterAnimation
{
    Q_OBJECT
public:
    void Play(QGraphicsTextItem * item) override;
};

class FadeInAnimation : public EnterAnimation
{
    Q_OBJECT
public:
    void Play(QGraphicsTextItem * item) override;
};

class SlideInFromLeftAnimation : public EnterAnimation
{
    Q_OBJECT
public:
    void Play(QGraphicsTextItem * item) override;
};

class BounceInAnimation : public EnterAnimation
{
    Q_OBJECT
public:
    void Play(QGraphicsTextItem * item) override;
};

class SlideInFromRightAnimation : public EnterAnimation
{
    Q_OBJECT
public:
    void Play(QGraphicsTextItem * item) override;
};

class FadeOutAndInAnimation : public EnterAnimation
{
    Q_OBJECT
public:
    void Play(QGraphicsTextItem * item) override;
};

class ShakeAnimation : public EnterAnimation
{
    Q_OBJECT
public:
    void Play(QGraphicsTextItem * item) override;
};

class RotateInAnimation : public EnterAnimation
{
    Q_OBJECT
public:
    void Play(QGraphicsTextItem * item) override;
};

class PopUpAnimation : public EnterAnimation
{
    Q_OBJECT
public:
    void Play(QGraphicsTextItem * item) override;
};

#endif // ifndef EnterAnimation_H_
