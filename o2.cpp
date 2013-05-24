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
#include "o2globals.h"

#define trace() if (1) qDebug()
// define trace() if (0) qDebug()

static inline quint64 getHash()
{
    return QCryptographicHash::hash(ENC_KEY, QCryptographicHash::Sha1).toULongLong();
}

O2::O2(QObject *parent): QObject(parent), crypt_(getHash()) {
    manager_ = new QNetworkAccessManager(this);
    replyServer_ = new O2ReplyServer(this);
    grantFlow_ = GrantFlowAuthorizationCode;
    localPort_ = 0;
    qRegisterMetaType<QNetworkReply::NetworkError>("QNetworkReply::NetworkError");
    connect(replyServer_, SIGNAL(verificationReceived(QMap<QString,QString>)),
            this, SLOT(onVerificationReceived(QMap<QString,QString>)));
}

O2::~O2() {}

O2::GrantFlow O2::grantFlow() {
    return grantFlow_;
}

void O2::setGrantFlow(O2::GrantFlow value) {
    grantFlow_ = value;
    emit grantFlowChanged();
}

QString O2::clientId() {
    return clientId_;
}

void O2::setClientId(const QString &value) {
    clientId_ = value;
    emit clientIdChanged();
}

QString O2::clientSecret() {
    return clientSecret_;
}

void O2::setClientSecret(const QString &value) {
    clientSecret_ = value;
    emit clientSecretChanged();
}

QString O2::scope() {
    return scope_;
}

void O2::setScope(const QString &value) {
    scope_ = value;
    emit scopeChanged();
}

QString O2::requestUrl() {
    return requestUrl_.toString();
}

void O2::setRequestUrl(const QString &value) {
    requestUrl_ = value;
    emit requestUrlChanged();
}

QString O2::tokenUrl() {
    return tokenUrl_.toString();
}

void O2::setTokenUrl(const QString &value) {
    tokenUrl_= value;
    emit tokenUrlChanged();
}

QString O2::refreshTokenUrl() {
    return refreshTokenUrl_.toString();
}

void O2::setRefreshTokenUrl(const QString &value) {
    refreshTokenUrl_ = value;
    emit refreshTokenUrlChanged();
}

int O2::localPort() {
    return localPort_;
}

void O2::setLocalPort(int value) {
    localPort_ = value;
    emit localPortChanged();
}

void O2::link() {
    trace() << "O2::link";
    if (linked()) {
        trace() << " Linked already";
        return;
    }

    // Start listening to authentication replies
    replyServer_->listen(QHostAddress::Any, localPort_);

    // Save redirect URI, as we have to reuse it when requesting the access token
    redirectUri_ = QString(CALLBACK_URL).arg(replyServer_->serverPort());

    // Assemble intial authentication URL
    QList<QPair<QString, QString> > parameters;
    parameters.append(qMakePair(QString(OAUTH2_RESP_TYPE), (grantFlow_ == GrantFlowAuthorizationCode) ? QString(OAUTH2_CODE) : QString(OAUTH2_TOK)));
    parameters.append(qMakePair(QString(OAUTH2_CLIENT_ID), clientId_));
    parameters.append(qMakePair(QString(OAUTH2_REDIRECT_URI), redirectUri_));
    // parameters.append(qMakePair(QString(OAUTH2_REDIRECT_URI), QString(QUrl::toPercentEncoding(redirectUri_))));
    parameters.append(qMakePair(QString(OAUTH2_SCOPE), scope_));

    // Show authentication URL with a web browser
    QUrl url(requestUrl_);
    url.setQueryItems(parameters);
    trace() << "Emit openBrowser" << url.toString();
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
    emit linkingSucceeded();
}

bool O2::linked() {
    return token().length();
}

