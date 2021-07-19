#include "usettings.h"

USettings::USettings(QString filePath, QObject *parent) : MySettings(filePath, QSettings::Format::IniFormat, parent)
{
    restoreSettings();
}

void USettings::restoreSettings()
{
    host = s("net/host");
    accessToken = s("net/accessToken");
    fileHost = s("net/fileHost");

    beginGroup("banner");
    assign(bannerFloatPixel, "floatPixel");
    assign(bannerFixedWidth, "fixedWidth");
    assign(bannerContentMaxHeight, "contentMaxHeight");
    bannerContentWidth = bannerFixedWidth - bannerContentWidthDelta;
    bannerFloatSide = Side(i("side", int(bannerFloatSide)));
    bannerFloatDirection = Direction(i("direction", bannerFloatDirection));
    assign(bannerFloatPixel, "pixel");
    assign(bannerSpacing, "spacing");
    assign(bannerFixedWidth, "width");
    assign(bannerThumbnailProp, "thumbnailProp");
    assign(bannerBgColor, "bgColor");
    assign(bannerTitleColor, "titleColor");
    assign(bannerContentColor, "contentColor");
    assign(bannerUseHeaderColor, "useHeaderColor");
    assign(bannerUseHeaderGradient, "useHeaderGradient");
    assign(bannerColorfulGroupMember, "colorfulGroupMember");
    assign(bannerDisplayDuration, "displayDuration");
    assign(bannerTextReadSpeed, "textReadSpeed");

    assign(bannerShowMySend, "showMySend");
    assign(bannerAutoShowReply, "autoShowReply");
    assign(bannerAutoFocusReply, "autoFocusReply");
    assign(bannerCloseAfterReply, "closeAfterReply");
    assign(bannerFrostedGlassBg, "frostedGlassBg");
    assign(bannerFrostedGlassOpacity, "frostedGlassOpacity");
    assign(bannerAIReply, "AIReply");
    assign(bannerPrivateKeepShowing, "privateKeepShowing");
    assign(bannerGroupKeepShowing, "groupKeepShowing");
    endGroup();

    beginGroup("group");
    assign(enableGroupNotification, "enabled");
    assign(enabledGroups, "enables");
    endGroup();

    beginGroup("importance");
    assign(userImportance, "userImportance");
    assign(groupImportance, "groupImportance");
    assign(lowestImportance, "lowestImportance");
    endGroup();

    beginGroup("autoCache");
    assign(autoCacheImage, "image");
    assign(autoCachePrivateVideo, "privateVideo");
    assign(autoCacheSmallVideo, "smallVideo");
    assign(autoCacheFileMaxSize, "fileMaxSize");
    assign(autoCachePrivateFile, "privateFile");
    assign(autoCachePrivateFileType, "privateFileType");
    autoCachePrivateFileTypes = s("privateFileTypes",
                                  "doc docx ppt pptx xls xlsx pdf")
            .split(" ", QString::SkipEmptyParts);
    endGroup();

    beginGroup("remoteControl");
    assign(remoteControlPrefixs, "prefixs");
    endGroup();

    beginGroup("leave");
    assign(leaveMode, "leaveMode");
    assign(aiReplyPrivate, "aiReplyPrivate");
    assign(aiReplyPrefix, "aiReplyPrefix");
    assign(aiReplySuffix, "aiReplySuffix");
    assign(aiReplyDefault, "aiReplyDefault");
    assign(aiReplyInterval, "aiReplyInterval");
    assign(aiReplyUsers, "aiReplyUsers");
    endGroup();

    beginGroup("app");
    assign(startOnPowerOn, "startOnPowerOn");
    endGroup();

    beginGroup("debug");
    assign(showWidgetBorder, "showWidgetBorder");
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
