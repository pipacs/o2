#include <QCryptographicHash>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QDateTime>
#include <QByteArray>
#include <QDebug>
#include <QDataStream>
#include <QStringList>

#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

#if QT_VERSION >= 0x050100
#include <QMessageAuthenticationCode>
#endif

#include "o1.h"
#include "o2replyserver.h"
#include "o2globals.h"
#include "o2settingsstore.h"

#define trace() if (1) qDebug()
// #define trace() if (0) qDebug()

O1::O1(QObject *parent) :
    O2BaseAuth(parent) {
    setSignatureMethod(O2_SIGNATURE_TYPE_HMAC_SHA1);
    manager_ = new QNetworkAccessManager(this);
    replyServer_ = new O2ReplyServer(this);
    qRegisterMetaType<QNetworkReply::NetworkError>("QNetworkReply::NetworkError");
    connect(replyServer_, SIGNAL(verificationReceived(QMap<QString,QString>)),
            this, SLOT(onVerificationReceived(QMap<QString,QString>)));
}

QString O1::signatureMethod() {
    return signatureMethod_;
}

void O1::setSignatureMethod(const QString &value) {
    trace() << "O1::setSignatureMethod: " << value;
    signatureMethod_ = value;
}

void O1::unlink() {
    trace() << "O1::unlink";
    setLinked(false);
    setToken("");
    setTokenSecret("");
    setExtraTokens(QVariantMap());
    emit linkingSucceeded();
}

#if QT_VERSION < 0x050100
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
#endif

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
QByteArray O1::encodeHeaders(const QList<O1RequestParameter> &headers) {
    return QUrl::toPercentEncoding(createQueryParams(headers));
}

/// Construct query string from list of headers
QByteArray O1::createQueryParams(const QList<O1RequestParameter> &params) {
    QByteArray ret;
    bool first = true;
    foreach (O1RequestParameter h, params) {
        if (first) {
            first = false;
        } else {
            ret.append("&");
        }
        ret.append(QUrl::toPercentEncoding(h.name) + "=" + QUrl::toPercentEncoding(h.value));
    }
    return ret;
}

