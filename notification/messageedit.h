#ifndef MESSAGEEDIT_H
#define MESSAGEEDIT_H

#include <QTextEdit>
#include "msgbean.h"

class MessageEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit MessageEdit(QWidget *parent = nullptr);

    void setMessage(const MsgBean &msg);

    QSize adjustSizeByTextWidth(int w);

signals:

public slots:
};

#endif // MESSAGEEDIT_H
