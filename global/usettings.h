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

#define Unimportant -1
#define UnimportantText "不重要"
#define NormalImportant 0
#define NormalImportantText "一般"
#define Important 1
#define ImportantText "重要"
#define VeryImportant 2
#define VeryImportantText "很重要"

class USettings : public MySettings
{
public:
    USettings(QString filePath, QObject* parent = nullptr);

private:
    void restoreSettings();

public:
    bool isGroupShow(qint64 groupId);

public:
    QString host;
    QString accessToken;

    // 横幅通知位置
    Side bannerFloatSide = SideRight; // 边：0上，1左，2右，3下，-1任意
    Direction bannerFloatDirection = TopToBottom; // 方向：0从左到右，1从右到左，2从上到下，3从下到上
    int bannerFloatPixel = 150; // 按照这条边的绝对像素高度
    int bannerFixedWidth = 300;  // 横幅的固定宽度
    int bannerContentWidthDelta = 86; // 内容和整个宽度差：左9-头像32-间距6~(实际大小)~-右9-阴影12-列表18
    int bannerContentWidth = bannerFixedWidth - bannerContentWidthDelta; // 内容的宽度
    int bannerContentMaxHeight = 130; // 内容最大像素高度
    int bannerHeaderSize = 32; // 头像大小
    int bannerSpacing = 10; // 横幅之间的间距
    int bannerAnimationDuration = 300; // 横幅的动画时长
    int bannerShowEasingCurve = 6;     // 动画的曲线设定
    int bannerDisplayDuration = 7000;  // 横幅显示的时长
    int bannerTextReadSpeed = 10; // 按字数增加时长，每秒钟看多少字
    int bannerRetentionDuration = 1000; // 失去焦点后滞留时长
    QColor bannerTitleColor = QColor(0, 0, 0, 255); // 默认背景颜色
    QColor bannerContentColor = QColor(32, 32, 32, 255); // 默认背景颜色
    QColor bannerBgColor = QColor(255, 255, 255, 255); // 默认背景颜色
    bool bannerUseHeaderColor = true; // 使用头像颜色填充背景颜色
    bool bannerUseHeaderGradient = false; // 使用头像渐变
    bool bannerColorfulGroupMember = false; // 群组用户昵称使用头像颜色
    bool bannerFrostedGlassBg = false; // 毛玻璃背景
    int bannerFrostedGlassOpacity = 32; // 毛玻璃不透明度
    int bannerBgRadius = 5; // 圆角大小
    int bannerBgShadow = 12; // 阴影大小，右下角会空出这么多的margin
    int bannerScreenIndex = 0; // 使用哪一个屏幕
    int bannerTitleLarger = 3; // 大标题（发送者/群名）变大
    int bannerSubTitleLarger= 2; // 二级标题（群聊的发送者）字体
    int bannerMessageAlpha = 255; // 消息透明度
    int bannerMaxMsgCount = 10; // 同时最多显示10条消息
    int bannerMsgMaxLength = 100; // 消息最大长度
    bool bannerSingleSender = false; // 单个横幅卡片只放同一个人的消息
    int bannerMessageSpacing = 8; // 同一个卡片里面的消息的间距：itemSpace+margin*2
    bool bannerMessageSmoothScroll = true; // 平滑滚动
    int bannerMessageLoadCount = 20; // 一次性加载多少条历史记录
    int bannerReplyIgnoreWithin = 200; // 热键聚焦时，忽视短期内来的卡片
    int bannerAIReply = false; // 触发回复时，进行AI提示
    bool bannerPrivateKeepShowing = false; // 私聊保持显示直到交互
    bool bannerGroupKeepShowing = false; // 群聊保持显示直到交互

    // 回复
    bool bannerShowMySend = false; // 回复后，显示自己发送的消息；可能会与接收到的重复
    bool bannerAutoShowReply = false; // 自动显示回复框
    bool bannerAutoFocusReply = false; // hover的时候自动聚焦回复框
    bool bannerCloseAfterReply = true; // 回复后关闭对话框（Ctrl+Enter切换）

    // 群组同志
    bool enableGroupNotification = true; // 群组通知总开关
    QList<qint64> enabledGroups; // 群组通知白名单

    // 重要性
    QHash<qint64, int> userImportance; // 用户重要性
    QHash<qint64, int> groupImportance; // 群组重要性
    int lowestImportance = NormalImportant; // 当前弹窗的最低重要程度
    int userDefaultImportance = Important; // 未设置用户的默认重要性
    int groupDefaultImportance = NormalImportant; // 未设置群组的默认重要性

    // 文件管理
    bool autoCacheImage = true; // 显示图片，还是只显示[图片]
    bool autoCachePrivateVideo = true; // 自动缓存用户私聊的视频
    bool autoCacheSmallVideo = true; // 自动缓存所有视频
    bool autoCachePrivateFile = true; // 自动缓存用户私聊的视频
    bool autoCachePrivateFileType = true; // 仅自动缓存指定格式的文件
    QStringList autoCachePrivateFileTypes; // 自动缓存用户私聊的文件类型（后缀名）
    int autoCacheFileMaxSize = 10; // 自动缓存的文件上限，单位MB

    // 远程控制
    QStringList remoteControlPrefixs;

    // 离开模式
    bool leaveMode = false; // 离开模式（开启回复）
    bool aiReplyPrivate = false; // 自动回复私聊
    QList<qint64> aiReplyUsers; // AI回复的用户的白名单
    QString aiReplyPrefix = "[AI回复] "; // 自动回复前缀
    QString aiReplySuffix = ""; // 自动回复后缀
    QString aiReplyDefault = "[自动回复] 您好，我现在有事不在，一会儿也不和您联系~"; // 自动回复默认内容（需包含前缀后缀）
    int aiReplyInterval = 3000; // 自动回复的最短间隔

    // 程序
    bool startOnPowerOn = false; // 开机自启

    // 调试
    bool showWidgetBorder = false; // 显示控件边界，调试布局
};

extern USettings* us;

#endif // USERSETTINGS_H
