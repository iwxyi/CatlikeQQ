#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QTimer>
#include <QClipboard>
#include <QMimeData>
#include <QProcess>
#include <QStyleOptionTab>
#include "fileutil.h"
#include "messageview.h"
#include "defines.h"
#include "runtime.h"
#include "usettings.h"
#include "netimageutil.h"
#include "accountinfo.h"
#include "myjson.h"
#include "facilemenu.h"
#include "signaltransfer.h"
#include "netutil.h"

MessageView::MessageView(QWidget *parent)
    : QWidget(parent),
      msg(0, "")
{
#ifdef MESSAGE_LABEL
    contentWidget = new QLabel(this);
    contentWidget->setWordWrap(true);
    contentWidget->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
    contentWidget->setTextFormat(Qt::RichText);

    connect(contentWidget, &QLabel::linkActivated, this, [=](const QString& link) {
        qInfo() << "打开链接：" << link;
        if (link.startsWith("msg://"))
        {
            // 聚焦到消息
            QString messageId = link.right(link.length() - 6);
            emit focusMessage(messageId.toLongLong());
        }
        else if (link.startsWith("at://"))
        {
            QString qqId = link.right(link.length() - 5);
            emit replyText("[CQ:at,qq=" + qqId + "] ");
        }
        else
        {
            // 打开网页
            QDesktopServices::openUrl(QUrl(link));
        }
    });
#else
    contentWidget = new QTextBrowser(this);
    contentWidget->setReadOnly(true);
    contentWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    contentWidget->setWordWrapMode(QTextOption::WrapMode::WrapAnywhere);
#endif
    contentWidget->setContentsMargins(0, 0, 0, 0);
    contentWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    contentWidget->connect(contentWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showMenu()));

    // 设置样式
    this->setObjectName("MessageBubble");
    contentWidget->setObjectName("ContentWidget");

    // 设置布局
    vlayout = new QVBoxLayout(this);
    vlayout->addWidget(contentWidget);
    vlayout->setMargin(us->bannerBubblePadding);
    vlayout->setSpacing(us->bannerBubblePadding);
}

/// 这个是最简单的文字替换
/// 不涉及到富文本、文件下载等等
QString MessageView::simpleMessage(const MsgBean &msg)
{
    QString text = msg.message;
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

    text.replace(QRegularExpression("\\[CQ:at,qq=all\\]"), "[@全体成员]");

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
    text.replace(QRegularExpression("(?<!['\"])((http|ftp)s?://[\\w\\.]+\\.\\w{2,5}([\\?\\/][^\\s<]*)?)"), "<a href=\"\\1\">\\1</a>");
    text.replace(QRegularExpression("([a-zA-Z0-9_-]+@[a-zA-Z0-9_-]+(\\.[a-zA-Z0-9_-]+)+)"), "<a href=\"mailto:\\1\">\\1</a>");

    // #设置显示
    return text;
}

/// 预先设置宽度
/// 避免 adjustSizeByWidth 时 adjustSize 会导致宽度不对以至于后期固定为最大宽度时留下空白的问题
void MessageView::setPrevContentWidth(int w)
{
    this->fixedWidth = w;
    contentWidget->setFixedWidth(w);
}

/// 把形如 @123456 的格式统统替换为 @某某
void MessageView::replaceGroupAt()
{
    const auto members = ac->groupList.value(msg.groupId).members;
    if (!members.size())
        return ;

#ifdef MESSAGE_LABEL
    QString text = contentWidget->text();
#else
    QString text = contentWidget->toPlainText();
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
        contentWidget->setText(text);
}

