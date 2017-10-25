//
// Created by michaelpollind on 3/13/17.
//
#ifndef O2_O0KEYCHAINSTORE_H
#define O2_O0KEYCHAINSTORE_H

#include <QtCore/QMap>
#include "o0abstractstore.h"
#include <QString>

namespace QKeychain {
class Job;
}

/// Calling persist(), fetchFromKeychain() and clearFromKeychain() member
/// functions is the responsibility of the user of this class.
/// This is important to minimize the number of keychain accesses (and
/// potentially the number of user password prompts).
/// For example: fetchFromKeychain() can be called immediately after
/// creating a keychain store; persist() - after a successful authorization;
/// clearFromKeychain() - when the user logs out from the service.
class O0_EXPORT o0keyChainStore  : public  O0AbstractStore{
    Q_OBJECT
public:
    explicit o0keyChainStore(const QString& app,const QString& name,QObject *parent = 0);

    /// Retrieve a string value by key.
    QString value(const QString &key, const QString &defaultValue = QString());

    /// Set a string value for a key.
    void setValue(const QString &key, const QString &value);

    void persist();
    void fetchFromKeychain();
    void clearFromKeychain();
private:
    void initJob(QKeychain::Job &job) const;
    void executeJob(QKeychain::Job &job, const char *actionName) const;

    QString app_;
    QString name_;
    QMap<QString,QString> pairs_;

};


#endif //O2_O0KEYCHAINSTORE_H
