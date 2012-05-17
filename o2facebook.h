#ifndef O2FACEBOOK_H
#define O2FACEBOOK_H

#include "o2.h"

/// Facebook's dialect of OAuth 2.0
class O2Facebook: public O2 {
    Q_OBJECT

public:
    explicit O2Facebook(const QString &clientId, const QString &clientSecret, const QString &scope, QObject *parent = 0);

signals:

public slots:

protected slots:
    void onVerificationReceived(QMap<QString, QString>);
};

#endif // O2FACEBOOK_H
