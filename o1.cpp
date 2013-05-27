#include <QCryptographicHash>
#include <QSettings>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QDateTime>
#include <QByteArray>
#include <QDebug>
#include <QStringList>

#include "o1.h"
#include "o2replyserver.h"
#include "o2globals.h"

#define trace() if (1) qDebug()
// #define trace() if (0) qDebug()

static quint64 getHash() {
    return QCryptographicHash::hash(O2_ENCRYPTION_KEY, QCryptographicHash::Sha1).toULongLong();
}

O1::O1(QObject *parent) :
    QObject(parent), crypt_(getHash()) {
    manager_ = new QNetworkAccessManager(this);
    replyServer_ = new O2ReplyServer(this);
    localPort_ = 0;
    qRegisterMetaType<QNetworkReply::NetworkError>("QNetworkReply::NetworkError");
    connect(replyServer_, SIGNAL(verificationReceived(QMap<QString,QString>)),
            this, SLOT(onVerificationReceived(QMap<QString,QString>)));
}

O1::~O1() {
}

bool O1::linked() {
    return !token().isEmpty();
}

QString O1::tokenSecret() {
    QString key = QString(O2_KEY_TOKEN_SECRET).arg(clientId_);
    return crypt_.decryptToString(QSettings().value(key).toString());
}

void O1::setTokenSecret(const QString &v) {
    QString key = QString(O2_KEY_TOKEN_SECRET).arg(clientId_);
    QSettings().setValue(key, crypt_.encryptToString(v));
}

QString O1::token() {
    QString key = QString(O2_KEY_TOKEN).arg(clientId_);
    return crypt_.decryptToString(QSettings().value(key).toString());
}

void O1::setToken(const QString &v) {
    QString key = QString(O2_KEY_TOKEN).arg(clientId_);
    QSettings().setValue(key, crypt_.encryptToString(v));
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
    trace() << "O1::unlink";
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
static QByteArray getRequestBase(const QList<O1RequestParameter> &oauthParams, const QList<O1RequestParameter> &otherParams, const QUrl &url, QNetworkAccessManager::Operation op) {
    QByteArray base;

    // Initialize base string with the operation name (e.g. "GET") and the base URL
    base.append(getOperationName(op).toUtf8() + "&");
    base.append(QUrl::toPercentEncoding(url.toString(QUrl::RemoveQuery)) + "&");

    // Append a sorted+encoded list of all request parameters to the base string
    QList<O1RequestParameter> headers(oauthParams);
    headers.append(otherParams);
    qSort(headers);
    base.append(encodeHeaders(headers));

    return base;
}

QByteArray O1::sign(const QList<O1RequestParameter> &oauthParams, const QList<O1RequestParameter> &otherParams, const QUrl &url, QNetworkAccessManager::Operation op, const QString &consumerSecret, const QString &tokenSecret) {
    QByteArray baseString = getRequestBase(oauthParams, otherParams, url, op);
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
            ret.append(",");
        }
        ret.append(h.name);
        ret.append("=\"");
        ret.append(QUrl::toPercentEncoding(h.value));
        ret.append("\"");
    }
    return ret;
}

