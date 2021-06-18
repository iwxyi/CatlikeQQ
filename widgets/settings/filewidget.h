#ifndef FILEWIDGET_H
#define FILEWIDGET_H

#include <QWidget>

namespace Ui {
class FileWidget;
}

class FileWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileWidget(QWidget *parent = nullptr);
    ~FileWidget();

private slots:
    void on_autoCacheImageCheck_clicked();

    void on_autoCacheSmallVideoCheck_clicked();

    void on_autoCachePrivateVideoCheck_clicked();

    void on_autoCacheFileMaxSizeSpin_editingFinished();

    void on_autoCachePrivateFileTypesEdit_editingFinished();

private:
    Ui::FileWidget *ui;
};

#endif // FILEWIDGET_H
