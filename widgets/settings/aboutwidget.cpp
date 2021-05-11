#include <QDesktopServices>
#include "aboutwidget.h"
#include "ui_aboutwidget.h"

AboutWidget::AboutWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AboutWidget)
{
    ui->setupUi(this);

    ui->officialButton->adjustSize();
    ui->officialButton->setBorderWidth(1);
    ui->officialButton->setBorderColor(Qt::gray);
    ui->officialButton->setFixedForeSize();

    ui->githubButton->adjustSize();
    ui->githubButton->setBorderWidth(1);
    ui->githubButton->setBorderColor(Qt::gray);

    ui->githubButton->setFixedForeSize();
}

AboutWidget::~AboutWidget()
{
    delete ui;
}

void AboutWidget::on_officialButton_clicked()
{
    QDesktopServices::openUrl(QUrl("http://lyixi.com"));
}

void AboutWidget::on_githubButton_clicked()
{
    QDesktopServices::openUrl(QUrl("https://github.com/iwxyi/CatlikeQQ"));
}
