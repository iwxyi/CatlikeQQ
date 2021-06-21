#include <QRegularExpressionMatch>
#include <QUrl>
#include <QProcess>
#include "remotecontrolservie.h"
#include "fileutil.h"
#include "runtime.h"

RemoteControlServie::RemoteControlServie(QObject *parent) : QObject(parent)
{

}

void RemoteControlServie::execCmd(QString cmd)
{
    QRegularExpressionMatch match;
    if (cmd.endsWith(".bat"))
        cmd = cmd.left(cmd.length() - 4);
    QString path;

    auto hasCmd =[=](QString cmd) {
        return isFileExist(rt->DATA_PATH + "control/" + cmd);
    };

    // 执行 bat
    if (isFileExist(path = rt->DATA_PATH + "control/" + cmd + ".bat"))
    {
        qInfo() << "执行bat脚本：" << path;
        QProcess p(nullptr);
        p.start(path);
        if (!p.waitForFinished())
            qWarning() << "执行bat脚本失败：" << path << p.errorString();
        return ;
    }

    if (isFileExist(path = rt->DATA_PATH + "control/" + cmd + ".vbs"))
    {
        qInfo() << "执行vbs脚本：" << path;
        QDesktopServices::openUrl("file:///" + path);
        return ;
    }

    if (hasCmd("打开网址") &&  (cmd.startsWith("http://") || cmd.startsWith("https://")))
    {
        qInfo() << "打开网址" << cmd;
        QDesktopServices::openUrl(cmd);
        return ;
    }

}

