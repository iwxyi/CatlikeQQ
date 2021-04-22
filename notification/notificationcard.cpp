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
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlag(Qt::WindowStaysOnTopHint, true);

    displayTimer = new QTimer(this);
    displayTimer->setInterval(us->bannerDisplayDuration);
    connect(displayTimer, SIGNAL(timeout()), this, SLOT(toHide()));

    ui->headerLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->nicknameLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->messageLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    // 绘制背景
    bg = new InteractiveButtonBase(this);
    bg->lower();
    bg->setRadius(us->bannerBgRadius);
    bg->setBgColor(us->bannerBgColor);
    CREATE_SHADOW(bg);
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
    ui->nicknameLabel->setText(msg.nickname);
    ui->messageLabel->setText(msg.displayString());
}

/**
 * 测试能否直接添加到这里，就是消息内容直接添加一行（可能是很长一行）
 * @return 修改后的卡片高度差
 */
int NotificationCard::append(const MsgBean &msg)
{
    if (this->userId != msg.senderId || this->groupId != msg.groupId)
        return 0;

    int h = height();

    // 插入到自己的消息
    msgs.append(msg);

    // 段落显示
    QString s = msg.message;
    s.replace("<", "&lt;").replace(">", "&gt;");
    showText.append("<p>" + s + "</p>");
    ui->messageLabel->setText(showText);

    this->layout()->activate();
    return this->height() - h;
}

bool NotificationCard::isHidding() const
{
    return hidding;
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
