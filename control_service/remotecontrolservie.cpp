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
    QString batPath = rt->DATA_PATH + "control/" + cmd + ".bat";
    if (isFileExist(batPath))
    {
        qInfo() << "执行脚本：" << batPath;
        QProcess p(nullptr);
        p.start(batPath);
        if (!p.waitForFinished())
            qWarning() << "执行bat脚本失败：" << batPath << p.errorString();
        return ;
    }
}
