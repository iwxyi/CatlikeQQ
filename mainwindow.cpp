#include <QDebug>
#include "facilemenu.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "usettings.h"
#include "myjson.h"
#include "fileutil.h"
#include "imageutil.h"
#include "netutil.h"
#include "netimageutil.h"
#include "signaltransfer.h"
#include "headerutil.h"
#include "windows.h"
#include "widgets/customtabstyle.h"
#include "widgets/settings/accountwidget.h"
#include "widgets/settings/debugwidget.h"
#include "widgets/settings/groupwidget.h"
#include "widgets/settings/bannerwidget.h"
#include "widgets/settings/replywidget.h"
#include "widgets/settings/aboutwidget.h"
#include "widgets/settings/leavemodewidget.h"
#include "widgets/settings/remotecontrolwidget.h"
#include "widgets/settings/filewidget.h"
#include "widgets/settings/applicationwidget.h"
#include "widgets/settings/specialwidget.h"
#include "widgets/settings/countwidget.h"
#include "widgets/settings/speechwidget.h"
#include "widgets/settings/stylewidget.h"
#include "contact/contactpage.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initService();
    initView(); // 因为要绑定 service，所以要在 initService 后面
    initTray();
    initKey();

    startMessageLoop();
    NetImageUtil::saveNetFile("http://pubminishare-30161.picsz.qpic.cn/4f404643-0499-4faf-b1c0-a07707aed182", "D:/4f404643-0499-4faf-b1c0-a07707aed182.png");
}

MainWindow::~MainWindow()
{
    delete ui;

    deleteDir(rt->CACHE_PATH);
}

void MainWindow::initView()
{
    // 设置属性
    ui->settingsTabWidget->setAttribute(Qt::WA_StyledBackground);
    ui->settingsTabWidget->setTabPosition(QTabWidget::West);
    ui->settingsTabWidget->tabBar()->setStyle(new CustomTabStyle);
    ui->settingsTabWidget->setCurrentIndex(us->i("mainwindow/settingsTabIndex"));

    ui->auxiliaryTabWidget->setAttribute(Qt::WA_StyledBackground);
    ui->auxiliaryTabWidget->setTabPosition(QTabWidget::West);
    ui->auxiliaryTabWidget->tabBar()->setStyle(new CustomTabStyle);
    ui->auxiliaryTabWidget->setCurrentIndex(us->i("mainwindow/settingsTabIndex"));

    ui->dataTabWidget->setAttribute(Qt::WA_StyledBackground);
    ui->dataTabWidget->setTabPosition(QTabWidget::West);
    ui->dataTabWidget->tabBar()->setStyle(new CustomTabStyle);
    ui->dataTabWidget->setCurrentIndex(us->i("mainwindow/settingsTabIndex"));

    // 添加各种Tab页
    ui->settingsTabWidget->clear();
    ui->settingsTabWidget->addTab(new AccountWidget(cqhttpService, this), QIcon("://icons/account.png"), "账号绑定");
    ui->settingsTabWidget->addTab(new GroupWidget(this), QIcon("://icons/group.png"), "通知过滤");
    ui->settingsTabWidget->addTab(new BannerWidget(this), QIcon("://icons/banner.png"), "横幅弹窗");
    ui->settingsTabWidget->addTab(new ReplyWidget(this), QIcon("://icons/reply.png"), "消息回复");
    ui->settingsTabWidget->addTab(new FileWidget(this), QIcon("://icons/file.png"), "文件管理");
    ui->settingsTabWidget->addTab(new SpecialWidget(this), QIcon("://icons/care.png"), "特别关心");
    ui->settingsTabWidget->addTab(new StyleWidget(this), QIcon("://icons/bubble.png"), "卡片样式");
    // ui->settingsTabWidget->addTab(new QWidget(this), QIcon("://icons/animation.png"), "动画调整");
    ui->settingsTabWidget->addTab(new ApplicationWidget(this), QIcon("://icons/startup.png"), "程序设置");

    ui->auxiliaryTabWidget->clear();
    ui->auxiliaryTabWidget->addTab(new SpeechWidget(this), QIcon("://icons/speech.png"), "语音识别");
    ui->auxiliaryTabWidget->addTab(new LeaveModeWidget(this), QIcon("://icons/ai.png"), "离开模式");
    ui->auxiliaryTabWidget->addTab(new RemoteControlWidget(this), QIcon("://icons/control.png"), "远程控制");
    ui->auxiliaryTabWidget->addTab(new QWidget(this), QIcon("://icons/reply.png"), "快速回复");
    ui->auxiliaryTabWidget->addTab(new QWidget(this), QIcon("://icons/model.png"), "模型训练");

    ui->dataTabWidget->clear();
    ui->dataTabWidget->addTab(new AboutWidget(this), QIcon("://icons/about.png"), "关于程序");
    ui->dataTabWidget->addTab(new CountWidget(this), QIcon("://icons/statistical.png"), "数据统计");
    ui->dataTabWidget->addTab(new QWidget(this), QIcon("://icons/history_message.png"), "历史消息");
    ui->dataTabWidget->addTab(new DebugWidget(cqhttpService, this), QIcon("://icons/debug.png"), "开发调试");

    // 回复上次索引及索引改变信号
    ui->sideButtons->setCurrentRow(us->i("mainwindow/sideIndex"));
    ui->settingsTabWidget->setCurrentIndex(us->i("mainwindow/settingsTabIndex"));
    ui->auxiliaryTabWidget->setCurrentIndex(us->i("mainwindow/auxiliaryTabIndex"));
    ui->dataTabWidget->setCurrentIndex(us->i("mainwindow/dataTabIndex"));

    connect(ui->settingsTabWidget, &QTabWidget::currentChanged, this, [=](int index) {
        us->set("mainwindow/settingsTabIndex", index);
    });
    connect(ui->auxiliaryTabWidget, &QTabWidget::currentChanged, this, [=](int index) {
        us->set("mainwindow/auxiliaryTabIndex", index);
    });
    connect(ui->dataTabWidget, &QTabWidget::currentChanged, this, [=](int index) {
        us->set("mainwindow/dataTabIndex", index);
    });

    // 应用更改按钮
    confirmButton = new InteractiveButtonBase("应用更改", this);
    confirmButton->setBorderColor(Qt::gray);
    confirmButton->setFixedForeSize();
    confirmButton->setFixedForePos();
    confirmButton->move(this->rect().bottomRight() - QPoint(confirmButton->width(), confirmButton->height()));
    confirmButton->setFocusPolicy(Qt::StrongFocus);
    if (ui->sideButtons->currentRow() == 2)
        confirmButton->hide();
    confirmButton->setCursor(Qt::PointingHandCursor);
}

