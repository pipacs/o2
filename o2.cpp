#include <QList>
#include <QPair>
#include <QDebug>
#include <QTcpServer>
#include <QMap>
#include <QSettings>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QScriptEngine>
#include <QDateTime>
#include <QCryptographicHash>
#include <QTimer>

#include "o2.h"
#include "o2replyserver.h"
#include "simplecrypt.h"

O2::O2(const QString &clientId, const QString &clientSecret, const QString &scope, const QUrl &requestUrl, const QUrl &tokenUrl, const QUrl &refreshTokenUrl, quint16 localPort, QObject *parent):
    QObject(parent),
    clientId_(clientId),
    clientSecret_(clientSecret),
    scope_(scope), requestUrl_(requestUrl),
    tokenUrl_(tokenUrl), refreshTokenUrl_(refreshTokenUrl),
    localPort_(localPort) {
    QByteArray hash = QCryptographicHash::hash(clientSecret.toUtf8() + "12345678", QCryptographicHash::Sha1);
    crypt_ = new SimpleCrypt(*((quint64 *)(void *)hash.data()));
    manager_ = new QNetworkAccessManager(this);
    replyServer_ = new O2ReplyServer(this);
    qRegisterMetaType<QNetworkReply::NetworkError>("QNetworkReply::NetworkError");
    connect(replyServer_, SIGNAL(verificationReceived(QMap<QString,QString>)), this, SLOT(onVerificationReceived(QMap<QString,QString>)));
}

O2::~O2() {
    delete crypt_;
}

void O2::link() {
    qDebug() << "O2::link";
    if (linked()) {
        qDebug() << " Linked already";
        return;
    }

    // Start listening to authentication replies
    replyServer_->listen(QHostAddress::Any, localPort_);

    // Save redirect URI, as we have to reuse it when requesting the access token
    redirectUri_ = QString("http://localhost:%1/").arg(replyServer_->serverPort());

    // Assemble intial authentication URL
    QList<QPair<QString, QString> > parameters;
    parameters.append(qMakePair(QString("response_type"), QString("code")));
    parameters.append(qMakePair(QString("client_id"), clientId_));
    parameters.append(qMakePair(QString("redirect_uri"), redirectUri_));
    parameters.append(qMakePair(QString("scope"), scope_));

    // Show authentication URL with a web browser
    QUrl url(requestUrl_);
    url.setQueryItems(parameters);
    emit openBrowser(url);
}

void O2::unlink() {
    if (!linked()) {
        return;
    }
    setToken(QString());
    setRefreshToken(QString());
    setExpires(0);
    emit linkedChanged();
}

bool O2::linked() {
    return token().length();
}

