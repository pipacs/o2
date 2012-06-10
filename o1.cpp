#include <QCryptographicHash>
#include <QSettings>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QDateTime>
#include <QByteArray>
#include <QDebug>
#include <QStringList>

#include "o1.h"
#include "simplecrypt.h"
#include "o2replyserver.h"

O1::O1(QObject *parent): QObject(parent) {
    QByteArray hash = QCryptographicHash::hash("12345678", QCryptographicHash::Sha1);
    crypt_ = new SimpleCrypt(*((quint64 *)(void *)hash.data()));
    manager_ = new QNetworkAccessManager(this);
    replyServer_ = new O2ReplyServer(this);
    qRegisterMetaType<QNetworkReply::NetworkError>("QNetworkReply::NetworkError");
    connect(replyServer_, SIGNAL(verificationReceived(QMap<QString,QString>)), this, SLOT(onVerificationReceived(QMap<QString,QString>)));
    qsrand(QTime::currentTime().msec());
}

O1::~O1() {
    delete crypt_;
    delete manager_;
}

bool O1::linked() {
    return !token().isEmpty();
}

QString O1::tokenSecret() {
    QString key = QString("tokensecret.%1").arg(clientId_);
    return crypt_->decryptToString(QSettings().value(key).toString());
}

void O1::setTokenSecret(const QString &v) {
    QString key = QString("tokensecret.%1").arg(clientId_);
    QSettings().setValue(key, crypt_->encryptToString(v));
}

QString O1::token() {
    QString key = QString("token.%1").arg(clientId_);
    return crypt_->decryptToString(QSettings().value(key).toString());
}

void O1::setToken(const QString &v) {
    QString key = QString("token.%1").arg(clientId_);
    QSettings().setValue(key, crypt_->encryptToString(v));
}

QString O1::clientId() {
    return clientId_;
}

void O1::setClientId(const QString &value) {
    clientId_ = value;
    emit clientIdChanged();
}

QString O1::clientSecret() {
    return clientSecret_;
}

void O1::setClientSecret(const QString &value) {
    clientSecret_ = value;
    QByteArray hash = QCryptographicHash::hash(clientSecret_.toUtf8() + "12345678", QCryptographicHash::Sha1);
    delete crypt_;
    crypt_ = new SimpleCrypt(*((quint64 *)(void *)hash.data()));
    emit clientSecretChanged();
}

int O1::localPort() {
    return localPort_;
}

void O1::setLocalPort(int value) {
    localPort_ = value;
    emit localPortChanged();
}

QUrl O1::requestTokenUrl() {
    return requestTokenUrl_;
}

void O1::setRequestTokenUrl(const QUrl &v) {
    requestTokenUrl_ = v;
    emit requestTokenUrlChanged();
}

QUrl O1::authorizeUrl() {
    return authorizeUrl_;
}

void O1::setAuthorizeUrl(const QUrl &value) {
    authorizeUrl_ = value;
    emit authorizeUrlChanged();
}

QUrl O1::accessTokenUrl() {
    return accessTokenUrl_;
}

void O1::setAccessTokenUrl(const QUrl &value) {
    accessTokenUrl_ = value;
    emit accessTokenUrlChanged();
}

void O1::unlink() {
    if (linked()) {
        setToken("");
        setTokenSecret("");
        emit linkedChanged();
    }
    emit linkingSucceeded();
}

/// Calculate the HMAC variant of SHA1 hash.
/// @author     http://qt-project.org/wiki/HMAC-SHA1.
/// @copyright  Creative Commons Attribution-ShareAlike 2.5 Generic.
static QByteArray hmacSha1(QByteArray key, QByteArray baseString) {
    int blockSize = 64;
    if (key.length() > blockSize) {
        key = QCryptographicHash::hash(key, QCryptographicHash::Sha1);
    }
    QByteArray innerPadding(blockSize, char(0x36));
    QByteArray outerPadding(blockSize, char(0x5c));
    for (int i = 0; i < key.length(); i++) {
        innerPadding[i] = innerPadding[i] ^ key.at(i);
        outerPadding[i] = outerPadding[i] ^ key.at(i);
    }
    QByteArray total = outerPadding;
    QByteArray part = innerPadding;
    part.append(baseString);
    total.append(QCryptographicHash::hash(part, QCryptographicHash::Sha1));
    QByteArray hashed = QCryptographicHash::hash(total, QCryptographicHash::Sha1);
    return hashed.toBase64();
}

