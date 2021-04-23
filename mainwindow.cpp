#include <QDebug>
#include "facilemenu.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "widgets/customtabstyle.h"
#include "usersettings.h"
#include "widgets/settings/accountwidget.h"
#include "widgets/settings/debugwidget.h"
#include "myjson.h"
#include "fileutil.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initService();
    initView(); // 因为要绑定 service，所以要在 initService 后面
    initTray();

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
    ui->settingsTabWidget->addTab(new QWidget(), QIcon("://icons/group.png"), "群组消息");
    ui->settingsTabWidget->addTab(new QWidget(), QIcon("://icons/care.png"), "特别关心");
    ui->settingsTabWidget->addTab(new QWidget(), QIcon("://icons/banner.png"), "横幅通知");
    ui->settingsTabWidget->addTab(new QWidget(), QIcon("://icons/bubble.png"), "气泡样式");
    ui->settingsTabWidget->addTab(new QWidget(), QIcon("://icons/animation.png"), "动画调整");
    ui->settingsTabWidget->addTab(new QWidget(), QIcon("://icons/startup.png"), "程序启动");

    ui->auxiliaryTabWidget->clear();
    ui->auxiliaryTabWidget->addTab(new QWidget(), QIcon("://icons/reply.png"), "快速回复");
    ui->auxiliaryTabWidget->addTab(new QWidget(), QIcon("://icons/ai.png"), "智能聊天");
    ui->auxiliaryTabWidget->addTab(new QWidget(), QIcon("://icons/model.png"), "模型训练");

    ui->dataTabWidget->clear();
    ui->dataTabWidget->addTab(new DebugWidget(service, this), QIcon("://icons/debug.png"), "开发调试");
    ui->dataTabWidget->addTab(new QWidget(), QIcon("://icons/history_message.png"), "历史消息");
    ui->dataTabWidget->addTab(new QWidget(), QIcon("://icons/statistical.png"), "数据统计");


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
}

void MainWindow::on_sideButtons_currentRowChanged(int currentRow)
{
    ui->stackedWidget->setCurrentIndex(currentRow);
    us->set("mainwindow/sideIndex", currentRow);
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
    return QGuiApplication::screenAt(QCursor::pos())->geometry();
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
            this->showNormal();
        break;
    case QSystemTrayIcon::MiddleClick:

        break;
    case QSystemTrayIcon::Context:
    {
        FacileMenu* menu = new FacileMenu;
        menu->addAction(QIcon("://icons/silent.png"), "静默", [=] {

        });
        menu->addAction(QIcon("://icons/quit.png"), "退出", [=] {
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

void MainWindow::showMessage(const MsgBean &msg)
{
    // 判断现有的有没有
    int delta = 0;
    foreach (auto card, notificationCards)
    {
        if (card->canMerge() && card->append(msg, delta))
        {
            adjustUnderCardsTop(notificationCards.indexOf(card), delta);
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
    switch (int(us->floatSide))
    {
    case SideRight: // 右
    {
        int top = us->floatPixel;
        // 统计所有横幅的位置
        foreach (auto card, notificationCards)
            top += card->height() + us->bannerSpacing;
        startPos = QPoint(screenGeometry().width()-5, top);
        showPos = QPoint(screenGeometry().width() - us->bannerWidth - us->bannerSpacing, top);
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
    connect(card, &NotificationCard::signalHided, this, [=]{
        int index = notificationCards.indexOf(card);
        notificationCards.removeOne(card);
        adjustUnderCardsTop(index, card->height() + us->bannerSpacing);
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
    }
}
