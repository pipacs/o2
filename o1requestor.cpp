#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QNetworkReply>
#include <QNetworkAccessManager>

#include "o1requestor.h"
#include "o2globals.h"

/// A timer connected to a network reply.
class TimedReply: public QTimer {
    Q_OBJECT

public:
    explicit TimedReply(QNetworkReply *parent): QTimer(parent) {
        setSingleShot(true);
        setInterval(60 * 1000); // FIXME: Expose me
        connect(this, SIGNAL(error(QNetworkReply::NetworkError)), parent, SIGNAL(error(QNetworkReply::NetworkError)));
        connect(this, SIGNAL(timeout()), this, SLOT(onTimeout()));
    }

signals:
    void error(QNetworkReply::NetworkError);

public slots:
    void onTimeout() {emit error(QNetworkReply::TimeoutError);}
};

O1Requestor::O1Requestor(QNetworkAccessManager *manager, O1 *authenticator, QObject *parent): QObject(parent) {
    manager_ = manager;
    authenticator_ = authenticator;
}

QNetworkReply *O1Requestor::get(const QNetworkRequest &req, const QList<O1RequestParameter> &signingParameters) {
    QNetworkRequest request = setup(req, signingParameters, QNetworkAccessManager::GetOperation);
    return addTimer(manager_->get(request));
}

QNetworkReply *O1Requestor::post(const QNetworkRequest &req, const QList<O1RequestParameter> &signingParameters, const QByteArray &data) {
    QNetworkRequest request = setup(req, signingParameters, QNetworkAccessManager::PostOperation);
    return addTimer(manager_->post(request, data));
}

QNetworkReply *O1Requestor::put(const QNetworkRequest &req, const QList<O1RequestParameter> &signingParameters, const QByteArray &data) {
    QNetworkRequest request = setup(req, signingParameters, QNetworkAccessManager::PutOperation);
    return addTimer(manager_->put(request, data));
}

QNetworkReply *O1Requestor::addTimer(QNetworkReply *reply) {
    (void)new TimedReply(reply);
    return reply;
}

QNetworkRequest O1Requestor::setup(const QNetworkRequest &req, const QList<O1RequestParameter> &signingParameters, QNetworkAccessManager::Operation operation) {
    // Collect OAuth parameters
    QList<O1RequestParameter> oauthParams;
    oauthParams.append(O1RequestParameter(OAUTH_CONSUMER_KEY, authenticator_->clientId().toAscii()));
    oauthParams.append(O1RequestParameter(OAUTH_VERSION, "1.0"));
    oauthParams.append(O1RequestParameter(OAUTH_TOK, authenticator_->token().toAscii()));
    oauthParams.append(O1RequestParameter(OAUTH_SIG_METHOD, SIG_TYPE_HMAC_SHA1));
    oauthParams.append(O1RequestParameter(OAUTH_NONCE, O1::nonce()));
    oauthParams.append(O1RequestParameter(OAUTH_TIMESTAMP, QString::number(QDateTime::currentDateTimeUtc().toTime_t()).toAscii()));

    // Add signature parameter
    QByteArray signature = authenticator_->sign(oauthParams, signingParameters, req.url(), operation, authenticator_->clientSecret(), authenticator_->tokenSecret());
    oauthParams.append(O1RequestParameter(OAUTH_SIG, signature));

    // Return a copy of the original request with authorization header set
    QNetworkRequest request(req);
    request.setRawHeader(HTTP_AUTH_HEADER, O1::buildAuthorizationHeader(oauthParams));
    return request;
}

#include "o1requestor.moc"
