#include <QRegularExpression>
#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QNetworkReply>
#include <QPainter>
#include <QMovie>
#include <QDesktopServices>
#include <QBitmap>
#include <QTimer>
#include <QMultimedia>
#include <QHBoxLayout>
#include <QClipboard>
#include <QMimeData>
#include <QProcess>
#include "fileutil.h"
#include "messageview.h"
#include "defines.h"
#include "runtime.h"
#include "usettings.h"
#include "netimageutil.h"
#include "accountinfo.h"
#include "myjson.h"
#include "video/videolabel.h"
#include "video/videowidget.h"
#include "facilemenu.h"
#include "signaltransfer.h"

MessageView::MessageView(QWidget *parent)
#ifdef MESSAGE_LABEL
    : QLabel(parent),
#else
    : QTextBrowser(parent),
#endif
      msg(0, "")
{
#ifdef MESSAGE_LABEL
    setWordWrap(true);
    setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
    setTextFormat(Qt::RichText);

    connect(this, &QLabel::linkActivated, this, [=](const QString& link) {
        qInfo() << "打开链接：" << link;
        QDesktopServices::openUrl(QUrl(link));
    });
#else
    setReadOnly(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setWordWrapMode(QTextOption::WrapMode::WrapAnywhere);
#endif
    setContentsMargins(0, 0, 0, 0);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showMenu()));

    if (us->showWidgetBorder)
        setStyleSheet("QLabel, QTextBrowser { background: transparent; border: 1px solid red; }"); // 测试边框
}

