#ifndef REPLYEDIT_H
#define REPLYEDIT_H

#include <QLineEdit>
#include <QKeyEvent>
#include <QMimeData>
#include <QMimeData>
#include <QApplication>
#include <QClipboard>

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
    void signalPasteFile(QString* path);

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
            if (key == Qt::Key_V)
            {
                auto clipboard = QApplication::clipboard();
                if (clipboard->mimeData()->hasImage() && !clipboard->mimeData()->hasText())
                {
                    // 粘贴图片
                    QString path = "";
                    emit signalPasteFile(&path);
                    if (!path.isEmpty())
                    {
                        // 发送图片
                        emit signalDropFile(QList<QUrl>{QUrl::fromLocalFile(path)});
                        return ;
                    }
                }
                else if (clipboard->mimeData()->hasUrls())
                {
                    if (insertImageUrls(clipboard->mimeData()->urls()))
                        return ;
                }
            }
            else if (key >= Qt::Key_0 && key <= Qt::Key_9)
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
        else if (modifies & Qt::AltModifier)
        {
            if (key == Qt::Key_S)
            {
                emit returnPressed();
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
        if (mime->hasImage())
        {
            e->acceptProposedAction();
        }
        else if (mime->hasUrls())
        {
            insertImageUrls(mime->urls());
            e->acceptProposedAction();
        }
        else
            return ;
        QLineEdit::dropEvent(e);
    }

    bool insertImageUrls(QList<QUrl> urls)
    {
        foreach (auto url, urls)
        {
            if (!url.isLocalFile())
                return false;
            auto path = url.toLocalFile();
            QPixmap pixmap;
            if (!pixmap.load(path))
                return false;
            if (pixmap.isNull())
                return false;
        }
        return true;
    }
};

#endif // REPLYEDIT_H
