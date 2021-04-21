#ifndef USERSETTINGS_H
#define USERSETTINGS_H

#include <mysettings.h>

class UserSettings : public MySettings
{
public:
    UserSettings(QObject* parent = nullptr);

private:
    void restoreSettings();

public:
    QString host;
};

#endif // USERSETTINGS_H