/// 设置带有表情、图片等多种类型的Message
/// 设置动图表情的方法：https://blog.csdn.net/qq_46495964/article/details/113795814
void MessageView::setMessage(const MsgBean& msg)
{
    this->msg = msg;
    QString text = msg.message;
    if (msg.isPrivate() && msg.senderId == ac->myId)
        text.insert(0, "你：");
    QRegularExpression re;
    QRegularExpressionMatch match;

    auto grayText = [=](QString text) {
        return text;
        // return "<font color='gray'>" + text + "</font>";
    };

    auto linkText = [=](QString text, QString link) {
        return "<a href=\"" +link + "\"><span style=\"text-decoration: none; color:#8cc2d4;\">" + text + "</span></a>";
    };

    // #处理HTML
    text.replace("<", "&lt;").replace(">", "&gt;");

    // #替换CQ
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
            text.replace(QRegularExpression("\\[CQ:face,id=(\\d+)\\]"), grayText("[表情]"));
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
            text.replace(QRegularExpression("\\[CQ:face,id=(\\d+)\\]"), "<img src=\"" + path + "\"/>");
        }
    }

    // 闪照
    // [CQ:image,type=flash,file=27194ea0bc4ef666c06ba6fe716e31ad.image]
    text.replace(QRegularExpression("\\[CQ:image,type=flash,.+\\]"), grayText("[闪照]"));

    // 图片
    // 图片格式：[CQ:image,file=e9f40e7fb43071e7471a2add0df33b32.image,url=http://gchat.qpic.cn/gchatpic_new/707049914/3934208404-2722739418-E9F40E7FB43071E7471A2ADD0DF33B32/0?term=3]
    if (text.indexOf(QRegularExpression("\\[CQ:image,file=(.+?).image,.*url=(.+)\\]"), 0, &match) > -1)
    {
        if (!us->autoCacheImage)
        {
            text.replace(QRegularExpression("\\[CQ:image,[^\\]]+\\]"), "[图片]");
        }
        else
        {
            // 下载原图
            QString id = match.captured(1);
            QString url = match.captured(2);
            QString path = rt->imageCache(id);
            if (!isFileExist(path)) // 可能重复发送，也可能从历史消息加载，所以不重复读取
                NetImageUtil::saveNetFile(url, path);
            this->filePath = path;

            // 图片尺寸
            int maxWidth = us->bannerContentWidth;
            int maxHeight = us->bannerContentMaxHeight - us->bannerHeaderSize;
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
                        if (sz.width() < maxWidth / us->bannerThumbnailProp
                                && sz.height() < maxHeight / us->bannerThumbnailProp)
                        {
                            // 图片过小，不压缩了
                        }
                        else if (sz.width() / us->bannerThumbnailProp < maxWidth
                                && sz.height() / us->bannerThumbnailProp < maxHeight)
                        {
                            // 缩放，不足最大尺寸
                            sz /= us->bannerThumbnailProp;
                        }
                        else
                        {
                            // 满max缩放
                            sz.scale(maxWidth, maxHeight, Qt::KeepAspectRatio);
                        }
                        /* if (sz.width() > maxWidth)
                            sz = QSize(maxWidth, sz.height() * maxWidth / sz.width());
                        if (sz.height() > maxHeight)
                            sz = QSize(sz.width() * maxHeight / sz.height(), maxHeight); */
                        movie->setScaledSize(sz);
                    }
                    else
                    {
                        qWarning() << "无法获取到 gif 的大小" << id;
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
            // 静态图片；缩略图的伸缩、圆角
            QString originPath = path;
            QPixmap pixmap(path, "1");
            this->filePixmap = pixmap;
            maxWidth -= us->bannerBgRadius * 2; // 有个莫名的偏差
            if (pixmap.width() < maxWidth / us->bannerThumbnailProp
                    && pixmap.height() < maxHeight / us->bannerThumbnailProp)
            {
                // 图片过小，不压缩
            }
            else if (pixmap.width() / us->bannerThumbnailProp < maxWidth
                    && pixmap.height() / us->bannerThumbnailProp < maxHeight)
            {
                // 按固定比例压缩
                pixmap = pixmap.scaled(pixmap.size() / us->bannerThumbnailProp, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }
            else if (pixmap.width() > maxWidth || pixmap.height() > maxHeight)
            {
                // 这个会缩 得更小
                pixmap = pixmap.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }
            pixmap = NetImageUtil::toRoundedPixmap(pixmap, us->bannerBgRadius);
            path = rt->imageSCache(id);
            pixmap.save(path);
            // 替换为图片标签
            text.replace(match.captured(0), "<a href=\"file:///" + originPath + "\"><img src=\"" + path + "\" /></a>");
            this->setMinimumWidth(pixmap.width());
        }
    }

    // 回复
    text.replace(QRegularExpression("\\[CQ:reply,id=-?\\d+\\](\\[CQ:at,qq=\\d+\\])?"), grayText("[回复]"));

    // 艾特
    re = QRegularExpression("\\[CQ:at,qq=(\\d+)\\]");
    int pos = 0;
    if (msg.isGroup() && text.indexOf(re) > -1)
    {
        const auto members = ac->groupList.value(msg.groupId).members;
        while ((pos = text.indexOf(re, pos, &match)) > -1)
        {
            if (!members.size())
            {
                emit needMemberNames();
                break;
            }

            qint64 userId = match.captured(1).toLongLong();
            if (ac->groupList.value(msg.groupId).members.contains(userId))
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
                    text.replace(match.captured(0), "<font color=" + QVariant(c).toString() + ">@" + members.value(userId).username() + "</font>");
                }
                else
                {
                    text.replace(match.captured(0), "@" + members.value(userId).username());
                }
            }
            else // 群组里没有这个人，刚加入？
            {
                emit needMemberNames();
                pos++;
            }
        }
    }
    text.replace(QRegularExpression("\\[CQ:at,qq=(\\d+)\\]"), "@\\1"); // 万一有没有替换完的呢
    text.replace(QRegularExpression("\\[CQ:at,qq=all\\]"), "@全体成员"); // @全体

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

    // video
    if (text.indexOf(QRegularExpression("\\[CQ:video,file=(.+?)(?:.video)?,url=(.+)\\]"), 0, &match) > -1)
    {
        QString file = match.captured(1); // avsdqwezc.video
        QString url = match.captured(2).replace("&amp;", "&"); // http://xxx.xx?ver=xxx&rkey=xx&filetype=1003&videotype=1&subvideotype=0&term=unknow
        QString path = rt->videoCache(file);

        if (us->autoCacheSmallVideo // 缓存所有视频
                || (msg.isPrivate() && us->autoCachePrivateVideo) ) // 缓存私聊视频
        {
            if (!isFileExist(path)) // 可能重复发送，也可能从历史消息加载，所以不重复读取
                NetImageUtil::saveNetFile(url, path);
            this->filePath = path;

            // 图片尺寸
            int maxWidth = us->bannerContentWidth;
            int maxHeight = us->bannerContentMaxHeight - us->bannerHeaderSize;

            // 控件
            QHBoxLayout* lay = new QHBoxLayout(this);
            VideoLabel* vw = new VideoLabel(this);
//            VideoWidget* vw = new VideoWidget(this); // 透明窗口不能使用 QVideoWidget，很无奈
            lay->addWidget(vw);
            lay->setMargin(0);

            // 播放视频
            vw->player->setMuted(true);
            vw->setRadius(us->bannerBgRadius);
            vw->setMedia(path);
            vw->show();
            this->setFixedHeight(maxHeight);

            vw->setCursor(Qt::PointingHandCursor);
            connect(vw, &ClickLabel::leftClicked, this, [=]{
                QDesktopServices::openUrl(QUrl(path));
            });

            // 设置文字
            text = "";
        }
        else // 不缓存视频
        {
            // text.replace(match.captured(0), "<a href='" + url + "'>[video]</a>"); // 加上超链接
            text = linkText("[video]", match.captured(0));
        }
    }

    // 文件
    if (!msg.fileId.isEmpty())
    {
        QString suffix = "";
        int index = msg.fileName.lastIndexOf(".");
        if (index != -1)
            suffix = msg.fileName.right(msg.fileName.length() - index - 1);
        if (msg.isPrivate()
                && us->autoCachePrivateFile
                && (!us->autoCachePrivateFileType
                    || (!suffix.isEmpty() && us->autoCachePrivateFileTypes.contains(suffix)))
                && (us->autoCacheFileMaxSize == 0 || msg.fileSize <= us->autoCacheFileMaxSize * 1024 * 1024)
                && !msg.fileUrl.isEmpty())
        {
            QString path = rt->fileCache(msg.fileName);
            if (!isFileExist(path))
            {
                qInfo() << "自动下载文件：" << path;
                NetImageUtil::saveNetFile(msg.fileUrl, path);
            }
            this->filePath = path;
            text = linkText("[文件]" + msg.fileName, path);
        }
        else // 不下载文件
        {
            if (msg.fileUrl.isEmpty())
                text = grayText("[文件] " + msg.fileName);
            else
                text = linkText("[文件] " + msg.fileName, msg.fileUrl);
        }
    }

    // 其他格式
    text.replace(QRegularExpression("\\[CQ:(\\w+),.+\\]"), grayText("[\\1]"));

    // 实体
    text.replace("&#91;", "[").replace("&#93;", "]");

    // 超链接
    text.replace(QRegularExpression("(?<!['\"])((http|ftp)s?://[\\w\\.]+\\.\\w{2,5}([\\?\\/][\\S]*)?)"), "<a href=\"\\1\">\\1</a>");
    text.replace(QRegularExpression("([a-zA-Z0-9_-]+@[a-zA-Z0-9_-]+(\\.[a-zA-Z0-9_-]+)+)"), "<a href=\"mailto:\\1\">\\1</a>");

    // #处理长度（注意要忽略各种标签）