void MainWindow::on_sideButtons_currentRowChanged(int currentRow)
{
    ui->stackedWidget->setCurrentIndex(currentRow);
    us->set("mainwindow/sideIndex", currentRow);

    // 只有设置才显示应用更改
    if (confirmButton)
    {
        if (currentRow == 0 || currentRow == 1)
            this->confirmButton->show();
        else
            this->confirmButton->hide();
    }
}

void MainWindow::startMessageLoop()
{
    if (!us->host.isEmpty() && !us->host.contains("*"))
    {
        emit sig->hostChanged(us->host, us->accessToken);
    }
}

void MainWindow::showHistoryListMenu()
{
    // 显示最多十个好友/群组
    qint64 time = QDateTime::currentMSecsSinceEpoch() - 60 * 60 * 1000; // 只显示最近一小时的
    QList<QList<MsgBean>> msgss = ac->userMsgHistory.values() + ac->groupMsgHistory.values();
    QList<MsgBean> msgs;
    for (int i = 0; i < msgss.size(); i++)
    {
        if (msgss.at(i).count() == 0 || msgss.at(i).last().timestamp < time)
            msgss.removeAt(i--);
        else
            msgs.append(msgss.at(i).last());
    }

    std::sort(msgs.begin(), msgs.end(), [=](MsgBean a, MsgBean b) {
        return a.timestamp > b.timestamp;
    });

    // 如果没有消息，就不做操作了吧
    if (!msgs.size())
        return ;

    newFacileMenu;
    int maxIndex = msgs.size() - 1;
    if (maxIndex > 10)
        maxIndex = 10;
    for (int i = 0; i <= maxIndex; i++) // 正序（最新消息在上面，符合习惯）
    // for (int i = maxIndex; i >= 0; i--) // 倒序（最新消息在下面，交互方便）
    {
        MsgBean msg = msgs.at(i);
        QString name;
        QPixmap pixmap;
        AccountInfo::CardColor cc;
        if (msg.isPrivate())
        {
            name = ac->friendName(msg.friendId);
            name = us->userLocalNames.value(msg.friendId, name);
            if (isFileExist(rt->userHeader(msg.friendId)))
                pixmap = NetImageUtil::toRoundedPixmap(QPixmap(rt->userHeader(msg.friendId)));
            cc = ac->userHeaderColor.value(msg.friendId);
        }
        else if (msg.isGroup())
        {
            name = ac->groupName(msg.groupId);
            name = us->groupLocalNames.value(msg.groupId, name);

            if (isFileExist(rt->groupHeader(msg.groupId)))
                pixmap = NetImageUtil::toRoundedPixmap(QPixmap(rt->groupHeader(msg.groupId)));
            cc = ac->groupHeaderColor.value(msg.groupId);
        }

        // 添加自定义控件
        InteractiveButtonBase *w = new InteractiveButtonBase(menu);
        QLabel* headerLabel = new QLabel(w);
        QLabel* titleLabel = new QLabel(w);
        QVBoxLayout* vlayout = new QVBoxLayout;
        vlayout->addWidget(titleLabel);
        QHBoxLayout* hlayout = new QHBoxLayout(w);
        hlayout->addWidget(headerLabel);
        hlayout->addLayout(vlayout);

        headerLabel->setPixmap(pixmap.isNull() ? QPixmap("://icons/ignore") : pixmap);
        titleLabel->setText(name);
        titleLabel->setMaximumWidth(us->bannerFixedWidth);
        headerLabel->setScaledContents(true);
        titleLabel->adjustSize();

        // 遍历显示消息历史
        int hei = titleLabel->height();
        int maxCount = us->showMultiMessageHistories ? 3 : 1;
//        while (maxCount--)
//        {
            QLabel* messageLabel = new QLabel(w);
            vlayout->addWidget(messageLabel);
            QString mess = MessageView::simpleMessage(msg);
            if (mess.contains("\n"))
                mess = mess.left(mess.indexOf("\n"));
            if (msg.senderId == ac->myId)
                messageLabel->setText("你：" + mess);
            else if (msg.isPrivate())
                messageLabel->setText(mess);
            else
            {
                QString s = msg.username();
                // 昵称简化
                s = s.replace(QRegularExpression("^(?:id|ID|Id)[：|:](.+)$"), "\\1")
                        .replace(QRegularExpression("^(.+)\\s*[\\(（].+[\\)）]$"), "\\1");
                messageLabel->setText(s + ": " + mess);
            }
            messageLabel->setMaximumWidth(us->bannerFixedWidth);
            messageLabel->adjustSize();
            hei += messageLabel->height() + hlayout->margin();
//        }

        headerLabel->setMaximumSize(hei, hei);
        hei += vlayout->spacing() + hlayout->margin();
        w->setFixedSize(us->bannerFixedWidth, hei);
        w->setDoubleClicked(true);
        connect(w, &InteractiveButtonBase::clicked, this, [=]{
            // 根据聊天信息，重新打开对应的对话框
            focusOrShowMessageCard(msg, false, "", true);
        });
        connect(w, &InteractiveButtonBase::doubleClicked, this, [=]{
            // 因为要聚焦，所有这个popup的菜单必须要关闭
            menu->close();
            // 根据聊天信息，重新打开对应的对话框
            focusOrShowMessageCard(msg, true, "", true);
        });
        if (cc.isValid() && us->bannerUseHeaderColor)
        {
            w->setBgColor(cc.bg);
            titleLabel->setStyleSheet("color:" + QVariant(cc.fg).toString());
            messageLabel->setStyleSheet("color:" + QVariant(cc.fg).toString());
        }
        menu->addWidget(w);

        // 显示的时候还可以实时更新消息
        connect(cqhttpService, &CqhttpService::signalMessage, w, [=](const MsgBean& m){
            if (!msg.isMsg())
                return ;
            if (!msg.isSameObject(m))
                return ;
            if (msg.isPrivate())
                messageLabel->setText(MessageView::simpleMessage(m));
            else if (msg.senderId == ac->myId)
                messageLabel->setText("你：" + mess);
            else
                messageLabel->setText(m.nickname + ": " + MessageView::simpleMessage(m));
        });

        // 使用默认的菜单机制（太朴素了）
        /* auto action = menu->addAction(pixmap.isNull() ? QIcon("://icons/hideView") : QIcon(pixmap), name, [=]{
            // 根据聊天信息，重新打开对应的对话框
            focusOrShowMessageCard(msg, true);
        });
        if (cc.isValid())
        {
            action->fgColor(cc.fg)->bgColor(cc.bg);
        } */
    }

    menu->adjustSize();
    menu->setAppearAnimation(false);
    menu->exec();
}

