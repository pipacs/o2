#include <QCryptographicHash>
#include <QSettings>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QDateTime>
#include <QByteArray>

#include "o1.h"
#include "simplecrypt.h"
#include "o2replyserver.h"

/// HTTP request header name + value.
struct RequestHeader {
    RequestHeader(const QByteArray &n, const QByteArray &v): name(n), value(v) {}
    RequestHeader(const char *n, const QByteArray &v): name(n), value(v) {}
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
    int blockSize = 64; // HMAC-SHA-1 block size, defined in SHA-1 standard
    if (key.length() > blockSize) { // if key is longer than block size (64), reduce key length with SHA-1 compression
        key = QCryptographicHash::hash(key, QCryptographicHash::Sha1);
    }

    QByteArray innerPadding(blockSize, char(0x36)); // initialize inner padding with char "6"
    QByteArray outerPadding(blockSize, char(0x5c)); // initialize outer padding with char "\"
    // ascii characters 0x36 ("6") and 0x5c ("\") are selected because they have large
    // Hamming distance (http://en.wikipedia.org/wiki/Hamming_distance)

    for (int i = 0; i < key.length(); i++) {
        innerPadding[i] = innerPadding[i] ^ key.at(i); // XOR operation between every byte in key and innerpadding, of key length
        outerPadding[i] = outerPadding[i] ^ key.at(i); // XOR operation between every byte in key and outerpadding, of key length
    }

    // result = hash ( outerPadding CONCAT hash ( innerPadding CONCAT baseString ) ).toBase64
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
    base.append(QUrl::toPercentEncoding(baseUrl.toString(QUrl::RemoveQuery)) + "&");

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

    qDebug() << "getRequestBase: Signature base string:" << base;
    return base;
}

/// Sign a request with HMAC-SHA1.
static QByteArray sign(const QList<RequestHeader> &oauthHeaders, const QList<RequestHeader> &otherHeaders, const QUrl &baseUrl, QNetworkAccessManager::Operation op, const QString &consumerSecret, const QString &tokenSecret) {
    QByteArray baseString = getRequestBase(oauthHeaders, otherHeaders, baseUrl, op);
    QByteArray secret = QUrl::toPercentEncoding(consumerSecret) + "&" + QUrl::toPercentEncoding(tokenSecret);
    return hmacSha1(baseString, secret);
}

/// Build the "Authorization:" header value from a list of OAuth headers.
static QByteArray getAuthorizationHeader(const QList<RequestHeader> &oauthHeaders) {
    bool first = true;
    QByteArray ret("OAuth ");
    foreach (RequestHeader h, oauthHeaders) {
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
    connect(replyServer_, SIGNAL(verificationReceived(QMap<QString,QString>)), this, SLOT(onVerificationReceived(QMap<QString,QString>)));

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

    QNetworkRequest request(requestTokenUrl());
    request.setRawHeader("Authorization", getAuthorizationHeader(headers));
    delete manager_;
    manager_ = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager_->post(request, QByteArray());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onTokenRequestError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(finished()), this, SLOT(onTokenRequestFinished()));

//    requestParameters.append( qMakePair( OAUTH_KEY_CALLBACK, oauthCallbackUrl.toString()) );  // This is so ugly that it is almost beautiful.
//    requestParameters.append( qMakePair( OAUTH_KEY_SIGNATURE_METHOD, oauthSignatureMethod) );
//    requestParameters.append( qMakePair( OAUTH_KEY_CONSUMER_KEY, oauthConsumerKey ));
//    requestParameters.append( qMakePair( OAUTH_KEY_VERSION, oauthVersion ));
//    requestParameters.append( qMakePair( OAUTH_KEY_TIMESTAMP, this->oauthTimestamp() ));
//    requestParameters.append( qMakePair( OAUTH_KEY_NONCE, this->oauthNonce() ));

//            const QString OAUTH_KEY_CONSUMER("oauth_consumer");
//            const QString OAUTH_KEY_CONSUMER_KEY("oauth_consumer_key");
//            const QString OAUTH_KEY_TOKEN("oauth_token");
//            const QString OAUTH_KEY_TOKEN_SECRET("oauth_token_secret");
//            const QString OAUTH_KEY_SIGNATURE_METHOD("oauth_signature_method");
//            const QString OAUTH_KEY_TIMESTAMP("oauth_timestamp");
//            const QString OAUTH_KEY_NONCE("oauth_nonce");
//            const QString OAUTH_KEY_SIGNATURE("oauth_signature");
//            const QString OAUTH_KEY_CALLBACK("oauth_callback");
//            const QString OAUTH_KEY_VERIFIER("oauth_verifier");
//            const QString OAUTH_KEY_VERSION("oauth_version");

}

void O1::onTokenRequestError(QNetworkReply::NetworkError error) {
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    qDebug() << "O1::onTokenRequestError:" << (int)error << reply->errorString();
    emit linkingFailed();
}

void O1::onTokenRequestFinished() {
    qDebug() << "O1::onTokenRequestFinished";
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "" << reply->readAll();
        // FIXME: Continue authorization flow
        emit linkingSucceeded();
    }
    reply->deleteLater();
}

void O1::onVerificationReceived(QMap<QString, QString> params) {
    qDebug() << "O1::onVerificationReceived";
    qDebug() << "" << params;
}
