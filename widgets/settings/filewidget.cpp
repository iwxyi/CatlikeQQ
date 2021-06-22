#include "filewidget.h"
#include "ui_filewidget.h"
#include "usettings.h"

FileWidget::FileWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileWidget)
{
    ui->setupUi(this);

    ui->autoCacheImageCheck->setChecked(us->autoCacheImage);
    ui->autoCachePrivateVideoCheck->setChecked(us->autoCachePrivateVideo);
    ui->autoCacheSmallVideoCheck->setChecked(us->autoCacheSmallVideo);
    ui->autoCachePrivateFileCheck->setChecked(us->autoCachePrivateVideo);
    ui->autoCachePrivateFileTypeCheck->setChecked(us->autoCachePrivateFileType);
    ui->autoCachePrivateFileTypesEdit->setText(us->autoCachePrivateFileTypes.join(" "));
    ui->autoCacheFileMaxSizeSpin->setValue(us->autoCacheFileMaxSize);
}

FileWidget::~FileWidget()
{
    delete ui;
}

void FileWidget::on_autoCacheImageCheck_clicked()
{
    us->set("autoCache/image", us->autoCacheImage = ui->autoCacheImageCheck->isChecked());
}

void FileWidget::on_autoCacheSmallVideoCheck_clicked()
{
    us->set("autoCache/smallVideo", us->autoCacheSmallVideo = ui->autoCacheSmallVideoCheck->isChecked());
}

void FileWidget::on_autoCachePrivateVideoCheck_clicked()
{
    us->set("autoCache/privateVideo", us->autoCachePrivateVideo = ui->autoCachePrivateVideoCheck->isChecked());
}

void FileWidget::on_autoCacheFileMaxSizeSpin_editingFinished()
{
    us->set("autoCache/fileMaxSize", us->autoCacheFileMaxSize = ui->autoCacheFileMaxSizeSpin->value());
}

void FileWidget::on_autoCachePrivateFileTypesEdit_editingFinished()
{
    QString s = ui->autoCachePrivateFileTypesEdit->text();
    us->set("autoCache/privateFileTypes", s);
    us->autoCachePrivateFileTypes = s.split(" ", QString::SkipEmptyParts);
}

void FileWidget::on_autoCachePrivateFileCheck_clicked()
{
    us->set("autoCache/privateFile", us->autoCachePrivateFile = ui->autoCachePrivateFileCheck->isChecked());
}

void FileWidget::on_autoCachePrivateFileTypeCheck_clicked()
{
    us->set("autoCache/privateFileType", us->autoCachePrivateFileType = ui->autoCachePrivateFileTypeCheck->isChecked());
}
