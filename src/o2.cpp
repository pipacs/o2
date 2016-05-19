#include <QList>
#include <QPair>
#include <QDebug>
#include <QTcpServer>
#include <QMap>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDateTime>
#include <QCryptographicHash>
#include <QTimer>
#include <QVariantMap>

#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#else
#include <QScriptEngine>
#include <QScriptValueIterator>
#endif

#include "o2.h"
#include "o2replyserver.h"
#include "o0globals.h"
#include "o0settingsstore.h"

/// Parse JSON data into a QVariantMap
static QVariantMap parseTokenResponse(const QByteArray &data) {
#if QT_VERSION >= 0x050000
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "parseTokenResponse: Failed to parse token response due to err:" << err.errorString();
        return QVariantMap();
    }

    if (!doc.isObject()) {
        qWarning() << "parseTokenResponse: Token response is not an object";
        return QVariantMap();
    }

    return doc.object().toVariantMap();
#else
    QScriptEngine engine;
    QScriptValue value = engine.evaluate("(" + QString(data) + ")");
    QScriptValueIterator it(value);
    QVariantMap map;

    while (it.hasNext()) {
        it.next();
        map.insert(it.name(), it.value().toVariant());
    }

    return map;
#endif
}

/// Add query parameters to a query
static void addQueryParametersToUrl(QUrl &url,  QList<QPair<QString, QString> > parameters) {
#if QT_VERSION < 0x050000
    url.setQueryItems(parameters);
#else
    QUrlQuery query(url);
    query.setQueryItems(parameters);
    url.setQuery(query);
#endif
}

O2::O2(QObject *parent): O0BaseAuth(parent) {
    manager_ = new QNetworkAccessManager(this);
    replyServer_ = new O2ReplyServer(this);
    grantFlow_ = GrantFlowAuthorizationCode;
    localhostPolicy_ = QString(O2_CALLBACK_URL);
    qRegisterMetaType<QNetworkReply::NetworkError>("QNetworkReply::NetworkError");
    connect(replyServer_, SIGNAL(verificationReceived(QMap<QString,QString>)), this, SLOT(onVerificationReceived(QMap<QString,QString>)));
}

O2::GrantFlow O2::grantFlow() {
    return grantFlow_;
}

void O2::setGrantFlow(O2::GrantFlow value) {
    grantFlow_ = value;
    emit grantFlowChanged();
}

QString O2::username() {
    return username_;
}

void O2::setUsername(const QString &value) {
    username_ = value;
    emit usernameChanged();
}

QString O2::password() {
    return password_;
}

