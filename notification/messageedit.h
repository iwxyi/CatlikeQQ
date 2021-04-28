#ifndef MESSAGEEDIT_H
#define MESSAGEEDIT_H

#include <QTextEdit>

class MessageEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit MessageEdit(QWidget *parent = nullptr);

    void setMessage(QString text);

signals:

public slots:
};

#endif // MESSAGEEDIT_H