QRect MainWindow::screenGeometry() const
{
    auto screens = QGuiApplication::screens();
    int& index = us->bannerScreenIndex;
    if (index >= screens.size())
        index = screens.size() - 1;
    if (index < 0)
        return QRect();
    return screens.at(us->bannerScreenIndex)->geometry();
}

void MainWindow::initTray()
{
    tray = new QSystemTrayIcon(this);
    tray->setIcon(QIcon("://appicon"));
    tray->setToolTip(APPLICATION_NAME);
    tray->show();
    connect(tray,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(trayAction(QSystemTrayIcon::ActivationReason)));

    trayRestoreTimer = new QTimer(this);
    trayRestoreTimer->setInterval(us->trayShowIconDuration);
    connect(trayRestoreTimer, &QTimer::timeout, this, [=]{
        tray->setIcon(ac->myHeader.isNull() ? QIcon("://appicon") : QIcon(ac->myHeader));
        currentTrayMsg = MsgBean();
        trayFlashPixmap = QPixmap();
        trayHiding = false;
        trayHideTimer->stop();
    });

    trayHideTimer = new QTimer(this);
    trayHideTimer->setInterval(us->trayFlashingInterval);
    connect(trayHideTimer, &QTimer::timeout, this, [=]{
        if (!trayHiding) // 隐藏图标
        {
            QPixmap pixmap(32, 32);
            pixmap.fill(Qt::transparent);
            tray->setIcon(pixmap);
            trayHiding = true;
        }
        else // 显示图标
        {
            tray->setIcon(trayFlashPixmap.isNull() ? ac->myHeader.isNull() ? QIcon("://appicon") : QIcon(ac->myHeader) : trayFlashPixmap);
            trayHiding = false;
            trayHideTimer->stop();
        }
    });
}

void MainWindow::trayAction(QSystemTrayIcon::ActivationReason reason)
{
    if (us->autoPauseByOtherDevice && us->isPausingByOtherDevice)
    {
        qInfo() << "从暂停通知中恢复";
        us->isPausingByOtherDevice = false;
    }
    rt->mySendCount = 0;
    switch(reason)
    {
    case QSystemTrayIcon::Trigger:
        showHistoryListMenu();
        break;
    case QSystemTrayIcon::MiddleClick:
        if (!this->isHidden())
        {
            this->hide();
        }
        else
        {
            this->showNormal();
            this->activateWindow();
        }
        break;
    case QSystemTrayIcon::Context:
    {
        FacileMenu* menu = new FacileMenu;

        menu->addAction(QIcon("://icons/leaveMode.png"), "临时离开", [=] {
            // 这里的离开模式不会保存，重启后还是以设置中为准
            us->leaveMode = !us->leaveMode;
        })->check(us->leaveMode)->tooltip("开启离开模式，可能开启了私聊AI回复\n重启后将恢复原来状态")->hide();

        menu->addAction(QIcon("://icons/silent.png"), "临时静默", [=] {
            // 这里的静默模式不会保存，重启后还是以设置中为准
            rt->notificationSlient = !rt->notificationSlient;
            if (rt->notificationSlient)
                closeAllCard();
        })->check(rt->notificationSlient)->tooltip("临时屏蔽所有消息\n重启后将恢复原来状态");

        auto importanceMenu = menu->split()->addMenu(QIcon("://icons/importance.png"), "过滤重要性");
        auto setImportance = [=](int im) {
            us->set("importance/lowestImportance", us->lowestImportance = im);
        };

        importanceMenu->addAction(QIcon("://icons/veryImportant.png"), "很重要", [=]{
            setImportance(VeryImportant);
        })->check(us->lowestImportance == VeryImportant);

        importanceMenu->addAction(QIcon("://icons/important.png"), "重要", [=]{
            setImportance(LittleImportant);
        })->check(us->lowestImportance == LittleImportant);

        importanceMenu->addAction(QIcon("://icons/normalImportant.png"), "一般", [=]{
            setImportance(NormalImportant);
        })->check(us->lowestImportance == NormalImportant);

        importanceMenu->addAction(QIcon("://icons/unimportant.png"), "不重要", [=]{
            setImportance(Unimportant);
        })->check(us->lowestImportance == Unimportant);

        menu->split()->addAction(QIcon("://icons/settings.png"), "设置", [=]{
            this->show();
            this->activateWindow();
        });

        menu->addAction(QIcon("://icons/search.png"), "搜索", [=]{
            ContactPage::getInstance()->show();
        });

        menu->addAction(QIcon("://icons/quit.png"), "退出", [=]{
            qApp->quit();
        });

        menu->exec(QCursor::pos());
    }
        break;
    default:
        break;
    }
}

