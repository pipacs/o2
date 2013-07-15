#include <QDebug>
#include <QDateTime>
#include <QMap>
#include <QString>
#include <QStringList>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

#include "o2facebook.h"
#include "o2globals.h"

static const char *FbEndpoint = "https://graph.facebook.com/oauth/authorize?display=touch";
static const char *FbTokenUrl = "https://graph.facebook.com/oauth/access_token";
static const quint16 FbLocalPort = 1965;

#define SECONDS_IN_2_HOURS (2 * 60 * 60)

O2Facebook::O2Facebook(QObject *parent): O2(parent) {
    setRequestUrl(FbEndpoint);
    setTokenUrl(FbTokenUrl);
    setLocalPort(FbLocalPort);
}

void O2Facebook::onVerificationReceived(const QMap<QString, QString> response) {
    emit closeBrowser();
    if (response.contains("error")) {
        qWarning() << "O2Facebook::onVerificationReceived: Verification failed";
        foreach (QString key, response.keys()) {
            qWarning() << "O2Facebook::onVerificationReceived:" << key << response.value(key);
        }
        emit linkingFailed();
        return;
    }

    // Save access code
    setCode(response.value(O2_OAUTH2_CODE));

    // Exchange access code for access/refresh tokens
    QUrl url(tokenUrl_);
#if QT_VERSION < 0x050000
    url.addQueryItem(O2_OAUTH2_CLIENT_ID, clientId_);
    url.addQueryItem(O2_OAUTH2_CLIENT_SECRET, clientSecret_);
    url.addQueryItem(O2_OAUTH2_SCOPE, scope_);
    url.addQueryItem(O2_OAUTH2_CODE, code());
    url.addQueryItem(O2_OAUTH2_REDIRECT_URI, redirectUri_);
#else
    QUrlQuery query(url);
    query.addQueryItem(O2_OAUTH2_CLIENT_ID, clientId_);
    query.addQueryItem(O2_OAUTH2_CLIENT_SECRET, clientSecret_);
    query.addQueryItem(O2_OAUTH2_SCOPE, scope_);
    query.addQueryItem(O2_OAUTH2_CODE, code());
    query.addQueryItem(O2_OAUTH2_REDIRECT_URI, redirectUri_);
    url.setQuery(query);
#endif

    QNetworkRequest tokenRequest(url);
    QNetworkReply *tokenReply = manager_->get(tokenRequest);
    timedReplies_.add(tokenReply);
    connect(tokenReply, SIGNAL(finished()), this, SLOT(onTokenReplyFinished()), Qt::QueuedConnection);
    connect(tokenReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenReplyError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
}

void O2Facebook::onTokenReplyFinished() {
    QNetworkReply *tokenReply = qobject_cast<QNetworkReply *>(sender());
    if (tokenReply->error() == QNetworkReply::NoError) {

        // Process reply
        QByteArray replyData = tokenReply->readAll();
        QMap<QString, QString> reply;
        foreach (QString pair, QString(replyData).split("&")) {
            QStringList kv = pair.split("=");
            if (kv.length() == 2) {
                reply.insert(kv[0], kv[1]);
            }
        }

        // Interpret reply
        setToken(reply.contains(O2_OAUTH2_ACCESS_TOKEN)? reply.value(O2_OAUTH2_ACCESS_TOKEN): "");
        // FIXME: I have no idea how to interpret Facebook's "expires" value. So let's use a default of 2 hours
        setExpires(QDateTime::currentMSecsSinceEpoch() / 1000 + SECONDS_IN_2_HOURS);
        setRefreshToken(reply.contains(O2_OAUTH2_REFRESH_TOKEN)? reply.value(O2_OAUTH2_REFRESH_TOKEN): "");

        timedReplies_.remove(tokenReply);
        emit linkedChanged();
        emit tokenChanged();
        emit linkingSucceeded();
    }
}

void O2Facebook::unlink() {
    O2::unlink();
    // FIXME: Delete relevant cookies, too
}
