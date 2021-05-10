#include "usettings.h"

USettings::USettings(QObject *parent) : MySettings("settings.ini", QSettings::Format::IniFormat, parent)
{
    restoreSettings();
}

void USettings::restoreSettings()
{
    host = s("net/host");

    beginGroup("banner");
    assign(bannerFloatPixel, "floatPixel");
    bannerFloatSide = Side(i("side", int(bannerFloatSide)));
    bannerFloatDirection = Direction(i("direction", bannerFloatDirection));
    assign(bannerFloatPixel, "pixel");
    assign(bannerSpacing, "spacing");
    assign(bannerWidth, "width");
    assign(bannerUseHeaderColor, "useHeaderColor");
    assign(bannerUseHeaderGradient, "useHeaderGradient");
    assign(bannerColorfulGroupMember, "colorfulGroupMember");
    assign(bannerDisplayDuration, "displayDuration");
    assign(bannerTextReadSpeed, "textReadSpeed");
    assign(bannerAutoShowReply, "autoShowReply");
    assign(bannerAutoFocusReply, "autoFocusReply");
    assign(bannerCloseAfterReply, "closeAfterReply");
    endGroup();

    beginGroup("group");
    assign(enableGroupNotification, "enabled");
    assign(enabledGroups, "enables");
    endGroup();
}

/// 是否展示该群组的通知
bool USettings::isGroupShow(qint64 groupId)
{
    if (!enableGroupNotification)
        return false;

    // 过滤显示通知的群组
    return enabledGroups.contains(groupId);
}