void MainWindow::initService()
{
    // 网络服务
    cqhttpService = new CqhttpService(this);

    connect(cqhttpService, SIGNAL(signalMessage(const MsgBean&)), this, SLOT(messageReceived(const MsgBean&)));

    connect(cqhttpService, SIGNAL(signalMessage(const MsgBean&)), this, SLOT(autoReplyMessage(const MsgBean&)));

    connect(sig, &SignalTransfer::sendSocketText, cqhttpService, &CqhttpService::sendTextMessage);

    connect(sig, &SignalTransfer::sendSocketJson, cqhttpService, &CqhttpService::sendJsonMessage);

    connect(sig, &SignalTransfer::loadGroupMembers, cqhttpService, &CqhttpService::refreshGroupMembers);

    connect(sig, SIGNAL(getGroupMsgHistory(qint64, qint64)), cqhttpService, SLOT(getGroupMsgHistory(qint64, qint64)));

    connect(sig, &SignalTransfer::myHeader, this, [=](const QPixmap& pixmap) {
        ac->myHeader = NetImageUtil::toRoundedPixmap(pixmap);
        tray->setIcon(pixmap);
    });

    connect(sig, &SignalTransfer::openUserCard, this, [=](qint64 userId, const QString& username, const QString& text) {
        MsgBean msg(userId, username);
        msg.privt(userId, userId);
        focusOrShowMessageCard(msg, true, text);
    });

    connect(sig, &SignalTransfer::openGroupCard, this, [=](qint64 groupId, const QString& text) {
        MsgBean msg(0, "");
        msg.group(groupId, ac->groupName(groupId));
        focusOrShowMessageCard(msg, true, text);
    });

    connect(sig, SIGNAL(showTrayIcon(const MsgBean&)), this, SLOT(showTrayIcon(const MsgBean&)));

    // 远程控制
    remoteControlService = new RemoteControlServie(this);

    // 其他API服务
    // 刷新百度API的token
    if (!us->baiduSpeechAccessToken.isEmpty())
    {
        qint64 time = QDateTime::currentSecsSinceEpoch();
        qint64 prevTime = us->l("baiduSpeech/tokenRefreshTime", time);
        if (time - prevTime >= 30 * 24 * 3600)
        {
            SpeechWidget::refreshToken();
        }
    }
}

void MainWindow::initKey()
{
#if defined(ENABLE_SHORTCUT)
    editShortcut = new QxtGlobalShortcut(this);
    QString def_key = us->value("banner/replyKey", "shift+alt+x").toString();
    editShortcut->setShortcut(QKeySequence(def_key));
    connect(editShortcut, &QxtGlobalShortcut::activated, this, [=]() {
        if (us->autoPauseByOtherDevice && us->isPausingByOtherDevice)
        {
            qInfo() << "从暂停通知中恢复";
            us->isPausingByOtherDevice = false;
        }
        rt->mySendCount = 0;
#if defined(Q_OS_WIN32)
        auto hwnd = GetForegroundWindow();
        bool isMe = false;
        foreach (auto card, notificationCards)
            if (HWND(card->winId()) == hwnd)
            {
                isMe = true;
                break;
            }
        // 如果不是自己的通知卡片
        if (!isMe)
            prevWindow = hwnd;
#endif
        // this->activateWindow();
        focusCardReply();
    });
#endif

    connect(sig, &SignalTransfer::setReplyKey, this, [=](QString key) {
        editShortcut->setShortcut(QKeySequence(key));
    });
}

void MainWindow::showEvent(QShowEvent *e)
{
    QMainWindow::showEvent(e);

    us->set("mainwindow/hide", false);

    static bool first = true;
    if (first)
    {
        first = false;
        restoreGeometry(us->value("mainwindow/geometry").toByteArray());
        restoreState(us->value("mainwindow/state").toByteArray());
    }
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    us->setValue("mainwindow/geometry", this->saveGeometry());
    us->setValue("mainwindow/state", this->saveState());
	us->sync();

#if defined(ENABLE_TRAY)
    e->ignore();
    this->hide();

    // 因为关闭程序也会触发这个，所以需要定时一下
    QTimer::singleShot(5000, [=]{
        us->set("mainwindow/hide", true);
    });

    QTimer::singleShot(5000, [=]{
        if (!this->isHidden())
            return ;
        us->setValue("mainwindow/autoShow", false);
    });
#else
    QMainWindow::closeEvent(e);
#endif
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);

    confirmButton->move(this->rect().bottomRight() - QPoint(confirmButton->width() + us->bannerSpacing, confirmButton->height() + us->bannerSpacing));
}

