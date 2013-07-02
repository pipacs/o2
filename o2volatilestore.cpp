#include "o2volatilestore.h"

O2VolatileStore::O2VolatileStore(QObject *parent): O2AbstractStore(parent) {
}

O2VolatileStore::~O2VolatileStore() {
}

QString O2VolatileStore::value(const QString &key, const QString &defaultValue) {
    return map_.value(key, defaultValue);
}

void O2VolatileStore::setValue(const QString &key, const QString &value) {
    map_.insert(key, value);
}
