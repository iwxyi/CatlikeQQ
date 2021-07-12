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
#include "netutil.h"
#include "myjson.h"

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

    ui->headerLabel->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    ui->nicknameLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->listWidget->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->replyButton->setRadius(us->bannerBgRadius);
    ui->messageEdit->hide();

    QFont font;
    font.setPointSize(font.pointSize() + us->bannerTitleLarger);
    ui->nicknameLabel->setFont(font);
    cardColor = AccountInfo::CardColor{us->bannerBgColor, us->bannerTitleColor};

    ui->listWidget->setFixedHeight(0);
    ui->listWidget->setMinimumHeight(0);
    ui->listWidget->setMaximumHeight(us->bannerContentMaxHeight);
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
    this->userId = msg.friendId;
    this->groupId = msg.groupId;
    msgs.append(msg);
    int h = height();

    if (msg.isPrivate())
    {
        setPrivateMsg(msg);
        connectUserHeader(ui->headerLabel);
    }
    else
    {
        setGroupMsg(msg);
        connectGroupHeader(ui->headerLabel);
    }

    // 设置大小
    setFixedWidth(us->bannerFixedWidth);
    this->layout()->activate();
    resize(this->sizeHint());

    // 调整显示的时长
    if (us->bannerTextReadSpeed)
    {
        displayTimer->setInterval(getReadDisplayDuration(msg.displayMessage()));
    }

    // 自动展开
    if (us->bannerAutoShowReply)
    {
        showReplyEdit(false);
    }

    this->layout()->activate();
    this->resize(this->sizeHint());
    int hDelta = this->height() - h;
    if (hDelta)
        emit signalHeightChanged(hDelta);
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
    if (this->isPrivate() && this->userId != msg.senderId && this->userId != msg.targetId)
        return false;

    int h = height();

    if (msg.isPrivate())
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
        if (msg.senderId != ac->myId
                && ((msg.isPrivate() && us->bannerPrivateKeepShowing)
                    || (msg.isGroup() && us->bannerGroupKeepShowing))) // 收到消息一直显示，除非自己回复
        {
            displayTimer->stop();
        }
        else // 收到消息，调整显示时长
        {
            if (msg.senderId != ac->myId)
            {
                displayTimer->setInterval(qMax(0, displayTimer->remainingTime() - us->bannerDisplayDuration) + getReadDisplayDuration(msg.displayMessage()));
                displayTimer->start();
            }
        }
    }
    else if (!fixing && msg.senderId == ac->myId) // 自己回复了，该隐藏了
    {
        if (!bg->isInArea(bg->mapFromGlobal(QCursor::pos())) && !ui->messageEdit->hasFocus())
        {
            displayTimer->setInterval(us->bannerRetentionDuration);
            displayTimer->start();
        }
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
    if (msg.senderId == ac->myId) // 自己的消息，需要手动更新昵称
        ui->nicknameLabel->setText(ac->friendList.contains(msg.friendId) ? ac->friendList.value(msg.targetId).username() : "[未备注]");
    else // 直接设置名字
        ui->nicknameLabel->setText(msg.displayNickname());

    // 设置头像
    // 用户头像API：http://q1.qlogo.cn/g?b=qq&nk=QQ号&s=100&t=
    QPixmap headerPixmap;
    if (isFileExist(rt->userHeader(this->userId)))
    {
        headerPixmap = QPixmap(rt->userHeader(this->userId));
    }
    else // 没有头像，联网获取
    {
        QString url = "http://q1.qlogo.cn/g?b=qq&nk=" + snum(this->userId) + "&s=100&t=";
        headerPixmap = NetImageUtil::loadNetPixmap(url);
        headerPixmap.save(rt->userHeader(this->userId));
    }

    if (!us->bannerUseHeaderGradient)
    {
        if (!headerPixmap.isNull())
            ui->headerLabel->setPixmap(NetImageUtil::toRoundedPixmap(headerPixmap));
    }
    else
    {
        QString path = rt->userHeader("ctg_" + snum(this->userId));
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
        if (ac->userHeaderColor.contains(this->userId))
            cardColor = ac->userHeaderColor.value(this->userId);
        else
            ImageUtil::getBgFgColor(ImageUtil::extractImageThemeColors(headerPixmap.toImage(), 8), &cardColor.bg, &cardColor.fg);
    }
    setColors(cardColor.bg, cardColor.fg, cardColor.fg);

    // 添加消息
    createPureMsgView(msg);

    connect(ui->headerLabel, &ClickLabel::leftClicked, this, [=]{
        showUserInfo(this->userId);
    });
    connect(ui->nicknameLabel, &ClickLabel::leftClicked, this, [=]{
        showUserInfo(this->userId);
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
        if (!headerPixmap.isNull())
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
    }
    setColors(cardColor.bg, cardColor.fg,cardColor.fg);

    // 列表是需要移动到外面来的（头像单独一列）
    ui->verticalLayout->removeWidget(ui->listWidget);
    ui->verticalLayout_2->insertWidget(1, ui->listWidget);

    // 添加消息组
    createMsgBox(msg);

    connect(sig, &SignalTransfer::groupMembersLoaded, this, [=](qint64 groupId) {
        if (groupId != this->groupId)
            return ;

        Q_ASSERT(ac->groupMemberNames.contains(groupId));
        foreach (auto view, msgViews)
        {
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
    if (displayTimer->isActive() && us->bannerPrivateKeepShowing && msg.senderId != ac->myId)
        displayTimer->stop();
    if (msg.targetId == this->userId) // 自己发给对面的
    {
        MsgBean m = msg;
        m.message.insert(0, "你:");
        createPureMsgView(m);
    }
    else
    {
        createPureMsgView(msg); // 对面发过来的
    }
}

/// 添加一个群组消息，每条都有可能是独立的头像、昵称（二级标题）
void NotificationCard::appendGroupMsg(const MsgBean &msg)
{
    if (displayTimer->isActive() && us->bannerGroupKeepShowing && msg.senderId != ac->myId)
        displayTimer->stop();
    if (!msgs.size() || msgs.last().senderId != msg.senderId)
        createMsgBox(msg);
    else
        createBlankMsgBox(msg);
}

/// 一个卡片只显示一个人的消息的情况
void NotificationCard::addSingleSenderMsg(const MsgBean &msg)
{
    createPureMsgView(msg);
}

/// 仅创建消息正文
void NotificationCard::createPureMsgView(const MsgBean& msg, int index)
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
    ui->listWidget->setFixedHeight(qMin(sumHeight, us->bannerContentMaxHeight));
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
    QLabel* nameLabel = new QLabel(msg.groupName.isEmpty() ? msg.nickname : msg.groupName, box);
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

    connectUserHeader(headerLabel);
    spacer->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    headerLabel->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    nameLabel->setAttribute(Qt::WA_TransparentForMouseEvents, false);
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
    bool headerValid = false;
    if (isFileExist(rt->userHeader(msg.senderId)))
    {
        headerLabel->setPixmap(NetImageUtil::toRoundedPixmap(QPixmap(rt->userHeader(msg.senderId)).scaled(headerLabel->size())));
        headerValid = true;
    }
    else // 没有头像，联网获取
    {
        QString url = "http://q1.qlogo.cn/g?b=qq&nk=" + snum(msg.senderId) + "&s=100&t=";
        QPixmap pixmap = NetImageUtil::loadNetPixmap(url);
        if (!us->bannerUseHeaderColor)
            pixmap = NetImageUtil::toRoundedPixmap(pixmap);
        pixmap.save(rt->userHeader(msg.senderId));
        if (!pixmap.isNull())
        {
            headerLabel->setPixmap(NetImageUtil::toRoundedPixmap(pixmap.scaled(headerLabel->size())));
            headerValid = true;
        }
    }

    // 彩色用户昵称
    if (us->bannerColorfulGroupMember && headerValid)
    {
        auto getGroupMemberColor = [=](qint64 groupId, qint64 userId) -> QColor {
            if (ac->groupMemberColor.contains(groupId))
            {
                if (ac->groupMemberColor.value(groupId).contains(userId))
                {
                    return ac->groupMemberColor.value(groupId).value(userId);
                }
            }
            else // 连群组都没有
            {
                ac->groupMemberColor.insert(groupId, QHash<qint64, QColor>());
            }

            AccountInfo::CardColor cc;
            auto colors = ImageUtil::extractImageThemeColors(headerLabel->pixmap()->toImage(), 4);
            auto color = ImageUtil::getFastestColor(us->bannerUseHeaderColor ? cardColor.bg : us->bannerBgColor, colors, 1); // 获取色差最大的
            // auto color = colors.first().toColor();
            ac->groupMemberColor[groupId].insert(this->userId, color);
            return color;
        };

        QColor c = getGroupMemberColor(msg.groupId, msg.senderId);
        pa.setColor(QPalette::Text, c);
        nameLabel->setStyleSheet("QLabel { color: " + QVariant(c).toString() + "; }");
    }
    else
    {
        nameLabel->setStyleSheet("QLabel { color: " + QVariant(cardColor.fg).toString() + "; }");
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
    ui->listWidget->setFixedHeight(qMin(sumHeight, us->bannerContentMaxHeight));
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
void NotificationCard::createBlankMsgBox(const MsgBean &msg, int index)
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
    ui->listWidget->setFixedHeight(qMin(sumHeight, us->bannerContentMaxHeight));
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

    connect(view, &MessageView::keepShowing, this, [=]{
        suspendHide();
    });

    connect(view, &MessageView::restoreTimerIfNecessary, this, [=]{
        shallToHide();
    });

    return view;
}

/// 连接群组头像事件
void NotificationCard::connectGroupHeader(QLabel *label)
{
    label->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(label, &QLabel::customContextMenuRequested, this, [=](const QPoint&){
        suspendHide();
        FacileMenu* menu = new FacileMenu(this);

        menu->addAction("群组信息", [=]{

        })->disable();

        menu->addAction("群成员", [=]{

        })->disable();

        menu->exec();
        menu->finished([=]{
            shallToHide();
        });
    });
}

/// 连接用户头像事件
/// 要区分私聊的主图像、群聊的次头像
void NotificationCard::connectUserHeader(QLabel* label)
{
    label->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(label, &QLabel::customContextMenuRequested, this, [=](const QPoint&){
        suspendHide();
        FacileMenu* menu = new FacileMenu(this);

        menu->addAction("查看资料", [=]{

        })->disable();

        menu->split()->addAction("发送消息", [=]{

        })->disable();

        menu->addAction("@ TA", [=]{

        })->disable();

        menu->split()->addAction("特别关注", [=]{

        })->disable();

        menu->addAction("屏蔽此人", [=]{
            // 本地屏蔽
        })->disable();

        menu->exec();
        menu->finished([=]{
            shallToHide();
        });
    });
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
    return !hidding;
}

bool NotificationCard::isFixing() const
{
    return fixing;
}

void NotificationCard::setFastFocus()
{
    this->fastFocus = true;
}

const QList<MsgBean> &NotificationCard::getMsgs() const
{
    return msgs;
}

int NotificationCard::getImportance() const
{
    if (!msgs.size())
        return NormalImportant;
    auto msg = msgs.last();
    if (isPrivate())
        return us->userImportance.value(msg.senderId, us->userDefaultImportance);
    if (isGroup())
        return us->groupImportance.value(msg.groupId, us->groupDefaultImportance);
    return NormalImportant;
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
    if (fixing)
        return ;
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
        if (!ui->messageEdit->text().isEmpty())
            ui->messageEdit->selectAll();
    }
    ui->replyHLayout->removeItem(ui->horizontalSpacer);
    displayTimer->stop();

    if (us->bannerAIReply && ui->messageEdit->text().isEmpty())
        triggerAIReply();
}

void NotificationCard::hideReplyEdit()
{
    ui->messageEdit->hide(); // 会触发 FocusOut 事件
    ui->replyHLayout->insertItem(0, ui->horizontalSpacer);
    ui->replyButton->setText("回复");
}

void NotificationCard::showGrougInfo(qint64 groupId, QPoint pos)
{
    Q_UNUSED(groupId)
    Q_UNUSED(pos)
}

void NotificationCard::showUserInfo(qint64 userId, QPoint pos)
{
    Q_UNUSED(userId)
    Q_UNUSED(pos)
}

void NotificationCard::sendReply()
{
    QString text = ui->messageEdit->text();
    if (text.isEmpty())
        return ;
    int h = this->height();

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
    if (us->bannerShowMySend) // 不显示自己发送的消息
    {
        MsgBean msg(ac->myId, ac->myNickname, "你: " + text, 0, "");
        if (isGroup())
            createBlankMsgBox(msg);
        else
            appendPrivateMsg(msg);
    }

    // 清空输入框
    ui->messageEdit->clear();

    this->layout()->activate();
    this->resize(this->sizeHint());
    int hDelta = this->height() - h;
    if (hDelta)
        emit signalHeightChanged(hDelta);
}

/**
 * 预先设置坐标
 * 展示前可能会出现改变
 */
void NotificationCard::setShowPos(QPoint startPos, QPoint showPos)
{
    this->hidePoint = startPos;
    this->showPoint = showPos;
    move(startPos);
}

/**
 * 从某一个点开始出现，直到完全展示出来
 */
void NotificationCard::showFrom()
{
    createFrostGlass();
    hidding = false;

    QPropertyAnimation* ani = new QPropertyAnimation(this, "pos");
    ani->setStartValue(this->hidePoint);
    ani->setEndValue(this->showPoint);
    ani->setEasingCurve(QEasingCurve::Type(us->bannerShowEasingCurve));
    ani->setDuration(us->bannerAnimationDuration);
    connect(ani, SIGNAL(finished()), ani, SLOT(deleteLater()));
    ani->start();
    show();
}

void NotificationCard::setColors(QColor bg, QColor title, QColor content)
{
    this->bg->setBgColor(bg);
    QPalette pa(ui->nicknameLabel->palette());
    pa.setColor(QPalette::Foreground, title);
    pa.setColor(QPalette::Text, title);
    ui->nicknameLabel->setPalette(pa); // 不知道为什么没有用

    pa.setColor(QPalette::Foreground, content);
    pa.setColor(QPalette::Text, content);
    ui->listWidget->setPalette(pa);
    ui->messageEdit->setPalette(pa);
    ui->replyButton->setPalette(pa);
    ui->replyButton->setTextColor(content);

    QString qss = "QLabel { color: " + QVariant(cardColor.fg).toString() + "; }";
    ui->nicknameLabel->setStyleSheet(qss);

    // qss = "QLineEdit { color: " + QVariant(cardColor.fg).toString() + "; background: transparent; border: 1px solid" + QVariant(cardColor.fg).toString() + "; border-radius: " + snum(us->bannerBgRadius) + "px; padding-left:2px; padding-right: 2px;}";
    qss = "QLineEdit { color: " + QVariant(cardColor.fg).toString() + "; background: transparent; }";
    ui->messageEdit->setStyleSheet(qss);

    // 再绘制一层淡淡的毛玻璃效果
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

void NotificationCard::triggerAIReply(int retry)
{
    auto msg = msgs.last();
    QString text = getValiableMessage(msg.message);
    if (text.isEmpty())
        return ;

    // 参数信息
    QString url = "https://api.ai.qq.com/fcgi-bin/nlp/nlp_textchat";
    QString nonce_str = "replyAPPKEY";
    QStringList params{"app_id", "2159207490",
                       "nonce_str", nonce_str,
                "question", text,
                "session", snum(msg.senderId) + snum(retry),
                "time_stamp", QString::number(QDateTime::currentSecsSinceEpoch()),
                      };

    // 接口鉴权
    QString pinjie;
    for (int i = 0; i < params.size()-1; i+=2)
        if (!params.at(i+1).isEmpty())
            pinjie += params.at(i) + "=" + QUrl::toPercentEncoding(params.at(i+1)) + "&";
    QString appkey = "sTuC8iS3R9yLNbL9";
    pinjie += "app_key="+appkey;

    QString sign = QString(QCryptographicHash::hash(pinjie.toLocal8Bit(), QCryptographicHash::Md5).toHex().data()).toUpper();
    params << "sign" << sign;
//    qInfo() << pinjie << sign;

    // 获取信息
    connect(new NetUtil(url, params), &NetUtil::finished, this, [=](QString result){
        // 已经有字打上去了
        if (!ui->messageEdit->text().isEmpty())
            return ;

        MyJson json(result.toUtf8());
        if (json.value("ret").toInt() != 0)
        {
            QString msg = json.value("msg").toString();
            qWarning() << "AI回复：" << msg << text;
            if (msg == "chat answer not found" && retry < 3)
                triggerAIReply(retry + 1);
            return ;
        }

        QString answer = json.value("data").toObject().value("answer").toString();

        // 过滤文字
        if (answer.contains("未搜到")
                || answer.isEmpty())
            return ;

        ui->messageEdit->setText(answer);
        ui->messageEdit->selectAll();
    });
}

/// 看情况是否需要隐藏
/// 前面有暂停时钟，但是不一定需要隐藏
void NotificationCard::shallToHide()
{
    if (!displayTimer->isActive() && !fixing
            && !bg->isInArea(bg->mapFromGlobal(QCursor::pos()))
            && !ui->messageEdit->hasFocus())
        displayTimer->start();
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
    menu->addAction(QIcon("://icons/close.png"), "立即关闭", [=]{
        this->toHide();
    });
    menu->addAction(QIcon("://icons/fixed.png"), "固定卡片", [=]{
        fixing = !fixing;
        if (fixing)
            displayTimer->stop();
        else
            if (bg->isInArea(bg->mapFromGlobal(QCursor::pos())))
                displayTimer->start();
    })->text(fixing, "取消固定");


    auto importanceMenu = menu->addMenu(QIcon("://icons/importance.png"), "消息重要性");
    importanceMenu->setDisabled(!isPrivate() && !isGroup());
    auto setImportance = [=](int im){
        auto msg = msgs.last();
        if (isPrivate())
        {
            if (im == us->userDefaultImportance)
                us->userImportance.remove(msg.senderId);
            else
                us->userImportance[msg.senderId] = im;
            us->set("importance/userImportance", us->userImportance);
        }
        else if (isGroup())
        {
            if (im == us->groupDefaultImportance)
                us->groupImportance.remove(msg.groupId);
            else
                us->groupImportance[msg.groupId] = im;
            us->set("importance/groupImportance", us->groupImportance);
        }
    };
    int importance = getImportance();
    importanceMenu->addAction(QIcon("://icons/veryImportant.png"), "很重要", [=]{
        setImportance(VeryImportant);
    })->check(importance == VeryImportant);
    importanceMenu->addAction(QIcon("://icons/important.png"), "重要", [=]{
        setImportance(Important);
    })->check(importance == Important);
    importanceMenu->addAction(QIcon("://icons/normalImportant.png"), "一般", [=]{
        setImportance(NormalImportant);
    })->check(importance == NormalImportant);
    importanceMenu->addAction(QIcon("://icons/unimportant.png"), "不重要", [=]{
        setImportance(Unimportant);
    })->check(importance == Unimportant);

    menu->split()->addAction(QIcon("://icons/closeUser.png"), "不显示该群通知", [=]{
        us->enabledGroups.removeOne(groupId);
        us->set("group/enables", us->enabledGroups);
        this->toHide();
        qInfo() << "不显示群组通知：" << groupId;
    })->hide(!groupId);
    menu->addAction(QIcon("://icons/hideView.png"), "立刻关闭所有通知", [=]{
        emit signalCloseAllCards();
    });

    bool ti = displayTimer->isActive();
    displayTimer->stop();
    menu->exec();
    connect(menu, &FacileMenu::signalHidden, this, [=]{
        if (ti)
            shallToHide();
    });
}

int NotificationCard::getReadDisplayDuration(QString text) const
{
    text = getValiableMessage(text);
    int length = text.length();
    return us->bannerDisplayDuration + (length * 1000 / us->bannerTextReadSpeed);
}

/// 创建对应的位置
/// 已有 showPoint，根据最大大小保存
void NotificationCard::createFrostGlass()
{
    if (!us->bannerFrostedGlassBg || !us->bannerFrostedGlassOpacity)
        return ;

    QRect cardRect(showPoint.x(), showPoint.y(), us->bannerFixedWidth, us->bannerContentWidth + (this->height() - ui->listWidget->height()));
    QT_BEGIN_NAMESPACE
        extern Q_WIDGETS_EXPORT void qt_blurImage( QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0 );
    QT_END_NAMESPACE

    // 获取窗口背景
    const int radius = 12; // 模糊半径
    QScreen* screen = QApplication::screenAt(QCursor::pos());
    QPixmap bg = screen->grabWindow(QApplication::desktop()->winId(), cardRect.left() - radius, cardRect.top() - radius, cardRect.width() + radius*2, cardRect.height() + radius*2);

    // 开始模糊
    QPixmap pixmap = bg;
    QPainter painter( &pixmap );
    QImage img = pixmap.toImage(); // img -blur-> painter(pixmap)
    qt_blurImage( &painter, img, radius, true, false );

    QPixmap blured(pixmap.size());
    blured.fill(Qt::transparent);
    QPainter painter2(&blured);
    painter2.setOpacity(us->bannerFrostedGlassOpacity / 255.0);
    painter2.drawPixmap(blured.rect(), pixmap);

    // 裁剪掉边缘（模糊后会有黑边）
    int c = qMin(bg.width(), bg.height());
    c = qMin(c/2, radius);
    frostGlassPixmap = blured.copy(c, c, blured.width()-c*2, blured.height()-c*2);

    // 设置label的图片
    frostGlassLabel = new QLabel(this);
    frostGlassLabel->lower();
    this->bg->lower();
    frostGlassLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    frostGlassLabel->setPixmap(frostGlassPixmap);
    frostGlassLabel->setFixedSize(cardRect.size());
    frostGlassLabel->move(this->bg->x(), 0);

    // 设置Mask
    {
        QPixmap mask(this->size());
        mask.fill(Qt::transparent);
        QPainter painter(&mask);
        QPainterPath path;
        path.addRoundedRect(this->bg->geometry(), us->bannerBgRadius, us->bannerBgRadius);
        QPainterPath clipPath;
        clipPath.addRect(this->rect());
        clipPath -= path;
        painter.fillPath(clipPath, Qt::white);
        frostGlassLabel->setMask(mask.mask());
    }
}

void NotificationCard::suspendHide()
{
    displayTimer->stop();
}

QString NotificationCard::getValiableMessage(QString text)
{
    return text.replace(QRegularExpression("<.+?>"), "").replace(QRegularExpression("\\[CQ:.+?\\]"), "").trimmed();
}

void NotificationCard::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    auto msg = msgs.last();
    if ((msg.isPrivate() && us->bannerPrivateKeepShowing)
            || (msg.isGroup() && us->bannerGroupKeepShowing))
    {
        displayTimer->stop();
    }
    else
    {
        displayTimer->start();
    }
}

void NotificationCard::paintEvent(QPaintEvent *)
{
}

void NotificationCard::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    bg->resize(event->size() - QSize(us->bannerBgShadow, us->bannerBgShadow));

    // 设置Mask
    if (frostGlassLabel)
    {
        QPixmap mask(this->size());
        mask.fill(Qt::transparent);
        QPainter painter(&mask);
        QPainterPath path;
        path.addRoundedRect(this->bg->geometry(), us->bannerBgRadius, us->bannerBgRadius);
        QPainterPath clipPath;
        clipPath.addRect(this->rect());
        clipPath -= path;
        painter.fillPath(clipPath, Qt::white);
        frostGlassLabel->clearMask();
        frostGlassLabel->setMask(mask);
    }
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
        if (!ac->userMsgHistory.contains(this->userId))
            return ;
        histories = &ac->userMsgHistory[this->userId];
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
            createPureMsgView(msg, index);
        }
        else
        {
            if (msg.senderId != senderId)
            {
                createMsgBox(msg, index);
            }
            else
            {
                createBlankMsgBox(msg, index);
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
