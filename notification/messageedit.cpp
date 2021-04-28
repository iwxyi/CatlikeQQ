#include <QRegularExpression>
#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QNetworkReply>
#include <QPainter>
#include "fileutil.h"
#include "messageedit.h"
#include "global.h"

MessageEdit::MessageEdit(QWidget *parent) : QTextEdit(parent)
{
    setReadOnly(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setStyleSheet("QTextEdit{ background: transparent; border: none; padding: 0px; margin: 0px; }");
    setWordWrapMode(QTextOption::WrapMode::WordWrap);
}

/// 设置带有表情、图片等多种类型的Message
/// 设置动图表情的方法：https://blog.csdn.net/qq_46495964/article/details/113795814
void MessageEdit::setMessage(const MsgBean& msg)
{
    QString text = msg.message;
    QRegularExpression re;
    QRegularExpressionMatch match;

//    // #替换CQ
//    // 文件
//    if (!msg.fileId.isEmpty())
//    {
//        text = "[文件] " + msg.fileName;
//    }

//    // 表情
//    text.replace(QRegExp("\\[CQ:face,id=\\d+\\]"), "[表情]");

//    // 图片
//    // 图片格式：[CQ:image,file=e9f40e7fb43071e7471a2add0df33b32.image,url=http://gchat.qpic.cn/gchatpic_new/707049914/3934208404-2722739418-E9F40E7FB43071E7471A2ADD0DF33B32/0?term=3]
//    if (text.indexOf(QRegularExpression("\\[CQ:image,file=(.+?).image,.*url=(.+)\\]"), 0, &match) > -1)
//    {
//        if (us->bannerShowImages && text.indexOf(QRegularExpression("^\\[CQ:image,file=(.+?).image,.*url=(.+)\\]$"), 0, &match) > -1)
//        {
//            QString id = match.captured(1);
//            QString url = match.captured(2);
//            QString path = rt->imageCache(id);
//            saveNetImage(url, path);
//            text = "[图片]";
//        }
//        else
//        {
//            text.replace(QRegExp("\\[CQ:image,[^\\]]+\\]"), "[图片]");
//        }
//    }

//    // 回复
//    text.replace(QRegExp("\\[CQ:reply,id=-?\\d+\\]\\[CQ:at,qq=\\d+\\]"), "[回复]");

//    // 艾特
//    text.replace(QRegExp("\\[CQ:at,qq=(\\d+)\\]"), "@\\1");

//    // JSON格式
//    text.replace(QRegExp("\\[CQ:json,data=.+\\]"), "[json]");

//    // #处理长度
//    if (text.length() > us->msgMaxLength)
//        text = text.left(us->msgMaxLength) + "...";

    // #设置显示
    setText(text);
}

QSize MessageEdit::adjustSizeByTextWidth(int w)
{
    QTextDocument* doc = this->document();
    doc->setTextWidth(w);
    doc->adjustSize();
    return (doc->size() + QSizeF(4, 4)).toSize();
}

void MessageEdit::setTextColor(QColor c)
{
    QPalette pa(this->palette());
    pa.setColor(QPalette::Foreground, c);
    pa.setColor(QPalette::Text, c);
    setPalette(pa);
}