void O2::onVerificationReceived(const QMap<QString, QString> response) {
    trace() << "O2::onVerificationReceived";
    trace() << "" << response;

    emit closeBrowser();
    if (response.contains("error")) {
        trace() << " Verification failed";
        emit linkingFailed();
        return;
    }

    if (grantFlow_ == GrantFlowAuthorizationCode) {
        // Save access code
        setCode(response.value(QString(OAUTH2_CODE)));

        // Exchange access code for access/refresh tokens
        QNetworkRequest tokenRequest(tokenUrl_);
        tokenRequest.setHeader(QNetworkRequest::ContentTypeHeader, HTTP_AUTH_HEADER);
        QMap<QString, QString> parameters;
        parameters.insert(OAUTH2_CODE, code());
        parameters.insert(OAUTH2_CLIENT_ID, clientId_);
        parameters.insert(OAUTH2_CLIENT_SECRET, clientSecret_);
        parameters.insert(OAUTH2_REDIRECT_URI, redirectUri_);
        parameters.insert(OAUTH2_GRANT_TYPE, GRANT_TYPE_AUTH_CODE);
        QByteArray data = buildRequestBody(parameters);
        QNetworkReply *tokenReply = manager_->post(tokenRequest, data);
        timedReplies_.add(tokenReply);
        connect(tokenReply, SIGNAL(finished()), this, SLOT(onTokenReplyFinished()), Qt::QueuedConnection);
        connect(tokenReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenReplyError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
    } else {
        setToken(response.value(OAUTH2_ACCESS_TOK));
        setRefreshToken(response.value(OAUTH2_REFRESH_TOK));
    }
}

QString O2::code() {
    QString key = QString(KEY_CODE).arg(clientId_);
    return crypt_.decryptToString(QSettings().value(key).toString());
}

void O2::setCode(const QString &c) {
    QString key = QString(KEY_CODE).arg(clientId_);
    QSettings().setValue(key, crypt_.encryptToString(c));
}

void O2::onTokenReplyFinished() {
    trace() << "O2::onTokenReplyFinished";
    QNetworkReply *tokenReply = qobject_cast<QNetworkReply *>(sender());
    if (tokenReply->error() == QNetworkReply::NoError) {
        QByteArray replyData = tokenReply->readAll();
        QScriptValue value;
        QScriptEngine engine;
        value = engine.evaluate("(" + QString(replyData) + ")");
        setToken(value.property(OAUTH2_ACCESS_TOK).toString());
        int expiresIn = value.property(OAUTH2_EXPIRES_IN).toInteger();
        if (expiresIn > 0) {
            trace() << "Token expires in" << expiresIn << "seconds";
            setExpires(QDateTime::currentMSecsSinceEpoch() / 1000 + expiresIn);
        }
        setRefreshToken(value.property(OAUTH2_REFRESH_TOK).toString());
        timedReplies_.remove(tokenReply);
        emit linkedChanged();
        emit tokenChanged();
        emit linkingSucceeded();
    }
    tokenReply->deleteLater();
}

void O2::onTokenReplyError(QNetworkReply::NetworkError error) {
    QNetworkReply *tokenReply = qobject_cast<QNetworkReply *>(sender());
    trace() << "O2::onTokenReplyError" << error << tokenReply->errorString();
    trace() << "" << tokenReply->readAll();
    setToken(QString());
    setRefreshToken(QString());
    timedReplies_.remove(tokenReply);
    emit linkedChanged();
    emit tokenChanged();
    emit linkingFailed();
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
    QString key = QString(KEY_TOK).arg(clientId_);
    return crypt_.decryptToString(QSettings().value(key).toString());
}

void O2::setToken(const QString &v) {
    QString key = QString(KEY_TOK).arg(clientId_);
    QSettings().setValue(key, crypt_.encryptToString(v));
}

int O2::expires() {
    QString key = QString(KEY_EXPIRES).arg(clientId_);
    return QSettings().value(key).toInt();
}

void O2::setExpires(int v) {
    QString key = QString(KEY_EXPIRES).arg(clientId_);
    QSettings().setValue(key, v);
}

QString O2::refreshToken() {
    QString key = QString(KEY_REFRESH_TOK).arg(clientId_);
    QString ret = crypt_.decryptToString(QSettings().value(key).toString());
    return ret;
}

void O2::setRefreshToken(const QString &v) {
    trace() << "O2::setRefreshToken" << v.left(4) << "...";
    QString key = QString(KEY_REFRESH_TOK).arg(clientId_);
    QSettings().setValue(key, crypt_.encryptToString(v));
}

void O2::refresh() {
    trace() << "O2::refresh: Token: ..." << refreshToken().right(7);

    if (refreshToken().isEmpty()) {
        qWarning() << "O2::refresh: No refresh token";
        onRefreshError(QNetworkReply::AuthenticationRequiredError);
        return;
    }
    if (refreshTokenUrl_.isEmpty()) {
        qWarning() << "O2::refresh: Refresh token URL not set";
        onRefreshError(QNetworkReply::AuthenticationRequiredError);
        return;
    }

    QNetworkRequest refreshRequest(refreshTokenUrl_);
    refreshRequest.setHeader(QNetworkRequest::ContentTypeHeader, HTTP_AUTH_HEADER);
    QMap<QString, QString> parameters;
    parameters.insert(OAUTH2_CLIENT_ID, clientId_);
    parameters.insert(OAUTH2_CLIENT_SECRET, clientSecret_);
    parameters.insert(OAUTH2_REFRESH_TOK, refreshToken());
    parameters.insert(OAUTH2_GRANT_TYPE, OAUTH2_REFRESH_TOK);
    QByteArray data = buildRequestBody(parameters);
    QNetworkReply *refreshReply = manager_->post(refreshRequest, data);
    timedReplies_.add(refreshReply);
    connect(refreshReply, SIGNAL(finished()), this, SLOT(onRefreshFinished()), Qt::QueuedConnection);
    connect(refreshReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRefreshError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
}

void O2::onRefreshFinished() {
    QNetworkReply *refreshReply = qobject_cast<QNetworkReply *>(sender());
    trace() << "O2::onRefreshFinished: Error" << (int)refreshReply->error() << refreshReply->errorString();
    if (refreshReply->error() == QNetworkReply::NoError) {
        QByteArray reply = refreshReply->readAll();
        QScriptValue value;
        QScriptEngine engine;
        value = engine.evaluate("(" + QString(reply) + ")");
        setToken(value.property(OAUTH2_ACCESS_TOK).toString());
        setExpires(QDateTime::currentMSecsSinceEpoch() / 1000 + value.property(OAUTH2_EXPIRES_IN).toInteger());
        setRefreshToken(value.property(OAUTH2_REFRESH_TOK).toString());
        timedReplies_.remove(refreshReply);
        emit linkingSucceeded();
        emit tokenChanged();
        emit linkedChanged();
        emit refreshFinished(QNetworkReply::NoError);
        trace() << " New token expires in" << expires() << "seconds";
    }
    refreshReply->deleteLater();
}

void O2::onRefreshError(QNetworkReply::NetworkError error) {
    QNetworkReply *refreshReply = qobject_cast<QNetworkReply *>(sender());
    qWarning() << "O2::onRefreshFailed: Error" << error << ", resetting tokens";
    setToken(QString());
    setRefreshToken(QString());
    timedReplies_.remove(refreshReply);
    emit tokenChanged();
    emit linkingFailed();
    emit linkedChanged();
    emit refreshFinished(error);
}
