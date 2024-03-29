#ifndef SIGNALTRANSFER_H
#define SIGNALTRANSFER_H

#include <QObject>

class MsgBean;
class MyJson;

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
    void userLocalNameChanged(qint64 id, QString newName);
    void groupLocalNameChanged(qint64 id, QString newName);

    void sendSocketText(const QString& text);
    void sendSocketJson(const MyJson& json);

    void loadGroupMembers(qint64 groupId);
    void groupMembersLoaded(qint64 groupId);

    void setReplyKey(QString key);
    void myReplyUser(qint64 userId, const QString& message, qint64 fromGroupId);
    void myReplyGroup(qint64 groupId, const QString& message);
    void recallMessage(qint64 friendId, qint64 groupId, qint64 messageId);
    void setGroupBan(qint64 groupId, qint64 userId, qint64 duration);

    void openUserCard(qint64 userId, const QString& username, const QString& text);
    void openGroupCard(qint64 groupId, const QString& text);
    void getGroupMsgHistory(qint64 groupId, qint64 messageId);
    void groupMsgHistoryLoaded(qint64 groupId, qint64 messageId, int count);

    void showTrayIcon(const MsgBean& msg);

    void runCode(const QString& code, const MsgBean& msg);
};

extern SignalTransfer* sig;

#endif // SIGNALTRANSFER_H
