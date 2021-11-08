#include <QKeyEvent>
#include "searchedit.h"

SearchEdit::SearchEdit(QWidget *parent) : QLineEdit(parent)
{

}

void SearchEdit::keyPressEvent(QKeyEvent *e)
{
    auto key = e->key();
    auto modifiers = e->modifiers();

    if (modifiers == Qt::NoModifier)
    {
        if (key == Qt::Key_Up)
        {
            emit signalKeyUp();
        }
        else if (key == Qt::Key_Down)
        {
            emit signalKeyDown();
        }
        else if (key == Qt::Key_Escape)
        {
            emit signalKeyESC();
        }
    }

    QLineEdit::keyPressEvent(e);
}
