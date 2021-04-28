#ifndef ACCOUNTINFO_H
#define ACCOUNTINFO_H

#include <QString>
#include <QHash>

class AccountInfo
{
public:
    qint64 myId = 0; // 自己的QQ号
    QString myNickname; // 自己的昵称
    QHash<qint64, QString> friendNames; // 好友列表
    QHash<qint64, QString> groupNames; // 群列表
    QHash<qint64, QHash<qint64, QString>> groupMemberNames; // 群成员名字
};

#endif // ACCOUNTINFO_H