void MainWindow::returnToPrevWindow()
{
#ifdef Q_OS_WIN32
        if (this->prevWindow)
            SwitchToThisWindow(prevWindow, true);
        prevWindow = nullptr;
#endif
}

/// 接收到的所有消息都会在这里（只要有解析过的）
/// 在这里判断要不要显示
void MainWindow::messageReceived(const MsgBean &msg, bool blockSelf)
{
    // ========== 记录消息 ==========
    if (msg.isMsg())
    {
        us->addCount(us->countReceiveAll, "receiveAll");
        if (msg.isPrivate())
        {
            if (ac->friendList.contains(msg.friendId))
                ac->friendList[msg.friendId].lastMsgTime = QDateTime::currentMSecsSinceEpoch();

            if (!ac->userMsgHistory.contains(msg.friendId))
                ac->userMsgHistory.insert(msg.friendId, QList<MsgBean>());
            ac->userMsgHistory[msg.friendId].append(msg);
            ac->receiveCountAfterMySendPrivate[msg.friendId] = ac->receiveCountAfterMySendPrivate.value(msg.friendId) + 1;

            if (msg.senderId != ac->myId)
                us->addCount(us->countReceivePrivate, "receivePrivate");
        }
        else if (msg.isGroup())
        {
            if (ac->groupList.contains(msg.groupId))
                ac->groupList[msg.groupId].lastMsgTime = QDateTime::currentMSecsSinceEpoch();

            if (!ac->groupMsgHistory.contains(msg.groupId))
                ac->groupMsgHistory.insert(msg.groupId, QList<MsgBean>());
            ac->groupMsgHistory[msg.groupId].append(msg);
            ac->receivedCountAfterMySentGroup[msg.groupId] = ac->receivedCountAfterMySentGroup.value(msg.groupId) + 1;

            if (msg.senderId != ac->myId)
                us->addCount(us->countReceiveGroup, "receiveGroup");
        }

        // 自己发送的消息
        if (msg.senderId == ac->myId)
        {
            // 判断是不是自己当前程序发送的
            if (us->autoPauseByOtherDevice && rt->mySendCount == 0)
            {
                qInfo() << "其他设备发送消息，暂停通知";
                us->isPausingByOtherDevice = true;
            }
        }
    }

    if (us->trayShowAllMessageIcon)
        showTrayIcon(msg);

    // ========== 显示通知 ==========
    // 如果已经显示了卡片，那么忽略所有开关
    NotificationCard* currentCard = nullptr;
    foreach (auto card, notificationCards)
    {
        if (card->is(msg))
        {
            currentCard = card;
            break;
        }
    }

    if (!currentCard) // 需要显示新卡片
    {
        if (!canNewCardShow(msg))
            return ;
    }

    // 解除智能聚焦
    // 重要的必定发送，除非静默
    if (us->smartFocus && msg.senderId != ac->myId)
    {
        if (msg.isPrivate())
        {
            if (ac->askUser.contains(msg.senderId))
            {
                ac->askUser.remove(msg.senderId);
                qInfo() << "移除私聊疑问智能聚焦：" << msg.senderId;
            }
        }
        else if (msg.isGroup())
        {
            if (ac->askGroup.contains(msg.groupId))
            {
                ac->askGroup.remove(msg.groupId);
                qInfo() << "移除群聊疑问智能聚焦：" << msg.groupId << msg.senderId;
            }
            if (ac->groupList.value(msg.groupId).atMember.contains(msg.senderId))
            {
                ac->groupList[msg.groupId].atMember.remove(msg.senderId);
                qInfo() << "移除群聊艾特智能聚焦：" << msg.groupId << msg.senderId;
            }
        }
    }

    showMessageCard(msg, blockSelf);
}