//    if (text.length() > us->msgMaxLength)
//        text = text.left(us->msgMaxLength) + "...";

    // #设置显示
    setText(text);
}

/// 这个是最简单的文字替换
/// 不涉及到富文本、文件下载等等
QString MessageView::simpleMessage(const MsgBean &msg)
{
    QString text = msg.message;
    if (msg.senderId == ac->myId)
        text.insert(0, "你:");
    QRegularExpression re;
    QRegularExpressionMatch match;

    // #处理HTML
    text.replace("<", "&lt;").replace(">", "&gt;");

    // #替换CQ
    // 表情
    text.replace(QRegularExpression("\\[CQ:face,id=(\\d+)\\]"), "[表情]");

    // 闪照 [CQ:image,type=flash,file=27194ea0bc4ef666c06ba6fe716e31ad.image]
    text.replace(QRegularExpression("\\[CQ:image,type=flash,.+?\\]"), "[闪照]");

    // 图片 [CQ:image,file=e9f40e7fb43071e7471a2add0df33b32.image,url=http://gchat.qpic.cn/gchatpic_new/707049914/3934208404-2722739418-E9F40E7FB43071E7471A2ADD0DF33B32/0?term=3]
    text.replace(QRegularExpression("\\[CQ:image,.+?\\]"), "[图片]");

    // 回复
    text.replace(QRegularExpression("\\[CQ:reply,id=-?\\d+\\](\\[CQ:at,qq=\\d+\\])?"), "[回复]");

    // 艾特
    re = QRegularExpression("\\[CQ:at,qq=(\\d+)\\]");
    int pos = 0;
    if (msg.groupId && text.indexOf(re) > -1)
    {
        const auto members = ac->groupList.value(msg.groupId).members;
        while ((pos = text.indexOf(re, pos, &match)) > -1)
        {
            if (!members.size())
            {
                break;
            }

            qint64 userId = match.captured(1).toLongLong();
            if (ac->groupList.value(msg.groupId).members.contains(userId))
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
                    text.replace(match.captured(0), "<font color=" + QVariant(c).toString() + ">@" + members.value(userId).username() + "</font>");
                }
                else
                {
                    text.replace(match.captured(0), "@" + members.value(userId).username());
                }
            }
            else // 群组里没有这个人，刚加入？
            {
                pos++;
            }
        }
        text.replace(QRegularExpression("\\[CQ:at,qq=(\\d+)\\]"), "@\\1"); // 万一有没有替换完的呢
    }

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
        else
        {
            text.replace(match.captured(0), "[json]");
        }
    }

    // video
    if (text.indexOf(QRegularExpression("\\[CQ:video,.+\\]")) > -1)
        text = "[视频]";

    // 文件
    if (!msg.fileId.isEmpty())
        text = "[文件]";

    // 其他格式
    text.replace(QRegularExpression("\\[CQ:(\\w+),.+\\]"), "[\\1]");

    // 实体
    text.replace("&#91;", "[").replace("&#93;", "]");

    // 超链接
    text.replace(QRegularExpression("(?<!['\"])((http|ftp)s?://[\\w\\.]+\\.\\w{2,5}([\\?\\/][\\S]*)?)"), "<a href=\"\\1\">\\1</a>");
    text.replace(QRegularExpression("([a-zA-Z0-9_-]+@[a-zA-Z0-9_-]+(\\.[a-zA-Z0-9_-]+)+)"), "<a href=\"mailto:\\1\">\\1</a>");

    // #设置显示
    return text;
}

