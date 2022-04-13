#ifndef DEVCODERUNNER_H
#define DEVCODERUNNER_H

#include <QObject>
#include "msgbean.h"

class DevCodeRunner : public QObject
{
    Q_OBJECT
public:
    DevCodeRunner();

public slots:
    void runCode(const QString& code, const MsgBean& msg);
};

#endif // DEVCODERUNNER_H
