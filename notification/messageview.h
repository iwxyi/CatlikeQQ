#ifndef MESSAGEEDIT_H
#define MESSAGEEDIT_H

#include <QTextBrowser>
#include <QLabel>
#include "msgbean.h"

#define MESSAGE_LABEL

class MessageView : public QLabel
{
    Q_OBJECT
public:
    explicit MessageView(QWidget *parent = nullptr);

    void setMessage(const MsgBean &msg);

    QSize adjustSizeByTextWidth(int w);

    void setTextColor(QColor c);

    void setGroupMembers(QHash<qint64, QString> *memberNames);

signals:
    void needMemberNames();

public slots:
    void replaceGroupAt();

private:
    QHash<qint64, QString> *memberNames = nullptr;
};

#endif // MESSAGEEDIT_H
