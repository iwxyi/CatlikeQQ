#include <QPropertyAnimation>
#include <QPainter>
#include <QPainterPath>
#include <QMovie>
#include <QDesktopServices>
#include <QScrollBar>
#include "notificationcard.h"
#include "ui_notificationcard.h"
#include "defines.h"
#include "netimageutil.h"
#include "fileutil.h"
#include "imageutil.h"
#include "facilemenu.h"
#include "clicklabel.h"

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
    connect(displayTimer, SIGNAL(timeout()), this, SLOT(displayTimeout()));

    ui->headerLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->nicknameLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    // ui->listWidget->setAttribute(Qt::WA_TransparentForMouseEvents, true); // 不能穿透
    ui->listWidget->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->replyButton->setRadius(us->bannerBgRadius);
    ui->messageEdit->hide();

    QFont font;
    font.setPointSize(font.pointSize() + us->bannerTitleLarger);
    ui->nicknameLabel->setFont(font);

    ui->listWidget->setFixedHeight(0);
    ui->listWidget->setMinimumHeight(0);
    ui->listWidget->setMaximumHeight(us->bannerContentHeight);
//    ui->listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(ui->replyButton, SIGNAL(clicked()), this, SLOT(showReplyEdit()));

    // 绘制背景
    bg = new InteractiveButtonBase(this);
    bg->lower();
    bg->setRadius(us->bannerBgRadius);
    bg->setBgColor(us->bannerBgColor);
    bg->setContextMenuPolicy(Qt::CustomContextMenu);
    CREATE_SHADOW(bg);

    // 焦点处理
    connect(bg, SIGNAL(signalMouseEnter()), this, SLOT(mouseEnter()));
    connect(bg, SIGNAL(signalMouseLeave()), this, SLOT(mouseLeave()));
    connect(bg, SIGNAL(clicked()), this, SLOT(cardClicked()));
    connect(bg, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(cardMenu()));

    connect(ui->listWidget, SIGNAL(signalLoadTop()), this, SLOT(loadMsgHistory()));

    connect(ui->messageEdit, &ReplyEdit::signalESC, this, [=]{
        hideReplyEdit();
        focusOut();
        emit signalCancelReply();
    });
    connect(ui->messageEdit, &ReplyEdit::signalFocusOut, this, [=]{
        if (bg->isInArea(bg->mapFromGlobal(QCursor::pos())))
            return ;
        focusOut();
    });
    connect(ui->messageEdit, SIGNAL(signalUp()), this, SIGNAL(signalFocusPrevCard()));
    connect(ui->messageEdit, SIGNAL(signalDown()), this, SIGNAL(signalFocusNextCard()));
    connect(ui->messageEdit, &ReplyEdit::signalMove, this, [=](int index){
        // 因为键盘上从1开始，而数据里从0开始
        if (index > 0)
            index--;
        emit signalFocusCard(index);
    });
    connect(ui->messageEdit, &ReplyEdit::returnPressed, this, [=]{
        sendReply();

        // 关闭对话框
        if (us->bannerCloseAfterReply)
        {
            hideReplyEdit();
            focusOut();
            emit signalCancelReply();
        }
    });
    connect(ui->messageEdit, &ReplyEdit::signalCtrlEnter, this, [=]{
        sendReply();

        // 关闭对话框
        if (!us->bannerCloseAfterReply)
        {
            hideReplyEdit();
            focusOut();
            emit signalCancelReply();
        }
    });

    // 样式表
    QString qss = "/* 整个滚动条背景 */\
                    QScrollBar:vertical\
                    {\
                        width:7px;\
                        background:rgba(205,205,205,0%);\
                        margin:0px,0px,0px,0px;\
                        padding-top:0px;\
                        padding-bottom:0px;\
                    }\
                    \
                    /* 可以按的那一块背景 */\
                    QScrollBar::handle:vertical\
                    {\
                        width:7px;\
                        background:rgba(205, 205, 205, 64);\
                        border-radius:3px;\
                        min-height:20;\
                    }\
                    \
                    /* 可以按的那一块鼠标悬浮 */\
                    QScrollBar::handle:vertical:hover\
                    {\
                        width:7px;\
                        background:rgba(205, 205, 205, 128);\
                        border-radius:3px;\
                        min-height:20;\
                    }\
                    \
                    /* 可以按的那一块鼠标拖动 */\
                    QScrollBar::handle:vertical:pressed\
                    {\
                        width:7px;\
                        background:rgba(205, 205, 205, 255);\
                        border-radius:3px;\
                        min-height:20;\
                    }\
                    \
                    QScrollBar::sub-line:vertical\
                    {\
                        height:9px;width:8px;\
                        border-image:url(:/images/a/1.png);\
                        subcontrol-position:top;\
                    }\
                    \
                    QScrollBar::add-line:vertical\
                    {\
                        height:9px;width:8px;\
                        border-image:url(:/images/a/3.png);\
                        subcontrol-position:bottom;\
                    }\
                    \
                    QScrollBar::add-page:vertical,QScrollBar::sub-page:vertical\
                    {\
                        background:rgba(0,0,0,0%);\
                        border-radius:3px;\
                    }\
        ";
    ui->listWidget->verticalScrollBar()->setStyleSheet(qss);
}

