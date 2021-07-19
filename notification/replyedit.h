#ifndef REPLYEDIT_H
#define REPLYEDIT_H

#include <QLineEdit>
#include <QKeyEvent>
#include <QMimeData>

class ReplyEdit : public QLineEdit
{
    Q_OBJECT
public:
    ReplyEdit(QWidget* parent = nullptr) : QLineEdit(parent)
    {
        setAcceptDrops(true);
    }

signals:
    void signalESC();
    void signalFocusOut();
    void signalUp();
    void signalDown();
    void signalMove(int index);
    void signalCtrlEnter();
    void signalDropFile(QList<QUrl> urls);

protected:
    void keyPressEvent(QKeyEvent *e) override
    {
        auto key = e->key();
        auto modifies = e->modifiers();
        if (key == Qt::Key_Escape)
        {
            emit signalESC();
            e->accept();
            return ;
        }
        else if (key == Qt::Key_Up)
        {
            emit signalUp();
            return e->accept();
        }
        else if (key == Qt::Key_Down)
        {
            emit signalDown();
            return e->accept();
        }

        if (modifies & Qt::ControlModifier)
        {
            if (key >= Qt::Key_0 && key <= Qt::Key_9)
            {
                emit signalMove(key - Qt::Key_0);
                return e->accept();
            }
            else if (key == Qt::Key_Enter || key == Qt::Key_Return)
            {
                emit signalCtrlEnter();
                return e->accept();
            }
        }

        QLineEdit::keyPressEvent(e);
    }

    void focusOutEvent(QFocusEvent *e) override
    {
        QLineEdit::focusOutEvent(e);
        emit signalFocusOut();
    }

    void dragEnterEvent(QDragEnterEvent *e) override
    {
        e->acceptProposedAction();
        QLineEdit::dragEnterEvent(e);
    }

    void dropEvent(QDropEvent *e) override
    {
        auto mime = e->mimeData();
        if (mime->hasUrls())
        {
            auto urls = mime->urls();
            emit signalDropFile(urls);
            return ;
        }
        else if (mime->hasText())
        {

        }
        QLineEdit::dropEvent(e);
    }
};

#endif // REPLYEDIT_H
