#include <QRegularExpression>
#include <QMediaPlaylist>
#include <QMovie>
#include <QPainter>
#include <QBitmap>
#include <QMultimedia>
#include <QMediaPlayer>
#include <QDesktopServices>
#include "video/videolabel.h"
#include "video/videowidget.h"
#include "messageview.h"
#include "accountinfo.h"
#include "runtime.h"
#include "defines.h"
#include "myjson.h"
#include "netutil.h"
#include "fileutil.h"
#include "usettings.h"
#include "netimageutil.h"


/// 设置带有表情、图片等多种类型的Message
/// 设置动图表情的方法：https://blog.csdn.net/qq_46495964/article/details/113795814
void MessageView::setMessage(const MsgBean& msg, int recursion)
{
    this->msg = msg;
    this->replyRecursion = recursion;

    if (!msg.isMsg())
    {
        if (msg.is(ActionRecall))
        {
            // 这个不在这里设置，不用管
        }
        else if (msg.is(ActionJoin))
        {

        }
        else if (msg.is(ActionExit))
        {

        }

        return ;
    }

    // 是消息了
    QString text = msg.message;
    // 处理HTML
    text.replace("<", "&lt;").replace(">", "&gt;");
    text = text.replace("\r\n", "<br/>");
    // 设置昵称前缀
    if ((msg.isPrivate() || recursion) && msg.senderId == ac->myId)
        text.insert(0, "你：");
    else if (recursion)
    {
        QString newText = "<a href=\"at://" + snum(msg.senderId) + "\"><span style=\"text-decoration: none; color:" + QVariant(us->bannerLinkColor).toString() + ";\">" + msg.username() + "</span></a>";
        text.insert(0, newText + "：");
    }
    QRegularExpression re;
    QRegularExpressionMatch match;

    auto grayText = [=](QString text) {
        return text;
        // return "<font color='gray'>" + text + "</font>";
    };

    auto linkText = [=](QString text, QString link) {
        return "<a href=\"" +link + "\"><span style=\"text-decoration: none; color:" + QVariant(us->bannerLinkColor).toString() + ";\">" + text + "</span></a>";
    };

    // #先判断回复
    if (text.indexOf(QRegularExpression("\\[CQ:reply,id=(-?\\d+)\\](\\[CQ:at,qq=\\d+\\])?"), 0, &match) > -1)
    {
        qint64 messageId = match.captured(1).toLongLong();
        text.replace(match.captured(0), "");
        MsgBean replyMsg;

        QList<MsgBean>* list = nullptr;
        if (msg.isPrivate())
        {
            list = &ac->userMsgHistory[msg.friendId];
        }
        else if (msg.isGroup())
        {
            list = &ac->groupMsgHistory[msg.groupId];
        }

        if (list)
        {
            for (int i = list->size() - 1; i >= 0; --i)
            {
                if (list->at(i).messageId == messageId)
                {
                    replyMsg = list->at(i);
                    break;
                }
            }
        }

        if (replyMsg.isValid()) // 找到回复
        {
            replyWidget = setRepyMessage(replyMsg, recursion + 1);
            vlayout->insertWidget(0, replyWidget);
        }
        else // 没找到回复
        {
            text.insert(0, "<a href=\"msg://" + snum(messageId) + "\"><span style=\"text-decoration: none; color:" + QVariant(us->bannerLinkColor).toString() + ";\">[回复]</span></a>");
        }
    }

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
                int sz = contentWidget->fontMetrics().height();
                sz = qMin(sz * 2, 56); // 直接显示大表情
                movie->setScaledSize(QSize(sz, sz));
                contentWidget->setFixedSize(sz, sz);
                contentWidget->setMovie(movie);
                movie->start();
                text = "[表情]";
                singleImage = true;

                // 设置圆角
                /* QPixmap pixmap(movie->currentPixmap().size());
                pixmap.fill(Qt::transparent);
                QPainter painter(&pixmap);
                painter.setRenderHint(QPainter::Antialiasing);
                QPainterPath path;
                path.addRoundedRect(pixmap.rect(), us->bannerBgRadius, us->bannerBgRadius);
                painter.fillPath(path, Qt::white);
                contentWidget->setMask(pixmap.mask()); */
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
                int sz = contentWidget->fontMetrics().height();
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
    if (text.indexOf(QRegularExpression("\\[CQ:image,file=([^\\]]+?).image,.*?url=([^\\]]+?)\\]"), 0, &match) > -1)
    {
        if (!us->autoCacheImage)
        {
            text.replace(QRegularExpression("\\[CQ:image,[^\\]]+?\\]"), "[图片]");
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
            int maxWidth = us->bannerContentWidth - us->bannerBubblePadding * 2;
            int maxHeight = (us->bannerContentMaxHeight - us->bannerHeaderSize - us->bannerBubblePadding * 2) * 2 / 3;
            int lineHeight = QFontMetrics(this->font()).lineSpacing() * 2;
            // int scale1Threshold = 64; // 不压缩
            // int scale1_5Threshold = 128; // 压缩1.5
            // int scale2Threshold = 256; // 压缩2
            singleImage = text.indexOf(QRegularExpression("^\\[CQ:image,file=([^\\]]+?).image,url=([^\\]]+?)\\]$")) > -1;

#ifdef MESSAGE_LABEL
            // 如果是单张图片，支持显示gif
            if (singleImage)
            {
                // 支持GIF
                QMovie* movie = new QMovie(path, "gif", this);
                if (movie->frameCount() > 1) // 有帧，表示是GIF
                {
                    // 调整图片大小
                    movie->jumpToFrame(0);
                    QSize sz = movie->frameRect().size();
                    if (sz.height() > 0 && sz.width() > 0)
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
                            // sz /= us->bannerThumbnailProp;
                            sz.scale(maxWidth / us->bannerThumbnailProp, maxHeight / us->bannerThumbnailProp, Qt::KeepAspectRatio);
                        }
                        else
                        {
                            // 满max缩放
                            sz.scale(maxWidth, maxHeight, Qt::KeepAspectRatio);
                        }

                        // 有些gif不知道为啥没用
                        movie->setScaledSize(sz);
                    }
                    else // 空大小，也不知道多大
                    {
                        qWarning() << "无法获取到 gif 的大小" << id;
                        movie->setScaledSize(sz = QSize(maxWidth, maxHeight));
                    }

                    contentWidget->setFixedSize(sz);
                    // contentWidget->setMaximumSize(maxWidth, maxHeight);
                    contentWidget->setMovie(movie);
                    movie->start();
                    useFixedSize = true;

                    // 设置圆角
                    QPixmap pixmap(sz);
                    pixmap.fill(Qt::transparent);
                    QPainter painter(&pixmap);
                    painter.setRenderHint(QPainter::Antialiasing);
                    QPainterPath path;
                    path.addRoundedRect(pixmap.rect(), us->bannerBgRadius, us->bannerBgRadius);
                    painter.fillPath(path, Qt::white);
                    contentWidget->setMask(pixmap.mask());
                    return ;
                }
                delete movie;
            }
#endif
            // 多张图片、静态图片；缩略图的伸缩、圆角
            int pos = 0;
            maxWidth -= us->bannerBgRadius * 2; // 有个莫名的偏差
            int imageCount = 0;
            QSize lastPixmapSize;
            while (true)
            {
                if (text.indexOf(QRegularExpression("\\[CQ:image,file=(.+?).image,.*?url=(.+?)\\]"), pos, &match) == -1)
                    break;
                id = match.captured(1);
                url = match.captured(2);
                path = rt->imageCache(id);
                if (!isFileExist(path)) // 可能重复发送，也可能从历史消息加载，所以不重复读取
                    NetImageUtil::saveNetFile(url, path);

                QString originPath = path;
                QPixmap pixmap(path, "1");
                this->filePixmap = pixmap;
                if (pixmap.width() < maxWidth / us->bannerThumbnailProp
                        && pixmap.height() < maxHeight / us->bannerThumbnailProp)
                {
                    // 图片过小，不压缩
                }
                else if (pixmap.width() / us->bannerThumbnailProp < maxWidth
                        && pixmap.height() / us->bannerThumbnailProp < maxHeight)
                {
                    // 按固定比例压缩
                    pixmap = pixmap.scaled(QSize(maxWidth, maxHeight) / us->bannerThumbnailProp, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                }
                else if (pixmap.width() > maxWidth || pixmap.height() > maxHeight)
                {
                    // 这个会缩得更小
                    pixmap = pixmap.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                }
                lastPixmapSize = pixmap.size();
                pixmap = NetImageUtil::toRoundedPixmap(pixmap, us->bannerBgRadius);
                path = rt->imageSCache(id);
                pixmap.save(path);
                imageCount++;

                // 判断是否需要换行显示图片
                bool breakLeft = false, breakRight = false;
                if (pixmap.height() > lineHeight)
                {
                    // 判断前面的内容，是否需要换行
                    int pos = match.capturedStart();
                    while (--pos > 0)
                    {
                        QString c = text.at(pos);
                        if (c == "\n" || c == ">") // 自动换行，或者标签不换行
                            break;
                        else if (c == " ")
                            continue;
                        else
                        {
                            // 需要换行
                            breakLeft = true;
                            break;
                        }
                    }

                    // 判断后面的内容，是否需要换行
                    pos = match.capturedEnd() - 1;
                    while (++pos < text.length())
                    {
                        QString c = text.at(pos);
                        if (c == "\n" || c == "<" || c == "[")
                            break;
                        else if (c == " ")
                            continue;
                        else
                        {
                            breakRight = true;
                            break;
                        }
                    }
                }

                // 替换为图片标签
                QString rep = "<a href=\"file:///" + originPath + "\"><img src=\"" + path + "\" /></a>";
                if (breakLeft)
                    rep = "<br/>" + rep;
                if (breakRight)
                    rep = rep + "<br/>";
                text.replace(match.captured(0), rep);
                pos = match.capturedStart() + rep.length();
                if (singleImage)
                {
                    int minimum = qMax(this->minimumWidth(),  pixmap.width());
                    contentWidget->setMinimumWidth(minimum);
                }
            }

            // 只有一张图片
            if (singleImage)
            {
                contentWidget->adjustSize();
                contentWidget->setFixedSize(lastPixmapSize);
            }
        }
    }

    // 回复
    if (text.indexOf(QRegularExpression("\\[CQ:reply,id=(-?\\d+)\\](\\[CQ:at,qq=\\d+\\])?"), 0, &match) > -1)
    {
        text.replace(match.captured(0), "");
        QString messageId = match.captured(1);
        text.insert(0, "<a href=\"msg://" + messageId + "\"><span style=\"text-decoration: none; color:" + QVariant(us->bannerLinkColor).toString() + ";\">[回复]</span></a>");
    }

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
                if (!c.isValid())
                    c = us->bannerLinkColor;

                QString newText = "<a href=\"at://" + match.captured(1) + "\"><span style=\"text-decoration: none; color:" + QVariant(c).toString() + ";\">@" + members.value(userId).username() + "</span></a>";
                text.replace(match.captured(0), newText);
            }
            else // 群组里没有这个人，刚加入？
            {
                emit needMemberNames();
                pos++;
            }
        }
    }
    text.replace(QRegularExpression("\\[CQ:at,qq=(\\d+)\\]"), "<a href=\"at://\\1\"><span style=\"text-decoration: none; color:" + QVariant(us->bannerLinkColor).toString() + ";\">@\\1</span></a>"); // 万一有没有替换完的呢
    text.replace(QRegularExpression("\\[CQ:at,qq=all\\]"), "<a href=\"at://all\"><span style=\"text-decoration: none; color:" + QVariant(us->bannerLinkColor).toString() + ";\">@全体成员</span></a>"); // @全体

    // json
    if (text.indexOf(QRegularExpression("\\[CQ:json,data=(.+?)\\]"), 0, &match) > -1)
    {
        QString s = match.captured(1);
        s.replace("\\\"", "\"").replace("&#44;", ",");

        // 显示文字
        MyJson json(s.toUtf8());
        QString prompt;
        if (json.contains("prompt"))
        {
            prompt = json.s("prompt");
        }
        else
        {
            // 去掉CQ码的纯文本
            QString tmp = text;
            tmp.replace(match.captured(0), "");
            prompt = tmp;
        }

        QString previewUrl; // 预览图
        QString jumpUrl; // 跳转地址
        auto getUrlFileName = [=](QString url) -> QString {
            QRegularExpression re("/([^/\\?]+)(\\?.+)?$");
            QRegularExpressionMatch match;
            if (url.indexOf(re, 0, &match) != -1)
                return match.captured(1);
            return url.toUtf8().toBase64();
        };
        if (json.contains("meta"))
        {
            MyJson meta = json.o("meta");
            if (meta.keys().size())
            {
                MyJson detail = meta.o(meta.keys().first());
                if (detail.contains("preview"))
                    previewUrl = detail.s("preview");
                else if (detail.contains("icon"))
                    previewUrl = detail.s("icon");
                if (!previewUrl.isEmpty() && !previewUrl.contains("://"))
                    previewUrl.insert(0, "http://");

                if (detail.contains("jumpUrl"))
                    jumpUrl = detail.s("jumpUrl");
                else if (detail.contains("qqdocurl"))
                    jumpUrl = detail.s("qqdocurl");
                if (!jumpUrl.contains("://"))
                    jumpUrl.insert(0, "https://");

                if (detail.contains("desc"))
                    prompt = detail.s("desc");
            }
        }

        // 重新构造显示
        text = "<a href='" + jumpUrl +"'>";
        text += "<span style=\"text-decoration: none; color:" + QVariant(us->bannerLinkColor).toString() + ";\">" + prompt + "</span>";

        // 下载图片
        QString imgPath;
        if (!previewUrl.isEmpty())
        {
            QString fileId = getUrlFileName(previewUrl);
            imgPath = rt->imageSCache(fileId);

            if (!isFileExist(imgPath))
            {
                imgPath = rt->imageCache(fileId); // 原图路径
                NetImageUtil::saveNetFile(previewUrl, imgPath);

                // 调整图片大小
                QPixmap pixmap(imgPath, "1");
                int maxWidth = us->bannerContentWidth - us->bannerBubblePadding * 2;
                int maxHeight = (us->bannerContentMaxHeight - us->bannerHeaderSize - us->bannerBubblePadding * 2) * 2 / 3;
                this->filePixmap = pixmap;
                if (pixmap.width() < maxWidth / us->bannerThumbnailProp
                        && pixmap.height() < maxHeight / us->bannerThumbnailProp)
                {
                    // 图片过小，不压缩
                }
                else if (pixmap.width() / us->bannerThumbnailProp < maxWidth
                        && pixmap.height() / us->bannerThumbnailProp < maxHeight)
                {
                    // 按固定比例压缩
                    pixmap = pixmap.scaled(QSize(maxWidth, maxHeight) / us->bannerThumbnailProp, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                }
                else if (pixmap.width() > maxWidth || pixmap.height() > maxHeight)
                {
                    // 这个会缩得更小
                    pixmap = pixmap.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                }
                pixmap = NetImageUtil::toRoundedPixmap(pixmap, us->bannerBgRadius);
                imgPath = rt->imageSCache(fileId); //  缩略图路径
                pixmap.save(imgPath);
            }

            text += "<br /><img src='" + imgPath + "' />";
        }

        text += "</a>";
    }

    // record
    if (text.indexOf(QRegularExpression("\\[CQ:record,file=(.+?)(?:\\.(?:si?lk|amr))?,url=(.*)\\]"), 0, &match) > -1)
    {
        QString file = match.captured(1); // avsdqwezc.video
        QString url = match.captured(2).replace("&amp;", "&"); // http://xxx.xx?ver=xxx&rkey=xx&filetype=1003&videotype=1&subvideotype=0&term=unknow
        QString path = rt->audioCache(file);

        // 需要下载语音
        if (!url.isEmpty() && (us->autoPlaySpeech || us->autoTransSpeech))
        {
            if (!isFileExist(path)) // 可能重复发送，也可能从历史消息加载，所以不重复读取
                NetImageUtil::saveNetFile(url, path);
            this->filePath = path;

            // 自动播放语音
            if (us->autoPlaySpeech)
            {
                QMediaPlayer* player = new QMediaPlayer(nullptr);
                QMediaPlaylist* list = new QMediaPlaylist;
                list->addMedia(QUrl::fromLocalFile(path));
                list->setPlaybackMode(QMediaPlaylist::CurrentItemOnce);
                player->setPlaylist(list);
                player->play();
                connect(player, &QMediaPlayer::stateChanged, player, [=](QMediaPlayer::State state) {
                    if (state == QMediaPlayer::State::StoppedState)
                        player->deleteLater();
                });
            }

            // 自动转换语音
            if (us->autoTransSpeech && !us->baiduSpeechAccessToken.isEmpty())
            {
                QFile file(path);
                if (file.open(QIODevice::ReadOnly))
                {
                    QByteArray ba = file.readAll();
                    file.close();
                    QByteArray base64 = ba.toBase64();

                    // 百度语音API说明：https://ai.baidu.com/ai-doc/SPEECH/ek38lxj1u
                    MyJson json;
                    json.insert("format", "amr");
                    json.insert("rate", 8000);
                    json.insert("channel", 1);
                    json.insert("cuid", snum(ac->myId));
                    json.insert("token", us->baiduSpeechAccessToken);
                    json.insert("dev_pid", 1537);
                    json.insert("speech", QString(base64));
                    json.insert("len", file.size());

                    QByteArray rst = NetUtil::postJsonData("http://vop.baidu.com/server_api", json);
                    json = MyJson(rst);
                    QStringList results;
                    if (json.i("err_no") != 0)
                        qWarning() << "获取语音失败：" << json;
                    json.eachVal("result", [&](QJsonValue val) {
                        results.append(val.toString());
                    });

                    // 点击调用系统播放器进行播放
                    text.replace(match.captured(0), linkText("[语音] " + results.join("\n"), path));
                }
                else
                {
                    qWarning() << "加载音频文件失败：" << path;
                    text.replace(match.captured(0), linkText("[语音] ", path));
                }
            }
        }
        else
        {
            // 跳到下载地址
            text.replace(match.captured(0), linkText("[语音] ", url));
        }
    }

    // video
    if (text.indexOf(QRegularExpression("\\[CQ:video,file=(.+?)(?:\\.video)?,url=(.+)\\]"), 0, &match) > -1)
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
            int maxWidth = us->bannerContentWidth - us->bannerBubblePadding * 2;
            int maxHeight = (us->bannerContentMaxHeight - us->bannerHeaderSize - us->bannerBubblePadding * 2) * 2 / 3;
            Q_UNUSED(maxWidth)

            // 控件
            QHBoxLayout* lay = new QHBoxLayout(contentWidget);
            VideoLabel* vw = new VideoLabel(contentWidget);
//            VideoWidget* vw = new VideoWidget(contentWidget); // 透明窗口不能使用 QVideoWidget，很无奈
            lay->addWidget(vw);
            lay->setMargin(0);

            // 播放视频
            vw->player->setMuted(true);
            vw->setRadius(us->bannerBgRadius);
            vw->setMedia(path);
            vw->show();
            contentWidget->setMinimumHeight(maxHeight / 2);
            contentWidget->setMaximumHeight(maxHeight); // TODO: 设置成视频高度才合适

            // 设置文字
            text.replace(match.captured(0), "");
            if (!text.trimmed().isEmpty()) // 文字视频结合
            {
                QLayout* l = contentWidget->layout();
                auto vl = new QVBoxLayout;
                vl->setParent(l);
                QLabel* label = new QLabel(text, this);
                label->adjustSize();
                vl->addWidget(label);
                vl->addStretch(1);
                l->addItem(vl);
                l->addWidget(vw);
                l->setAlignment(Qt::AlignTop | Qt::AlignLeft);
                text = "";
                contentWidget->setMinimumHeight(maxHeight);
                contentWidget->setMaximumHeight(maxHeight);
                vw->setMinimumHeight(maxHeight);
                vw->setMaximumHeight(maxHeight);
                vw->setMinimumWidth(maxWidth - label->width() - l->margin() * 2 - l->spacing());
            }
            else // 单个视频
            {
                // 会自适应宽高
                contentWidget->layout()->addWidget(vw);
            }

            vw->setCursor(Qt::PointingHandCursor);
            connect(vw, &ClickLabel::leftClicked, this, [=]{
                emit signalOpenOut();
                QDesktopServices::openUrl(QUrl(path));
            });

            singleImage = true;
        }
        else // 不缓存视频
        {
            // text.replace(match.captured(0), "<a href='" + url + "'>[video]</a>"); // 加上超链接
            text.replace(match.captured(0), linkText("[视频]",url));
            // TODO: 打开网络视频播放器
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
            text.replace(match.captured(0), linkText("[文件]" + msg.fileName, path));
        }
        else // 不下载文件
        {
            if (msg.fileUrl.isEmpty())
                text.replace(match.captured(0), grayText("[文件] " + msg.fileName));
            else
                text.replace(match.captured(0), linkText("[文件] " + msg.fileName, msg.fileUrl));
        }
    }

    // 红包
    text.replace(QRegularExpression("\\[CQ:redbag.*?\\]"), grayText("[红包]"));

    // 其他格式
    text.replace(QRegularExpression("\\[CQ:(\\w+),.+?\\]"), grayText("[\\1]"));

    // 实体
    text.replace("&#91;", "[").replace("&#93;", "]");

    // 超链接
    text.replace(QRegularExpression("(?<!['\"])((http|ftp)s?://[\\w\\._\\-]+\\.\\w{2,5}([\\?\\/][^\\s<]*)?)"), "<a href=\"\\1\">\\1</a>");
    text.replace(QRegularExpression("([a-zA-Z0-9_-]+@[a-zA-Z0-9_-]+(\\.[a-zA-Z0-9_-]+)+)"), "<a href=\"mailto:\\1\">\\1</a>");

    // #处理长度（注意要忽略各种标签）
    // if (text.length() > us->msgMaxLength)
    //     text = text.left(us->msgMaxLength) + "...";

    // 处理换行
    text.replace("\n", "<br/>");

    // #设置显示
    contentWidget->setText(text);

    // 重新调整一下界面
    updateStyleSheet();
}

MessageView* MessageView::setRepyMessage(const MsgBean &replyMsg, int recursion)
{
    replyWidget = new MessageView(this);
    replyWidget->setTextColor(this->textColor);
    replyWidget->setMessage(replyMsg, recursion);
    emit connectNewMessageView(replyWidget);
    return replyWidget;
}