void O1::link() {
    trace() << "O1::link";
    if (linked()) {
        trace() << "O1::link: Linked already";
        emit linkingSucceeded();
        return;
    }

    // Start reply server
    replyServer_->listen(QHostAddress::Any, localPort());

    // Create initial token request
    QList<O1RequestParameter> headers;
    headers.append(O1RequestParameter(O2_OAUTH_CALLBACK, QString(O2_CALLBACK_URL).arg(replyServer_->serverPort()).toAscii()));
    headers.append(O1RequestParameter(O2_OAUTH_CONSUMER_KEY, clientId().toAscii()));
    headers.append(O1RequestParameter(O2_OAUTH_NONCE, nonce()));
    headers.append(O1RequestParameter(O2_OAUTH_SIGNATURE_METHOD, O2_SIGNATURE_TYPE_HMAC_SHA1));
    headers.append(O1RequestParameter(O2_OAUTH_TIMESTAMP, QString::number(QDateTime::currentDateTimeUtc().toTime_t()).toAscii()));
    headers.append(O1RequestParameter(O2_OAUTH_VERSION, "1.0"));
    QByteArray signature = sign(headers, QList<O1RequestParameter>(), requestTokenUrl(), QNetworkAccessManager::PostOperation, clientSecret(), "");
    headers.append(O1RequestParameter(O2_OAUTH_SIGNATURE, signature));

    // Clear request token
    requestToken_.clear();
    requestTokenSecret_.clear();

    // Post request
    QNetworkRequest request(requestTokenUrl());
    request.setRawHeader(O2_HTTP_AUTHORIZATION_HEADER, buildAuthorizationHeader(headers));
    request.setHeader(QNetworkRequest::ContentTypeHeader, O2_MIME_TYPE_XFORM);
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
    trace() << "O1::onTokenRequestFinished";
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        return;
    }

    // Get request token and secret
    QByteArray data = reply->readAll();
    QMap<QString, QString> response = parseResponse(data);
    requestToken_ = response.value(O2_OAUTH_TOKEN, "");
    requestTokenSecret_ = response.value(O2_OAUTH_TOKEN_SECRET, "");
    // Checking for "oauth_callback_confirmed" is present and set to true
    QString oAuthCbConfirmed = response.value(O2_OAUTH_CALLBACK_CONFIRMED, "false");
    if (requestToken_.isEmpty() || requestTokenSecret_.isEmpty() || (oAuthCbConfirmed == "false")) {
        qWarning() << "O1::onTokenRequestFinished: No oauth_token, oauth_token_secret or oauth_callback_confirmed in response :" << data;
        emit linkingFailed();
        return;
    }

    // Continue authorization flow in the browser
    QUrl url(authorizeUrl());
    url.addQueryItem(O2_OAUTH_TOKEN, requestToken_);
    url.addQueryItem(O2_OAUTH_CALLBACK, QString(O2_CALLBACK_URL).arg(replyServer_->serverPort()).toAscii());
    emit openBrowser(url);
}

void O1::onVerificationReceived(QMap<QString, QString> params) {
    trace() << "O1::onVerificationReceived";
    emit closeBrowser();
    verifier_ = params.value(O2_OAUTH_VERFIER, "");
    if (params.value(O2_OAUTH_TOKEN) == requestToken_) {
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
    oauthParams.append(O1RequestParameter(O2_OAUTH_SIGNATURE_METHOD, O2_SIGNATURE_TYPE_HMAC_SHA1));
    oauthParams.append(O1RequestParameter(O2_OAUTH_CONSUMER_KEY, clientId().toAscii()));
    oauthParams.append(O1RequestParameter(O2_OAUTH_VERSION, "1.0"));
    oauthParams.append(O1RequestParameter(O2_OAUTH_TIMESTAMP, QString::number(QDateTime::currentDateTimeUtc().toTime_t()).toAscii()));
    oauthParams.append(O1RequestParameter(O2_OAUTH_NONCE, nonce()));
    oauthParams.append(O1RequestParameter(O2_OAUTH_TOKEN, requestToken_.toAscii()));
    oauthParams.append(O1RequestParameter(O2_OAUTH_VERFIER, verifier_.toAscii()));

    QByteArray signature = sign(oauthParams, QList<O1RequestParameter>(), accessTokenUrl(), QNetworkAccessManager::PostOperation, clientSecret(), requestTokenSecret_);
    oauthParams.append(O1RequestParameter(O2_OAUTH_SIGNATURE, signature));

    // Post request
    QNetworkRequest request(accessTokenUrl());
    request.setRawHeader(O2_HTTP_AUTHORIZATION_HEADER, buildAuthorizationHeader(oauthParams));
    request.setHeader(QNetworkRequest::ContentTypeHeader, O2_MIME_TYPE_XFORM);
    QNetworkReply *reply = manager_->post(request, QByteArray());
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
    if (response.contains(O2_OAUTH_TOKEN) && response.contains(O2_OAUTH_TOKEN_SECRET)) {
        QString token = response.value(O2_OAUTH_TOKEN);
        QString secret = response.value(O2_OAUTH_TOKEN_SECRET);
        setToken(token);
        setTokenSecret(secret);
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

QByteArray O1::nonce() {
    static bool firstTime = true;
    if (firstTime) {
        firstTime = false;
        qsrand(QTime::currentTime().msec());
    }
    QString u = QString::number(QDateTime::currentDateTimeUtc().toTime_t());
    u.append(QString::number(qrand()));
    return u.toAscii();
}