void O2::setPassword(const QString &value) {
    password_ = value;
    emit passwordChanged();
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

void O2::link() {
    trace() << "O2::link";

    if (linked()) {
        trace() << "O2::link: Linked already";
        emit linkingSucceeded();
        return;
    }

    setLinked(false);
    setToken("");
    setTokenSecret("");
    setExtraTokens(QVariantMap());
    setRefreshToken(QString());
    setExpires(0);

    if (grantFlow_ == GrantFlowAuthorizationCode) {
        // Start listening to authentication replies
        replyServer_->listen(QHostAddress::Any, localPort_);

        // Save redirect URI, as we have to reuse it when requesting the access token
        redirectUri_ = localhostPolicy_.arg(replyServer_->serverPort());

        // Assemble intial authentication URL
        QList<QPair<QString, QString> > parameters;
        parameters.append(qMakePair(QString(O2_OAUTH2_RESPONSE_TYPE), (grantFlow_ == GrantFlowAuthorizationCode)? QString(O2_OAUTH2_GRANT_TYPE_CODE): QString(O2_OAUTH2_GRANT_TYPE_TOKEN)));
        parameters.append(qMakePair(QString(O2_OAUTH2_CLIENT_ID), clientId_));
        parameters.append(qMakePair(QString(O2_OAUTH2_REDIRECT_URI), redirectUri_));
        parameters.append(qMakePair(QString(O2_OAUTH2_SCOPE), scope_));
        parameters.append(qMakePair(QString(O2_OAUTH2_API_KEY), apiKey_));

        // Show authentication URL with a web browser
        QUrl url(requestUrl_);
        addQueryParametersToUrl(url, parameters);
        trace() << "O2::link: Emit openBrowser" << url.toString();
        emit openBrowser(url);
    } else if (grantFlow_ == GrantFlowResourceOwnerPasswordCredentials) {
        QList<O0RequestParameter> parameters;
        parameters.append(O0RequestParameter(O2_OAUTH2_CLIENT_ID, clientId_.toUtf8()));
        parameters.append(O0RequestParameter(O2_OAUTH2_CLIENT_SECRET, clientSecret_.toUtf8()));
        parameters.append(O0RequestParameter(O2_OAUTH2_USERNAME, username_.toUtf8()));
        parameters.append(O0RequestParameter(O2_OAUTH2_PASSWORD, password_.toUtf8()));
        parameters.append(O0RequestParameter(O2_OAUTH2_GRANT_TYPE, O2_OAUTH2_GRANT_TYPE_PASSWORD));
        parameters.append(O0RequestParameter(O2_OAUTH2_SCOPE, scope_.toUtf8()));
        parameters.append(O0RequestParameter(O2_OAUTH2_API_KEY, apiKey_.toUtf8()));
        QByteArray payload = O0BaseAuth::createQueryParameters(parameters);

        QUrl url(tokenUrl_);
        QNetworkRequest tokenRequest(url);
        tokenRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        QNetworkReply *tokenReply = manager_->post(tokenRequest, payload);

        connect(tokenReply, SIGNAL(finished()), this, SLOT(onTokenReplyFinished()), Qt::QueuedConnection);
        connect(tokenReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenReplyError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
    }
}

void O2::unlink() {
    trace() << "O2::unlink";
    setLinked(false);
    setToken(QString());
    setRefreshToken(QString());
    setExpires(0);
    setExtraTokens(QVariantMap());
    emit linkingSucceeded();
}

void O2::onVerificationReceived(const QMap<QString, QString> response) {
    trace() << "O2::onVerificationReceived:" << response;
    trace() << "O2::onVerificationReceived: Emitting closeBrowser()";
    emit closeBrowser();

    if (response.contains("error")) {
        qWarning() << "O2::onVerificationReceived: Verification failed: " << response;
        emit linkingFailed();
        return;
    }

    if (grantFlow_ == GrantFlowAuthorizationCode) {
        // Save access code
        setCode(response.value(QString(O2_OAUTH2_GRANT_TYPE_CODE)));

        // Exchange access code for access/refresh tokens
        QNetworkRequest tokenRequest(tokenUrl_.toString() +
              (apiKey_.isEmpty() ? "" : ("?" + QString(O2_OAUTH2_API_KEY) + "=" + apiKey_)));
        tokenRequest.setHeader(QNetworkRequest::ContentTypeHeader, O2_MIME_TYPE_XFORM);
        QMap<QString, QString> parameters;
        parameters.insert(O2_OAUTH2_GRANT_TYPE_CODE, code());
        parameters.insert(O2_OAUTH2_CLIENT_ID, clientId_);
        parameters.insert(O2_OAUTH2_CLIENT_SECRET, clientSecret_);
        parameters.insert(O2_OAUTH2_REDIRECT_URI, redirectUri_);
        parameters.insert(O2_OAUTH2_GRANT_TYPE, O2_AUTHORIZATION_CODE);
        QByteArray data = buildRequestBody(parameters);
        QNetworkReply *tokenReply = manager_->post(tokenRequest, data);
        timedReplies_.add(tokenReply);
        connect(tokenReply, SIGNAL(finished()), this, SLOT(onTokenReplyFinished()), Qt::QueuedConnection);
        connect(tokenReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenReplyError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
    } else {
        setToken(response.value(O2_OAUTH2_ACCESS_TOKEN));
        setRefreshToken(response.value(O2_OAUTH2_REFRESH_TOKEN));
    }
}

QString O2::code() {
    QString key = QString(O2_KEY_CODE).arg(clientId_);
    return store_->value(key);
}

void O2::setCode(const QString &c) {
    QString key = QString(O2_KEY_CODE).arg(clientId_);
    store_->setValue(key, c);
}

void O2::onTokenReplyFinished() {
    trace() << "O2::onTokenReplyFinished";
    QNetworkReply *tokenReply = qobject_cast<QNetworkReply *>(sender());
    if (tokenReply->error() == QNetworkReply::NoError) {
        QByteArray replyData = tokenReply->readAll();
        QVariantMap tokens = parseTokenResponse(replyData);

        // Check for mandatory tokens
        if (tokens.contains(O2_OAUTH2_ACCESS_TOKEN)) {
            setToken(tokens.take(O2_OAUTH2_ACCESS_TOKEN).toString());
            bool ok = false;
            int expiresIn = tokens.take(O2_OAUTH2_EXPIRES_IN).toInt(&ok);
            if (ok) {
                trace() << "O2::onTokenReplyFinished: Token expires in" << expiresIn << "seconds";
                setExpires(QDateTime::currentMSecsSinceEpoch() / 1000 + expiresIn);
            }
            setRefreshToken(tokens.take(O2_OAUTH2_REFRESH_TOKEN).toString());
            setExtraTokens(tokens);
            timedReplies_.remove(tokenReply);
            setLinked(true);
            emit linkingSucceeded();
        } else {
            qWarning() << "O2::onTokenReplyFinished: oauth_token missing from response" << replyData;
            emit linkingFailed();
        }
    }
    tokenReply->deleteLater();
}

void O2::onTokenReplyError(QNetworkReply::NetworkError error) {
    QNetworkReply *tokenReply = qobject_cast<QNetworkReply *>(sender());
    qWarning() << "O2::onTokenReplyError: " << error << ": " << tokenReply->errorString();
    trace() << "O2::onTokenReplyError: " << tokenReply->readAll();
    setToken(QString());
    setRefreshToken(QString());
    timedReplies_.remove(tokenReply);
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

int O2::expires() {
    QString key = QString(O2_KEY_EXPIRES).arg(clientId_);
    return store_->value(key).toInt();
}

void O2::setExpires(int v) {
    QString key = QString(O2_KEY_EXPIRES).arg(clientId_);
    store_->setValue(key, QString::number(v));
}

QString O2::refreshToken() {
    QString key = QString(O2_KEY_REFRESH_TOKEN).arg(clientId_);
    return store_->value(key);
}

void O2::setRefreshToken(const QString &v) {
    trace() << "O2::setRefreshToken" << v.left(4) << "...";
    QString key = QString(O2_KEY_REFRESH_TOKEN).arg(clientId_);
    store_->setValue(key, v);
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
    refreshRequest.setHeader(QNetworkRequest::ContentTypeHeader, O2_MIME_TYPE_XFORM);
    QMap<QString, QString> parameters;
    parameters.insert(O2_OAUTH2_CLIENT_ID, clientId_);
    parameters.insert(O2_OAUTH2_CLIENT_SECRET, clientSecret_);
    parameters.insert(O2_OAUTH2_REFRESH_TOKEN, refreshToken());
    parameters.insert(O2_OAUTH2_GRANT_TYPE, O2_OAUTH2_REFRESH_TOKEN);

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
        QVariantMap tokens = parseTokenResponse(reply);
        setToken(tokens.value(O2_OAUTH2_ACCESS_TOKEN).toString());
        setExpires(QDateTime::currentMSecsSinceEpoch() / 1000 + tokens.value(O2_OAUTH2_EXPIRES_IN).toInt());
        setRefreshToken(tokens.value(O2_OAUTH2_REFRESH_TOKEN).toString());
        timedReplies_.remove(refreshReply);
        setLinked(true);
        emit linkingSucceeded();
        emit refreshFinished(QNetworkReply::NoError);
        trace() << " New token expires in" << expires() << "seconds";
    }
    refreshReply->deleteLater();
}

void O2::onRefreshError(QNetworkReply::NetworkError error) {
    QNetworkReply *refreshReply = qobject_cast<QNetworkReply *>(sender());
    qWarning() << "O2::onRefreshError: " << error;
    unlink();
    timedReplies_.remove(refreshReply);
    emit refreshFinished(error);
}

QString O2::localhostPolicy() const {
    return localhostPolicy_;
}

void O2::setLocalhostPolicy(const QString &value) {
    localhostPolicy_ = value;
}

QString O2::apiKey() {
    return apiKey_;
}

void O2::setApiKey(const QString &value) {
    apiKey_ = value;
}

QByteArray O2::replyContent() {
    return replyServer_->replyContent();
}

void O2::setReplyContent(const QByteArray &value) {
    replyServer_->setReplyContent(value);
}

bool O2::ignoreSslErrors() {
    return timedReplies_.ignoreSslErrors();
}

void O2::setIgnoreSslErrors(bool ignoreSslErrors) {
    timedReplies_.setIgnoreSslErrors(ignoreSslErrors);
}