NotificationCard::~NotificationCard()
{
    delete ui;
}

void NotificationCard::setMsg(const MsgBean &msg)
{
    this->senderId = msg.senderId;
    this->groupId = msg.groupId;
    msgs.append(msg);

    if (msg.isPrivate())
    {
        setPrivateMsg(msg);
    }
    else
    {
        setGroupMsg(msg);
    }

    // 设置大小
    setFixedWidth(us->bannerWidth);
    this->layout()->activate();
    resize(this->sizeHint());

    // 调整显示的时长
    if (us->bannerTextReadSpeed)
    {
        displayTimer->setInterval(getReadDisplayDuration(msg.displayString()));
    }

    // 自动展开
    if (us->bannerAutoShowReply)
    {
        showReplyEdit(false);
    }
}

/**
 * 测试能否直接添加到这里，就是消息内容直接添加一行（可能是很长一行）
 * @return 修改后的卡片高度差
 */
bool NotificationCard::append(const MsgBean &msg)
{
    if (isHidding())
        return false;
    if (this->groupId != msg.groupId)
        return false;
    if (this->isPrivate() && this->senderId != msg.senderId)
        return false;

    int h = height();

    if (!msg.isGroup())
    {
        appendPrivateMsg(msg);
    }
    else
    {
        appendGroupMsg(msg);
    }

    // 插入到自己的消息
    msgs.append(msg);

    // 调整显示时间
    if (displayTimer->isActive())
    {
        displayTimer->setInterval(qMax(0, displayTimer->remainingTime() - us->bannerDisplayDuration) + getReadDisplayDuration(msg.displayString()));
        displayTimer->start();
    }

    // 调整尺寸
    this->layout()->activate();
    this->resize(this->sizeHint());
    int hDelta = this->height() - h;
    if (hDelta)
        emit signalHeightChanged(hDelta);

    // 如果正则隐藏的时候收到了消息
    /* if (isHidding())
    {
        showFrom(this->pos(), showPoint);
    } */

    return true;
}

void NotificationCard::adjustTop(int delta)
{
    showPoint.ry() += delta;
    hidePoint.ry() += delta;

    QPropertyAnimation* ani = new QPropertyAnimation(this, "pos");
    ani->setStartValue(this->pos());
    ani->setEndValue(showPoint);
    ani->setEasingCurve(QEasingCurve::Type(us->bannerShowEasingCurve));
    ani->setDuration(us->bannerAnimationDuration);
    connect(ani, SIGNAL(finished()), ani, SLOT(deleteLater()));
    ani->start();
}

