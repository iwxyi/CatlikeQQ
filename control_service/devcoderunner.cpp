#include <QRegularExpression>
#include "devcoderunner.h"
#include "runtime.h"
#include "usettings.h"
#include "accountinfo.h"
#include "signaltransfer.h"
#include "cqhttpservice.h"
#include "conditionutil.h"
#include "netutil.h"
#include "myjson.h"
#include "stringutil.h"

#define CALC_DEB if (0) qDebug() // 输出计算相关的信息

DevCodeRunner::DevCodeRunner(CqhttpService *cqhttp) : cqhttpService(cqhttp)
{
}

void DevCodeRunner::runCode(const QString &_code, const MsgBean &msg)
{
    QString code = _code;
    QRegularExpressionMatch match;
    QRegularExpression re;

    // 最前面的标记 :-a全部执行
    bool arg_executeAll = false; // 符合条件的代码行是全部执行还是只执行第一个
    if (code.startsWith(":"))
    {
        int pos = code.indexOf("\n");
        if (pos == -1)
            pos = code.length();
        QString argLine = code.left(pos);
        code = code.right(code.length() - pos - 1);

        // 判断参数
        QStringList args = argLine.split(" ", QString::SkipEmptyParts);
        if (args.contains("-a"))
            arg_executeAll = true;
    }

    // 去掉注释
    re = QRegularExpression("(?<!:)//.*?(?=\\n|$|\\\\n)");
    code.replace(re, "");

    // 软换行符
    re = QRegularExpression("\\s*\\\\\\s*\\n\\s*");
    code.replace(re, "");

    // 替换变量
    re = QRegularExpression("%[\\w_]+%");
    int matchPos = 0;
    bool ok;
    while ((matchPos = code.indexOf(re, matchPos, &match)) > -1)
    {
        QString mat = match.captured(0);
        QString rpls = replaceVariants(mat, msg, &ok);
        if (ok)
        {
            code.replace(mat, rpls);
            matchPos = matchPos + rpls.length();
        }
        else
        {
            if (mat.length() > 1 && mat.endsWith("%"))
                matchPos += mat.length() - 1;
            else
                matchPos += mat.length();
        }
    }

    // 替换可变内容
    bool find = true;
    while (find)
    {
        find = false;

        // 读取配置文件的变量
        re = QRegularExpression("%\\{([^(%(\\{|\\[|>))]*?)\\}%");
        while (code.indexOf(re, 0, &match) > -1)
        {
            QString _var = match.captured(0);
            QString key = match.captured(1);
            QVariant var = heaps->value(key);
            code.replace(_var, var.toString()); // 默认使用变量类型吧
            find = true;
        }

        // 进行数学计算的变量
        re = QRegularExpression("%\\[([^(%(\\{|\\[|>))]*?)\\]%");
        while (code.indexOf(re, 0, &match) > -1)
        {
            QString _var = match.captured(0);
            QString text = match.captured(1);
            text = snum(ConditionUtil::calcIntExpression(text));
            code.replace(_var, text); // 默认使用变量类型吧
            find = true;
        }

        // 函数替换
        /* re = QRegularExpression("%>(\\w+)\\s*\\(([^(%(\\{|\\[|>))]*?)\\)%");
        matchPos = 0;
        while ((matchPos = code.indexOf(re, matchPos, &match)) > -1)
        {
            QString rpls = replaceDynamicFunctions(match.captured(1), match.captured(2), msg);
            code.replace(match.captured(0), rpls);
            matchPos += rpls.length();
            find = true;
        } */
    }

    // 分割行，获取每一行代码
    QStringList lines = code.split("\n", QString::SkipEmptyParts);
    for (int i = 0; i < lines.size(); i++)
    {
        // 判断每一行，从上到下
        // 要么执行全部符合条件的，要么直到第一条能执行的
        if (runConditionLine(lines[i], msg) && (!arg_executeAll || lines[i].contains(">break()")))
            return ;
    }
}

/**
 * 替换变量
 */
