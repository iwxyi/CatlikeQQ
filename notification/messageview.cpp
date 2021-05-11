#include <QRegularExpression>
#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QNetworkReply>
#include <QPainter>
#include <QMovie>
#include <QDesktopServices>
#include <QBitmap>
#include <QTimer>
#include "fileutil.h"
#include "messageview.h"
#include "defines.h"
#include "runtime.h"
#include "usettings.h"
#include "netimageutil.h"
#include "accountinfo.h"
#include "myjson.h"

MessageView::MessageView(QWidget *parent) : QLabel(parent)
{
#ifdef MESSAGE_LABEL
    setWordWrap(true);
    setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
    setTextFormat(Qt::RichText);
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

    // #处理HTML
    text.replace("<", "&lt;").replace(">", "&gt;");

    // #替换CQ
    // 文件
    if (!msg.fileId.isEmpty())
    {
        text = "[文件] " + msg.fileName;
    }

    // 表情
    if (text.indexOf(QRegularExpression("\\[CQ:face,id=(\\d+)\\]"), 0, &match) > -1)
    {
#ifdef MESSAGE_LABEL
        // 如果是单张图片，支持显示gif
        if (text.indexOf(QRegularExpression("^\\[CQ:face,id=(\\d+)\\]$"), 0, &match) > -1)
        {
            // 支持GIF
            QMovie* movie = new QMovie(":/qq/qq-face/" + match.captured(1) + ".gif", "gif", this);
            if (movie->frameCount() > 0) // 有帧，表示是GIF
            {
                // 调整图片大小
                movie->jumpToFrame(0);
                int sz = this->fontMetrics().height();
                movie->setScaledSize(QSize(sz, sz));
                setMaximumSize(sz, sz);
                setMovie(movie);
                movie->start();
                text = "[表情]";

                // 设置圆角
                QPixmap pixmap(movie->currentPixmap().size());
                pixmap.fill(Qt::transparent);
                QPainter painter(&pixmap);
                painter.setRenderHint(QPainter::Antialiasing);
                QPainterPath path;
                path.addRoundedRect(pixmap.rect(), us->bannerBgRadius, us->bannerBgRadius);
                painter.fillPath(path, Qt::white);
                this->setMask(pixmap.mask());
                return ;
            }
            delete movie;
        }
#endif
        if (!isFileExist(":/qq/qq-face/" + match.captured(1) + ".png")) // 不在表情库中的表情
        {
            text.replace(QRegExp("\\[CQ:face,id=(\\d+)\\]"), "[表情]");
        }
        else
        {
            // 压缩表情图片至字体等大小
            QString id = match.captured(1);
            QString path = rt->faceCache(id);
            if (!isFileExist(path))
            {
                int sz = this->fontMetrics().height();
                QPixmap pixmap(":/qq/qq-face/" + id + ".png");
                pixmap = pixmap.scaled(sz, sz, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                pixmap.save(path);
            }
            text.replace(QRegExp("\\[CQ:face,id=(\\d+)\\]"), "<img src=\"" + path + "\"/>");
        }
    }

    // 闪照
    // [CQ:image,type=flash,file=27194ea0bc4ef666c06ba6fe716e31ad.image]
    text.replace(QRegExp("\\[CQ:image,type=flash,.+\\]"), "[闪照]");

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
            if (!isFileExist(path)) // 可能重复发送，也可能从历史消息加载，所以不重复读取
                NetImageUtil::saveNetFile(url, path);

            // 图片尺寸
            int maxWidth = us->bannerContentWidth;
            int maxHeight = us->bannerContentHeight - us->bannerHeaderSize;
#ifdef MESSAGE_LABEL
            // 如果是单张图片，支持显示gif
            if (text.indexOf(QRegularExpression("^\\[CQ:image,file=(.+?).image,.*url=(.+)\\]$")) > -1)
            {
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

                    // 设置圆角
                    QPixmap pixmap(movie->currentPixmap().size());
                    pixmap.fill(Qt::transparent);
                    QPainter painter(&pixmap);
                    painter.setRenderHint(QPainter::Antialiasing);
                    QPainterPath path;
                    path.addRoundedRect(pixmap.rect(), us->bannerBgRadius, us->bannerBgRadius);
                    painter.fillPath(path, Qt::white);
                    this->setMask(pixmap.mask());
                    return ;
                }
                delete movie;
            }
#endif
            // 伸缩、圆角
            QString originPath = path;
            QPixmap pixmap(path, "1");
            if (pixmap.width() > maxWidth || pixmap.height() > maxHeight)
                pixmap = pixmap.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            pixmap = NetImageUtil::toRoundedPixmap(pixmap, us->bannerBgRadius);
            path = rt->imageSCache(id);
            pixmap.save(path);
            // 替换为图片标签
            text.replace(match.captured(0), "<a href=\"file:///" + originPath + "\"><img src=\"" + path + "\" /></a>");
            this->setMinimumWidth(pixmap.width());
        }
    }

    // 回复
    text.replace(QRegExp("\\[CQ:reply,id=-?\\d+\\]\\[CQ:at,qq=\\d+\\]"), "[回复]");

    // 艾特
    re = QRegularExpression("\\[CQ:at,qq=(\\d+)\\]");
    int pos = 0;
    while ((pos = text.indexOf(re, pos, &match)) > -1)
    {
        if (!memberNames)
        {
            emit needMemberNames();
            break;
        }

        qint64 userId = match.captured(1).toLongLong();
        if (memberNames->contains(userId))
        {
            QColor c = QColor::Invalid;
            if (ac->groupMemberColor.contains(msg.groupId))
            {
                auto memberColor = ac->groupMemberColor.value(msg.groupId);
                if (memberColor.contains(userId))
                    c = memberColor.value(userId);
            }
            if (c.isValid())
            {
                text.replace(match.captured(0), "<font color=" + QVariant(c).toString() + ">@" + memberNames->value(userId) + "</font>");
            }
            else
            {
                text.replace(match.captured(0), "@" + memberNames->value(userId));
            }
        }
        else // 群组里没有这个人，刚加入？
        {
            emit needMemberNames();
            pos++;
        }
    }
    text.replace(QRegExp("\\[CQ:at,qq=(\\d+)\\]"), "@\\1"); // 万一有没有替换完的呢

    // json
    if (text.indexOf(QRegularExpression("\\[CQ:json,data=(.+?)\\]"), 0, &match) > -1)
    {
        QString s = match.captured(1);
        s.replace("\\\"", "\"").replace("&#44;", ",");
        MyJson json(s.toUtf8());
        if (json.contains("prompt"))
        {
            JS(json, prompt);
            text.replace(match.captured(0), prompt);
        }
    }

    // 其他格式
    text.replace(QRegExp("\\[CQ:(\\w+),.+\\]"), "[\\1]");

    // 实体
    text.replace("&#91;", "[").replace("&#93;", "]");

    // 超链接
    text.replace(QRegExp("((http|ftp)s?://[\\w\\.]+\\.\\w{2,5}([\\?\\/][\\S]*)?)"), "<a href=\"\\1\">\\1</a>");
    text.replace(QRegExp("([a-zA-Z0-9_-]+@[a-zA-Z0-9_-]+(\\.[a-zA-Z0-9_-]+)+)"), "<a href=\"mailto:\\1\">\\1</a>");

    // #处理长度（注意要忽略各种标签）
//    if (text.length() > us->msgMaxLength)
//        text = text.left(us->msgMaxLength) + "...";

    // #设置显示
    setText(text);
}

/// 把形如 @123456 的格式统统替换为 @某某
void MessageView::replaceGroupAt()
{
    if (!memberNames)
        return ;
    QString text = this->text();
    if (text.isEmpty())
        return ;

    QRegularExpression re("@(\\d+)");
    QRegularExpressionMatch match;
    int pos = 0;
    bool replaced = false;
    while ((pos = text.indexOf(re, pos, &match)) > -1)
    {
        qint64 userId = match.captured(1).toInt();
        if (!memberNames->contains(userId))
        {
            pos++;
            continue;
        }

        text.replace(match.captured(0), "@" + memberNames->value(userId));
        replaced = true;
    }
    if (replaced) // 没有替换的话，就不重新设置了（怕引起大小变化等等）
        setText(text);
}

QSize MessageView::adjustSizeByTextWidth(int w)
{
#ifdef MESSAGE_LABEL
    setFixedWidth(w);
    adjustSize();
    setFixedHeight(this->height());
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

void MessageView::setGroupMembers(QHash<qint64, QString> *memberNames)
{
    this->memberNames = memberNames;
}

