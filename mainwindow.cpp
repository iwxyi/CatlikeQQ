#include <QDebug>
#include "facilemenu.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "usettings.h"
#include "myjson.h"
#include "fileutil.h"
#include "imageutil.h"
#include "netutil.h"
#include "signaltransfer.h"
#include "windows.h"
#include "widgets/customtabstyle.h"
#include "widgets/settings/accountwidget.h"
#include "widgets/settings/debugwidget.h"
#include "widgets/settings/groupwidget.h"
#include "widgets/settings/bannerwidget.h"
#include "widgets/settings/replywidget.h"
#include "widgets/settings/aboutwidget.h"
#include "widgets/settings/leavemodewidget.h"

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
}

MainWindow::~MainWindow()
{
    delete ui;

    deleteDir(rt->CACHE_PATH);
}

void MainWindow::initView()
{
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


    ui->settingsTabWidget->clear();
    ui->settingsTabWidget->addTab(new AccountWidget(service, this), QIcon("://icons/account.png"), "账号绑定");
    ui->settingsTabWidget->addTab(new GroupWidget(this), QIcon("://icons/group.png"), "群组消息");
    ui->settingsTabWidget->addTab(new BannerWidget(this), QIcon("://icons/banner.png"), "横幅通知");
    ui->settingsTabWidget->addTab(new ReplyWidget(this), QIcon("://icons/reply.png"), "通知回复");
    ui->settingsTabWidget->addTab(new QWidget(this), QIcon("://icons/bubble.png"), "气泡样式");
    ui->settingsTabWidget->addTab(new QWidget(this), QIcon("://icons/care.png"), "特别关心");
    ui->settingsTabWidget->addTab(new QWidget(this), QIcon("://icons/animation.png"), "动画调整");
    ui->settingsTabWidget->addTab(new QWidget(this), QIcon("://icons/startup.png"), "程序启动");

    ui->auxiliaryTabWidget->clear();
    ui->auxiliaryTabWidget->addTab(new LeaveModeWidget(this), QIcon("://icons/ai.png"), "离开模式");
    ui->auxiliaryTabWidget->addTab(new QWidget(this), QIcon("://icons/reply.png"), "快速回复");
    ui->auxiliaryTabWidget->addTab(new QWidget(this), QIcon("://icons/model.png"), "模型训练");

    ui->dataTabWidget->clear();
    ui->dataTabWidget->addTab(new AboutWidget(this), QIcon("://icons/about.png"), "关于程序");
    ui->dataTabWidget->addTab(new QWidget(this), QIcon("://icons/statistical.png"), "数据统计");
    ui->dataTabWidget->addTab(new QWidget(this), QIcon("://icons/history_message.png"), "历史消息");
    ui->dataTabWidget->addTab(new DebugWidget(service, this), QIcon("://icons/debug.png"), "开发调试");


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
        emit sig->hostChanged(us->host);
    }
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
    QSystemTrayIcon* tray = new QSystemTrayIcon(this);
    tray->setIcon(QIcon("://appicon"));
    tray->setToolTip(APPLICATION_NAME);
    tray->show();

    connect(tray,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(trayAction(QSystemTrayIcon::ActivationReason)));
}