/// 设置第一个私聊消息
void NotificationCard::setPrivateMsg(const MsgBean &msg)
{
    // 设置标题
    ui->nicknameLabel->setText(msg.displayNickname());

    // 设置头像
    // 用户头像API：http://q1.qlogo.cn/g?b=qq&nk=QQ号&s=100&t=
    QPixmap headerPixmap;
    if (isFileExist(rt->userHeader(msg.senderId)))
    {
        headerPixmap = QPixmap(rt->userHeader(msg.senderId));
    }
    else // 没有头像，联网获取
    {
        QString url = "http://q1.qlogo.cn/g?b=qq&nk=" + snum(msg.senderId) + "&s=100&t=";
        headerPixmap = NetImageUtil::loadNetPixmap(url);
        headerPixmap.save(rt->userHeader(msg.senderId));
    }

    if (!us->bannerUseHeaderGradient)
    {
        ui->headerLabel->setPixmap(NetImageUtil::toRoundedPixmap(headerPixmap));
    }
    else
    {
        QString path = rt->userHeader("ctg_" + snum(msg.senderId));
        if (!isFileExist(path))
            NetImageUtil::toCornelTransparentGradient(headerPixmap.scaledToWidth(us->bannerHeaderSize*3), us->bannerBgRadius).save(path);
        bg->setIcon(QIcon(path));
        bg->setAlign(Qt::AlignTop | Qt::AlignRight);
        bg->setFixedForePos(true);
        bg->setFixedForeSize(true);
        bg->setPaddings(0);
        ui->verticalSpacer->changeSize(1, ui->headerLabel->height());
        ui->headerLabel->hide();
    }

    // 设置背景颜色
    if (us->bannerUseHeaderColor)
    {
        if (ac->userHeaderColor.contains(msg.senderId))
            cardColor = ac->userHeaderColor.value(msg.senderId);
        else
            ImageUtil::getBgFgColor(ImageUtil::extractImageThemeColors(headerPixmap.toImage(), 8), &cardColor.bg, &cardColor.fg);
        setColors(cardColor.bg, cardColor.fg);
    }

    // 添加消息
    createMsgEdit(msg);

    connect(ui->headerLabel, &ClickLabel::leftClicked, this, [=]{
        showUserInfo(msg.senderId);
    });
    connect(ui->nicknameLabel, &ClickLabel::leftClicked, this, [=]{
        showUserInfo(msg.senderId);
    });
}

/// 设置第一个群聊消息
void NotificationCard::setGroupMsg(const MsgBean &msg)
{
    Q_ASSERT(us->isGroupShow(msg.groupId));

    // 设置标题
    ui->nicknameLabel->setText(msg.groupName);

    // 设置头像
    // 群头像API：http://p.qlogo.cn/gh/群号/群号/100
    QPixmap headerPixmap;
    if (isFileExist(rt->groupHeader(msg.groupId)))
    {
        headerPixmap = QPixmap(rt->groupHeader(msg.groupId));
    }
    else // 没有头像，联网获取
    {
        QString url = "http://p.qlogo.cn/gh/" + snum(msg.groupId) + "/" + snum(msg.groupId) + "/100";
        headerPixmap = NetImageUtil::loadNetPixmap(url);
        headerPixmap.save(rt->groupHeader(msg.groupId));
    }

    if (!us->bannerUseHeaderGradient)
    {
        ui->headerLabel->setPixmap(NetImageUtil::toRoundedPixmap(headerPixmap));
    }
    else
    {
        QString path = rt->groupHeader("ctg_" + snum(msg.groupId));
        if (!isFileExist(path))
            NetImageUtil::toCornelTransparentGradient(headerPixmap.scaledToWidth(us->bannerHeaderSize*3), us->bannerBgRadius).save(path);
        bg->setIcon(QIcon(path));
        bg->setAlign(Qt::AlignTop | Qt::AlignRight);
        bg->setFixedForePos(true);
        bg->setFixedForeSize(true);
        bg->setPaddings(0);
        ui->verticalSpacer->changeSize(1, ui->headerLabel->height());
        ui->headerLabel->hide();
    }

    // 设置背景颜色
    if (us->bannerUseHeaderColor)
    {
        if (ac->groupHeaderColor.contains(msg.groupId))
            cardColor = ac->groupHeaderColor.value(msg.groupId);
        else
            ImageUtil::getBgFgColor(ImageUtil::extractImageThemeColors(headerPixmap.toImage(), 8), &cardColor.bg, &cardColor.fg);
        setColors(cardColor.bg, cardColor.fg);
    }

    // 列表是需要移动到外面来的（头像单独一列）
    ui->verticalLayout->removeWidget(ui->listWidget);
    ui->verticalLayout_2->insertWidget(1, ui->listWidget);

    // 添加消息组
    createMsgBox(msg);

    connect(sig, &SignalTransfer::groupMembersLoaded, this, [=](qint64 groupId) {
        if (groupId != this->groupId)
            return ;

        Q_ASSERT(ac->groupMemberNames.contains(groupId));
        QHash<qint64, QString> *groupMembers = &ac->groupMemberNames[groupId];
        foreach (auto view, msgViews)
        {
            view->setGroupMembers(groupMembers);
            view->replaceGroupAt();
        }
    });

    connect(ui->headerLabel, &ClickLabel::leftClicked, this, [=]{
        showGrougInfo(msg.groupId);
    });
    connect(ui->nicknameLabel, &ClickLabel::leftClicked, this, [=]{
        showGrougInfo(msg.groupId);
    });
}

