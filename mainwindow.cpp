#include <QDebug>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "widgets/customtabstyle.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tabWidget->setAttribute(Qt::WA_StyledBackground);
    ui->tabWidget->setTabPosition(QTabWidget::West);
    ui->tabWidget->tabBar()->setStyle(new CustomTabStyle);

    ui->sideButtons->setCurrentRow(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadSettingsTabs()
{
    for (int i = 0; i < ui->tabWidget->count(); i++)
        ui->tabWidget->widget(i)->deleteLater();
    ui->tabWidget->clear();

    ui->tabWidget->addTab(new QWidget(), QIcon("://icons/account.png"), "账号绑定");
    ui->tabWidget->addTab(new QWidget(), QIcon("://icons/group.png"), "群组消息");
    ui->tabWidget->addTab(new QWidget(), QIcon("://icons/care.png"), "特别关心");
    ui->tabWidget->addTab(new QWidget(), QIcon("://icons/banner.png"), "横幅通知");
    ui->tabWidget->addTab(new QWidget(), QIcon("://icons/bubble.png"), "气泡样式");
    ui->tabWidget->addTab(new QWidget(), QIcon("://icons/animation.png"), "动画调整");
    ui->tabWidget->addTab(new QWidget(), QIcon("://icons/startup.png"), "程序启动");
}

void MainWindow::loadAuxiliaryTabs()
{
    for (int i = 0; i < ui->tabWidget->count(); i++)
        ui->tabWidget->widget(i)->deleteLater();
    ui->tabWidget->clear();

    ui->tabWidget->addTab(new QWidget(), QIcon("://icons/reply.png"), "快速回复");
    ui->tabWidget->addTab(new QWidget(), QIcon("://icons/ai.png"), "智能聊天");
    ui->tabWidget->addTab(new QWidget(), QIcon("://icons/model.png"), "模型训练");
}

void MainWindow::loadDataTabs()
{
    for (int i = 0; i < ui->tabWidget->count(); i++)
        ui->tabWidget->widget(i)->deleteLater();
    ui->tabWidget->clear();

    ui->tabWidget->addTab(new QWidget(), QIcon("://icons/history_message.png"), "历史消息");
    ui->tabWidget->addTab(new QWidget(), QIcon("://icons/statistical.png"), "数据统计");
}

void MainWindow::on_sideButtons_currentRowChanged(int currentRow)
{
    switch (currentRow)
    {
    case 0:
        loadSettingsTabs();
        break;
    case 1:
        loadAuxiliaryTabs();
        break;
    case 2:
        loadDataTabs();
        break;
    }
}

