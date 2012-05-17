#ifndef O2_H
#define O2_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QList>
#include <QPair>

#include "o2reply.h"

class O2ReplyServer;
class SimpleCrypt;

/// Simple OAuth2 authenticator.
class O2: public QObject {
    Q_OBJECT
    Q_PROPERTY(bool linked READ linked NOTIFY linkedChanged)
    Q_PROPERTY(QString token READ token NOTIFY tokenChanged)

public:
    /// Request type.
    enum RequestType {
        Get, Post
    };

    /// Constructor.
    /// @param  clientId        Client ID.
    /// @param  clientSecret    Client secret.
    /// @param  scope           Scope of authentication.
    /// @param  requestUrl      Authentication request target.
    /// @param  tokenUrl        Token exchange target.
    /// @param  refreshTokenUrl Token refresh target.
    /// @param  parent          Parent object.
    explicit O2(const QString &clientId, const QString &clientSecret, const QString &scope, const QUrl &requestUrl, const QUrl &tokenUrl, const QUrl &refreshTokenUrl, quint16 localPort, QObject *parent);

    /// Destructor.
    virtual ~O2();

    /// Are we authenticated?
    bool linked();

    /// Get authentication code.
    QString code();

    /// Get authentication token.
    QString token();

    /// Get refresh token.
    QString refreshToken();

    /// Get token expiration time (seconds from Epoch).
    int expires();

public slots:
    /// Authenticate.
    Q_INVOKABLE void link();

    /// De-authenticate.
    Q_INVOKABLE void unlink();

    /// Refresh token.
    void refresh();

signals:
    /// Emitted when client needs to open a web browser window, with the given URL.
    void openBrowser(const QUrl &url);

    /// Emitted when client can close the browser window.
    void closeBrowser();

    /// Emitted when authentication succeeded.
    void linkingSucceeded();

    /// Emitted when authentication failed.
    void linkingFailed();

    /// Emitted when the authentication status changed.
    void linkedChanged();

    /// Emitted when the request token changed.
    void tokenChanged();

    /// Emitted when a token refresh has been completed or failed.
    void refreshFinished(QNetworkReply::NetworkError error);

protected slots:
    /// Handle verification response.
    void onVerificationReceived(QMap<QString, QString>);

    /// Handle completion of a token request.
    void onTokenReplyFinished();

    /// Handle failure of a token request.
    void onTokenReplyError(QNetworkReply::NetworkError error);

    /// Handle completion of a refresh request.
    void onRefreshFinished();

    /// Handle failure of a refresh request.
    void onRefreshError(QNetworkReply::NetworkError error);

protected:
    /// Build HTTP request body.
    QByteArray buildRequestBody(const QMap<QString, QString> &parameters);

    /// Set authentication code.
    void setCode(const QString &v);

    /// Set authentication token.
    void setToken(const QString &v);

    /// Set refresh token.
    void setRefreshToken(const QString &v);

    /// Set token expiration time.
    void setExpires(int v);

protected:
    QString clientId_;
    QString clientSecret_;
    QString scope_;
    QUrl requestUrl_;
    QUrl tokenUrl_;
    QUrl refreshTokenUrl_;
    QString redirectUri_;
    QNetworkAccessManager *manager_;
    O2ReplyServer *replyServer_;
    QString code_;
    SimpleCrypt *crypt_;
    O2ReplyList timedReplies_;
    quint16 localPort_;
};

#endif // O2_H
