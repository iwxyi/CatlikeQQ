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
    endGroup();

    beginGroup("group");
    assign(enableGroupNotification, "enabled");
    endGroup();
}
