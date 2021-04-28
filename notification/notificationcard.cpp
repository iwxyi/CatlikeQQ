#include <QPropertyAnimation>
#include <QPainter>
#include <QPainterPath>
#include <QMovie>
#include <QDesktopServices>
#include "notificationcard.h"
#include "ui_notificationcard.h"
#include "global.h"
#include "netimageutil.h"
#include "fileutil.h"
#include "imageutil.h"

NotificationCard::NotificationCard(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NotificationCard)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::MSWindowsFixedSizeDialogHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlag(Qt::WindowStaysOnTopHint, true);

    displayTimer = new QTimer(this);
    displayTimer->setInterval(us->bannerDisplayDuration);
    connect(displayTimer, SIGNAL(timeout()), this, SLOT(toHide()));

    ui->headerLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->nicknameLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    // ui->listWidget->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->replyButton->setRadius(us->bannerBgRadius);
    ui->messageEdit->hide();
    ui->listWidget->setFixedHeight(0);
    ui->listWidget->setMinimumHeight(0);
    ui->listWidget->setMaximumHeight(us->bannerMaximumHeight);

    connect(ui->replyButton, SIGNAL(clicked()), this, SLOT(showReplyEdit()));
    connect(ui->messageEdit, SIGNAL(returnPressed()), this, SLOT(sendReply()));

    // 绘制背景
    bg = new InteractiveButtonBase(this);
    bg->lower();
    bg->setRadius(us->bannerBgRadius);
    bg->setBgColor(us->bannerBgColor);
    CREATE_SHADOW(bg);

    // 焦点处理
    connect(bg, SIGNAL(signalMouseEnter()), this, SLOT(focusIn()));
    connect(bg, SIGNAL(signalMouseLeave()), this, SLOT(focusOut()));
    connect(ui->messageEdit, &ReplyEdit::signalESC, this, [=]{
        hideReplyEdit();
        focusOut();
    });
    connect(ui->messageEdit, SIGNAL(signalFocusOut()), this, SLOT(focusOut()));
    connect(bg, SIGNAL(clicked()), this, SLOT(cardClicked()));
}

NotificationCard::~NotificationCard()
{
    delete ui;
}

void NotificationCard::setMsg(const MsgBean &msg)
{
    this->userId = msg.senderId;
    this->groupId = msg.groupId;
    msgs.append(msg);

    if (!msg.isGroup())
    {
        setPrivateMsg(msg);
    }
    else
    {
        setGroupMsg(msg);
    }

    /* // 显示
    if (!groupId)
        ui->nicknameLabel->setText(msg.displayNickname());
    else
        ui->nicknameLabel->setText(msg.displayNickname() + " · " + msg.groupName);
    showText = msg.displayString();
    showText.replace("<", "&lt;").replace(">", "&gt;");
    if (msg.imageId.isEmpty())
    {
        ui->messageLabel->setText(showText);
    }
    else
    {
        int maxWidth = us->bannerWidth;
        int maxHeight = us->bannerWidth/3;
        QString path = rt->imageCache(msg.imageId);

        // 支持GIF
        QMovie* movie = new QMovie(rt->imageCache(msg.imageId), "gif", this);
        if (movie->frameCount()) // 有帧，表示是GIF
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

            ui->messageLabel->setMaximumSize(maxWidth, maxHeight);
            ui->messageLabel->setMovie(movie);
            movie->start();
        }
        else // 是静态图
        {
            delete movie; // 释放文件锁
            QPixmap pixmap(path, "1");
            ui->messageLabel->setPixmap(pixmap.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }

        // 其他设置
        single = true; // 不允许合并（因为没法合并啊……）
    } */

    // 设置大小
    setFixedWidth(us->bannerWidth);
    this->layout()->activate();
    resize(this->sizeHint());

    // 调整显示的时长
    if (us->bannerTextReadSpeed)
    {
        displayTimer->setInterval(getReadDisplayDuration(msg.displayString().length()));
    }
}

/**
 * 测试能否直接添加到这里，就是消息内容直接添加一行（可能是很长一行）
 * @return 修改后的卡片高度差
 */
bool NotificationCard::append(const MsgBean &msg, int &delta)
{
    if (this->userId != msg.senderId || this->groupId != msg.groupId)
        return false;

    int h = height();

    // 插入到自己的消息
    msgs.append(msg);

    if (!msg.isGroup())
    {
        appendPrivateMsg(msg);
    }
    else
    {
        appendGroupMsg(msg);
    }

    /* // 段落显示
    QString s = msg.displayString();
    s.replace("<", "&lt;").replace(">", "&gt;");
    showText.append("<p>" + s + "</p>");
    ui->messageLabel->setText(showText); */

    // 调整显示时间
    if (displayTimer->isActive())
    {
        displayTimer->setInterval(qMax(0, displayTimer->remainingTime() - us->bannerDisplayDuration) + getReadDisplayDuration(msg.displayString().length()));
        displayTimer->start();
    }

    // 调整尺寸
    this->layout()->activate();
    resize(this->sizeHint());
    delta = height() - h;
    return true;
}

