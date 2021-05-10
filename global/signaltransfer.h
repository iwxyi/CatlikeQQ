#ifndef SIGNALTRANSFER_H
#define SIGNALTRANSFER_H

#include <QObject>

class SignalTransfer : public QObject
{
    Q_OBJECT
signals:
    void hostChanged(QString host);
    void socketStateChanged(bool connected);
    void myAccount(qint64 id, QString nickname);

    void loadGroupMembers(qint64 groupId);
    void groupMembersLoaded(qint64 groupId);

    void setReplyKey(QString key);
};

extern SignalTransfer* sig;

#endif // SIGNALTRANSFER_H
