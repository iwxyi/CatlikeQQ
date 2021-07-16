#ifndef SIGNALTRANSFER_H
#define SIGNALTRANSFER_H

#include <QObject>

class SignalTransfer : public QObject
{
    Q_OBJECT
signals:
    void hostChanged(QString host, QString token);
    void socketStateChanged(bool connected);

    void myAccount(qint64 id, QString nickname);
    void myHeader(const QPixmap& pixmap);
    void myFriendsLoaded();
    void myGroupsLoaded();

    void loadGroupMembers(qint64 groupId);
    void groupMembersLoaded(qint64 groupId);

    void setReplyKey(QString key);
    void myReplyUser(qint64 userId, QString message);
    void myReplyGroup(qint64 groupId, QString message);

    void openUserCard(qint64 userId, QString username);
    void openGroupCard(qint64 groupId);
};

extern SignalTransfer* sig;

#endif // SIGNALTRANSFER_H
