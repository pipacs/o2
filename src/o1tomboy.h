#ifndef O1TOMBOY_H
#define O1TOMBOY_H

#include "o1.h"

class O1Tomboy : public O1 {
    Q_OBJECT
public:
    explicit O1Tomboy(QObject *parent = 0) : O1(parent) {

    }

    void setBaseURL(const QString &value) {
        setRequestTokenUrl(QUrl(value + "/oauth/request_token"));
        setAuthorizeUrl(QUrl(value + "/oauth/authorize"));
        setAccessTokenUrl(QUrl(value + "/oauth/access_token"));
        setClientId("anyone");
        setClientSecret("anyone");
    }

    QString getRequestToken() {
        return requestToken_;
    }

    QString getRequestTokenSecret() {
        return requestTokenSecret_;
    }

    void restoreAuthData(const QString &token, const QString &secret) {
        requestToken_ = token;
        requestTokenSecret_ = secret;
        setLinked(true);
    }

};

#endif // O1TOMBOY_H
