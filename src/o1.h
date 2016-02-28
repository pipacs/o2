#ifndef O1_H
#define O1_H

#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkReply>

#include "o2baseauth.h"

class O2ReplyServer;

/// Request parameter (name-value pair) participating in authentication.
struct O1RequestParameter {
    O1RequestParameter(const QByteArray &n, const QByteArray &v): name(n), value(v) {
    }
    bool operator <(const O1RequestParameter &other) const {
        return (name == other.name)? (value < other.value): (name < other.name);
    }
    QByteArray name;
    QByteArray value;
};

/// Simple OAuth 1.0 authenticator.
class O1: public O2BaseAuth {
    Q_OBJECT

public:
    /// Signature method
    Q_PROPERTY(QString signatureMethod READ signatureMethod WRITE setSignatureMethod NOTIFY signatureMethodChanged)
    QString signatureMethod();
    void setSignatureMethod(const QString &value);

    /// Constructor.
    explicit O1(QObject *parent = 0);

    /// Parse a URL-encoded response string.
    static QMap<QString, QString> parseResponse(const QByteArray &response);

    /// Build the value of the "Authorization:" header.
    static QByteArray buildAuthorizationHeader(const QList<O1RequestParameter> &oauthParams);

    /// Create unique bytes to prevent replay attacks.
    static QByteArray nonce();

    /// Generate signature string depending on signature method type
    QByteArray generateSignature(const QList<O1RequestParameter> headers, const QNetworkRequest &req, const QList<O1RequestParameter> &signingParameters, QNetworkAccessManager::Operation operation);

    /// Calculate the HMAC-SHA1 signature of a request.
    /// @param  oauthParams     OAuth parameters.
    /// @param  otherParams     Other parameters participating in signing.
    /// @param  URL             Request URL. May contain query parameters, but they will not be used for signing.
    /// @param  op              HTTP operation.
    /// @param  consumerSecret  Consumer (application) secret.
    /// @param  tokenSecret     Authorization token secret (empty if not yet available).
    /// @return Signature that can be used as the value of the "oauth_signature" parameter.
    static QByteArray sign(const QList<O1RequestParameter> &oauthParams, const QList<O1RequestParameter> &otherParams, const QUrl &url, QNetworkAccessManager::Operation op, const QString &consumerSecret, const QString &tokenSecret);

    /// Build a base string for signing.
    static QByteArray getRequestBase(const QList<O1RequestParameter> &oauthParams, const QList<O1RequestParameter> &otherParams, const QUrl &url, QNetworkAccessManager::Operation op);

    /// Build a concatenated/percent-encoded string from a list of headers.
    static QByteArray encodeHeaders(const QList<O1RequestParameter> &headers);

    /// Construct query string from list of headers
    static QByteArray createQueryParams(const QList<O1RequestParameter> &params);

public slots:
    /// Authenticate.
    Q_INVOKABLE virtual void link();

    /// De-authenticate.
    Q_INVOKABLE virtual void unlink();

signals:
    void signatureMethodChanged();

protected slots:
    /// Handle verification received from the reply server.
    virtual void onVerificationReceived(QMap<QString,QString> params);

    /// Handle token request error.
    virtual void onTokenRequestError(QNetworkReply::NetworkError error);

    /// Handle token request finished.
    virtual void onTokenRequestFinished();

    /// Handle token exchange error.
    void onTokenExchangeError(QNetworkReply::NetworkError error);

    /// Handle token exchange finished.
    void onTokenExchangeFinished();

protected:
    /// Exchange temporary token to authentication token
    void exchangeToken();

    QString verifier_;
    QString signatureMethod_;
    QNetworkAccessManager *manager_;
    O2ReplyServer *replyServer_;
};

#endif // O1_H
