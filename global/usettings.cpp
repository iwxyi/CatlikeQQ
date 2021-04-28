#include "usettings.h"

USettings::USettings(QObject *parent) : MySettings("settings.ini", QSettings::Format::IniFormat, parent)
{
    restoreSettings();
}

void USettings::restoreSettings()
{
    host = s("net/host");

    beginGroup("float");
    floatSide = Side(i("side", int(floatSide)));
    floatDirection = Direction(i("direction", floatDirection));
    assign(floatPixel, "pixel");
    endGroup();

    beginGroup("banner");
    assign(bannerSpacing, "spacing");
    assign(bannerWidth, "width");
    assign(bannerUseHeaderColor, "useHeaderColor");
    assign(bannerDisplayDuration, "displayDuration");
    assign(bannerTextReadSpeed, "textReadSpeed");
    endGroup();

    beginGroup("group");
    assign(enableGroupNotification, "enabled");
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
