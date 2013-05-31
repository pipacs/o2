#include <QDateTime>
#include <QDebug>

#include "oxtwitter.h"
#include "o2globals.h"

#define trace() if (1) qDebug()

const char XAUTH_USERNAME[] = "x_auth_username";
const char XAUTH_PASSWORD[] = "x_auth_password";
const char XAUTH_MODE[] = "x_auth_mode";
const char XAUTH_MODE_VALUE[] = "client_auth";

OXTwitter::OXTwitter(QObject *parent): O1Twitter(parent) {
}

void OXTwitter::setXAuthParameters(const QString &username, const QString &password) {
    xAuthParams_.append(O1RequestParameter(QByteArray(XAUTH_USERNAME), username.toLatin1()));
    xAuthParams_.append(O1RequestParameter(QByteArray(XAUTH_PASSWORD), password.toLatin1()));
    xAuthParams_.append(O1RequestParameter(QByteArray(XAUTH_MODE), QByteArray(XAUTH_MODE_VALUE)));
}

void OXTwitter::link() {
    trace() << "OXTwitter::link";
    if (linked()) {
        trace() << "Linked already";
        return;
    }

    if (xAuthParams_.isEmpty()) {
        qWarning() << "Error: XAuth parameters not set. Aborting!";
        return;
    }

    QList<O1RequestParameter> oauthParams;
    oauthParams.append(O1RequestParameter(O2_OAUTH_SIGNATURE_METHOD, O2_SIGNATURE_TYPE_HMAC_SHA1));
    oauthParams.append(O1RequestParameter(O2_OAUTH_CONSUMER_KEY, clientId().toLatin1()));
    oauthParams.append(O1RequestParameter(O2_OAUTH_VERSION, "1.0"));
    oauthParams.append(O1RequestParameter(O2_OAUTH_TIMESTAMP, QString::number(QDateTime::currentDateTimeUtc().toTime_t()).toLatin1()));
    oauthParams.append(O1RequestParameter(O2_OAUTH_NONCE, nonce()));
    oauthParams.append(O1RequestParameter(O2_OAUTH_TOKEN, QByteArray("")));
    oauthParams.append(O1RequestParameter(O2_OAUTH_VERFIER, QByteArray("")));

    QByteArray signature = sign(oauthParams, xAuthParams_, accessTokenUrl(), QNetworkAccessManager::PostOperation, clientSecret(), "");
    oauthParams.append(O1RequestParameter(O2_OAUTH_SIGNATURE, signature));

    // Post request
    QNetworkRequest request(accessTokenUrl());
    request.setRawHeader(O2_HTTP_AUTHORIZATION_HEADER, buildAuthorizationHeader(oauthParams));
    request.setHeader(QNetworkRequest::ContentTypeHeader, O2_MIME_TYPE_XFORM);
    QNetworkReply *reply = manager_->post(request, createQueryParams(xAuthParams_));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenExchangeError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(finished()), this, SLOT(onTokenExchangeFinished()));
}
