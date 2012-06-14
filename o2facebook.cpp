#include <QDebug>
#include <QDateTime>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QDesktopServices>

#include "o2facebook.h"

static const char *FbEndpoint = "https://graph.facebook.com/oauth/authorize?display=touch";
static const char *FbTokenUrl = "https://graph.facebook.com/oauth/access_token";
static const quint16 FbLocalPort = 1965;

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
    setCode(response.value(QString("code")));

    // Exchange access code for access/refresh tokens
    QUrl url(tokenUrl_);
    url.addQueryItem("client_id", clientId_);
    url.addQueryItem("client_secret", clientSecret_);
    url.addQueryItem("scope", scope_);
    url.addQueryItem("code", code());
    url.addQueryItem("redirect_uri", redirectUri_);

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
        setToken(reply.contains("access_token")? reply.value("access_token"): "");
        // FIXME: I have no idea how to interpret Facebook's "expires" value. So let's use a default of 2 hours
        setExpires(QDateTime::currentMSecsSinceEpoch() / 1000 + 2 * 60 * 60);
        setRefreshToken(reply.contains("refresh_token")? reply.value("refresh_token"): "");

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