/// Get HTTP operation name.
static QString getOperationName(QNetworkAccessManager::Operation op) {
    switch (op) {
    case QNetworkAccessManager::GetOperation: return "GET";
    case QNetworkAccessManager::PostOperation: return "POST";
    case QNetworkAccessManager::PutOperation: return "PUT";
    case QNetworkAccessManager::DeleteOperation: return "DEL";
    default: return "";
    }
}

/// Build a concatenated/percent-encoded string from a list of headers.
static QByteArray encodeHeaders(const QList<O1RequestParameter> &headers) {
    QByteArray ret;
    bool first = true;
    foreach (O1RequestParameter h, headers) {
        if (first) {
            first = false;
        } else {
            ret.append("&");
        }
        ret.append(QUrl::toPercentEncoding(h.name) + "=" + QUrl::toPercentEncoding(h.value));
    }
    return QUrl::toPercentEncoding(ret);
}

/// Build a base string for signing.
static QByteArray getRequestBase(const QList<O1RequestParameter> &oauthParams, const QList<O1RequestParameter> &otherParams, const QUrl &baseUrl, QNetworkAccessManager::Operation op) {
    QByteArray base;

    // Initialize base string with the operation name (e.g. "GET") and the base URL
    base.append(getOperationName(op).toUtf8() + "&");
    base.append(QUrl::toPercentEncoding(baseUrl.toString()) + "&");

    // Append a sorted+encoded list of all request parameters to the base string
    QList<O1RequestParameter> headers;
    foreach (O1RequestParameter header, oauthParams) {
        headers.append(header);
    }
    foreach (O1RequestParameter header, otherParams) {
        headers.append(header);
    }
    qSort(headers);
    base.append(encodeHeaders(headers));

    return base;
}

QByteArray O1::sign(const QList<O1RequestParameter> &oauthParams, const QList<O1RequestParameter> &otherParams, const QUrl &baseUrl, QNetworkAccessManager::Operation op, const QString &consumerSecret, const QString &tokenSecret) {
    QByteArray baseString = getRequestBase(oauthParams, otherParams, baseUrl, op);
    QByteArray secret = QUrl::toPercentEncoding(consumerSecret) + "&" + QUrl::toPercentEncoding(tokenSecret);
    return hmacSha1(secret, baseString);
}

QByteArray O1::buildAuthorizationHeader(const QList<O1RequestParameter> &oauthParams) {
    bool first = true;
    QByteArray ret("OAuth ");
    QList<O1RequestParameter> headers(oauthParams);
    qSort(headers);
    foreach (O1RequestParameter h, headers) {
        if (first) {
            first = false;
        } else {
            ret.append(", ");
        }
        ret.append(h.name);
        ret.append("=\"");
        ret.append(QUrl::toPercentEncoding(h.value));
        ret.append("\"");
    }
    return ret;
}

void O1::link() {
    if (linked()) {
        emit linkingSucceeded();
        return;
    }

    // Start reply server
    replyServer_->listen(QHostAddress::Any, localPort());

    // Create initial token request
    QList<O1RequestParameter> headers;
    headers.append(O1RequestParameter("oauth_callback", QString("http://localhost:%1").arg(replyServer_->serverPort()).toAscii()));
    headers.append(O1RequestParameter("oauth_consumer_key", clientId().toAscii()));
    headers.append(O1RequestParameter("oauth_nonce", QString::number(qrand()).toAscii()));
    headers.append(O1RequestParameter("oauth_signature_method", "HMAC-SHA1"));
    headers.append(O1RequestParameter("oauth_timestamp", QString::number(QDateTime::currentDateTimeUtc().toTime_t()).toAscii()));
    headers.append(O1RequestParameter("oauth_version", "1.0"));
    QByteArray signature = sign(headers, QList<O1RequestParameter>(), requestTokenUrl(), QNetworkAccessManager::PostOperation, clientSecret(), "");
    headers.append(O1RequestParameter("oauth_signature", signature));

    // Clear request token
    requestToken_ = "";
    requestTokenSecret_ = "";

    // Post request
    QNetworkRequest request(requestTokenUrl());
    request.setRawHeader("Authorization", buildAuthorizationHeader(headers));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    delete manager_;
    manager_ = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager_->post(request, QByteArray());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenRequestError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(finished()), this, SLOT(onTokenRequestFinished()));
}