QString DevCodeRunner::replaceVariants(const QString &key, const MsgBean& msg, bool* ok) const
{
    *ok = true;

    switch (shash_(key))
    {
    case "%is_me%"_shash:
        return ac->myId == msg.senderId ? "1" : "0";
    case "%message%"_shash:
        return toSingleLine(msg.message);
    case "%message_line%"_shash:
        return QString(msg.message).replace("\n", " ");
    case "%pure_text%"_shash:
        return toSingleLine(msg.message); // TODO:去掉CQ码之后的纯文字
    case "%sender_id%"_shash:
        return snum(msg.senderId);
    case "%nickname%"_shash:
        return msg.nickname;
    case "%title%"_shash:
        return msg.titleName();
    case "%message_id%"_shash:
        return snum(msg.messageId);
    case "%sub_type%"_shash:
        return msg.subType;
    case "%remark%"_shash:
        return msg.remark;
    case "%target_id%"_shash:
        return snum(msg.targetId);
    case "%friend_id%"_shash:
        return snum(msg.friendId);
    case "%group_id%"_shash:
        return snum(msg.groupId);
    case "%group_name%"_shash:
        return msg.groupName;
    case "%group_card%"_shash:
        return msg.groupCard;
    case "%from_group_id%"_shash:
        return snum(msg.fromGroupId);
    case "%file_id%"_shash:
        return msg.fileId;
    case "%file_name%"_shash:
        return msg.fileName;
    case "%file_size%"_shash:
        return snum(msg.fileSize);
    case "%file_url%"_shash:
        return msg.fileUrl;
    case "%image_id%"_shash:
        return msg.imageId;
    case "%timestamp%"_shash:
        return snum(msg.timestamp);
    case "%display_message%"_shash:
        return msg.displayMessage();
    case "%display_nickname%"_shash:
        return msg.displayNickname();
    case "%key_id%"_shash:
        return snum(msg.keyId());
    case "%time%"_shash:
        return QDateTime::fromMSecsSinceEpoch(msg.timestamp).toString("yyyy-MM-dd hh:mm:ss");
    case "%my_id%"_shash:
        return snum(ac->myId);
    case "%my_nickname%"_shash:
        return ac->myNickname;
    case "%send_msg_count%"_shash: // 用户发送总数量
    case "%msg_count%"_shash:
        if (msg.isPrivate())
            return snum(heaps->i("private_msg_count/" + snum(msg.senderId)));
        else if (msg.isGroup())
            return snum(heaps->i("group_member_msg_count/" + snum(msg.groupId) + "_" + snum(msg.senderId)));
        else
            return "0";
    case "%repeat_msg_count%"_shash: // 该用户上面连续重复消息数量，包括这一次
    case "%repeat%"_shash:
    {
        QList<MsgBean>* pList = nullptr;
        if (msg.isPrivate())
        {
            pList = &ac->userMsgHistory[msg.senderId];
        }
        else if (msg.isGroup())
        {
            pList = &ac->groupMsgHistory[msg.groupId];
        }
        else
            return "0";
        int count = 0;
        for (int i = pList->size() - 1; i >= 0; i--)
        {
            if (pList->at(i).senderId != msg.senderId || pList->at(i).message != msg.message)
                break;
            else
                count++;
        }
        return snum(count);
    }
    case "%brush_msg_count%"_shash: // 该用户本次连续消息数量，包括这一次
    case "%brush%"_shash:
    {
        QList<MsgBean>* pList = nullptr;
        if (msg.isPrivate())
        {
            pList = &ac->userMsgHistory[msg.senderId];
        }
        else if (msg.isGroup())
        {
            pList = &ac->groupMsgHistory[msg.groupId];
        }
        else
            return "0";
        int count = 0;
        for (int i = pList->size() - 1; i >= 0; i--)
        {
            if (pList->at(i).senderId != msg.senderId)
                break;
            else
                count++;
        }
        return snum(count);
    }

    default:
        *ok = false;
        return "";
    }
}

/**
 * 执行带有先行条件的命令
 * 从上往下的优先级来执行
 */
