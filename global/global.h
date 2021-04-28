#ifndef GLOBAL_H
#define GLOBAL_H

#include "runtime.h"
#include "usettings.h"
#include "accountinfo.h"
#include "signaltransfer.h"

#define snum(x) QString::number(x)

extern QString APPLICATION_NAME;
extern QString VERSION_CODE;

extern Runtime* rt;
extern USettings* us;
extern AccountInfo* ac;
extern SignalTransfer* sig;

#endif // GLOBAL_H