/// Build a base string for signing.
QByteArray O1::getRequestBase(const QList<O1RequestParameter> &oauthParams, const QList<O1RequestParameter> &otherParams, const QUrl &url, QNetworkAccessManager::Operation op) {
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
#if QT_VERSION >= 0x050100
    return QMessageAuthenticationCode::hash(baseString, secret, QCryptographicHash::Sha1).toBase64();
#else
    return hmacSha1(secret, baseString);
#endif
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

QByteArray O1::generateSignature(const QList<O1RequestParameter> headers, const QNetworkRequest &req, const QList<O1RequestParameter> &signingParameters, QNetworkAccessManager::Operation operation) {
    QByteArray signature;

    if (signatureMethod() == O2_SIGNATURE_TYPE_HMAC_SHA1) {
        signature = sign(headers, signingParameters, req.url(), operation, clientSecret(), tokenSecret());
    } else if (signatureMethod() == O2_SIGNATURE_TYPE_PLAINTEXT) {
        signature = clientSecret().toLatin1() + "&" + tokenSecret().toLatin1();
    }
    return signature;
}

void O1::link() {
    trace() << "O1::link";
    if (linked()) {
        trace() << "O1::link: Linked already";
        emit linkingSucceeded();
        return;
    }

    setLinked(false);
    setToken("");
    setTokenSecret("");

    // Start reply server
    replyServer_->listen(QHostAddress::Any, localPort());

    // Create request
    QNetworkRequest request(requestTokenUrl());

    // Create initial token request
    QList<O1RequestParameter> headers;
    headers.append(O1RequestParameter(O2_OAUTH_CALLBACK, QString(O2_CALLBACK_URL).arg(replyServer_->serverPort()).toLatin1()));
    headers.append(O1RequestParameter(O2_OAUTH_CONSUMER_KEY, clientId().toLatin1()));
    headers.append(O1RequestParameter(O2_OAUTH_NONCE, nonce()));
    headers.append(O1RequestParameter(O2_OAUTH_TIMESTAMP, QString::number(QDateTime::currentDateTimeUtc().toTime_t()).toLatin1()));
    headers.append(O1RequestParameter(O2_OAUTH_VERSION, "1.0"));
    headers.append(O1RequestParameter(O2_OAUTH_SIGNATURE_METHOD, signatureMethod().toLatin1()));
    headers.append(O1RequestParameter(O2_OAUTH_SIGNATURE, generateSignature(headers, request, QList<O1RequestParameter>(), QNetworkAccessManager::PostOperation)));

    // Clear request token
    requestToken_.clear();
    requestTokenSecret_.clear();

    // Post request
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
        qWarning() << "O1::onTokenRequestFinished: " << reply->errorString();
        return;
    }

    // Get request token and secret
    QByteArray data = reply->readAll();
    QMap<QString, QString> response = parseResponse(data);
    requestToken_ = response.value(O2_OAUTH_TOKEN, "");
    requestTokenSecret_ = response.value(O2_OAUTH_TOKEN_SECRET, "");
    setToken(requestToken_);
    setTokenSecret(requestTokenSecret_);

    // Checking for "oauth_callback_confirmed" is present and set to true
    QString oAuthCbConfirmed = response.value(O2_OAUTH_CALLBACK_CONFIRMED, "false");
    if (requestToken_.isEmpty() || requestTokenSecret_.isEmpty() || (oAuthCbConfirmed == "false")) {
        qWarning() << "O1::onTokenRequestFinished: No oauth_token, oauth_token_secret or oauth_callback_confirmed in response :" << data;
        emit linkingFailed();
        return;
    }

    // Continue authorization flow in the browser
    QUrl url(authorizeUrl());
#if QT_VERSION < 0x050000
    url.addQueryItem(O2_OAUTH_TOKEN, requestToken_);
    url.addQueryItem(O2_OAUTH_CALLBACK, QString(O2_CALLBACK_URL).arg(replyServer_->serverPort()).toLatin1());
#else
    QUrlQuery query(url);
    query.addQueryItem(O2_OAUTH_TOKEN, requestToken_);
    query.addQueryItem(O2_OAUTH_CALLBACK, QString(O2_CALLBACK_URL).arg(replyServer_->serverPort()).toLatin1());
    url.setQuery(query);
#endif
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
    trace() << "O1::exchangeToken";

    // Create token exchange request
    QNetworkRequest request(accessTokenUrl());
    QList<O1RequestParameter> oauthParams;
    oauthParams.append(O1RequestParameter(O2_OAUTH_CONSUMER_KEY, clientId().toLatin1()));
    oauthParams.append(O1RequestParameter(O2_OAUTH_VERSION, "1.0"));
    oauthParams.append(O1RequestParameter(O2_OAUTH_TIMESTAMP, QString::number(QDateTime::currentDateTimeUtc().toTime_t()).toLatin1()));
    oauthParams.append(O1RequestParameter(O2_OAUTH_NONCE, nonce()));
    oauthParams.append(O1RequestParameter(O2_OAUTH_TOKEN, requestToken_.toLatin1()));
    oauthParams.append(O1RequestParameter(O2_OAUTH_VERFIER, verifier_.toLatin1()));
    oauthParams.append(O1RequestParameter(O2_OAUTH_SIGNATURE_METHOD, signatureMethod().toLatin1()));
    oauthParams.append(O1RequestParameter(O2_OAUTH_SIGNATURE, generateSignature(oauthParams, request, QList<O1RequestParameter>(), QNetworkAccessManager::PostOperation)));

    // Post request
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
    trace() << "O1::onTokenExchangeFinished";

    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "O1::onTokenExchangeFinished: " << reply->errorString();
        return;
    }

    // Get access token and secret
    QByteArray data = reply->readAll();
    QMap<QString, QString> response = parseResponse(data);
    if (response.contains(O2_OAUTH_TOKEN) && response.contains(O2_OAUTH_TOKEN_SECRET)) {
        setToken(response.take(O2_OAUTH_TOKEN));
        setTokenSecret(response.take(O2_OAUTH_TOKEN_SECRET));
        // Set extra tokens if any
        if (!response.isEmpty()) {
            QVariantMap extraTokens;
            foreach (QString key, response.keys()) {
               extraTokens.insert(key, response.value(key));
            }
            setExtraTokens(extraTokens);
        }
        setLinked(true);
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
    return u.toLatin1();
}