/// 添加一个私聊消息
void NotificationCard::appendPrivateMsg(const MsgBean &msg)
{
    createMsgEdit(msg);
}

/// 添加一个群组消息，每条都有可能是独立的头像、昵称（二级标题）
void NotificationCard::appendGroupMsg(const MsgBean &msg)
{
    if (!msgs.size() || msgs.last().senderId != msg.senderId)
        createMsgBox(msg);
    else
        createMsgBoxEdit(msg);
}

/// 一个卡片只显示一个人的消息的情况
void NotificationCard::addSingleSenderMsg(const MsgBean &msg)
{
    createMsgEdit(msg);
}

/// 仅创建消息正文
void NotificationCard::createMsgEdit(const MsgBean& msg, int index)
{
    // 先暂停时钟（获取图片有延迟）
    int remain = -1;
    if (displayTimer->isActive())
    {
        remain = displayTimer->remainingTime();
        displayTimer->stop();
    }

    auto scrollbar = ui->listWidget->verticalScrollBar();
    bool ending = (scrollbar->sliderPosition() >= scrollbar->maximum() || ui->listWidget->isToBottoming());

    MessageView* msgView = newMsgView();
    msgView->setMessage(msg);
    msgView->setTextColor(cardColor.fg);

    msgView->setAttribute(Qt::WA_TransparentForMouseEvents, false);

    QListWidgetItem* item;
    if (index < 0)
    {
        item = new QListWidgetItem(ui->listWidget);
    }
    else
    {
        item = new QListWidgetItem;
        ui->listWidget->insertItem(index, item);
    }
    ui->listWidget->setItemWidget(item, msgView);

    msgView->adjustSizeByTextWidth(us->bannerContentWidth);
    item->setSizeHint(msgView->size());

    int sumHeight = ui->listWidget->spacing();
    for (int i = 0; i < ui->listWidget->count(); i++)
    {
        auto widget = ui->listWidget->itemWidget(ui->listWidget->item(i));
        sumHeight += widget->height() + ui->listWidget->spacing() * 2;
    }
    ui->listWidget->setFixedHeight(qMin(sumHeight, us->bannerContentHeight));
    this->adjustSize();

    // 滚动
    if (index == -1 && ending)
    {
        ui->listWidget->scrollToBottom();
    }

    // 回复时钟
    if (remain >= 0)
    {
        displayTimer->setInterval(remain);
        displayTimer->start();
        // 显示出来后会自动增加新message需要的时间，所以只要恢复就行了
    }
}

