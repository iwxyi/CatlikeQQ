#ifndef MESSAGEEDIT_H
#define MESSAGEEDIT_H

#include <QTextBrowser>
#include <QLabel>
#include "msgbean.h"

#define MESSAGE_LABEL

#ifdef MESSAGE_LABEL
class MessageView : public QLabel
#else
class MessageView : public QTextBrowser
#endif
{
    Q_OBJECT
public:
    explicit MessageView(QWidget *parent = nullptr);

    void setMessage(const MsgBean &msg);

    QSize adjustSizeByTextWidth(int w);

    void setTextColor(QColor c);

signals:
    void needMemberNames();

public slots:
    void replaceGroupAt();

    void showMenu();

protected:
    QSize sizeHint() const override;

private:
    MsgBean msg;
    int fixedWidth = 0;

    QString filePath;
    QPixmap filePixmap;
};

#endif // MESSAGEEDIT_H