/// 设置第一个私聊消息
void NotificationCard::setPrivateMsg(const MsgBean &msg)
{
    // 设置标题
    ui->nicknameLabel->setText(msg.displayNickname());

    // 设置头像
    // 用户头像API：http://q1.qlogo.cn/g?b=qq&nk=QQ号&s=100&t=
    if (isFileExist(rt->userHeader(msg.senderId)))
    {
        ui->headerLabel->setPixmap(NetImageUtil::toRoundedPixmap(QPixmap(rt->userHeader(msg.senderId))));
    }
    else // 没有头像，联网获取
    {
        QString url = "http://q1.qlogo.cn/g?b=qq&nk=" + snum(msg.senderId) + "&s=100&t=";
        QPixmap pixmap = NetImageUtil::loadNetPixmap(url);
        if (!us->bannerUseHeaderColor)
            pixmap = NetImageUtil::toRoundedPixmap(pixmap);
        pixmap.save(rt->userHeader(msg.senderId));
        ui->headerLabel->setPixmap(NetImageUtil::toRoundedPixmap(pixmap));
    }

    // 设置背景颜色
    if (us->bannerUseHeaderColor)
    {
        if (ac->userHeaderColor.contains(msg.senderId))
            cardColor = ac->userHeaderColor.value(msg.senderId);
        else
            ImageUtil::getBgFgColor(ImageUtil::extractImageThemeColors(ui->headerLabel->pixmap()->toImage(), 2), &cardColor.bg, &cardColor.fg);
        setColors(cardColor.bg, cardColor.fg);
    }

    // 添加消息
    addNewEdit(msg);
}

/// 设置第一个群聊消息
void NotificationCard::setGroupMsg(const MsgBean &msg)
{
    // 设置标题
    ui->nicknameLabel->setText(msg.groupName);

    // 设置头像
    // 群头像API：http://p.qlogo.cn/gh/群号/群号/100
    if (us->isGroupShow(msg.groupId))
    {
        if (isFileExist(rt->groupHeader(msg.groupId)))
        {
            ui->headerLabel->setPixmap(NetImageUtil::toRoundedPixmap(QPixmap(rt->groupHeader(msg.groupId))));
        }
        else
        {
            QString url = "http://p.qlogo.cn/gh/" + snum(msg.groupId) + "/" + snum(msg.groupId) + "/100";
            QPixmap pixmap = NetImageUtil::loadNetPixmap(url);
            pixmap.save(rt->groupHeader(msg.groupId));
            ui->headerLabel->setPixmap(NetImageUtil::toRoundedPixmap(pixmap));
        }
    }

    // 设置背景颜色
    if (us->bannerUseHeaderColor)
    {
        if (ac->groupHeaderColor.contains(msg.groupId))
            cardColor = ac->groupHeaderColor.value(msg.groupId);
        else
            ImageUtil::getBgFgColor(ImageUtil::extractImageThemeColors(ui->headerLabel->pixmap()->toImage(), 2), &cardColor.bg, &cardColor.fg);
        setColors(cardColor.bg, cardColor.fg);
    }

    // 根据消息调整高度
    addNewEdit(msg);
}

/// 添加一个私聊消息
void NotificationCard::appendPrivateMsg(const MsgBean &msg)
{
    addNewEdit(msg);
}

/// 添加一个群组消息，每条都有可能是独立的头像、昵称（二级标题）
void NotificationCard::appendGroupMsg(const MsgBean &msg)
{

}

/// 一个卡片只显示一个人的消息的情况
void NotificationCard::addSingleSenderMsg(const MsgBean &msg)
{

}

/// 根据头像设置为对应的背景颜色
void NotificationCard::setBgColorByHeader(const QPixmap &pixmap)
{
    if (!us->bannerUseHeaderColor)
        return ;

    AccountInfo::CardColor co;
    ImageUtil::getBgFgColor(ImageUtil::extractImageThemeColors(pixmap.toImage(), 2), &co.bg, &co.fg);
    setColors(co.bg, co.fg);
}

void NotificationCard::addNewEdit(const MsgBean& msg)
{
    MessageEdit* edit = new MessageEdit(this);
    edit->setMessage(msg);
    edit->setTextColor(cardColor.fg);

    QListWidgetItem* item = new QListWidgetItem(ui->listWidget);
    ui->listWidget->setItemWidget(item, edit);

    QSize sz = edit->adjustSizeByTextWidth(ui->nicknameLabel->width());
    edit->resize(sz);
    item->setSizeHint(sz);

    int sumHeight = 0;
    for (int i = 0; i < ui->listWidget->count(); i++)
    {
        auto widget = ui->listWidget->itemWidget(ui->listWidget->item(i));
        sumHeight += widget->height();
    }
    ui->listWidget->setFixedHeight(sumHeight);

    this->adjustSize();
}

