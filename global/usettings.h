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

class USettings : public MySettings
{
public:
    USettings(QObject* parent = nullptr);

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
    int bannerShowEasingCurve = 6;     // 动画的曲线设定
    int bannerDisplayDuration = 7000;  // 横幅显示的时长
    int bannerReaderSpeed = 10; // 按字数增加时长，每秒钟看多少字
    int bannerRetentionDuration = 1000; // 失去焦点后滞留时长
    QColor bannerBgColor = QColor(255, 255, 255); // 默认背景颜色
    bool bannerBgColorByHeader = true; // 使用头像颜色填充背景颜色
    int bannerBgRadius = 5; // 圆角大小
    int bannerBgShadow = 12; // 阴影大小，右下角会空出这么多的margin
    int bannerMsgMaxLength = 300; // 消息最大长度
    bool bannerShowImages = true; // 显示图片，还是只显示[图片]
    bool bannerCloseAfterReply = true; // 回复后关闭对话框（Ctrl+Enter切换）
    int bannerScreenIndex = 0; // 使用哪一个屏幕
    int bannerTitleLarger = 4; // 大标题（发送者/群名）变大
    int bannerSubTitleLarger= 2; // 二级标题（群聊的发送者）字体
    int bannerMessageAlpha = 255; // 消息透明度
};

#endif // USERSETTINGS_H
