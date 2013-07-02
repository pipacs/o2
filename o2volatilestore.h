#ifndef O2VOLATILESTORE_H
#define O2VOLATILESTORE_H

#include <QMap>
#include <QString>

#include "o2abstractstore.h"

class O2VolatileStore : public O2AbstractStore
{
    Q_OBJECT

public:

    explicit O2VolatileStore(QObject *parent = 0);

    ~O2VolatileStore();

    QString value(const QString &key, const QString &defaultValue = QString());
    void setValue(const QString &key, const QString &value);

protected:
    QMap<QString, QString> map_;
};

#endif // O2VOLATILESTORE_H
