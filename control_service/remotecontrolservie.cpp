#include <QRegularExpressionMatch>
#include <QUrl>
#include <QProcess>
#include <QApplication>
#include <QClipboard>
#include "remotecontrolservie.h"
#include "fileutil.h"
#include "runtime.h"
#ifdef Q_OS_WIN
#include <windows.h>
#endif
#include "usettings.h"

RemoteControlServie::RemoteControlServie(QObject *parent) : QObject(parent)
{

}

bool RemoteControlServie::execCmd(QString cmd)
{
    if (us->remoteControlPrefixs.size())
    {
        bool find = false;
        foreach (auto prefix, us->remoteControlPrefixs)
        {
            if (!cmd.startsWith(prefix))
                continue ;
            cmd.remove(0, prefix.length());
            find = true;
            break;
        }
        if (!find)
            return false;
    }

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
    }

    else if (isFileExist(path = rt->DATA_PATH + "control/" + cmd + ".vbs"))
    {
        qInfo() << "执行vbs脚本：" << path;
        QDesktopServices::openUrl("file:///" + path);
    }

    else if (hasCmd("打开") &&  (cmd.startsWith("http://") || cmd.startsWith("https://") || cmd.startsWith("file:///")))
    {
        qInfo() << "打开URI：" << cmd;
        QDesktopServices::openUrl(cmd);
    }

    // 输入文字
    else if (hasCmd("输入") && cmd.startsWith("输入 "))
    {
        QString text = cmd.right(cmd.length() - 3);
        qInfo() << "输入文字：" << text;
        QApplication::clipboard()->setText(text);
        simulateKeys("ctrl+v", true);
    }

    // 模拟按键：Ctrl+W
    else if (hasCmd("按键") && cmd.startsWith("按键 "))
    {
        QString text = cmd.right(cmd.length() - 3);
        qInfo() << "模拟按键：" << text;
        simulateKeys(text);
    }

    // 菜单等自定义文字
    else if (hasCmd(cmd))
    {
        emit reply(readTextFile(rt->DATA_PATH + "control/" + cmd));
    }

    else
    {
        return false;
    }
    return true;
}

void RemoteControlServie::simulateKeys(QString seq, bool delay)
{
    if (seq.isEmpty())
        return ;

    // 模拟点击右键
    // mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
    // mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
    // keybd_event(VK_CONTROL, (BYTE) 0, 0, 0);
    // keybd_event('P', (BYTE)0, 0, 0);
    // keybd_event('P', (BYTE)0, KEYEVENTF_KEYUP, 0);
    // keybd_event(VK_CONTROL, (BYTE)0, KEYEVENTF_KEYUP, 0);
#if defined(Q_OS_WIN)
    // 字符串转KEY
    QList<int>keySeq;
    QStringList keyStrs = seq.toLower().split("+", QString::SkipEmptyParts);
    if (keyStrs.contains("ctrl"))
        keySeq.append(VK_CONTROL);
    if (keyStrs.contains("shift"))
        keySeq.append(VK_SHIFT);
    if (keyStrs.contains("alt"))
        keySeq.append(VK_MENU);
    keyStrs.removeOne("ctrl");
    keyStrs.removeOne("shift");
    keyStrs.removeOne("alt");
    for (int i = 0; i < keyStrs.size(); i++)
    {
        QString ch = keyStrs.at(i);
        if (ch.length() != 1)
            continue ;
        if (ch >= "0" && ch <= "9")
            keySeq.append(0x30 + ch.toInt());
        else if (ch >= "a" && ch <= "z")
            keySeq.append(0x41 + ch.at(0).toLatin1() - 'a');
    }

    // 开始模拟
    for (int i = 0; i < keySeq.size(); i++)
    {
        keybd_event(keySeq.at(i), (BYTE) 0, 0, 0);
        if (delay)
            Sleep(100);
    }

    for (int i = 0; i < keySeq.size(); i++)
        keybd_event(keySeq.at(i), (BYTE) 0, KEYEVENTF_KEYUP, 0);
#endif
}

