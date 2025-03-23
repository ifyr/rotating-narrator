#ifndef Subtitle_H
#define Subtitle_H

#include <QString>
#include <QColor>
#include <QFont>

enum class EnterAnimationType
{
    FadeIn = 0, // 淡入
    ZoomIn, // 缩放
    Typewriter, // 打字机
    SlideInFromLeft, // 从左滑入
    BounceIn, // 弹跳效果
    SlideInFromRight, // 从右滑入
    FadeOutAndIn, // 淡出再淡入
    PopUp, // 弹出效果
    Shake, // 摇晃
    RotateIn // 旋转进入
};

enum class MoveAnimationType
{
    MoveUp = 0,
    RotateClockwise,
    RotateCounterclockwise,
};

struct Subtitle
{
    int part = 0;
    QString text;
    qint64 startTimeMs;
    qint64 durationMs;
    QFont font;
    QColor fontColor = Qt::darkGray;
    EnterAnimationType enterAnimation;
    MoveAnimationType moveAnimation;
    double cameraZoom = 1.0;
};

static QString AnimationToString(EnterAnimationType animation)
{
    switch (animation) {
    case EnterAnimationType::FadeIn:
        return u8"淡入";

    case EnterAnimationType::ZoomIn:
        return u8"放大";

    case EnterAnimationType::Typewriter:
        return u8"打字机";

    case EnterAnimationType::SlideInFromLeft:
        return u8"从左划入";

    case EnterAnimationType::BounceIn:
        return u8"弹跳效果";

    case EnterAnimationType::SlideInFromRight:
        return u8"从右滑入";

    case EnterAnimationType::FadeOutAndIn:
        return u8"淡出再淡入";

    case EnterAnimationType::PopUp:
        return u8"弹出效果";

    case EnterAnimationType::Shake:
        return u8"摇晃";

    case EnterAnimationType::RotateIn:
        return u8"旋转进入";
    }

    return {};
}

static QString AnimationToString(MoveAnimationType animation)
{
    switch (animation) {
    case MoveAnimationType::MoveUp:
        return u8"向上";

    case MoveAnimationType::RotateClockwise:
        return u8"顺时针90";

    case MoveAnimationType::RotateCounterclockwise:
        return u8"逆时针90";
    }
    return {};
}

static inline bool IsPartSeparator(MoveAnimationType animation)
{
    switch (animation) {
    case MoveAnimationType::RotateClockwise:
    case MoveAnimationType::RotateCounterclockwise:
        return true;
    }
    return false;
}

#endif // Subtitle_H