/// 创建完整的头像、昵称、消息内容
void NotificationCard::createMsgBox(const MsgBean &msg, int index)
{
    // 先暂停时钟（获取图片有延迟）
    int remain = -1;
    if (displayTimer->isActive())
    {
        remain = displayTimer->remainingTime();
        displayTimer->stop();
    }

    auto scrollbar = ui->listWidget->verticalScrollBar();
    bool ending = (scrollbar->sliderPosition() >= scrollbar->maximum() || ui->listWidget->isToBottoming());

    // 创建控件
    QWidget* box = new QWidget(this);
    QLabel* headerLabel = new QLabel(box);
    QLabel* nameLabel = new QLabel(msg.groupName, box);
    MessageView* msgView = newMsgView();
    QWidget* spacer = new QWidget(this);
    QVBoxLayout* headerVlayout = new QVBoxLayout;
    QVBoxLayout* contentVlayout = new QVBoxLayout;
    QHBoxLayout* mainHlayout = new QHBoxLayout(box);

    headerVlayout->addWidget(headerLabel);
    headerVlayout->addWidget(spacer);
    headerVlayout->setStretch(0, 0);
    headerVlayout->setStretch(1, 1);
    contentVlayout->addWidget(nameLabel);
    contentVlayout->addWidget(msgView);
    contentVlayout->setStretch(0, 0);
    contentVlayout->setStretch(1, 1);
    mainHlayout->addLayout(headerVlayout);
    mainHlayout->addLayout(contentVlayout);
    mainHlayout->setStretch(0, 0);
    mainHlayout->setStretch(1, 1);
    mainHlayout->setAlignment(Qt::AlignLeft);
    mainHlayout->setMargin(0);

    spacer->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    headerLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    nameLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    msgView->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    spacer->setFixedWidth(1);
    headerLabel->setFixedSize(ui->headerLabel->size());
    nameLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);

    QFont font;
    font.setPointSize(font.pointSize() + us->bannerSubTitleLarger);
    nameLabel->setFont(font);

    // 设置颜色
    QPalette pa(ui->nicknameLabel->palette());
    pa.setColor(QPalette::Foreground, cardColor.fg);
    pa.setColor(QPalette::Text, cardColor.fg);
    msgView->setPalette(pa);

    // 设置昵称
    nameLabel->setText(msg.displayNickname());

    // 设置头像
    // 用户头像API：http://q1.qlogo.cn/g?b=qq&nk=QQ号&s=100&t=
    if (isFileExist(rt->userHeader(msg.senderId)))
    {
        headerLabel->setPixmap(NetImageUtil::toRoundedPixmap(QPixmap(rt->userHeader(msg.senderId)).scaled(headerLabel->size())));
    }
    else // 没有头像，联网获取
    {
        QString url = "http://q1.qlogo.cn/g?b=qq&nk=" + snum(msg.senderId) + "&s=100&t=";
        QPixmap pixmap = NetImageUtil::loadNetPixmap(url);
        if (!us->bannerUseHeaderColor)
            pixmap = NetImageUtil::toRoundedPixmap(pixmap);
        pixmap.save(rt->userHeader(msg.senderId));
        headerLabel->setPixmap(NetImageUtil::toRoundedPixmap(pixmap.scaled(headerLabel->size())));
    }

    // 彩色用户昵称
    if (us->bannerColorfulGroupMember)
    {
        auto getGroupMemberColor = [=](qint64 groupId, qint64 senderId) -> QColor {
            if (ac->groupMemberColor.contains(groupId))
            {
                if (ac->groupMemberColor.value(groupId).contains(senderId))
                {
                    return ac->groupMemberColor.value(groupId).value(senderId);
                }
            }
            else // 连群组都没有
            {
                ac->groupMemberColor.insert(groupId, QHash<qint64, QColor>());
            }

            AccountInfo::CardColor cc;
            auto colors = ImageUtil::extractImageThemeColors(headerLabel->pixmap()->toImage(), 4);
            auto color = ImageUtil::getFastestColor(us->bannerUseHeaderColor ? cardColor.bg : us->bannerBgColor, colors, 2); // 获取色差最大的
            // auto color = colors.first().toColor();
            ac->groupMemberColor[groupId].insert(senderId, color);
            return color;
        };

        QColor c = getGroupMemberColor(msg.groupId, msg.senderId);
        pa.setColor(QPalette::Text, c);
        nameLabel->setStyleSheet("color: " + QVariant(c).toString() + ";");
    }
    nameLabel->setPalette(pa);

    // 设置消息
    msgView->setMessage(msg);
    msgView->adjustSizeByTextWidth(us->bannerContentWidth); // 这里有个-12的，为什么呢
    msgView->setTextColor(cardColor.fg);
    box->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    box->adjustSize();

    // 设置列表项
    QListWidgetItem* item;
    if (index < 0)
    {
        item = new QListWidgetItem(ui->listWidget);
    }
    else
    {
        item = new QListWidgetItem;
        ui->listWidget->insertItem(index, item);
    }
    ui->listWidget->setItemWidget(item, box);
    item->setSizeHint(box->size());

    int sumHeight = 0;
    for (int i = 0; i < ui->listWidget->count(); i++)
    {
        auto widget = ui->listWidget->itemWidget(ui->listWidget->item(i));
        sumHeight += widget->height() + ui->listWidget->spacing() * 2; // us->bannerMessageSpacing;
    }
    ui->listWidget->setFixedHeight(qMin(sumHeight, us->bannerContentHeight));
    this->adjustSize();

    // 滚动
    if (index == -1 && ending)
        ui->listWidget->scrollToBottom();

    // 回复时钟
    if (remain >= 0)
    {
        displayTimer->setInterval(remain);
        displayTimer->start();
        // 显示出来后会自动增加新message需要的时间，所以只要恢复就行了
    }
}