void MainWindow::trayAction(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
    case QSystemTrayIcon::Trigger:
        if (!this->isHidden())
            this->hide();
        else
        {
            this->showNormal();
            this->activateWindow();
        }
        break;
    case QSystemTrayIcon::MiddleClick:
        break;
    case QSystemTrayIcon::Context:
    {
        FacileMenu* menu = new FacileMenu;

        menu->addAction(QIcon("://icons/leaveMode.png"), "临时离开", [=] {
            // 这里的离开模式不会保存，重启后还是以设置中为准
            us->leaveMode = !us->leaveMode;
        })->check(us->leaveMode);

        menu->addAction(QIcon("://icons/silent.png"), "静默模式", [=] {
            // 这里的静默模式不会保存，重启后还是以设置中为准
            rt->notificationSlient = !rt->notificationSlient;
            if (rt->notificationSlient)
                closeAllCard();
        })->check(rt->notificationSlient);

        auto importanceMenu = menu->split()->addMenu(QIcon("://icons/importance.png"), "过滤重要性");
        auto setImportance = [=](int im) {
            us->set("importance/lowestImportance", us->lowestImportance = im);
        };
        importanceMenu->addAction(QIcon("://icons/veryImportant.png"), "很重要", [=]{
            setImportance(VeryImportant);
        })->check(us->lowestImportance == VeryImportant);
        importanceMenu->addAction(QIcon("://icons/important.png"), "重要", [=]{
            setImportance(Important);
        })->check(us->lowestImportance == Important);
        importanceMenu->addAction(QIcon("://icons/normalImportant.png"), "一般", [=]{
            setImportance(NormalImportant);
        })->check(us->lowestImportance == NormalImportant);
        importanceMenu->addAction(QIcon("://icons/unimportant.png"), "不重要", [=]{
            setImportance(Unimportant);
        })->check(us->lowestImportance == Unimportant);

        menu->split()->addAction(QIcon("://icons/quit.png"), "退出", [=] {
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
    service = new CqhttpService(this);

    connect(service, SIGNAL(signalMessage(const MsgBean&)), this, SLOT(showMessage(const MsgBean&)));

    connect(service, SIGNAL(signalMessage(const MsgBean&)), this, SLOT(aiReplyMessage(const MsgBean&)));

    connect(sig, &SignalTransfer::loadGroupMembers, service, [=](qint64 groupId) {
        MyJson json;
        json.insert("action", "get_group_member_list");
        MyJson params;
        params.insert("group_id", groupId);
        json.insert("params", params);
        json.insert("echo", "get_group_member_list:" + snum(groupId));
        service->sendMessage(json.toBa());
    });
}

void MainWindow::initKey()
{
#if defined(ENABLE_SHORTCUT)
    editShortcut = new QxtGlobalShortcut(this);
    QString def_key = us->value("banner/replyKey", "shift+alt+x").toString();
    editShortcut->setShortcut(QKeySequence(def_key));
    connect(editShortcut, &QxtGlobalShortcut::activated, this, [=]() {
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

void MainWindow::closeEvent(QCloseEvent *e)
{
    us->setValue("mainwindow/geometry", this->saveGeometry());

#if defined(ENABLE_TRAY)
    e->ignore();
    this->hide();

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

void MainWindow::showMessage(const MsgBean &msg)
{
    // 静默模式
    if (rt->notificationSlient)
        return ;

    // 判断群组显示开关
    if (msg.isGroup() && !us->isGroupShow(msg.groupId)) // 群组消息开关
        return ;

    // 判断消息级别开关
    int im = NormalImportant;
    if (msg.isPrivate())
        im = us->userImportance.value(msg.senderId, us->userDefaultImportance);
    else if (msg.isGroup())
        im = us->groupImportance.value(msg.groupId, us->groupDefaultImportance);
    if (im < us->lowestImportance)
        return ;

    // 判断有没有现有的卡片
    foreach (auto card, notificationCards)
    {
        if (card->append(msg))
        {
            return ;
        }
    }

    // 没有现有的，新建一个卡片
    createNotificationBanner(msg);
}

void MainWindow::createNotificationBanner(const MsgBean &msg)
{
    QPoint startPos; // 开始出现的位置
    QPoint showPos;  // 显示的最终位置
    switch (int(us->bannerFloatSide))
    {
    case SideRight: // 右
    {
        /* // 统计所有横幅的位置
        int top = us->bannerFloatPixel;
        foreach (auto card, notificationCards)
            top += card->height() + us->bannerSpacing; */
        int top = us->bannerFloatPixel;
        if (notificationCards.size())
            top = notificationCards.last()->geometry().bottom() + us->bannerSpacing;
        startPos = QPoint(screenGeometry().width()-5, top);
        showPos = QPoint(screenGeometry().width() - us->bannerFixedWidth - us->bannerSpacing, top);
    }
        break;
    default:
        qCritical() << "暂不支持该位置";
        return ;
    }

    NotificationCard* card = new NotificationCard(nullptr);
    card->setMsg(msg);
    card->showFrom(startPos, showPos);

    notificationCards.append(card);
    connect(card, &NotificationCard::signalHeightChanged, this, [=](int delta) {
        adjustUnderCardsTop(notificationCards.indexOf(card), delta);
    });
    connect(card, &NotificationCard::signalHided, this, [=]{
        int index = notificationCards.indexOf(card);
        adjustUnderCardsTop(index, -(card->height() + us->bannerSpacing));
        notificationCards.removeOne(card);
    });
    connect(card, &NotificationCard::signalReplyPrivate, this, [=](qint64 userId, const QString& message) {
        MyJson json;
        json.insert("action", "send_private_msg");
        MyJson params;
        params.insert("user_id", userId);
        params.insert("message", message);
        json.insert("params", params);
        json.insert("echo", "send_private_msg");
        service->sendMessage(json.toBa());
        emit sig->myReplyUser(userId, message);
    });
    connect(card, &NotificationCard::signalReplyGroup, this, [=](qint64 groupId, const QString& message) {
        MyJson json;
        json.insert("action", "send_group_msg");
        MyJson params;
        params.insert("group_id", groupId);
        params.insert("message", message);
        json.insert("params", params);
        json.insert("echo", "send_group_msg");
        service->sendMessage(json.toBa());
        emit sig->myReplyGroup(groupId, message);
    });
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
}

/**
 * 调整下面所有卡片的位置
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

/// 聚焦到最近一次有消息的卡片的回复
/// 会忽视短期内的擦片
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
        targetCard->setFastFocus();
        targetCard->showReplyEdit(true);
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

void MainWindow::aiReplyMessage(const MsgBean &msg)
{
    if (msg.isPrivate() && us->leaveMode && us->aiReplyPrivate)
    {
        qint64 userId = msg.senderId;
        qint64 time = QDateTime::currentMSecsSinceEpoch();
        if (ac->aiReplyUserTime.value(userId, 0) + us->aiReplyInterval >= time) // 未到间隔时间
            return ;
        ac->aiReplyUserTime[userId] = time;
        triggerAiReply(msg);
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
        int ret = json.value("ret").toInt();
        if (ret != 0)
        {
            QString rep = json.value("msg").toString();
            qWarning() << "AI回复：" << rep << text;
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
            answer = us->aiReplyPrefix + answer + us->aiReplySuffix;
        }

        json = MyJson();
        json.insert("action", "send_private_msg");
        MyJson params;
        params.insert("user_id", msg.senderId);
        params.insert("message", answer);
        json.insert("params", params);
        json.insert("echo", "send_private_msg");
        service->sendMessage(json.toBa());

        emit sig->myReplyUser(msg.senderId, answer);
    });
}