bool MainWindow::canNewCardShow(const MsgBean &msg) const
{
    if (!msg.isMsg() && !msg.is(ActionJoin))
    {
        return false;
    }

    // 静默模式
    if (rt->notificationSlient || us->isPausingByOtherDevice)
    {
        if (us->trayShowSlientPrivateMessageIcon && msg.isPrivate())
            showTrayIcon(msg);

        // 不需要进行优先级的判断
        if (!us->trayShowAllSlientMessageIcon && !us->trayShowSlientSpecialMessageIcon && !us->trayShowLowImportanceMessageIcon)
            return false;
    }

    // 判断群组显示开关
    qint64 cur = QDateTime::currentMSecsSinceEpoch();
    if (msg.isGroup() && !us->isGroupShow(msg.groupId)) // 群组消息开关
    {
        if (ac->askGroup.contains(msg.groupId)) // 要显示群组通知
        {}
        else if (ac->groupList[msg.groupId].atMember.contains(msg.senderId)) // 艾特成员的消息
        {}
        else if (ac->mySendGroupTime.contains(msg.groupId)
                 && (cur - ac->mySendGroupTime[msg.groupId] <= 180000
                     || ac->receivedCountAfterMySentGroup[msg.groupId] <= 10))
        {}
        else // 无关紧要的群组
            return false;
    }

    // 特别关心（叠加）
    int special = (us->userSpecial.contains(msg.senderId) ? 1 : 0)
            + (us->groupMemberSpecial.contains(msg.senderId) ? 1 : 0);

    // 判断消息级别开关
    int im = NormalImportant;
    if (msg.isPrivate())
    {
        im = us->userImportance.value(msg.senderId, us->userDefaultImportance);
    }
    else if (msg.isGroup())
    {
        if (!us->enabledGroups.contains(msg.groupId)) // 本来是不显示通知的群组
            im = Unimportant;
        else
            im = us->groupImportance.value(msg.groupId, us->groupDefaultImportance);

        // 判断发送者的重要性，max(用户,群组)
        if (us->groupUseFriendImportance)
        {
            int im2 = us->userImportance.value(msg.senderId, im);
            im = qMax(im, im2);
        }
    }

    // 特别关心（好友/群内）
    im += special;

    // 全局提醒词
    bool globalRemind = false;
    for (auto w: us->globalRemindWords)
    {
        if (w.trimmed().isEmpty())
            continue;
        if (msg.message.contains(w))
        {
            globalRemind = true;
            im++;
            if (!us->remindOverlay)
                break;
        }
    }

    // 群组提醒词
    bool groupRemind = false;
    if ((us->remindOverlay || !globalRemind) && msg.isGroup() && us->groupRemindWords.contains(msg.groupId))
    {
        for (auto w: us->groupRemindWords.value(msg.groupId))
        {
            if (w.trimmed().isEmpty())
                continue;
            if (msg.message.contains(w))
            {
                groupRemind = true;
                im++;
                if (!us->remindOverlay)
                    break;
            }
        }
    }

    // @全体成员/@我
    if (us->improveAtAllImportance && msg.hasAt(0))
    {
        im++;
    }
    if (us->improveAtMeImportance && msg.hasAt(ac->myId))
    {
        im++;
    }

    // 动态重要性
    if (us->dynamicImportance)
    {
        qint64 time = 0;
        int count = 0;
        if (msg.isPrivate())
        {
            time = ac->mySendPrivateTime.value(msg.friendId, 0);
            count = ac->receiveCountAfterMySendPrivate.value(msg.friendId);
        }
        else if (msg.isGroup())
        {
            time = ac->mySendGroupTime.value(msg.groupId, 0);
            count = ac->receivedCountAfterMySentGroup.value(msg.groupId);
        }
        if (time > 0) // 自己发送过消息
        {
            qint64 delta = (cur - time) / 1000; // 距离自己发消息后过了多少秒
            count--; // 因为自己发送的消息也在里面，所以要-1
            if (delta <= 60 || (count >= 0 && count <= 3))
            {
                // 一分钟内，提升两个级别
                im += 2;
                qInfo() << "动态重要性2：" << count << delta;
            }
            else if (delta <= 180 || (count >= 0 && count <= 10))
            {
                // 三分钟内，提升一个级别
                im += 1;
                qInfo() << "动态重要性1：" << count << delta;
            }
        }
    }


    // 智能聚焦
    if (us->smartFocus)
    {
        if (msg.isPrivate())
        {
            if (ac->askUser.contains(msg.senderId))
            {
                // 至少要显示通知
                if (im < us->lowestImportance - 1)
                    im = us->lowestImportance;
                else
                    im++;
            }
        }
        else if (msg.isGroup())
        {
            if (ac->askGroup.contains(msg.groupId))
            {
                if (im < us->lowestImportance - 1)
                    im = us->lowestImportance;
                else
                    im++;
            }
            else if (ac->groupList.value(msg.groupId).atMember.contains(msg.senderId))
            {
                if (im < us->lowestImportance - 1)
                    im = us->lowestImportance;
                else
                    im++;
            }
        }
    }

    // 不足优先级，不通知
    if (im < us->lowestImportance)
    {
        // 静默消息的托盘图标
        if (((rt->notificationSlient || us->isPausingByOtherDevice) && (us->trayShowAllSlientMessageIcon|| (us->trayShowSlientSpecialMessageIcon && im >= VeryImportant)))
                || us->trayShowLowImportanceMessageIcon)
            showTrayIcon(msg);
        return false;
    }

    return !rt->notificationSlient && !us->isPausingByOtherDevice;
}

/// 显示通知卡片，可能是新卡片，也可能需要附加上去
/// @param blockSelf 自己发送的消息，是否也要创建新的卡片
NotificationCard* MainWindow::showMessageCard(const MsgBean &msg, bool blockSelf, bool showHistory)
{
    // 保存最后显示的（接收的消息不一定会显示）
    setLastOpenMsg(msg);
    us->addCount(us->countShowBanner, "showBanner");

    // 判断有没有现有的卡片
    foreach (auto card, notificationCards)
    {
        if (card->append(msg))
        {
            return card;
        }
    }

    // 自己发的消息，不新建卡片
    if (blockSelf && msg.senderId == ac->myId)
        return nullptr;

    // 没有现有的，新建一个卡片
    return createNotificationCard(msg, showHistory);
}