void O1::onTokenRequestError(QNetworkReply::NetworkError error) {
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    qWarning() << "O1::onTokenRequestError:" << (int)error << reply->errorString() << reply->readAll();
    emit linkingFailed();
}

void O1::onTokenRequestFinished() {
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        return;
    }

    // Get request token and secret
    QByteArray data = reply->readAll();
    QMap<QString, QString> response = parseResponse(data);
    requestToken_ = response.value("oauth_token", "");
    requestTokenSecret_ = response.value("oauth_token_secret", "");
    if (requestToken_.isEmpty() || requestTokenSecret_.isEmpty()) {
        qWarning() << "O1::onTokenRequestFinished: No oauth_token or oauth_token_secret in response:" << data;
        emit linkingFailed();
        return;
    }

    // Continue authorization flow in the browser
    QUrl url(authorizeUrl());
    url.addQueryItem("oauth_token", requestToken_);
    emit openBrowser(url);
}

void O1::onVerificationReceived(QMap<QString, QString> params) {
    emit closeBrowser();
    verifier_ = params.value("oauth_verifier", "");
    if (params.value("oauth_token") == requestToken_) {
        // Exchange request token for access token
        exchangeToken();
    } else {
        qWarning() << "O1::onVerificationReceived: oauth_token missing or doesn't match";
        emit linkingFailed();
    }
}

void O1::exchangeToken() {
    // Create token exchange request

    QList<O1RequestParameter> oauthParams;
    oauthParams.append(O1RequestParameter("oauth_consumer_key", clientId().toAscii()));
    oauthParams.append(O1RequestParameter("oauth_nonce", QString::number(qrand()).toAscii()));
    oauthParams.append(O1RequestParameter("oauth_signature_method", "HMAC-SHA1"));
    oauthParams.append(O1RequestParameter("oauth_timestamp", QString::number(QDateTime::currentDateTimeUtc().toTime_t()).toAscii()));
    oauthParams.append(O1RequestParameter("oauth_version", "1.0"));
    oauthParams.append(O1RequestParameter("oauth_token", requestToken_.toAscii()));

    QList<O1RequestParameter> extraHeaders;
    extraHeaders.append(O1RequestParameter("oauth_verifier", verifier_.toAscii()));

    QByteArray signature = sign(oauthParams, extraHeaders, requestTokenUrl(), QNetworkAccessManager::PostOperation, clientSecret(), "");
    oauthParams.append(O1RequestParameter("oauth_signature", signature));

    // Post request
    QNetworkRequest request(requestTokenUrl());
    request.setRawHeader("Authorization", buildAuthorizationHeader(oauthParams));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QByteArray body;
    body.append("oauth_verifier=");
    body.append(QUrl::toPercentEncoding(verifier_));
    request.setHeader(QNetworkRequest::ContentLengthHeader, body.length());
    QNetworkReply *reply = manager_->post(request, body);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenExchangeError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(finished()), this, SLOT(onTokenExchangeFinished()));
}

void O1::onTokenExchangeError(QNetworkReply::NetworkError error) {
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    qWarning() << "O1::onTokenExchangeError:" << (int)error << reply->errorString() << reply->readAll();
    emit linkingFailed();
}

void O1::onTokenExchangeFinished() {
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        return;
    }

    // Get access token and secret
    QByteArray data = reply->readAll();
    QMap<QString, QString> response = parseResponse(data);
    if (response.contains("oauth_token") && response.contains("oauth_token_secret")) {
        setToken(response.value("oauth_token"));
        setTokenSecret(response.value("oauth_token_secret"));
        emit linkedChanged();
        emit linkingSucceeded();
    } else {
        qWarning() << "O1::onTokenExchangeFinished: oauth_token or oauth_token_secret missing from response" << data;
        emit linkingFailed();
    }
}

QMap<QString, QString> O1::parseResponse(const QByteArray &response) {
    QMap<QString, QString> ret;
    foreach (QByteArray param, response.split('&')) {
        QList<QByteArray> kv = param.split('=');
        if (kv.length() == 2) {
            ret.insert(QUrl::fromPercentEncoding(kv[0]), QUrl::fromPercentEncoding(kv[1]));
        }
    }
    return ret;
}
