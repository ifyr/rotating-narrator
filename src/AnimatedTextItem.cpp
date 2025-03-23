#include "AnimatedTextItem.h"

#include <QtWidgets>

AnimatedTextItem::AnimatedTextItem(const Subtitle & subtitle, QGraphicsItem * parent)
  : QGraphicsTextItem(subtitle.text, parent)
{
    m_Subtitle = subtitle;
    setFont(m_Subtitle.font);
    setDefaultTextColor(m_Subtitle.fontColor);
    setZValue(1);
    m_EnterAnimationGroup = new QParallelAnimationGroup(this);
    m_MoveAnimationGroup = new QParallelAnimationGroup(this);
}

AnimatedTextItem::~AnimatedTextItem()
{
    if (m_EnterAnimationGroup) {
        m_EnterAnimationGroup->clear();
        m_EnterAnimationGroup->deleteLater();
        m_EnterAnimationGroup = nullptr;
    }
    if (m_MoveAnimationGroup) {
        m_MoveAnimationGroup->clear();
        m_MoveAnimationGroup->deleteLater();
        m_MoveAnimationGroup = nullptr;
    }
}

void AnimatedTextItem::SetReferenceItem(AnimatedTextItem * referenceItem, const bool & play)
{
    Subtitle refSubtitle = referenceItem->GetSubtitle();
    int delayMs = refSubtitle.startTimeMs - m_Subtitle.startTimeMs;
    bool executionPlay = play && delayMs > m_AnimationDelay;

    if (executionPlay) {
        m_ReferenceItem = referenceItem;
        PlayMoveAnimation(m_Subtitle.moveAnimation);
    } else {
        MoveAnimationType type = m_Subtitle.moveAnimation;
        if (type == MoveAnimationType::MoveUp) {
            setPos(pos() - QPointF(0, boundingRect().height()));
        } else if (type == MoveAnimationType::RotateClockwise) {
            UpdateTransformOriginPoint(referenceItem);
            setRotation(rotation() - 90);
        } else if (type == MoveAnimationType::RotateCounterclockwise) {
            UpdateTransformOriginPoint(referenceItem);
            setRotation(rotation() + 90);
        }
        setParentItem(referenceItem);
        referenceItem->SetChildItem(this);
    }
}

void AnimatedTextItem::ReferenceToParent()
{
    if (m_ReferenceItem != nullptr) {
        setParentItem(m_ReferenceItem);
        m_ReferenceItem->SetChildItem(this);
        m_ReferenceItem = nullptr;
    }
}

QRectF AnimatedTextItem::boundingRect() const
{
    return QGraphicsTextItem::boundingRect();
}

Subtitle AnimatedTextItem::GetSubtitle()
{
    return m_Subtitle;
}

void AnimatedTextItem::TerminateAnimation()
{
    if (m_EnterAnimationGroup) {
        m_EnterAnimationGroup->stop();
        for (int i = 0; i < m_EnterAnimationGroup->animationCount(); i++) {
            QAbstractAnimation * animation = m_EnterAnimationGroup->animationAt(i);
            animation->setParent(nullptr);
            animation->deleteLater();
        }
    }
    if (m_MoveAnimationGroup) {
        m_MoveAnimationGroup->stop();
        for (int i = 0; i < m_MoveAnimationGroup->animationCount(); i++) {
            QAbstractAnimation * animation = m_MoveAnimationGroup->animationAt(i);
            animation->setParent(nullptr);
            animation->deleteLater();
        }
    }
}

void AnimatedTextItem::PlayEnterAnimation(EnterAnimationType type)
{
    TerminateAnimation();
    EnterAnimation * animation = EnterAnimationFactory::CreateAnimation(type);
    if (animation) {
        animation->Play(this);
    }
}

