#ifndef O2FACEBOOK_H
#define O2FACEBOOK_H

#include "o2.h"

/// Facebook's dialect of OAuth 2.0
class O2Facebook: public O2 {
    Q_OBJECT

public:
    explicit O2Facebook(QObject *parent = 0);

public slots:
    Q_INVOKABLE void unlink();

protected slots:
    void onVerificationReceived(QMap<QString, QString>);
    virtual void onTokenReplyFinished();
};

#endif // O2FACEBOOK_H
