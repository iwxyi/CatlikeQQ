#ifndef RUNTIME_H
#define RUNTIME_H

#include <QObject>

class Runtime : public QObject
{
    Q_OBJECT
public:
    explicit Runtime(QObject *parent = nullptr);

signals:

public slots:


public:
    QString APP_PATH;
    QString DATA_PATH;
};

#endif // RUNTIME_H