void O2::onVerificationReceived(const QMap<QString, QString> response) {
    qDebug() << "O2::onVerificationReceived";
    emit closeBrowser();
    if (response.contains("error")) {
        qDebug() << " Verification failed";
        emit linkingFailed();
        return;
    }

    // Save access code
    setCode(response.value(QString("code")));

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
    qDebug() << " Request body:" << data;
    QNetworkReply *tokenReply = manager_->post(tokenRequest, data);
    timedReplies_.add(tokenReply);
    connect(tokenReply, SIGNAL(finished()), this, SLOT(onTokenReplyFinished()), Qt::QueuedConnection);
    connect(tokenReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenReplyError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
}

QString O2::code() {
    QString key = QString("code.%1").arg(clientId_);
    return crypt_->decryptToString(QSettings().value(key).toString());
}

void O2::setCode(const QString &c) {
    QString key = QString("code.%1").arg(clientId_);
    QSettings().setValue(key, crypt_->encryptToString(c));
}

void O2::onTokenReplyFinished() {
    QNetworkReply *tokenReply = qobject_cast<QNetworkReply *>(sender());
    if (tokenReply->error() == QNetworkReply::NoError) {
        QByteArray replyData = tokenReply->readAll();
        QScriptValue value;
        QScriptEngine engine;
        value = engine.evaluate("(" + QString(replyData) + ")");
        setToken(value.property("access_token").toString());
        setExpires(QDateTime::currentMSecsSinceEpoch() / 1000 + value.property("expires_in").toInteger());
        setRefreshToken(value.property("refresh_token").toString());
        qDebug() << "O2::onTokenReplyFinished: Token expires in" << value.property("expires_in").toInteger() << "seconds";
        timedReplies_.remove(tokenReply);
        emit linkingSucceeded();
        emit tokenChanged();
        emit linkedChanged();
    }
    tokenReply->deleteLater();
}

void O2::onTokenReplyError(QNetworkReply::NetworkError error) {
    QNetworkReply *tokenReply = qobject_cast<QNetworkReply *>(sender());
    qDebug() << "O2::onTokenReplyError" << error << tokenReply->errorString();
    qDebug() << "" << tokenReply->readAll();
    setToken(QString());
    setRefreshToken(QString());
    timedReplies_.remove(tokenReply);
    emit tokenChanged();
    emit linkingFailed();
    emit linkedChanged();
}

QByteArray O2::buildRequestBody(const QMap<QString, QString> &parameters) {
    QByteArray body;
    bool first = true;
    foreach (QString key, parameters.keys()) {
        if (first) {
            first = false;
        } else {
            body.append("&");
        }
        QString value = parameters.value(key);
        body.append(QUrl::toPercentEncoding(key) + QString("=").toUtf8() + QUrl::toPercentEncoding(value));
    }
    return body;
}

QString O2::token() {
    QString key = QString("token.%1").arg(clientId_);
    return crypt_->decryptToString(QSettings().value(key).toString());
}

void O2::setToken(const QString &v) {
    QString key = QString("token.%1").arg(clientId_);
    QSettings().setValue(key, crypt_->encryptToString(v));
}

int O2::expires() {
    QString key = QString("expires.%1").arg(clientId_);
    return QSettings().value(key).toInt();
}

void O2::setExpires(int v) {
    QString key = QString("expires.%1").arg(clientId_);
    QSettings().setValue(key, v);
}

QString O2::refreshToken() {
    QString key = QString("refreshtoken.%1").arg(clientId_);
    QString ret = crypt_->decryptToString(QSettings().value(key).toString());
    return ret;
}

void O2::setRefreshToken(const QString &v) {
    QString key = QString("refreshtoken.%1").arg(clientId_);
    QSettings().setValue(key, crypt_->encryptToString(v));
}

void O2::refresh() {
    qDebug() << "O2::refresh: Token: ..." << refreshToken().right(7);

    if (!refreshToken().length()) {
        qWarning() << "O2::refresh: No refresh token";
        onRefreshError(QNetworkReply::AuthenticationRequiredError);
        return;
    }

    QNetworkRequest refreshRequest(refreshTokenUrl_);
    refreshRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QMap<QString, QString> parameters;
    parameters.insert("client_id", clientId_);
    parameters.insert("client_secret", clientSecret_);
    parameters.insert("refresh_token", refreshToken());
    parameters.insert("grant_type", "refresh_token");
    QByteArray data = buildRequestBody(parameters);
    QNetworkReply *refreshReply = manager_->post(refreshRequest, data);
    timedReplies_.add(refreshReply);
    connect(refreshReply, SIGNAL(finished()), this, SLOT(onRefreshFinished()), Qt::QueuedConnection);
    connect(refreshReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRefreshError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
}

void O2::onRefreshFinished() {
    QNetworkReply *refreshReply = qobject_cast<QNetworkReply *>(sender());
    qDebug() << "O2::onRefreshFinished: Error" << (int)refreshReply->error() << refreshReply->errorString();
    if (refreshReply->error() == QNetworkReply::NoError) {
        QByteArray reply = refreshReply->readAll();
        QScriptValue value;
        QScriptEngine engine;
        value = engine.evaluate("(" + QString(reply) + ")");
        setToken(value.property("access_token").toString());
        setExpires(QDateTime::currentMSecsSinceEpoch() / 1000 + value.property("expires_in").toInteger());
        setRefreshToken(value.property("refresh_token").toString());
        timedReplies_.remove(refreshReply);
        emit linkingSucceeded();
        emit tokenChanged();
        emit linkedChanged();
        emit refreshFinished(QNetworkReply::NoError);
        qDebug() << "O2::refreshToken: New token expires in" << expires() << "seconds";
    }
    refreshReply->deleteLater();
}

void O2::onRefreshError(QNetworkReply::NetworkError error) {
    QNetworkReply *refreshReply = qobject_cast<QNetworkReply *>(sender());
    qDebug() << "O2::onRefreshFailed: Error" << error << ", resetting tokens";
    setToken(QString());
    setRefreshToken(QString());
    timedReplies_.remove(refreshReply);
    emit tokenChanged();
    emit linkingFailed();
    emit linkedChanged();
    emit refreshFinished(error);
}
