#ifndef USERSETTINGS_H
#define USERSETTINGS_H

#include <QColor>
#include "mysettings.h"

enum Side
{
    SideTop,
    SideLeft,
    SideRight,
    SideBottom
};

enum Direction
{
    LeftToRight,
    RightToLeft,
    TopToBottom,
    BottomToTop
};

class UserSettings : public MySettings
{
public:
    UserSettings(QObject* parent = nullptr);

private:
    void restoreSettings();

public:
    QString host;

    Side floatSide = SideRight; // 边：0上，1左，2右，3下，-1任意
    Direction floatDirection = TopToBottom; // 方向：0从左到右，1从右到左，2从上到下，3从下到上
    int floatPixel = 500; // 按照这条边的绝对像素高度

    int bannerSpacing = 10; // 横幅之间的间距
    int bannerWidth = 300;  // 横幅的固定宽度
    int bannerAnimationDuration = 300; // 横幅的动画时长
    int bannerShowEasingCurve = 0;     // 动画的曲线设定
    int bannerDisplayDuration = 5000;  // 横幅显示的时长
    QColor bannerBgColor = QColor(255, 250, 250);
    int bannerBgRadius = 12;
    int bannerBgShadow = 12;
};

#endif // USERSETTINGS_H
