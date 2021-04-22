#include "usersettings.h"

UserSettings::UserSettings(QObject *parent) : MySettings("settings.ini", QSettings::Format::IniFormat, parent)
{
    restoreSettings();
}

void UserSettings::restoreSettings()
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
}
