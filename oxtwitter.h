#ifndef OXTWITTER_H
#define OXTWITTER_H

#include "o1twitter.h"

class OXTwitter : public O1Twitter {
    Q_OBJECT

public:
    explicit OXTwitter(QObject *parent = 0);
    /// Set Twitter login parameters for doing XAuth
    void setXAuthParameters(const QString &username, const QString &password);

public slots:
    /// Authenticate.
    Q_INVOKABLE virtual void link();

private:
    QList<O1RequestParameter> xAuthParams_;
};

#endif // OXTWITTER_H