/// 仅显示编辑框，不显示头像
/// 但是头像的占位还在的
void NotificationCard::createMsgBoxEdit(const MsgBean &msg, int index)
{
    // 先暂停时钟（获取图片有延迟）
    int remain = -1;
    if (displayTimer->isActive())
    {
        remain = displayTimer->remainingTime();
        displayTimer->stop();
    }

    auto scrollbar = ui->listWidget->verticalScrollBar();
    bool ending = (scrollbar->sliderPosition() >= scrollbar->maximum() || ui->listWidget->isToBottoming());

    QWidget* box = new QWidget(this);
    QLabel* headerLabel = new QLabel(box);
    MessageView* msgView = newMsgView();
    QHBoxLayout* mainHlayout = new QHBoxLayout(box);
    headerLabel->setFixedSize(ui->headerLabel->width(), 1);
    headerLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    mainHlayout->addWidget(headerLabel);
    mainHlayout->addWidget(msgView);
    mainHlayout->setStretch(0, 0);
    mainHlayout->setStretch(1, 1);
    mainHlayout->setAlignment(Qt::AlignLeft);
    mainHlayout->setMargin(0);

    msgView->setAttribute(Qt::WA_TransparentForMouseEvents, false);

    // 设置颜色
    QPalette pa(ui->nicknameLabel->palette());
    pa.setColor(QPalette::Foreground, cardColor.fg);
    pa.setColor(QPalette::Text, cardColor.fg);
    msgView->setPalette(pa);

    // 设置消息
    msgView->setMessage(msg);
    msgView->setTextColor(cardColor.fg);
    msgView->adjustSizeByTextWidth(us->bannerContentWidth); // 这里有个-12的，为什么呢
    box->adjustSize();

    // 设置列表项
    QListWidgetItem* item;
    if (index < 0)
    {
        item = new QListWidgetItem(ui->listWidget);
    }
    else
    {
        item = new QListWidgetItem;
        ui->listWidget->insertItem(index, item);
    }
    ui->listWidget->setItemWidget(item, box);
    item->setSizeHint(box->size());

    int sumHeight = 0;
    for (int i = 0; i < ui->listWidget->count(); i++)
    {
        auto widget = ui->listWidget->itemWidget(ui->listWidget->item(i));
        sumHeight += widget->height() + ui->listWidget->spacing() * 2; // us->bannerMessageSpacing;
    }
    ui->listWidget->setFixedHeight(qMin(sumHeight, us->bannerContentHeight));
    this->adjustSize();

    // 滚动
    if (index == -1 && ending)
        ui->listWidget->scrollToBottom();

    // 回复时钟
    if (remain >= 0)
    {
        displayTimer->setInterval(remain);
        displayTimer->start();
        // 显示出来后会自动增加新message需要的时间，所以只要恢复就行了
    }
}

