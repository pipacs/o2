#include <QDebug>

#include "o2facebook.h"

static const char *FbEndpoint = "https://graph.facebook.com/oauth/authorize";
static const char *FbTokenUrl = "https://graph.facebook.com/oauth/access_token";
static const char *FbRefreshTokenUrl = "https://graph.facebook.com/oauth/access_token"; // FIXME
static const quint16 FbLocalPort = 1965;

O2Facebook::O2Facebook(const QString &clientId, const QString &clientSecret, const QString &scope, QObject *parent):
    O2(clientId, clientSecret, scope, QUrl(FbEndpoint), QUrl(FbTokenUrl), QUrl(FbRefreshTokenUrl), FbLocalPort, parent) {
}

void O2Facebook::onVerificationReceived(const QMap<QString, QString> response) {
    qDebug() << "> O2Facebook::onVerificationReceived";

    emit closeBrowser();
    if (response.contains("error")) {
        qDebug() << "Verification failed";
        foreach (QString key, response.keys()) {
            qDebug() << "" << key << response.value(key);
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