NotificationCard* MainWindow::createNotificationCard(const MsgBean &msg, bool showHistory)
{
    // 创建卡片
    NotificationCard* card = new NotificationCard(nullptr);

    // 判断卡片的位置
    QPoint startPos; // 开始出现的位置
    QPoint showPos;  // 显示的最终位置
    switch (int(us->bannerFloatSide)) // 这个int是去掉警告，因为很多位置尚不支持
    {
    case SideRight: // 右
    {
        int rightest = screenGeometry().width();
        startPos.setX(rightest - us->bannerMargin);
        showPos.setX(rightest - us->bannerFixedWidth - us->bannerSpacing);
        break;
    }
    case SideLeft:
    {
        startPos.setX(-us->bannerFixedWidth);
        showPos.setX(us->bannerMargin);
        break;
    }
    default:
        qCritical() << "Error: Unknow position!";
        return nullptr;
    }

    switch (int(us->bannerFloatDirection))
    {
    case TopToBottom:
    {
        int top = us->bannerFloatPixel;
        if (notificationCards.size())
            top = notificationCards.last()->geometry().bottom() + us->bannerSpacing;
        startPos.setY(top);
        showPos.setY(top);
        break;
    }
    case BottomToTop:
        int bottom = screenGeometry().bottom() - us->bannerFloatPixel;
        startPos.setY(bottom - card->height());
        showPos.setY(bottom - card->height());
        // 要调整原先卡片的高度
        slotCardHeightChanged(notificationCards.count() - 1, card->height() + us->bannerSpacing);
        break;
    }

    connect(card, &NotificationCard::signalHeightChanged, this, [=](int delta) {
        slotCardHeightChanged(card, delta);
    });
    connect(card, &NotificationCard::signalHided, this, [=]{
        slotCardHeightChanged(card, -(card->height() + us->bannerSpacing));
        notificationCards.removeOne(card);
    });
    connect(card, &NotificationCard::signalReplyPrivate, cqhttpService,  &CqhttpService::sendPrivateMsg);

    connect(card, &NotificationCard::signalReplyGroup, cqhttpService, &CqhttpService::sendGroupMsg);

    connect(card, &NotificationCard::signalCancelReply, this, [=]{
        returnToPrevWindow();
    });

    connect(card, &NotificationCard::signalFocusPrevCard, this, [=]{
        int index = notificationCards.indexOf(card);
        Q_ASSERT(index > -1);
        index--;
        if (index < 0)
            index = notificationCards.size() - 1;
        notificationCards.at(index)->showReplyEdit(true);
    });
    connect(card, &NotificationCard::signalFocusNextCard, this, [=]{
        int index = notificationCards.indexOf(card);
        Q_ASSERT(index > -1);
        index++;
        if (index >= notificationCards.size())
            index = 0;
        notificationCards.at(index)->showReplyEdit(true);
    });
    connect(card, &NotificationCard::signalFocusCard, this, [=](int index){
        Q_ASSERT(notificationCards.size());
        if (index < 0)
            index = 0;
        if (index >= notificationCards.size())
            index = notificationCards.size() - 1;
        notificationCards.at(index)->showReplyEdit(true);
    });
    connect(card, &NotificationCard::signalCloseAllCards, this, [=]{
        closeAllCard();
    });

    // 先确定位置，因为在下载文件的时候后面可能会进入相同的数据
    notificationCards.append(card);
    card->setShowPos(startPos, showPos);
    card->move(startPos);
    card->setMsg(msg); // 这个可能是一个非常耗时的操作，引起坐标的改变
    if (showHistory)
    {
        card->loadMsgHistoryByLocal();
        card->scrollToBottomE();
    }
    card->showFrom();
    return card;
}

/// 显示或者聚焦对应的用户/群组卡片，并带有一些特定操作
/// 如果没有，则新建一个
/// 双击历史消息触发
NotificationCard* MainWindow::focusOrShowMessageCard(const MsgBean &msg, bool focusEdit, const QString &insertText, bool showHistory)
{
    setLastOpenMsg(msg);
    foreach (auto card, notificationCards)
    {
        if (card->is(msg))
        {
            if (focusEdit)
            {
                card->showReplyEdit(true);
                if (showHistory)
                    card->loadMsgHistoryByLocal();
            }
            return card;
        }
    }

    auto card = createNotificationCard(msg, showHistory);
    Q_ASSERT(card);
    if (focusEdit)
    {
        card->showReplyEdit(true);
    }

    if (!insertText.isEmpty())
        card->addReplyText(insertText);

    /* if (showHistory)
    {
        card->loadMsgHistoryByLocal();
        card->scrollToBottomE();
    } */

    return card;
}

void MainWindow::slotCardHeightChanged(NotificationCard *card, int deltaHeight)
{
    slotCardHeightChanged(notificationCards.indexOf(card), deltaHeight);
}

void MainWindow::slotCardHeightChanged(int index, int deltaHeight)
{
    if (index == -1)
        return ;
    switch (int(us->bannerFloatDirection))
    {
    case TopToBottom:
        adjustUnderCardsTop(index, deltaHeight);
        break;
    case BottomToTop:
        adjustAboveCardsTop(index, -deltaHeight);
        break;
    }
}

/**
 * 调整下面所有卡片的位置（高度改变/卡片隐藏）
 * @param firstIndex  第一个开始调整的位置的上一个（即最后一个不需要调整的索引）
 * @param deltaHeight 位置差，可能正可能负
 */
void MainWindow::adjustUnderCardsTop(int aboveIndex, int deltaHeight)
{
    if (aboveIndex < 0)
        return ;

    for (int i = aboveIndex + 1; i < notificationCards.count(); i++)
    {
        // 移动动画的话，需要考虑到一些冲突什么的，大概还是比较麻烦的
        // 比如正在动画中，又需要临时调整，就很难搞
        auto card = notificationCards.at(i);
        if (card->isHidding())
            continue;
        card->adjustTop(deltaHeight);
    }
}

/**
 * 调整上面所有卡片的位置（高度改变/卡片插入/卡片隐藏）
 * @param lastIndex 最后一个需要调整位置的索引（包括自己）
 */
void MainWindow::adjustAboveCardsTop(int underIndex, int deltaHeight)
{
    if (underIndex < 0 || underIndex >= notificationCards.count())
        return ;

    for (int i = 0; i <= underIndex; i++)
    {
        auto card = notificationCards.at(i);
        if (card->isHidding())
            continue;
        card->adjustTop(deltaHeight);
    }
}

