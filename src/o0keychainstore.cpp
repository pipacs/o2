//
// Created by michaelpollind on 3/13/17.
//
#include "o0keychainstore.h"

#include <QDebug>
#include <keychain.h>
#include <QtCore/QDataStream>
#include <QtCore/QBuffer>
#include <QtCore/QEventLoop>

using namespace QKeychain;

o0keyChainStore::o0keyChainStore(const QString& app,const QString& name,QObject *parent):
    O0AbstractStore(parent), app_(app),name_(name),pairs_()
{
}

QString o0keyChainStore::value(const QString &key, const QString &defaultValue) {
    return pairs_.value(key, defaultValue);
}

void o0keyChainStore::setValue(const QString &key, const QString &value) {
    pairs_.insert(key,value);
}

void o0keyChainStore::persist() {
    WritePasswordJob job(app_);
    initJob(job);

    QByteArray data;
    QDataStream ds(&data,QIODevice::ReadWrite);
    ds << pairs_;
    job.setBinaryData(data);

    executeJob(job, "persist");
}

void o0keyChainStore::fetchFromKeychain() {
    ReadPasswordJob job(app_);
    initJob(job);
    executeJob(job, "fetch");

    QByteArray data;
    data.append(job.binaryData());
    QDataStream ds(&data,QIODevice::ReadOnly);
    ds >> pairs_;
}

void o0keyChainStore::clearFromKeychain() {
    DeletePasswordJob job(app_);
    initJob(job);
    executeJob(job, "clear");
}

void o0keyChainStore::initJob(QKeychain::Job &job) const {
    job.setAutoDelete(false);
    job.setKey(name_);
}

void o0keyChainStore::executeJob(QKeychain::Job &job, const char *actionName) const {
    QEventLoop loop;
    job.connect( &job, SIGNAL(finished(QKeychain::Job*)), &loop, SLOT(quit()) );
    job.start();
    loop.exec();

    const QKeychain::Error errorCode = job.error();
    if (errorCode != QKeychain::NoError) {
        qWarning() << "keychain store could not" << actionName << name_ << ":"
                   << job.errorString() << "(" << errorCode << ").";
    }
}