void MessageView::showMenu()
{
    emit keepShowing();
    FacileMenu* menu = new FacileMenu(this);

    if (!filePixmap.isNull())
    {
        menu->addAction(QIcon("://icons/copy_image.png"), "复制图片", [=]{
            QApplication::clipboard()->setPixmap(filePixmap);
        });
    }

    if (!filePath.isEmpty())
    {
        QString path = filePath;
        path.replace("/", "\\");

        menu->addAction(QIcon("://icons/copy_file.png"), "复制文件", [=]{
            QMimeData* mime = new QMimeData();
            mime->setText(QFileInfo(path).absoluteFilePath());
            mime->setUrls(QList<QUrl>{QUrl::fromLocalFile(path)});
            mime->setData("x-special/gnome-copied-files", QByteArray("copy\n") + QUrl::fromLocalFile(path).toEncoded());
            QApplication::clipboard()->setMimeData(mime);
        });

        menu->addAction(QIcon("://icons/open_folder.png"), "打开文件夹", [=]{
            const QString explorer = "explorer";
            QStringList param;
            param << QLatin1String("/select,");
            param << path;
            qInfo() << "文件所在路径：" << path;
            QProcess::startDetached(explorer, param);
        });
    }

    if (!filePath.isNull() || !filePath.isEmpty())
        menu->split();

    menu->addAction(QIcon("://icons/reply.png"), "回复", [=]{
        QString text = "[CQ:reply,id=" + snum(msg.messageId) + "][CQ:at,qq=" + snum(msg.senderId) + "] ";
        if (us->replyMessageContainsAt)
            text += "[CQ:at,qq=" + snum(msg.senderId) + "] ";
        emit replyText(text);
    });

    // 是自己发的消息
    if (msg.senderId == ac->myId)
    {
        menu->addAction(QIcon("://icons/undo.png"), "撤回", [=]{
            MyJson json;
            json.insert("action", "delete_msg");
            MyJson params;
            params.insert("message_id", msg.messageId);
            json.insert("params", params);
            if (msg.isPrivate())
                json.insert("echo", "msg_recall_private:" + snum(msg.friendId) + "_" + snum(msg.messageId));
            else if (msg.isGroup())
                json.insert("echo", "msg_recall_group:" + snum(msg.groupId) + "_" + snum(msg.messageId));
            else
                return ;
            emit sig->sendSocketText(json.toBa());
        });
    }

    if (msg.isGroup())
    {
        menu->addAction(QIcon("://icons/single_reply.png"), "单独回复", [=]{
            QString text = "[CQ:reply,id=" + snum(msg.messageId) + "]";
            emit sig->openUserCard(msg.senderId, msg.displayNickname(), text);
        });

        menu->addAction(QIcon("://icons/at.png"), "@TA", [=]{
            emit replyText("[CQ:at,qq=" + snum(msg.senderId) + "] ");
        })->text(msg.senderId == ac->myId, "@自己");
    }

    menu->addAction(QIcon("://icons/resay.png"), "+1", [=]{
        emit sendText(msg.rawMessage);
    });

    menu->split()->addAction(QIcon("://icons/code.png"), "CQ码", [=]{
        // 必须要有一个拷贝的副本，因为 msg 可能因为通知超时隐藏而删除
        QString cqCode = msg.rawMessage;
        auto btn = QMessageBox::information(this->parentWidget(), "CQ码", cqCode, "取消", "复制", nullptr, 0);
        if (btn == 1)
        {
            QApplication::clipboard()->setText(cqCode);
        }
    });


#ifdef MESSAGE_LABEL
    bool hasSelect = contentWidget->hasSelectedText();
    QString selectedText = contentWidget->selectedText();
    menu->addAction(QIcon("://icons/copy_image.png"), "复制", [=]{
        if (hasSelect)
        {
            QApplication::clipboard()->setText(selectedText);
        }
        else
        {
            QApplication::clipboard()->setText(contentWidget->text());
        }
    })->text(!contentWidget->hasSelectedText(), "复制全部");

    menu->addAction(QIcon("://icons/copy_image.png"), "全选", [=]{
        menu->close();
        contentWidget->setSelection(0, contentWidget->text().length());
    });
#else
#endif

    QString timeFormat = "MM-dd hh:mm";
    if (QDateTime::currentDateTime().date().dayOfYear() == QDateTime::fromMSecsSinceEpoch(msg.timestamp).date().dayOfYear())
        timeFormat = "hh:mm:ss";
    menu->split()->addTitle(QDateTime::fromMSecsSinceEpoch(msg.timestamp).toString(timeFormat));

    if (us->showWidgetBorder)
    {
        menu->addAction(QString("bdr: (%1, %2) -> (%3, %4)").arg(this->sizeHint().width())
                        .arg(this->sizeHint().height()).arg(this->width()).arg(this->height()));
        menu->addAction(QString("bdy: (%1, %2) -> (%3, %4)").arg(contentWidget->sizeHint().width())
                        .arg(contentWidget->sizeHint().height()).arg(contentWidget->width()).arg(contentWidget->height()));
        menu->addAction(QString("sig: %1").arg(singleImage ? "true" : "false"));
    }

    menu->exec();

    menu->finished([=]{
        emit restoreTimerIfNecessary();
    });
}

/// 这里是为了setStyleSheet有效
void MessageView::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}