/// 把形如 @123456 的格式统统替换为 @某某
void MessageView::replaceGroupAt()
{
    const auto members = ac->groupList.value(msg.groupId).members;
    if (!members.size())
        return ;

#ifdef MESSAGE_LABEL
    QString text = this->text();
#else
    QString text = this->toPlainText();
#endif
    if (text.isEmpty())
        return ;

    QRegularExpression re("@(\\d+)");
    QRegularExpressionMatch match;
    int pos = 0;
    bool replaced = false;
    while ((pos = text.indexOf(re, pos, &match)) > -1)
    {
        qint64 userId = match.captured(1).toLongLong();
        if (!members.contains(userId))
        {
            pos++;
            qWarning() << "不存在@的用户：" << userId << match.captured(1) << text;
            continue;
        }

        text.replace(match.captured(0), "@" + members.value(userId).username());
        replaced = true;
    }
    if (replaced) // 没有替换的话，就不重新设置了（怕引起大小变化等等）
        setText(text);
}

void MessageView::showMenu()
{
    emit keepShowing();
    FacileMenu* menu = new FacileMenu(this);

    if (!filePixmap.isNull())
    {
        menu->addAction("复制图片", [=]{
            QApplication::clipboard()->setPixmap(filePixmap);
        });
    }

    if (!filePath.isEmpty())
    {
        QString path = filePath;
        path.replace("/", "\\");
        menu->addAction("打开文件夹", [=]{
            const QString explorer = "explorer";
            QStringList param;
            param << QLatin1String("/select,");
            param << path;
            QProcess::startDetached(explorer, param);
        });

        menu->addAction("复制文件", [=]{
            QMimeData* mime = new QMimeData();
            mime->setText(QFileInfo(path).absoluteFilePath());
            mime->setUrls(QList<QUrl>{QUrl::fromLocalFile(path)});
            mime->setData("x-special/gnome-copied-files", QByteArray("copy\n") + QUrl::fromLocalFile(path).toEncoded());
            QApplication::clipboard()->setMimeData(mime);
        });
    }

    if (!filePath.isNull() || !filePath.isEmpty())
        menu->split();

    menu->addAction("回复", [=]{
        QString text = "[CQ:reply,id=" + snum(msg.messageId) + "][CQ:at,qq=" + snum(msg.senderId) + "] ";
        if (us->replyMessageContainsAt)
            text += "[CQ:at,qq=" + snum(msg.senderId) + "] ";
        emit replyText(text);
    });

    if (msg.isGroup())
    {
        menu->addAction("单独回复", [=]{
            QString text = "[CQ:reply,id=" + snum(msg.messageId) + "]";
            emit sig->openUserCard(msg.senderId, msg.displayNickname(), text);
        });

        menu->addAction("@TA", [=]{
            emit replyText("[CQ:at,qq=" + snum(msg.senderId) + "] ");
        })->text(msg.senderId == ac->myId, "@自己");
    }

    menu->addAction("+1", [=]{
        emit sendText(msg.rawMessage);
    });

    menu->split()->addAction("CQ码", [=]{
        // 必须要有一个拷贝的副本，因为 msg 可能因为通知超时隐藏而删除
        QString cqCode = msg.rawMessage;
        auto btn = QMessageBox::information(this->parentWidget(), "CQ码", cqCode, "取消", "复制", nullptr, 0);
        if (btn == 1)
        {
            QApplication::clipboard()->setText(cqCode);
        }
    });


#ifdef MESSAGE_LABEL
    menu->addAction("复制", [=]{
        if (this->hasSelectedText())
        {
            QApplication::clipboard()->setText(this->selectedText());
        }
        else
        {
            QApplication::clipboard()->setText(this->text());
        }
    })->text(!this->hasSelectedText(), "复制全部");

    menu->addAction("全选", [=]{
        this->setSelection(0, this->text().length());
    });
#else
#endif

    menu->exec();

    menu->finished([=]{
        emit restoreTimerIfNecessary();
    });
}

QSize MessageView::adjustSizeByTextWidth(int w)
{
#ifdef MESSAGE_LABEL
    this->fixedWidth = w;
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

QSize MessageView::sizeHint() const
{
    if (fixedWidth)
#ifdef MESSAGE_LABEL
        return QSize(fixedWidth, QLabel::sizeHint().height());
    return QLabel::sizeHint();
#else
        return QSize(fixedWidth, QTextBrowser::sizeHint().height());
    return QTextBrowser::sizeHint();
#endif
}

void MessageView::setTextColor(QColor c)
{
    QPalette pa(this->palette());
    pa.setColor(QPalette::Foreground, c);
    pa.setColor(QPalette::Text, c);
    setPalette(pa);
    setStyleSheet(this->styleSheet() + "QLabel { color: " + QVariant(c).toString() + "; }");
}

