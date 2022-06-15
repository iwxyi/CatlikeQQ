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

    ui->qqButton->adjustSize();
    ui->qqButton->setBorderWidth(1);
    ui->qqButton->setBorderColor(Qt::gray);
    ui->qqButton->setFixedForeSize();
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

void AboutWidget::on_qqButton_clicked()
{
    QDesktopServices::openUrl(QUrl("https://qm.qq.com/cgi-bin/qm/qr?k=L0lcp-KvdNkJDUqEVi6j5hBpGdD77lK5&jump_from=webapi"));
}

void AboutWidget::on_label_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(QUrl(link));
}
