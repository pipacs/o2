#ifndef O2ABSTRACTSTORE_H
#define O2ABSTRACTSTORE_H

#include <QObject>
#include <QString>

/// Storage for strings
class O2AbstractStore: public QObject {
    Q_OBJECT

public:
    /// Constructor
    explicit O2AbstractStore(QObject *parent = 0): QObject(parent) {
    }

    /// Retrieve a string value by key
    virtual QString value(const QString &key, const QString &defaultValue = QString()) = 0;

    /// Set a string value for a key
    virtual void setValue(const QString &key, const QString &value) = 0;
};

#endif // O2ABSTRACTSTORE_H
