#include <QDebug>
#include <QDateTime>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QDesktopServices>

#include "o2skydrive.h"

#define trace() if (1) qDebug()
// define trace() if (0) qDebug()

static const char *sbEndpoint = "https://login.live.com/oauth20_authorize.srf";
static const char *sbTokenUrl = "https://login.live.com/oauth20_token.srf";

// static const quint16 sbLocalPort = 1965;

O2Skydrive::O2Skydrive(QObject *parent): O2(parent) {
    setRequestUrl(sbEndpoint);
    setTokenUrl(sbTokenUrl);
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
    // parameters.append(qMakePair(QString("redirect_uri"), redirectUri_));
    parameters.append(qMakePair(QString("redirect_uri"), QString(QUrl::toPercentEncoding(redirectUri_))));
    parameters.append(qMakePair(QString("scope"), scope_));

    // Show authentication URL with a web browser
    QUrl url(requestUrl_);
    url.setQueryItems(parameters);
    trace() << " Emit openBrowser" << url.toString();
    emit openBrowser(url);

    // Access code can only be retrieved from the browser, upon detecting redirection to https://login.live.com/oauth20_desktop.srf.
    // After retrieved, the browser client should set it using "setToken".
}

#if 0

void O2Skydrive::onVerificationReceived(const QMap<QString, QString> response) {
    qDebug() << "> O2Skydrive::onVerificationReceived";

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

void O2Skydrive::onTokenReplyFinished() {
    qDebug() << "> O2Skydrive::onTokenReplyFinished";
    QNetworkReply *tokenReply = qobject_cast<QNetworkReply *>(sender());
    if (tokenReply->error() == QNetworkReply::NoError) {

        // Process reply
        QByteArray replyData = tokenReply->readAll();
        QMap<QString, QString> reply;
        foreach (QString pair, QString(replyData).split("&")) {
            QStringList kv = pair.split("=");
            if (kv.length() == 2) {
                qDebug() << kv[0];
                reply.insert(kv[0], kv[1]);
            }
        }

        // Interpret reply
        setToken(reply.contains("access_token")? reply.value("access_token"): "");
        // FIXME
        setExpires(QDateTime::currentMSecsSinceEpoch() / 1000 + 2 * 60 * 60);
        setRefreshToken(reply.contains("refresh_token")? reply.value("refresh_token"): "");

        timedReplies_.remove(tokenReply);
        emit linkedChanged();
        emit tokenChanged();
        emit linkingSucceeded();
    }
}

void O2Skydrive::unlink() {
    qDebug() << "> O2Skydrive::unlink";
    O2::unlink();
}

#endif
