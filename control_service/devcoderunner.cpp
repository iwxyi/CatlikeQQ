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
    }

    // 分割行，获取每一行代码
    QStringList lines = code.split("\n", QString::SkipEmptyParts);

    for (int i = 0; i < lines.size(); i++)
    {
        // 判断每一行
        if (runConditionLine(lines[i], msg))
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
    case "%message%"_shash:
        return msg.message;
    case "%sender_id%"_shash:
        return snum(msg.senderId);
    case "%nickname%"_shash:
        return msg.nickname;
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

    default:
        *ok = false;
        return "";
    }
}

/**
 * 执行带有先行条件的命令
 * 从上往下的优先级来执行
 */
bool DevCodeRunner::runConditionLine(const QString &line, const MsgBean& msg)
{
    QRegularExpressionMatch match;
    auto canMatch = [&](const QString& re) -> bool {
        return line.contains(QRegularExpression(re), &match);
    };
    if (canMatch("^\\[.*\\]$"))
    {
        // 事件判断，目前忽略
    }
    else if (canMatch("^\\s*(.*?)\\s*(?:-->|=>)\\s*(.*)\\s*$"))
    {
        // 表达式 -> 回复的消息
        const QString& re = match.captured(1);
        const QString& code = match.captured(2);
        if (!msg.message.contains(QRegularExpression(re)))
            return false;
        qInfo() << "发送内容：" << code;
        if (code.contains(QRegularExpression(re)))
        {
            qWarning() << "已阻止重复风险的消息：" << re << "匹配" << code;
            return true;
        }
        sendLine(code, msg);
        return true;
    }
    else if (canMatch("^\\s*\\[\\[\\[(.*)\\]\\]\\]\\s*(.+)")
             || canMatch("^\\s*\\[\\[(.*)\\]\\]\\s*(.+)")
             || canMatch("^\\s*\\[(.*)\\]\\s*(.+)"))
    {
        // [条件1,条件2,...] 回复的消息
        const QString& conditionStr = match.captured(1);
        const QString& content = match.captured(2);
        if (!ConditionUtil::judgeCondition(conditionStr))
            return false;
        qInfo() << "发送内容：" << content;
        QStringList codes = content.split("\\n");
        for (auto code: codes)
            sendLine(code, msg);
        return true;
    }
    return false;
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
        if (!executeFunc(func, args))
            qWarning() << "无法解析的命令：" << func << args;
    }
    else
    {
        // 普通文字
        if (msg.isGroup())
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
bool DevCodeRunner::executeFunc(const QString &func, const QString &args)
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
        if (!parseArgs("^(.+)\\s*,\\s*(.*)$"))
            return false;
        const QString& url = match.captured(1);
        const QString& data = match.captured(2);
        QString result = NetUtil::postWebData(url, data);
        qInfo() << result;
    }

    else if (func == "postJson")
    {
        if (!parseArgs("^(.+)\\s*,\\s*(.*)$"))
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

    else
    {
        return false;
    }
    return true;
}
