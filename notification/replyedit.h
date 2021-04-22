#ifndef REPLYEDIT_H
#define REPLYEDIT_H

#include <QLineEdit>
#include <QKeyEvent>

class ReplyEdit : public QLineEdit
{
    Q_OBJECT
public:
    ReplyEdit(QWidget* parent = nullptr) : QLineEdit(parent)
    {
    }

signals:
    void signalESC();
    void signalFocusOut();

protected:
    void keyPressEvent(QKeyEvent *e) override
    {
        auto key = e->key();
        if (key == Qt::Key_Escape)
        {
            emit signalESC();
            return ;
        }

        QLineEdit::keyPressEvent(e);
    }

    void focusOutEvent(QFocusEvent *e) override
    {
        QLineEdit::focusOutEvent(e);
        emit signalFocusOut();
    }
};

#endif // REPLYEDIT_H