MessageView *NotificationCard::newMsgView()
{
    MessageView* view = new MessageView(this);
    msgViews.append(view);

    if (groupId)
    {
        if (ac->groupMemberNames.contains(groupId))
            view->setGroupMembers(&ac->groupMemberNames[groupId]);

        connect(view, &MessageView::needMemberNames, this, [=]{
            if (ac->groupMemberNames.contains(groupId))
            {
                auto& list = ac->groupMemberNames[groupId];
                if (!list.size()) // 这里标记是空列表表示正在获取中
                    return ;

                list.clear(); // 有列表，但是还是需要，就清空，重新获取
            }

            // 没有获取中，或者是旧的列表（已清空）
            emit sig->loadGroupMembers(groupId);
        });
    }

    return view;
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

void NotificationCard::setFastFocus()
{
    this->fastFocus = true;
}

const QList<MsgBean> &NotificationCard::getMsgs() const
{
    return msgs;
}

void NotificationCard::mouseEnter()
{
    focusIn();

    if (us->bannerAutoFocusReply)
    {
        if (ui->messageEdit->isHidden())
        {
            fastFocus = true;
            showReplyEdit(true);
        }
    }
}

void NotificationCard::mouseLeave()
{
    if (fastFocus && us->bannerAutoFocusReply)
    {
        fastFocus = false;
        emit signalCancelReply();
    }
    else if (ui->messageEdit->isVisible() && ui->messageEdit->hasFocus())
    {
        return ;
    }
    focusOut();
}

void NotificationCard::displayTimeout()
{
    if (bg->isInArea(bg->mapFromGlobal(QCursor::pos())))
    {
        displayTimer->setInterval(us->bannerDisplayDuration);
        return ; // 会等待下一波的timeout
    }
    toHide();
}

/// 鼠标移入、快捷键回复，都会导致 focusIn
void NotificationCard::focusIn()
{
    displayTimer->stop();
}

void NotificationCard::focusOut()
{
    displayTimer->setInterval(us->bannerRetentionDuration);
    displayTimer->start();
}

void NotificationCard::showReplyEdit()
{
    if (ui->messageEdit->isHidden()) // 显示消息框
    {
        showReplyEdit(true);
    }
    else // 发送内容
    {
        sendReply();
    }
}

void NotificationCard::showReplyEdit(bool focus)
{
    ui->replyButton->setText("发送");
    ui->messageEdit->show();
    if (focus)
    {
        this->activateWindow(); // 只有活动中的窗口，setFocus 才有效
        ui->messageEdit->setFocus();
    }
    ui->replyHLayout->removeItem(ui->horizontalSpacer);
    displayTimer->stop();
}

void NotificationCard::hideReplyEdit()
{
    ui->messageEdit->hide(); // 会触发 FocusOut 事件
    ui->replyHLayout->insertItem(0, ui->horizontalSpacer);
    ui->replyButton->setText("回复");
}

void NotificationCard::showGrougInfo(qint64 groupId, QPoint pos)
{

}

void NotificationCard::showUserInfo(qint64 userId, QPoint pos)
{

}

void NotificationCard::sendReply()
{
    QString text = ui->messageEdit->text();
    if (text.isEmpty())
        return ;
    int h = this->height();

    // 回复消息
    if (!groupId && senderId)
        emit signalReplyPrivate(senderId, text);
    else if (groupId)
        emit signalReplyGroup(groupId, text);
    else
    {
        qCritical() << "回复失败，无法获取到 userId 或者 groupId";
        return ;
    }

    // 加到消息框中
    MsgBean msg(ac->myId, ac->myNickname, "你: " + text, 0, "");
    if (isGroup())
        createMsgBoxEdit(msg);
    else
        appendPrivateMsg(msg);

    // 清空输入框
    ui->messageEdit->clear();

    this->layout()->activate();
    this->resize(this->sizeHint());
    int hDelta = this->height() - h;
    if (hDelta)
        emit signalHeightChanged(hDelta);
}

/**
 * 从某一个点开始出现，直到完全展示出来
 */
void NotificationCard::showFrom(QPoint hi, QPoint sh)
{
    this->showPoint = sh;
    this->hidePoint = hi;
    move(hi);
    hidding = false;

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
    ui->nicknameLabel->setPalette(pa); // 不知道为什么没有用
    ui->listWidget->setPalette(pa);
    ui->messageEdit->setPalette(pa);
    ui->replyButton->setPalette(pa);
    ui->replyButton->setTextColor(fg);

    QString qss = "QLabel { color: " + QVariant(cardColor.fg).toString() + "; }";
    ui->nicknameLabel->setStyleSheet(qss);

    // qss = "QLineEdit { color: " + QVariant(cardColor.fg).toString() + "; background: transparent; border: 1px solid" + QVariant(cardColor.fg).toString() + "; border-radius: " + snum(us->bannerBgRadius) + "px; padding-left:2px; padding-right: 2px;}";
    qss = "QLineEdit { color: " + QVariant(cardColor.fg).toString() + "; background: transparent; }";
    ui->messageEdit->setStyleSheet(qss);
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

void NotificationCard::cardMenu()
{
    FacileMenu* menu = new FacileMenu(this);
    menu->addAction(QIcon(), "立即关闭", [=]{
        this->toHide();
    });
    menu->addAction(QIcon(), "消息历史", [=]{

    })->disable();
    menu->split()->addAction(QIcon(), "不显示该群通知", [=]{
        us->enabledGroups.removeOne(groupId);
        us->set("group/enables", us->enabledGroups);
        this->toHide();
        qInfo() << "不显示群组通知：" << groupId;
    })->hide(!groupId);
    menu->exec();
}

int NotificationCard::getReadDisplayDuration(QString text) const
{
    text.replace(QRegularExpression("<.+?>"), "").replace(QRegularExpression("\\[CQ:.+?\\]"), "");
    int length = text.length();
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

/// 加载消息记录
void NotificationCard::loadMsgHistory()
{
    QList<MsgBean> *histories;
    int h = this->height();

    Q_ASSERT(msgs.size());
    if (!groupId) // 加载私聊消息
    {
        if (!ac->userMsgHistory.contains(senderId))
            return ;
        histories = &ac->userMsgHistory[senderId];
    }
    else // 加载群组消息
    {
        if (!ac->groupMsgHistory.contains(groupId))
            return ;
        histories = &ac->groupMsgHistory[groupId];
    }

    auto firstMsg = msgs.first();

    // 获取当前消息的前一个位置（最后一条历史）
    int historyStart = histories->size()-1;
    qint64 timestamp = firstMsg.timestamp;
    while (--historyStart >= 0 && histories->at(historyStart).timestamp >= timestamp);
    int historyEnd = qMin(historyStart + 1, histories->size()); // 当前索引
    if (historyEnd <= 0) // 没有历史消息
        return ;

    // 群组的话，根据同一个人的连续消息，稍微向上延伸，最多一倍
    if (groupId && historyStart > 0)
    {
        int count = us->bannerMessageLoadCount;
        qint64 senderId = histories->at(historyStart).senderId;
        while (historyStart > 0 && count-- && histories->at(historyStart-1).senderId == senderId)
            historyStart--;
    }

    // 加载消息：index -> historyEnd-1
    auto bar = ui->listWidget->verticalScrollBar();
    int disBottom = bar->maximum() - bar->sliderPosition();
    qint64 senderId = -1;
    for (int i = historyStart; i < historyEnd; i++)
    {
        int index = i - historyStart;
        auto msg = histories->at(i);
        if (isPrivate())
        {
            createMsgEdit(msg, index);
        }
        else
        {
            if (msg.senderId != senderId)
            {
                createMsgBox(msg, index);
            }
            else
            {
                createMsgBoxEdit(msg, index);
            }
        }
        senderId = msg.senderId;
        msgs.insert(index, msg);
    }
    bar->setSliderPosition(bar->maximum() - disBottom);

    this->layout()->activate();
    this->resize(this->sizeHint());
    int hDelta = this->height() - h;
    if (hDelta)
        emit signalHeightChanged(hDelta);
}