void AnimatedTextItem::PlayMoveAnimation(MoveAnimationType type)
{
    TerminateAnimation();
    if (type == MoveAnimationType::MoveUp) {
        QRectF rect = boundingRect();
        QPropertyAnimation * moveUpAnim = new QPropertyAnimation(this, "pos");
        moveUpAnim->setDuration(m_AnimationDelay);
        moveUpAnim->setStartValue(pos());
        moveUpAnim->setEndValue(pos() - QPointF(0, rect.height()));
        m_MoveAnimationGroup->addAnimation(moveUpAnim);
    } else if (type == MoveAnimationType::RotateClockwise) {
        UpdateTransformOriginPoint(m_ReferenceItem);
        QPropertyAnimation * rotateAnim = new QPropertyAnimation(this, "rotation");
        qreal currentRotation = rotation();
        rotateAnim->setDuration(m_AnimationDelay);
        rotateAnim->setStartValue(currentRotation);
        rotateAnim->setEndValue(currentRotation - 90);
        m_MoveAnimationGroup->addAnimation(rotateAnim);
    } else if (type == MoveAnimationType::RotateCounterclockwise) {
        UpdateTransformOriginPoint(m_ReferenceItem);
        QPropertyAnimation * rotateAnim = new QPropertyAnimation(this, "rotation");
        qreal currentRotation = rotation();
        rotateAnim->setDuration(m_AnimationDelay);
        rotateAnim->setStartValue(currentRotation);
        rotateAnim->setEndValue(currentRotation + 90);
        m_MoveAnimationGroup->addAnimation(rotateAnim);
    }
    m_MoveAnimationGroup->start(QAbstractAnimation::KeepWhenStopped);
}

int AnimatedTextItem::GetConversationGroupWidth()
{
    qreal totalWidth = boundingRect().width();

    AnimatedTextItem * currentItem = dynamic_cast<AnimatedTextItem *>(ChildItem());
    while (currentItem) {
        MoveAnimationType type = currentItem->GetSubtitle().moveAnimation;
        if (IsPartSeparator(type)) {
            break;
        }
        totalWidth = qMax(totalWidth, currentItem->boundingRect().width());
        currentItem = dynamic_cast<AnimatedTextItem *>(currentItem->ChildItem());
    }
    return totalWidth;
}

AnimatedTextItem * AnimatedTextItem::ChildItem()
{
    return m_ChildItem;
}

void AnimatedTextItem::SetChildItem(AnimatedTextItem * item)
{
    m_ChildItem = item;
}

void AnimatedTextItem::SetAvoidOverlap(const bool & avoidOverlap)
{
    m_AvoidOverlap = avoidOverlap;
}

void AnimatedTextItem::HidePastPart(const int & partIndex)
{
    AnimatedTextItem * currentItem = this;
    while (currentItem) {
        if (currentItem->GetSubtitle().part < partIndex) {
            currentItem->setVisible(false);
            break;
        }
        currentItem = dynamic_cast<AnimatedTextItem *>(currentItem->ChildItem());
    }
}

void AnimatedTextItem::UpdateTransformOriginPoint(AnimatedTextItem * referenceItem)
{
    MoveAnimationType type = m_Subtitle.moveAnimation;
    if (type == MoveAnimationType::RotateClockwise) {
        QRectF rect = boundingRect();
        QPointF pivotPoint = referenceItem->mapToScene(referenceItem->boundingRect().bottomLeft());
        setTransformOriginPoint(mapFromScene(pivotPoint));
    } else if (type == MoveAnimationType::RotateCounterclockwise) {
        QRectF rect = boundingRect();
        if (m_AvoidOverlap) {
            rect.setWidth(GetConversationGroupWidth());
        }
        QPointF pivotPoint1 = referenceItem->mapToScene(referenceItem->boundingRect().bottomRight());
        QPointF pivotPoint2 = mapToScene(rect.bottomRight());
        QPointF pivotPoint = (pivotPoint1.x() > pivotPoint2.x()) ? pivotPoint1 : pivotPoint2;
        setTransformOriginPoint(mapFromScene(pivotPoint));
    }
}