/// 设置内容后，自动调整大小
/// 宽度可能是固定的，尽可能自适应高度
QSize MessageView::adjustSizeByTextWidth(int w)
{
    if (replyWidget)
    {
        replyWidget->adjustSizeByTextWidth(w - us->bannerBubblePadding * 2);
    }

#ifdef MESSAGE_LABEL
    this->fixedWidth = w;

    if (!us->bannerShowBubble)
    {
        // 不显示气泡
        if (!replyRecursion)
            vlayout->setMargin(0);
        contentWidget->setFixedWidth(fixedWidth);
        contentWidget->adjustSize();
        contentWidget->setFixedHeight(contentWidget->height());

        this->adjustSize();
        this->setFixedHeight(this->height());
    }
    else if (singleImage && !replyRecursion)
    {
        // 单张图片，不显示气泡
        vlayout->setMargin(0);
        if (!useFixedSize) // 例如gif，使用解析时的固定大小
            contentWidget->setFixedSize(contentWidget->sizeHint()); // 本来是图像大小，但是好像会有padding啥的
        this->setFixedSize(this->sizeHint());
    }
    else
    {
        // 显示气泡
        contentWidget->setFixedWidth(fixedWidth - us->bannerBubblePadding * 2); // 设置最大宽度
//        contentWidget->setMinimumWidth(0); // 这样会导致adjustSize固定宽度
//        contentWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored); // 没有效果
        contentWidget->adjustSize();

        contentWidget->setFixedHeight(contentWidget->height()); // 获取到合适高度
        // contentWidget->setFixedSize(contentWidget->sizeHint()); // 已经实现自动换行了，但是换行宽度不对

        // 这个，针对私聊是有效的……
//        contentWidget->setMinimumWidth(0); // 允许调整宽度
//        contentWidget->adjustSize(); // 根据合适高度再调整宽度
//        contentWidget->setFixedWidth(contentWidget->width()); // 再自适应宽度

        this->adjustSize();
        this->setFixedWidth(qMax(contentWidget->width(), replyWidget ? replyWidget->width() : 0) + us->bannerBubblePadding * 2);
        this->setFixedHeight(qMax(this->height(), contentWidget->height() + us->bannerBubblePadding * 2));
    }

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
        return QSize(fixedWidth, QWidget::sizeHint().height());
    return QWidget::sizeHint();
#else
        return QSize(fixedWidth, QTextBrowser::sizeHint().height());
    return QTextBrowser::sizeHint();
#endif
}

void MessageView::updateStyleSheet()
{
    // 设置内容文字颜色
    QString qss = "color: " + QVariant(textColor).toString() + ";";
    if (us->showWidgetBorder)
        qss += "border: 1px solid red;";
    contentWidget->setStyleSheet(qss);

    // 设置背景颜色
    qss = "";
    if (us->bannerShowBubble && !singleImage)
    {
        qss += "border-radius: " + snum(us->bannerBgRadius) + "px;";

        if (replyRecursion)
            qss += "background-color: " + QVariant(us->bannerBubbleReply).toString() + ";";
        else if (msg.senderId == ac->myId) // 自己发的，绿色
            qss += "background-color: " + QVariant(us->bannerBubbleMime).toString() + ";";
        else // 其他人发的
            qss += "background-color: " + QVariant(us->bannerBubbleOppo).toString() + ";";
    }
    else
    {
        // 不设置气泡，或者就单张图片
        // 默认情况下不显示背景
        if (selected)
            qss += "background-color: lightGray;";
        if (replyRecursion)
        {
            qss += "background-color: " + QVariant(us->bannerBubbleReply).toString() + ";";
            qss += "border-radius: " + snum(us->bannerBgRadius) + "px;";
        }
    }

    this->setStyleSheet("#MessageBubble { " + qss + " }");
    // this->setStyleSheet("MessageView { background-color: red;}");
}

/// 设置前景颜色
/// 第一次时可能还没设置相应的message
/// 设置message后必须有一次设置
void MessageView::setTextColor(QColor c)
{
    this->textColor = c;

    // 还没设置，不需要更改界面
    if (!msg.isValid())
        return ;

    QPalette pa(this->palette());
    pa.setColor(QPalette::Foreground, c);
    pa.setColor(QPalette::Text, c);
    setPalette(pa);
    updateStyleSheet();
}

void MessageView::markDeleted()
{
    QString text = contentWidget->text();
    if (text.endsWith("</a>"))
        text.append("<br/>");
    else
        text.append(" ");
    text.append("[已撤回]");
    contentWidget->setText(text);

    textColor.setAlpha(128);
    setTextColor(textColor);
}

void MessageView::markSelected()
{
    this->selected = true;
    updateStyleSheet();
    QTimer::singleShot(500, [=]{
        this->selected = false;
        updateStyleSheet();
    });
}
