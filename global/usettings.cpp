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
    bannerFloatSide = Side(i("floatSide", bannerFloatSide));
    bannerFloatDirection = Direction(i("floatDirection", bannerFloatDirection));
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
    assign(bannerLinkColor, "linkColor");
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

    assign(autoPlaySpeech, "autoPlaySpeech");
    assign(autoTransSpeech, "autoTransSpeech");

    assign(bannerShowBubble, "showBubble");
    assign(bannerReplyRecursion, "replyRecursion");
    assign(bannerBubbleMime, "bubbleMime");
    assign(bannerBubbleOppo, "bubbleOppo");
    assign(bannerBubbleReply, "bubbleReply");
    endGroup();

    beginGroup("group");
    assign(enableGroupNotification, "enabled");
    assign(enabledGroups, "enables");
    endGroup();

    beginGroup("importance");
    assign(userImportance, "userImportance");
    assign(groupImportance, "groupImportance");
    assign(lowestImportance, "lowestImportance");
    assign(groupUseFriendImportance, "groupUseFriendImportance");
    assign(improveAtMeImportance, "improveAtMeImportance");
    assign(improveAtAllImportance, "improveAtAllImportance");
    assign(keepImportantMessage, "keepImportantMessage");
    endGroup();

    beginGroup("special");
    assign(userSpecial, "user");
    assign(groupMemberSpecial, "groupMember");
    assign(specialKeep, "keep");
    assign(remindOverlay, "remindOverlay");
    assign(globalRemindWords, "globalRemindWords");
    assign(groupRemindWords, " ", "groupRemindWords");
    assign(smartFocus, "smartFocus");
    assign(dynamicImportance, "dynamicImportance");
    endGroup();

    beginGroup("local");
    assign(userLocalNames, "userLocalName");
    assign(groupLocalNames, "groupLocalName");
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

    beginGroup("count");
    assign(countReceiveAll, "receiveAll");
    assign(countReceivePrivate, "receivePrivate");
    assign(countReceiveGroup, "receiveGroup");
    assign(countShowBanner, "showBanner");
    assign(countSendAll, "sendAll");
    assign(countSendPrivate, "sendPrivate");
    assign(countSendGroup, "sendGroup");
    endGroup();

    beginGroup("app");
    assign(startOnPowerOn, "startOnPowerOn");
    endGroup();

    beginGroup("debug");
    assign(showWidgetBorder, "showWidgetBorder");
    endGroup();

    beginGroup("baiduSpeech");
    assign(baiduSpeechApiKey, "apiKey");
    assign(baiduSpeechSecretKey, "secretKey");
    assign(baiduSpeechAccessToken, "accessToken");
    endGroup();

    beginGroup("tray");
    assign(trayShowAllMessageIcon, "allMessage");
    assign(trayShowAllSlientMessageIcon, "allSlientMessage");
    assign(trayShowSlientPrivateMessageIcon, "slientPrivate");
    assign(trayShowSlientSpecialMessageIcon, "slientSpecial");
    assign(trayShowSlientSpecialMessageIcon, "lowImportance");
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

void USettings::addCount(int &val, QString key)
{
    set("count/" + key, ++val);
}

int USettings::bubblePadding() const
{
    return showWidgetBorder ? bannerBubblePadding : 0;
}
