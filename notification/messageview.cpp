#include <QRegularExpression>
#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QNetworkReply>
#include <QPainter>
#include <QMovie>
#include <QDesktopServices>
#include "fileutil.h"
#include "messageview.h"
#include "global.h"
#include "netimageutil.h"

MessageView::MessageView(QWidget *parent) : QLabel(parent)
{
#ifdef MESSAGE_LABEL
    setWordWrap(true);
    setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
#else
    setReadOnly(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setWordWrapMode(QTextOption::WrapMode::WrapAnywhere);
    setStyleSheet("QTextEdit{ background: transparent; border: none; padding: 0px; margin: 0px; }");
#endif
    setContentsMargins(0, 0, 0, 0);
    connect(this, &QLabel::linkActivated, this, [=](const QString& link) {
        qDebug() << "打开链接：" << link;
        QDesktopServices::openUrl(QUrl(link));
    });
}

/// 设置带有表情、图片等多种类型的Message
/// 设置动图表情的方法：https://blog.csdn.net/qq_46495964/article/details/113795814
void MessageView::setMessage(const MsgBean& msg)
{
    QString text = msg.message;
    QRegularExpression re;
    QRegularExpressionMatch match;

    // #替换CQ
    // 文件
    if (!msg.fileId.isEmpty())
    {
        text = "[文件] " + msg.fileName;
    }

    // 表情
    if (text.indexOf(QRegExp("\\[CQ:face,id=(\\d+)\\]")) > -1)
    {
#ifdef MESSAGE_LABEL
            // 如果是单张图片，支持显示gif
            if (text.indexOf(QRegularExpression("^\\[CQ:face,id=(\\d+)\\]$"), 0, &match) > -1)
            {
                // 不显示文字了，直接用图片！
                int maxWidth = us->bannerContentWidth;
                int maxHeight = us->bannerContentWidth/3;

                // 支持GIF
                QMovie* movie = new QMovie(":/qq/qq-face/" + match.captured(1) + ".gif", "gif", this);
                if (movie->frameCount() > 0) // 有帧，表示是GIF
                {
                    // 调整图片大小
                    movie->jumpToFrame(0);
                    QSize sz = movie->frameRect().size();
                    if (sz.height() && sz.width())
                    {
                        if (sz.width() > maxWidth)
                            sz = QSize(maxWidth, sz.height() * maxWidth / sz.width());
                        if (sz.height() > maxHeight)
                            sz = QSize(sz.width() * maxHeight / sz.height(), maxHeight);
                        movie->setScaledSize(sz);
                    }
                    else
                    {
                        movie->setScaledSize(QSize(maxWidth, maxHeight));
                    }

                    setMaximumSize(maxWidth, maxHeight);
                    setMovie(movie);
                    movie->start();
                    text = "[表情]";
                    return ;
                }
                delete movie;
            }
#endif
        text.replace(QRegExp("\\[CQ:face,id=(\\d+)\\]"), "<img src=\":/qq/qq-face/\\1.png\"/>");
    }

    // 图片
    // 图片格式：[CQ:image,file=e9f40e7fb43071e7471a2add0df33b32.image,url=http://gchat.qpic.cn/gchatpic_new/707049914/3934208404-2722739418-E9F40E7FB43071E7471A2ADD0DF33B32/0?term=3]
    if (text.indexOf(QRegularExpression("\\[CQ:image,file=(.+?).image,.*url=(.+)\\]"), 0, &match) > -1)
    {
        if (!us->bannerShowImages)
        {
            text.replace(QRegExp("\\[CQ:image,[^\\]]+\\]"), "[图片]");
        }
        else
        {
            // 下载原图
            QString id = match.captured(1);
            QString url = match.captured(2);
            QString path = rt->imageCache(id);
            NetImageUtil::saveNetImage(url, path);
#ifdef MESSAGE_LABEL
            // 如果是单张图片，支持显示gif
            if (text.indexOf(QRegularExpression("^\\[CQ:image,file=(.+?).image,.*url=(.+)\\]$")) > -1)
            {
                // 不显示文字了，直接用图片！
                int maxWidth = us->bannerContentWidth;
                int maxHeight = us->bannerContentWidth/3;

                // 支持GIF
                QMovie* movie = new QMovie(path, "gif", this);
                if (movie->frameCount() > 0) // 有帧，表示是GIF
                {
                    // 调整图片大小
                    movie->jumpToFrame(0);
                    QSize sz = movie->frameRect().size();
                    if (sz.height() && sz.width())
                    {
                        if (sz.width() > maxWidth)
                            sz = QSize(maxWidth, sz.height() * maxWidth / sz.width());
                        if (sz.height() > maxHeight)
                            sz = QSize(sz.width() * maxHeight / sz.height(), maxHeight);
                        movie->setScaledSize(sz);
                    }
                    else
                    {
                        movie->setScaledSize(QSize(maxWidth, maxHeight));
                    }

                    setMaximumSize(maxWidth, maxHeight);
                    setMovie(movie);
                    movie->start();
                    text = "[图片]";
                    return ;
                }
                delete movie;
            }
#endif
            // 伸缩、圆角
            QString originPath = path;
            QPixmap pixmap(path, "1");
            if (pixmap.width() > us->bannerContentWidth)
                pixmap = pixmap.scaled(us->bannerContentWidth, us->bannerContentWidth/3, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            pixmap = NetImageUtil::toRoundedPixmap(pixmap, us->bannerBgRadius);
            id = id + "_small";
            path = rt->imageCache(id);
            pixmap.save(path);
            // 替换为图片标签
            text.replace(match.captured(0), "<a href=\"file:///" + originPath + "\"><img src=\"" + path + "\" /></a>");
        }
    }

    // 回复
    text.replace(QRegExp("\\[CQ:reply,id=-?\\d+\\]\\[CQ:at,qq=\\d+\\]"), "[回复]");

    // 艾特
    text.replace(QRegExp("\\[CQ:at,qq=(\\d+)\\]"), "@\\1");

    // 其他格式
    text.replace(QRegExp("\\[CQ:(\\w+),.+\\]"), "[\\1]");

    // 处理HTML
    text.replace("<", "&lt;").replace(">", "&gt;");

    // #处理长度
//    if (text.length() > us->msgMaxLength)
//        text = text.left(us->msgMaxLength) + "...";

    // #设置显示
    setText(text);
}

QSize MessageView::adjustSizeByTextWidth(int w)
{
#ifdef MESSAGE_LABEL
    setFixedWidth(w);
    adjustSize();
    return this->size();
#else
//    setMaximumWidth(w);
//    QTextDocument* doc = this->document();
//    doc->setTextWidth(w);
//    doc->adjustSize();
//    return (doc->size() + QSizeF(4, 0)).toSize(); // 横向肯定要加
#endif
}

void MessageView::setTextColor(QColor c)
{
    QPalette pa(this->palette());
    pa.setColor(QPalette::Foreground, c);
    pa.setColor(QPalette::Text, c);
    setPalette(pa);
}