/// 聚焦到最近一次有消息的卡片的回复
void MainWindow::focusCardReply()
{
    qint64 current = QDateTime::currentMSecsSinceEpoch() - us->bannerReplyIgnoreWithin;

    qint64 maxium = 0;
    NotificationCard* targetCard = nullptr;
    foreach (auto card, notificationCards)
    {
        if (card->isHidding())
            continue;

        auto msgs = card->getMsgs();
        int index = msgs.size();
        while (--index >= 0)
        {
            qint64 time = msgs.at(index).timestamp;
            if (time > current) // 跳过新消息
                continue;
            if (time > maxium)
            {
                maxium = time;
                targetCard = card;
            }
            else // 后面的都是旧消息了
                break;
        }
    }

    if (targetCard)
    {
        targetCard->showReplyEdit(true);
        targetCard->setFastFocus();
        setLastOpenMsg(targetCard->getMsgs().last());
    }
    else // 没有卡片，那么就聚焦到最后一次有消息的
    {
        if (!ac->lastReceiveShowId) // 没有最后一次消息
        {
            qInfo() << "没有最后一次消息";
            return ;
        }

        // 显示卡片
        if (ac->lastReceiveShowIsUser)
        {
            const auto& history = ac->userMsgHistory.value(ac->lastReceiveShowId, QList<MsgBean>());
            if (!history.size())
            {
                qWarning() << "不存在的用户消息记录：" << ac->lastReceiveShowId;
                return ;
            }

            showMessageCard(history.last(), false, true);
        }
        else
        {
            const auto& history = ac->groupMsgHistory.value(ac->lastReceiveShowId, QList<MsgBean>());
            if (!history.size())
            {
                qWarning() << "不存在的群组消息记录：" << ac->lastReceiveShowId;
                return ;
            }

            showMessageCard(history.last(), false, true);
        }
        targetCard = notificationCards.last();

        // 再聚焦
        targetCard->showReplyEdit(true);
        targetCard->setFastFocus();
    }
}

void MainWindow::closeAllCard()
{
    foreach (auto card, notificationCards)
    {
        if (card->isFixing())
            continue;
        card->toHide();
    }
}

void MainWindow::setLastOpenMsg(const MsgBean &msg)
{
    if (msg.isPrivate())
    {
        ac->lastReceiveShowIsUser = true;
        ac->lastReceiveShowId = msg.friendId;
    }
    else if (msg.isGroup())
    {
        ac->lastReceiveShowIsUser = false;
        ac->lastReceiveShowId = msg.groupId;
    }
}

/// 收到消息（所有消息），判断是否需要自动回复
void MainWindow::autoReplyMessage(const MsgBean &msg)
{
    if (!msg.isMsg())
        return ;
    if (msg.senderId == ac->myId) // 自己的消息，不用管
    {
        if (msg.targetId == ac->myId) // 自己发给自己的，尝试进行远程控制
        {
            remoteControlService->execCmd(msg.message);
        }
        return ;
    }

    if (msg.isPrivate() && us->leaveMode && us->aiReplyPrivate) // 离开模式，自动回复
    {
        qint64 userId = msg.senderId;

        // 判断是否回复
        if (!us->aiReplyUsers.contains(userId))
            return ;

        // 判断回复间隔
        qint64 time = QDateTime::currentMSecsSinceEpoch();
        if (ac->aiReplyPrivateTime.value(userId, 0) + us->aiReplyInterval >= time)
            return ;
        ac->aiReplyPrivateTime[userId] = time;

        triggerAiReply(msg);
        return ;
    }
}

/// 触发AI回复
/// 已确保回复条件
void MainWindow::triggerAiReply(const MsgBean &msg, int retry)
{
    QString text = NotificationCard::getValiableMessage(msg.message);
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

    // 获取信息
    connect(new NetUtil(url, params), &NetUtil::finished, this, [=](QString result){
        MyJson json(result.toUtf8());
        QString answer;
        int ret = json.contains("ret") ? json.value("ret").toInt() : -1;
        if (ret != 0)
        {
            QString rep = json.value("msg").toString();
            qWarning() << "AI回复：" << ret << rep << text;
            if (rep == "chat answer not found" && retry < 3)
            {
                triggerAiReply(msg, retry + 1);
                return ;
            }
            else if (retry >= 3)
            {
            }
            else
            {
                return ;
            }
        }

        if (ret != 0) // 使用默认的
        {
            answer = us->aiReplyDefault;
            qInfo() << "[离开模式.默认回复]" << answer;
        }
        else
        {
            answer = json.value("data").toObject().value("answer").toString();
            qInfo() << "[离开模式.AI回复]" << answer;
            qWarning() << "AI.API返回：" << json;
            answer = us->aiReplyPrefix + answer + us->aiReplySuffix;
        }

        cqhttpService->sendPrivateMsg(msg.senderId, answer, msg.fromGroupId);
    });
}

void MainWindow::showTrayIcon(const MsgBean &msg) const
{
    // 自己发的消息，肯定是忽略掉的
    if (msg.senderId == ac->myId)
        return;

    // 一些操作，也要忽略掉
    if (!msg.is(ActionMsg) && !msg.is(ActionJoin))
        return ;

    trayRestoreTimer->start();

    // 还是这个消息，不做其他操作，只是延长时间
    if (currentTrayMsg.is(msg))
        return ;

    // 切换消息
    currentTrayMsg = msg;
    trayFlashPixmap = msg.isPrivate() ? HeaderUtil::userHeader(msg.senderId)
                                      : HeaderUtil::groupHeader(msg.groupId);
    trayFlashPixmap = NetImageUtil::toRoundedPixmap(trayFlashPixmap);
    if (trayFlashPixmap.isNull())
    {
        qWarning() << "无法显示托盘头像：" << msg.debugString();
        return ;
    }

    tray->setIcon(trayFlashPixmap);
    trayHiding = false;
    trayHideTimer->start();
}
