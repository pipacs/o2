#ifndef O2SKYDRIVE_H
#define O2SKYDRIVE_H

#include "o2.h"

/// Skydrive's dialect of OAuth 2.0
class O2Skydrive: public O2 {
    Q_OBJECT

public:
    explicit O2Skydrive(QObject *parent = 0);

#if 0
public slots:
    Q_INVOKABLE void unlink();

protected slots:
    void onVerificationReceived(QMap<QString, QString>);
    virtual void onTokenReplyFinished();
#endif
};

#endif // O2SKYDRIVE_H