bool NotificationCard::isPrivate() const
{
    return !groupId;
}

bool NotificationCard::isGroup() const
{
    return groupId;
}

bool NotificationCard::isHidding() const
{
    return hidding;
}

bool NotificationCard::canMerge() const
{
    return !hidding && !single;
}

void NotificationCard::focusIn()
{
    displayTimer->stop();
}

void NotificationCard::focusOut(bool force)
{
    if (!force && ((ui->messageEdit->isVisible() && ui->messageEdit->hasFocus())
            || bg->isInArea(bg->mapFromGlobal(QCursor::pos()))))
        return ;
    displayTimer->setInterval(us->bannerRetentionDuration);
    displayTimer->start();
}

void NotificationCard::showReplyEdit()
{
    if (ui->messageEdit->isHidden()) // 显示消息框
    {
        ui->replyButton->setText("发送");
        ui->messageEdit->show();
        ui->messageEdit->setFocus();
        ui->horizontalLayout_2->removeItem(ui->horizontalSpacer);
        // delete ui->horizontalSpacer;
    }
    else // 发送内容
    {
        sendReply();
    }
}

void NotificationCard::hideReplyEdit()
{
    ui->messageEdit->hide(); // 会触发 FocusOut 事件
    ui->horizontalLayout_2->insertItem(0, ui->horizontalSpacer);
    ui->replyButton->setText("回复");
}

void NotificationCard::sendReply()
{
    QString text = ui->messageEdit->text();
    if (text.isEmpty())
        return ;

    // 回复消息
    if (!groupId && userId)
        emit signalReplyPrivate(userId, text);
    else if (groupId)
        emit signalReplyGroup(groupId, text);
    else
    {
        qCritical() << "回复失败，无法获取到 userId 或者 groupId";
        return ;
    }

    // 加到消息框中
    // TODO: 添加自己的消息
    ui->messageEdit->clear();

    // 关闭对话框
    if (us->bannerCloseAfterReply)
    {
        hideReplyEdit();
        focusOut(true);
    }
}

/**
 * 从某一个点开始出现，直到完全展示出来
 */
void NotificationCard::showFrom(QPoint hi, QPoint sh)
{
    this->hidePoint = hi;
    move(hi);

    QPropertyAnimation* ani = new QPropertyAnimation(this, "pos");
    ani->setStartValue(hi);
    ani->setEndValue(sh);
    ani->setEasingCurve(QEasingCurve::Type(us->bannerShowEasingCurve));
    ani->setDuration(us->bannerAnimationDuration);
    connect(ani, SIGNAL(finished()), ani, SLOT(deleteLater()));
    ani->start();
    show();
}

void NotificationCard::setColors(QColor bg, QColor fg)
{
    this->bg->setBgColor(bg);
    QPalette pa(ui->nicknameLabel->palette());
    pa.setColor(QPalette::Foreground, fg);
    pa.setColor(QPalette::Text, fg);
    ui->nicknameLabel->setPalette(pa);
    ui->listWidget->setPalette(pa);
    ui->messageEdit->setPalette(pa);
    ui->replyButton->setTextColor(fg);
}

/**
 * 从当前位置移动到消失的点
 * 注意：如果支持手势的话，“消失的点”可能会变更
 */
void NotificationCard::toHide()
{
    hidding = true;

    QPropertyAnimation* ani = new QPropertyAnimation(this, "pos");
    ani->setStartValue(pos());
    ani->setEndValue(hidePoint);
    ani->setEasingCurve(QEasingCurve::Type(us->bannerShowEasingCurve));
    ani->setDuration(us->bannerAnimationDuration);
    connect(ani, &QPropertyAnimation::finished, this, [=]{
        emit signalHided();
        ani->deleteLater();
        this->deleteLater();
    });
    ani->start();
}

void NotificationCard::cardClicked()
{
    if (!msgs.size())
        return ;

    const MsgBean& msg = msgs.last();
    if (!msg.imageId.isEmpty())
    {
        QDesktopServices::openUrl(QUrl("file:///" + rt->imageCache(msg.imageId), QUrl::TolerantMode));
    }
}

int NotificationCard::getReadDisplayDuration(int length) const
{
    return us->bannerDisplayDuration + (length * 1000 / us->bannerTextReadSpeed);
}

void NotificationCard::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    displayTimer->start();
}

void NotificationCard::paintEvent(QPaintEvent *)
{
}

void NotificationCard::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    bg->resize(event->size() - QSize(us->bannerBgShadow, us->bannerBgShadow));
}

void NotificationCard::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    int row = ui->listWidget->row(item);
    const MsgBean msg = msgs.at(row);
    // 如果是图片

    // 如果是文件
}
