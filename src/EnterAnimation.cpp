#include "EnterAnimation.h"
#include <QtWidgets>

EnterAnimation * EnterAnimationFactory::CreateAnimation(EnterAnimationType type)
{
    switch (type) {
    case EnterAnimationType::FadeIn:
        return new FadeInAnimation();
    case EnterAnimationType::ZoomIn:
        return new ZoomInAnimation();
    case EnterAnimationType::Typewriter:
        return new TypewriterAnimation();
    case EnterAnimationType::SlideInFromLeft:
        return new SlideInFromLeftAnimation();
    case EnterAnimationType::BounceIn:
        return new BounceInAnimation();
    case EnterAnimationType::SlideInFromRight:
        return new SlideInFromRightAnimation();
    case EnterAnimationType::FadeOutAndIn:
        return new FadeOutAndInAnimation();
    case EnterAnimationType::PopUp:
        return new PopUpAnimation();
    case EnterAnimationType::Shake:
        return new ShakeAnimation();
    case EnterAnimationType::RotateIn:
        return new RotateInAnimation();
    default:
        return new FadeInAnimation();
    }
}

quint32 EnterAnimationFactory::GetAnimationDuration(EnterAnimationType type)
{
    switch (type) {
    case EnterAnimationType::FadeIn:
        return 300;
    case EnterAnimationType::ZoomIn:
        return 300;
    case EnterAnimationType::Typewriter:
        return 600;
    case EnterAnimationType::SlideInFromLeft:
        return 400;
    case EnterAnimationType::BounceIn:
        return 600;
    case EnterAnimationType::SlideInFromRight:
        return 400;
    case EnterAnimationType::FadeOutAndIn:
        return 450;
    case EnterAnimationType::PopUp:
        return 300;
    case EnterAnimationType::Shake:
        return 400;
    case EnterAnimationType::RotateIn:
        return 500;
    default:
        return 800;
    }
}

void TypewriterAnimation::Play(QGraphicsTextItem * item)
{
    // 实现打字机效果：逐字显示文本内容
    QTimer * timer = new QTimer(item);
    QString text = item->toPlainText();
    item->setPlainText("");
    int length = text.length();
    int interval = 600 / length;
    interval = qMin(interval, 50);
    int * index = new int(0);
    QObject::connect(timer, &QTimer::timeout, [=]() mutable {
        if (*index < length) {
            item->setPlainText(item->toPlainText() + text[*index]);
            (*index)++;
        } else {
            timer->stop();
            delete index;
        }
    });
    timer->start(interval - 5);
}

void FadeInAnimation::Play(QGraphicsTextItem * item)
{
    // 实现淡入效果：调整透明度从 0 到 1
    QPropertyAnimation * opacityAnim = new QPropertyAnimation(item, "opacity");
    opacityAnim->setDuration(300);
    opacityAnim->setStartValue(0);
    opacityAnim->setEndValue(1);
    opacityAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void ZoomInAnimation::Play(QGraphicsTextItem * item)
{
    // 实现缩放进入效果：从较小的比例放大到正常比例
    QPropertyAnimation * scaleAnim = new QPropertyAnimation(item, "scale");
    scaleAnim->setDuration(300);
    scaleAnim->setStartValue(0.5);
    scaleAnim->setEndValue(1.0);
    scaleAnim->setKeyValueAt(0.5, 1.1);
    scaleAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void SlideInFromLeftAnimation::Play(QGraphicsTextItem * item)
{
    // 实现从左侧滑入效果
    QPropertyAnimation * slideAnim = new QPropertyAnimation(item, "pos");
    slideAnim->setDuration(400);
    QPointF startPos = item->pos() - QPointF(100, 0);
    slideAnim->setStartValue(startPos);
    slideAnim->setEndValue(item->pos());
    slideAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void BounceInAnimation::Play(QGraphicsTextItem * item)
{
    // 实现弹跳效果：放大并多次缩放调整
    QPropertyAnimation * bounceAnim = new QPropertyAnimation(item, "scale");
    bounceAnim->setDuration(600);
    bounceAnim->setStartValue(0.3);
    bounceAnim->setKeyValueAt(0.3, 1.3);
    bounceAnim->setKeyValueAt(0.5, 0.8);
    bounceAnim->setKeyValueAt(0.7, 1.1);
    bounceAnim->setKeyValueAt(0.9, 0.95);
    bounceAnim->setEndValue(1.0);
    bounceAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void ShakeAnimation::Play(QGraphicsTextItem * item)
{
    // 实现摇晃效果：快速左右移动
    QPropertyAnimation * shakeAnim = new QPropertyAnimation(item, "pos");
    shakeAnim->setDuration(400);
    shakeAnim->setKeyValueAt(0.0, item->pos());
    shakeAnim->setKeyValueAt(0.2, item->pos() + QPointF(-5, 0));
    shakeAnim->setKeyValueAt(0.4, item->pos() + QPointF(5, 0));
    shakeAnim->setKeyValueAt(0.6, item->pos() + QPointF(-5, 0));
    shakeAnim->setKeyValueAt(0.8, item->pos() + QPointF(5, 0));
    shakeAnim->setKeyValueAt(1.0, item->pos());
    shakeAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void FadeOutAndInAnimation::Play(QGraphicsTextItem * item)
{
    // 实现淡出再淡入效果
    QSequentialAnimationGroup * fadeGroup = new QSequentialAnimationGroup(item);

    QPropertyAnimation * fadeOutAnim = new QPropertyAnimation(item, "opacity");
    fadeOutAnim->setDuration(200);
    fadeOutAnim->setStartValue(1);
    fadeOutAnim->setEndValue(0);

    QPropertyAnimation * fadeInAnim = new QPropertyAnimation(item, "opacity");
    fadeInAnim->setDuration(200);
    fadeInAnim->setStartValue(0);
    fadeInAnim->setEndValue(1);

    fadeGroup->addAnimation(fadeOutAnim);
    fadeGroup->addAnimation(fadeInAnim);
    fadeGroup->start(QAbstractAnimation::DeleteWhenStopped);
}

void SlideInFromRightAnimation::Play(QGraphicsTextItem * item)
{
    // 实现从右侧滑入效果
    QPropertyAnimation * slideAnim = new QPropertyAnimation(item, "pos");
    slideAnim->setDuration(400);
    QPointF startPos = item->pos() + QPointF(100, 0);
    slideAnim->setStartValue(startPos);
    slideAnim->setEndValue(item->pos());
    slideAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void RotateInAnimation::Play(QGraphicsTextItem * item)
{
    // 实现旋转进入效果：从 -90 度旋转到 0 度
    item->setTransformOriginPoint(item->boundingRect().center());
    QPropertyAnimation * rotateAnim = new QPropertyAnimation(item, "rotation");
    rotateAnim->setDuration(500);
    rotateAnim->setStartValue(-90);
    rotateAnim->setEndValue(0);
    rotateAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void PopUpAnimation::Play(QGraphicsTextItem * item)
{
    // 实现弹出效果：从略小于正常大小放大到正常大小
    QPropertyAnimation * popUpAnim = new QPropertyAnimation(item, "scale");
    popUpAnim->setDuration(300);
    popUpAnim->setStartValue(0.8);
    popUpAnim->setEndValue(1.0);
    popUpAnim->start(QAbstractAnimation::DeleteWhenStopped);
}