bool DevCodeRunner::runConditionLine(const QString &_line, const MsgBean& msg)
{
    QString line = _line;
    QRegularExpressionMatch match;
    auto canMatch = [&](const QString& re) -> bool {
        return line.contains(QRegularExpression(re), &match);
    };
    auto hasCondition = [&]() -> bool { // 带有条件的判断：[condition]操作
        return canMatch("^\\s*\\[\\[\\[(.*)\\]\\]\\]\\s*(.+)")
                         || canMatch("^\\s*\\[\\[(.*)\\]\\]\\s*(.+)")
                         || canMatch("^\\s*\\[(.*)\\]\\s*(.+)");
    };
    if (canMatch("^\\[.*\\]$"))
    {
        // 事件判断，目前忽略
    }
    else if (canMatch("^\\s*(.*?)\\s*(?:-->|=>)\\s*(.*)\\s*$")) // 简略消息：正则 => 操作
    {
        // 表达式 -> 回复的消息
        const QString& re = match.captured(1);
        const QString& code = match.captured(2);
        if (!msg.message.contains(QRegularExpression(re))) // 字符包含情况
            return false;
        qInfo() << "文字条件执行：" << code;
        if (code.contains(QRegularExpression(re)))
        {
            qWarning() << "已阻止重复风险的消息：" << re << "匹配" << code;
            return true;
        }

        line = code;
        if (hasCondition()) // 正则 => [条件] 操作
        {
            qDebug() << "跳转到条件1" << match.capturedTexts();
            goto CONTAINS_CONDITION_CODE;
        }
        else // 正则 => 操作
        {
            executeMultiOperation(code, msg);
        }
        return true;
    }
    else if (hasCondition())
    {
CONTAINS_CONDITION_CODE:
        // [条件1,条件2,...] 回复的消息
        const QString& conditionStr = match.captured(1);
        const QString& content = match.captured(2);
        if (!ConditionUtil::judgeCondition(conditionStr))
            return false;
        qInfo() << "符合条件执行：" << content;
        executeMultiOperation(content, msg);
        return true;
    }
    else
    {
        qWarning() << "无法执行的代码";
    }
    return false;
}

/**
 * 执行多行的代码命令
 * 按顺序依次执行
 */
void DevCodeRunner::executeMultiOperation(const QString &lineSeq, const MsgBean &msg)
{
    QStringList codes = lineSeq.split("\\n");
    for (auto code: codes)
        sendLine(code, msg);
}

/**
 * 执行一行命令，或发送一行消息
 */
void DevCodeRunner::sendLine(const QString &line, const MsgBean& msg)
{
    QRegularExpressionMatch match;
    if (line.contains(QRegularExpression("^\\s*>\\s*(\\w+)\\s*\\((.*)\\)\\s*$"), &match))
    {
        // 执行命令
        const QString& func = match.captured(1);
        const QString& args = match.captured(2);
        if (!executeFunc(func, args, msg))
            qWarning() << "无法解析的命令：" << func << args;
    }
    else
    {
        // 普通文字
        if (line.trimmed().isEmpty())
            ; // 空文本，不做操作
        else if (msg.isGroup())
            cqhttpService->sendGroupMsg(msg.groupId, line);
        else if (msg.isPrivate())
            cqhttpService->sendPrivateMsg(msg.friendId, line, msg.fromGroupId);
    }
}

/**
 * 执行命令操作
 * @param func 函数名
 * @param args 参数字符串
 * @param msg  消息对象
 */
