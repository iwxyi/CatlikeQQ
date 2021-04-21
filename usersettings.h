#ifndef USERSETTINGS_H
#define USERSETTINGS_H

#include <QSettings>

class UserSettings : public QSettings
{
public:
    UserSettings(QObject* parent = nullptr);
};

#endif // USERSETTINGS_H
