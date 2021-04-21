#include "usersettings.h"

UserSettings::UserSettings(QObject *parent) : MySettings("settings.ini", QSettings::Format::IniFormat, parent)
{
    restoreSettings();
}

void UserSettings::restoreSettings()
{
    this->host = s("net/host");
}
