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

/// HTTP request header name + value.
struct RequestHeader {
    RequestHeader(const QByteArray &n, const QByteArray &v): name(n), value(v) {}
    bool operator <(const RequestHeader &other) const {
        if (name == other.name) {
            return value < other.value;
        } else {
            return name < other.name;
        }
    }
    QByteArray name;
    QByteArray value;
};

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
    qDebug() << "O1::unlink";
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
static QByteArray encodeHeaders(const QList<RequestHeader> &headers) {
    QByteArray ret;
    bool first = true;

    foreach (RequestHeader h, headers) {
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
static QByteArray getRequestBase(const QList<RequestHeader> &oauthHeaders, const QList<RequestHeader> &otherHeaders, const QUrl &baseUrl, QNetworkAccessManager::Operation op) {
    QByteArray base;

    // Initialize base string with the operation name (e.g. "GET") and the base URL
    base.append(getOperationName(op).toUtf8() + "&");
    base.append(QUrl::toPercentEncoding(baseUrl.toString()) + "&");

    // Append a sorted+encoded list of all request parameters to the base string
    QList<RequestHeader> headers;
    foreach (RequestHeader header, oauthHeaders) {
        headers.append(header);
    }
    foreach (RequestHeader header, otherHeaders) {
        headers.append(header);
    }
    qSort(headers);
    base.append(encodeHeaders(headers));

    return base;
}

/// Sign a request with HMAC-SHA1.
static QByteArray sign(const QList<RequestHeader> &oauthHeaders, const QList<RequestHeader> &otherHeaders, const QUrl &baseUrl, QNetworkAccessManager::Operation op, const QString &consumerSecret, const QString &tokenSecret) {
    QByteArray baseString = getRequestBase(oauthHeaders, otherHeaders, baseUrl, op);
    QByteArray secret = QUrl::toPercentEncoding(consumerSecret) + "&" + QUrl::toPercentEncoding(tokenSecret);
    return hmacSha1(secret, baseString);
}

/// Build the "Authorization:" header value from a list of OAuth headers.
static QByteArray getAuthorizationHeader(const QList<RequestHeader> &oauthHeaders) {
    bool first = true;
    QByteArray ret("OAuth ");
    QList<RequestHeader> headers(oauthHeaders);
    qSort(headers);
    foreach (RequestHeader h, headers) {
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
    qDebug() << "O1::link";
    if (linked()) {
        qDebug() << " Linked already";
        emit linkingSucceeded();
        return;
    }

    // Start reply server
    replyServer_->listen(QHostAddress::Any, localPort());

    // Create initial token request
    QList<RequestHeader> headers;
    headers.append(RequestHeader("oauth_callback", QString("http://localhost:%1").arg(replyServer_->serverPort()).toAscii()));
    headers.append(RequestHeader("oauth_consumer_key", clientId().toAscii()));
    headers.append(RequestHeader("oauth_nonce", QString::number(qrand()).toAscii()));
    headers.append(RequestHeader("oauth_signature_method", "HMAC-SHA1"));
    headers.append(RequestHeader("oauth_timestamp", QString::number(QDateTime::currentDateTimeUtc().toTime_t()).toAscii()));
    headers.append(RequestHeader("oauth_version", "1.0"));
    QByteArray signature = sign(headers, QList<RequestHeader>(), requestTokenUrl(), QNetworkAccessManager::PostOperation, clientSecret(), "");
    headers.append(RequestHeader("oauth_signature", signature));

    // Clear request token
    requestToken_ = "";
    requestTokenSecret_ = "";

    // Post request
    QNetworkRequest request(requestTokenUrl());
    request.setRawHeader("Authorization", getAuthorizationHeader(headers));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    delete manager_;
    manager_ = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager_->post(request, QByteArray());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenRequestError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(finished()), this, SLOT(onTokenRequestFinished()));
}

void O1::onTokenRequestError(QNetworkReply::NetworkError error) {
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    qDebug() << "O1::onTokenRequestError:" << (int)error << reply->errorString();
    qDebug() << "" << reply->readAll();
    emit linkingFailed();
}

void O1::onTokenRequestFinished() {
    qDebug() << "O1::onTokenRequestFinished";
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        return;
    }

    // Get request token and secret
    QMap<QString, QString> response = parseResponse(reply->readAll());
    requestToken_ = response.value("oauth_token", "");
    requestTokenSecret_ = response.value("oauth_token_secret", "");
    if (requestToken_.isEmpty() || requestTokenSecret_.isEmpty()) {
        qDebug() << " No oauth_token or oauth_token_secret in response";
        qDebug() << "" << response;
        emit linkingFailed();
        return;
    }

    // Continue authorization flow in the browser
    QUrl url(authorizeUrl());
    url.addQueryItem("oauth_token", requestToken_);
    emit openBrowser(url);
}

void O1::onVerificationReceived(QMap<QString, QString> params) {
    qDebug() << "O1::onVerificationReceived";
    qDebug() << "" << params;
    emit closeBrowser();
    verifier_ = params.value("oauth_verifier", "");
    if (params.contains("oauth_token")) {
        if (params.value("oauth_token") == requestToken_) {
            qDebug() << " Access granted by user";
            // Exchange request token for access token
            exchangeToken();
            return;
        }
    }
    emit linkingFailed();
}

void O1::exchangeToken() {
    qDebug() << "O1::exchangeToken";
    // Create token exchange request

    QList<RequestHeader> oauthHeaders;
    oauthHeaders.append(RequestHeader("oauth_consumer_key", clientId().toAscii()));
    oauthHeaders.append(RequestHeader("oauth_nonce", QString::number(qrand()).toAscii()));
    oauthHeaders.append(RequestHeader("oauth_signature_method", "HMAC-SHA1"));
    oauthHeaders.append(RequestHeader("oauth_timestamp", QString::number(QDateTime::currentDateTimeUtc().toTime_t()).toAscii()));
    oauthHeaders.append(RequestHeader("oauth_version", "1.0"));
    oauthHeaders.append(RequestHeader("oauth_token", requestToken_.toAscii()));

    QList<RequestHeader> extraHeaders;
    extraHeaders.append(RequestHeader("oauth_verifier", verifier_.toAscii()));

    QByteArray signature = sign(oauthHeaders, extraHeaders, requestTokenUrl(), QNetworkAccessManager::PostOperation, clientSecret(), "");
    oauthHeaders.append(RequestHeader("oauth_signature", signature));

    // Post request
    QNetworkRequest request(requestTokenUrl());
    request.setRawHeader("Authorization", getAuthorizationHeader(oauthHeaders));
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
    qDebug() << "O1::onTokenExchangeError:" << (int)error << reply->errorString();
    qDebug() << "" << reply->readAll();
    emit linkingFailed();
}

void O1::onTokenExchangeFinished() {
    qDebug() << "O1::onTokenExchangeFinished";
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        return;
    }

    // Get access token and secret
    QMap<QString, QString> response = parseResponse(reply->readAll());
    if (response.contains("oauth_token") && response.contains("oauth_token_secret")) {
        setToken(response.value("oauth_token"));
        setTokenSecret(response.value("oauth_token_secret"));
        emit linkedChanged();
        emit linkingSucceeded();
    } else {
        qDebug() << " No oauth_token or oauth_token_secret in response";
        qDebug() << "" << response;
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
