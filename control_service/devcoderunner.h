#ifndef DEVCODERUNNER_H
#define DEVCODERUNNER_H

#include <QObject>
#include "msgbean.h"
#include "mysettings.h"
#include "cqhttpservice.h"

class DevCodeRunner : public QObject
{
    Q_OBJECT
public:
    DevCodeRunner(CqhttpService* cqhttp);

public slots:
    void runCode(const QString& _code, const MsgBean& msg);

protected:
    QString replaceVariants(const QString &key, const MsgBean &msg, bool *ok) const;

    bool runConditionLine(const QString& _line, const MsgBean &msg);

    void executeMultiOperation(const QString& lineSeq, const MsgBean &msg);

    void sendLine(const QString& line, const MsgBean &msg);

    bool executeFunc(const QString& func, const QString& args, const MsgBean &msg);

    QString toSingleLine(QString text) const;
    QString toMultiLine(QString text) const;
    QString toRunableCode(QString text) const;

private:
    CqhttpService* cqhttpService;
};

#endif // DEVCODERUNNER_H
