#include "messageedit.h"

MessageEdit::MessageEdit(QWidget *parent) : QTextEdit(parent)
{

}

/// 设置带有表情、图片等多种类型的Message
void MessageEdit::setMessage(QString text)
{
    setText(text);

    // 替换表情


    // 替换图片

}
