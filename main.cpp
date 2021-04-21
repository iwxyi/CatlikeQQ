#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication::setDesktopSettingsAware(true); // 据说是避免不同分辨率导致显示的字体大小不一致
#if (QT_VERSION > QT_VERSION_CHECK(5, 6, 0))
    // 设置高分屏属性以便支持 2K4K 等高分辨率，尤其是手机 app。
    // 必须写在 main 函数的 QApplication a (argc, argv); 的前面
    // 设置后，读取到的窗口会随着显示器倍数而缩小
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName(APPLICATION_NAME);
    QCoreApplication::setApplicationVersion(VERSION_CODE);

    MainWindow w;
    w.show();
    return a.exec();
}
