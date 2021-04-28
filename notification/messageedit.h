#ifndef MESSAGEEDIT_H
#define MESSAGEEDIT_H

#include <QTextBrowser>
#include <QLabel>
#include "msgbean.h"

#define MESSAGE_LABEL

class MessageEdit : public QLabel
{
    Q_OBJECT
public:
    explicit MessageEdit(QWidget *parent = nullptr);

    void setMessage(const MsgBean &msg);

    QSize adjustSizeByTextWidth(int w);

    void setTextColor(QColor c);

signals:

public slots:
};

#endif // MESSAGEEDIT_H