bool DevCodeRunner::executeFunc(const QString &func, const QString &args, const MsgBean& msg)
{
    qInfo() << "执行代码：" << func << args;
    QRegularExpressionMatch match;
    auto parseArgs = [&](const QString& re) -> bool {
        return args.contains(QRegularExpression(re), &match);
    };

    if (func == "getData")
    {
        QByteArray result = NetUtil::getWebData(args.trimmed());
        qInfo() << result;
    }

    else if (func == "postData")
    {
        if (!parseArgs("^(.+?)\\s*,\\s*(.*)$"))
            return false;
        const QString& url = match.captured(1);
        const QString& data = match.captured(2);
        QString result = NetUtil::postWebData(url, data);
        qInfo() << result;
    }

    else if (func == "postJson")
    {
        if (!parseArgs("^(.+?)\\s*,\\s*(.*)$"))
            return false;
        const QString& url = match.captured(1);
        const QString& data = match.captured(2);
        const MyJson json = MyJson::from(data.toLocal8Bit());
        QString result = NetUtil::postJsonData(url, json);
        qInfo() << result;
    }

    else if (func == "setValue")
    {
        if (!parseArgs("^\\s*(\\S+?)\\s*,\\s*(.*)$"))
            return false;
        const QString& key = match.captured(1);
        const QString& val = match.captured(2);
        heaps->setValue(key, val);
        qInfo() << "set value:" << key << "=" << val;
    }

    else if (func == "sendCqWs")
    {
        if (!parseArgs("^(.*)$"))
            return false;
        const QString& text = match.captured(1);
        emit sig->sendSocketText(text);
    }

    else if (func == "recallMessage" || func == "recallGroupMessage")
    {
        if (parseArgs("^(\\s*)$"))
        {
            emit sig->recallMessage(msg.senderId, msg.groupId, msg.messageId);
        }
        else if (parseArgs("^(\\d+)$"))
        {
            qint64 messageId = match.captured(1).toLongLong();
            emit sig->recallMessage(msg.senderId, msg.groupId, messageId);
        }
        else
            return false;
    }

    // 撤销该用户自程序启动以来所有消息（不包括云端历史）
    else if (func == "recallMessage_UserAll")
    {
        QList<MsgBean>* pList = nullptr;
        if (msg.isPrivate())
        {
            pList = &ac->userMsgHistory[msg.senderId];
        }
        else if (msg.isGroup())
        {
            pList = &ac->groupMsgHistory[msg.groupId];
        }
        else
            return false;
        for (int i = pList->size() - 1; i >= 0; i--)
        {
            if (pList->at(i).senderId == msg.senderId)
            {
                emit sig->recallMessage(msg.senderId, msg.groupId, pList->at(i).messageId);
            }
        }
    }

    // 撤销该消息及上面所有连续的该用户消息
    else if (func == "recallMessage_UserNear")
    {
        QList<MsgBean>* pList = nullptr;
        if (msg.isPrivate())
        {
            pList = &ac->userMsgHistory[msg.senderId];
        }
        else if (msg.isGroup())
        {
            pList = &ac->groupMsgHistory[msg.groupId];
        }
        else
            return false;
        for (int i = pList->size() - 1; i >= 0; i--)
        {
            if (pList->at(i).senderId != msg.senderId || pList->at(i).message != msg.message)
                break;
            else
                emit sig->recallMessage(msg.senderId, msg.groupId, pList->at(i).messageId);
        }
    }

    else if (func == "setGroupBan" || func == "banUser")
    {
        if (!parseArgs("^(\\d+)\\s*(.*?)$"))
            return false;
        qint64 val = match.captured(1).toLongLong();
        qint64 duration = val;
        QString unit = match.captured(2);
        if (unit.length() >= 2)
            unit = unit.toLower();
        if (unit.length() > 2 && unit.endsWith("s"))
            unit = unit.left(unit.length() - 1);
        if (unit == "") // 默认是分钟
            duration = val * 60;
        else if (unit == "ms" || unit == "msecond" || unit == "millisecond" || unit == "毫秒")
            duration /= 1000;
        else if (unit == "s" || unit == "second" || unit == "秒")
            duration = val;
        else if (unit == "m" || unit == "minute" || unit == "分" || unit == "分钟")
            duration = val * 60;
        else if (unit == "h" || unit == "hour" || unit == "时" || unit == "小时")
            duration = val * 60 * 60;
        else if (unit == "d" || unit == "day" || unit == "天")
            duration = val * 60 * 60 * 24;
        else if (unit == "w" || unit == "week" || unit == "weekend" || unit == "周" || unit == "星期")
            duration = val * 60 * 60 * 24;
        else if (unit == "M" || unit == "month" || unit == "月")
            duration = val * 60 * 60 * 24 * 30;

        emit sig->setGroupBan(msg.groupId, msg.senderId, duration);
    }

    else if (func == "log")
    {
        if (!parseArgs("^(.+?)$"))
            return false;

        qInfo() << match.captured(1);
    }

    else if (func == "break" || func == "continue")
    {
        // 这里什么都不操作，只是标记
    }

    else
    {
        return false;
    }
    return true;
}

/**
 * 从不安全的输入方式读取到的文本，如读取txt
 * 转换为代码中可以解析的安全的文本
 */
QString DevCodeRunner::toSingleLine(QString text) const
{
    return text.replace("\n", "%n%").replace("\\n", "%m%");
}

/**
 * 从代码中解析到的文本，变为可以保存的文本
 */
QString DevCodeRunner::toMultiLine(QString text) const
{
    return text.replace("%n%", "\n").replace("%m%", "\\n");
}

QString DevCodeRunner::toRunableCode(QString text) const
{
    return text.replace("\\%", "%");
}
