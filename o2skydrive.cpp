#include <QDebug>
#include <QDateTime>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QDesktopServices>

#include "o2skydrive.h"

#define trace() if (1) qDebug()
// define trace() if (0) qDebug()

O2Skydrive::O2Skydrive(QObject *parent): O2(parent) {
    setRequestUrl("https://login.live.com/oauth20_authorize.srf");
    setTokenUrl("https://login.live.com/oauth20_token.srf");
    setRefreshTokenUrl("https://login.live.com/oauth20_token.srf");
}

void O2Skydrive::link() {
    trace() << "O2::link";
    if (linked()) {
        trace() << "Linked already";
        return;
    }

    redirectUri_ = QString("https://login.live.com/oauth20_desktop.srf");

    // Assemble intial authentication URL
    QList<QPair<QString, QString> > parameters;
    parameters.append(qMakePair(QString("response_type"), (grantFlow_ == GrantFlowAuthorizationCode)? QString("code"): QString("token")));
    parameters.append(qMakePair(QString("client_id"), clientId_));
    parameters.append(qMakePair(QString("redirect_uri"), redirectUri_));
    parameters.append(qMakePair(QString("scope"), scope_));

    // Show authentication URL with a web browser
    QUrl url(requestUrl_);
    url.setQueryItems(parameters);
    emit openBrowser(url);
}

void O2Skydrive::redirected(const QUrl &url) {
    trace() << "O2::redirected" << url;

    emit closeBrowser();

    if (grantFlow_ == GrantFlowAuthorizationCode) {
        // Get access code
        QString urlCode = url.queryItemValue("code");
        if (urlCode.isEmpty()) {
            trace() << " Code not received";
            emit linkingFailed();
            return;
        }
        setCode(urlCode);

        // Exchange access code for access/refresh tokens
        QNetworkRequest tokenRequest(tokenUrl_);
        tokenRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        QMap<QString, QString> parameters;
        parameters.insert("code", code());
        parameters.insert("client_id", clientId_);
        parameters.insert("client_secret", clientSecret_);
        parameters.insert("redirect_uri", redirectUri_);
        parameters.insert("grant_type", "authorization_code");
        QByteArray data = buildRequestBody(parameters);
        QNetworkReply *tokenReply = manager_->post(tokenRequest, data);
        timedReplies_.add(tokenReply);
        connect(tokenReply, SIGNAL(finished()), this, SLOT(onTokenReplyFinished()), Qt::QueuedConnection);
        connect(tokenReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenReplyError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
    } else {
        // Get access token
        QString urlToken = "";
        QString urlRefreshToken = "";
        int urlExpiresIn = 0;

        QStringList parts = url.toString().split("#");
        if (parts.length() > 1) {
            foreach (QString item, parts[1].split("&")) {
                int index = item.indexOf("=");
                if (index == -1) {
                    continue;
                }
                QString key = item.left(index);
                QString value = item.mid(index + 1);
                trace() << "" << key;
                if (key == "access_token") {
                    urlToken = value;
                } else if (key == "expires_in") {
                    urlExpiresIn = value.toInt();
                } else if (key == "refresh_token") {
                    urlRefreshToken = value;
                }
            }
        }

        setToken(urlToken);
        setRefreshToken(urlRefreshToken);
        setExpires(QDateTime::currentMSecsSinceEpoch() / 1000 + urlExpiresIn);
        if (urlToken.isEmpty()) {
            emit linkingFailed();
        } else {
            emit linkedChanged();
            emit linkingSucceeded();
        }
    }
}
