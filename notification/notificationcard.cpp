#include <QPropertyAnimation>
#include <QPainter>
#include <QPainterPath>
#include <QMovie>
#include <QDesktopServices>
#include "notificationcard.h"
#include "ui_notificationcard.h"

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
    ui->messageLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->replyButton->setRadius(us->bannerBgRadius);
    ui->messageEdit->hide();
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

    // 显示
    if (!groupId)
        ui->nicknameLabel->setText(msg.nickname);
    else
        ui->nicknameLabel->setText(msg.nickname + " · " + msg.groupName);
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
    }

    ui->headerLabel->setPixmap(msg.header);

    setFixedWidth(us->bannerWidth);
    this->layout()->activate();
    resize(this->sizeHint());
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

    // 段落显示
    QString s = msg.message;
    s.replace("<", "&lt;").replace(">", "&gt;");
    showText.append("<p>" + s + "</p>");
    ui->messageLabel->setText(showText);

    // 调整显示时间
    if (displayTimer->isActive())
        displayTimer->start();

    // 调整尺寸
    this->layout()->activate();
    resize(this->sizeHint());
    delta = height() - h;
    return true;
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
    showText.append("<p>>" + text + "</p>");
    ui->messageLabel->setText(showText);
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
    ui->nicknameLabel->setPalette(pa);
    ui->messageLabel->setPalette(pa);
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

    const MsgBean& msg = msgs.first();
    if (!msg.imageId.isEmpty())
    {
        QDesktopServices::openUrl(QUrl("file:///" + rt->imageCache(msg.imageId), QUrl::TolerantMode));
    }
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
