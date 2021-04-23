#include <QPropertyAnimation>
#include <QPainter>
#include <QPainterPath>
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
        ui->messageEdit->hide(); // 也会触发 FocusOut 事件
        ui->horizontalLayout_2->insertItem(0, ui->horizontalSpacer);
        ui->replyButton->setText("回复");
        focusOut();
    });
    connect(ui->messageEdit, SIGNAL(signalFocusOut()), this, SLOT(focusOut()));
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
    ui->messageLabel->setText(showText);

    this->layout()->activate();
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

    this->layout()->activate();
    delta = height() - h;
    return true;
}

bool NotificationCard::isHidding() const
{
    return hidding;
}

void NotificationCard::focusIn()
{
    displayTimer->stop();
}

void NotificationCard::focusOut()
{
    if ((ui->messageEdit->isVisible() && ui->messageEdit->hasFocus())
            || bg->isInArea(bg->mapFromGlobal(QCursor::pos())))
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
    ani->setDuration(us->bannerAnimationDuration);
    connect(ani, SIGNAL(finished()), ani, SLOT(deleteLater()));
    ani->start();
    show();
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
    ani->setDuration(us->bannerAnimationDuration);
    connect(ani, &QPropertyAnimation::finished, this, [=]{
        emit signalHided();
        ani->deleteLater();
        this->deleteLater();
    });
    ani->start();
}

void NotificationCard::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    displayTimer->start();
}

void NotificationCard::paintEvent(QPaintEvent *event)
{

}

void NotificationCard::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    bg->resize(event->size() - QSize(us->bannerBgShadow, us->bannerBgShadow));
}
