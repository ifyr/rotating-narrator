#ifndef AnimatedTextItem_H_
#define AnimatedTextItem_H_

#include <QGraphicsTextItem>

#include "Subtitle.h"
#include "EnterAnimation.h"

class QParallelAnimationGroup;

class AnimatedTextItem : public QGraphicsTextItem
{
    Q_OBJECT

public:
    AnimatedTextItem(const Subtitle & subtitle, QGraphicsItem * parent = nullptr);
    ~AnimatedTextItem();

    QRectF boundingRect() const override;
    Subtitle GetSubtitle();
    void SetReferenceItem(AnimatedTextItem * referenceItem, const bool & play = false);
    void ReferenceToParent();
    void TerminateAnimation();
    void PlayEnterAnimation(EnterAnimationType type);
    void PlayMoveAnimation(MoveAnimationType type);
    int GetConversationGroupWidth();
    AnimatedTextItem * ChildItem();
    void SetChildItem(AnimatedTextItem * item);

    void SetAvoidOverlap(const bool & avoidOverlap);
    void HidePastPart(const int & partIndex);

private:
    void UpdateTransformOriginPoint(AnimatedTextItem * referenceItem);

private:
    AnimatedTextItem * m_ChildItem = nullptr;
    AnimatedTextItem * m_ReferenceItem = nullptr;
    QParallelAnimationGroup * m_EnterAnimationGroup = nullptr;
    QParallelAnimationGroup * m_MoveAnimationGroup = nullptr;
    Subtitle m_Subtitle;
    int m_AnimationDelay = 200;
    bool m_AvoidOverlap = false;
};

#endif // ifndef SoundManager_H_
