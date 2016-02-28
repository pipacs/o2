#ifndef OAUTHBASE_H
#define OAUTHBASE_H

#include <QByteArray>
#include <QObject>
#include <QMap>
#include <QString>
#include <QUrl>
#include <QVariantMap>

#include "o2abstractstore.h"
#include "o2replyserver.h"

/// Base class of OAuth 1 and 2 authenticators
class O2BaseAuth : public QObject {
    Q_OBJECT

public:
    explicit O2BaseAuth(QObject *parent = 0);

public:
    /// Are we authenticated?
    Q_PROPERTY(bool linked READ linked WRITE setLinked NOTIFY linkedChanged)
    bool linked();

    /// Authentication token.
    QString token();

    /// Authentication token secret.
    QString tokenSecret();

    /// Provider-specific extra tokens, available after a successful authentication
    Q_PROPERTY(QVariantMap extraTokens READ extraTokens NOTIFY extraTokensChanged)
    QVariantMap extraTokens();

    /// Client application ID.
    /// O1 instances with the same (client ID, client secret) share the same "linked", "token" and "tokenSecret" properties.
    Q_PROPERTY(QString clientId READ clientId WRITE setClientId NOTIFY clientIdChanged)
    QString clientId();
    void setClientId(const QString &value);

    /// Client application secret.
    /// O1 instances with the same (client ID, client secret) share the same "linked", "token" and "tokenSecret" properties.
    Q_PROPERTY(QString clientSecret READ clientSecret WRITE setClientSecret NOTIFY clientSecretChanged)
    QString clientSecret();
    void setClientSecret(const QString &value);

    /// Token request URL.
    Q_PROPERTY(QUrl requestTokenUrl READ requestTokenUrl WRITE setRequestTokenUrl NOTIFY requestTokenUrlChanged)
    QUrl requestTokenUrl();
    void setRequestTokenUrl(const QUrl &value);

    /// Authorization URL.
    Q_PROPERTY(QUrl authorizeUrl READ authorizeUrl WRITE setAuthorizeUrl NOTIFY authorizeUrlChanged)
    QUrl authorizeUrl();
    void setAuthorizeUrl(const QUrl &value);

    /// Access token URL.
    Q_PROPERTY(QUrl accessTokenUrl READ accessTokenUrl WRITE setAccessTokenUrl NOTIFY accessTokenUrlChanged)
    QUrl accessTokenUrl();
    void setAccessTokenUrl(const QUrl &value);

    /// TCP port number to use in local redirections.
    /// The OAuth "redirect_uri" will be set to "http://localhost:<localPort>/".
    /// If localPort is set to 0 (default), O2 will replace it with a free one.
    Q_PROPERTY(int localPort READ localPort WRITE setLocalPort NOTIFY localPortChanged)
    int localPort();
    void setLocalPort(int value);

    /// Sets the storage object to use for storing the OAuth tokens on a peristent medium
    void setStore(O2AbstractStore *store);

public slots:
    /// Authenticate.
    Q_INVOKABLE virtual void link() = 0;

    /// De-authenticate.
    Q_INVOKABLE virtual void unlink() = 0;

signals:
    /// Emitted when client needs to open a web browser window, with the given URL.
    void openBrowser(const QUrl &url);

    /// Emitted when client can close the browser window.
    void closeBrowser();

    /// Emitted when authentication/deauthentication succeeded.
    void linkingSucceeded();

    /// Emitted when authentication/deauthentication failed.
    void linkingFailed();

    // Property change signals

    void linkedChanged();
    void clientIdChanged();
    void clientSecretChanged();
    void requestTokenUrlChanged();
    void authorizeUrlChanged();
    void accessTokenUrlChanged();
    void localPortChanged();
    void extraTokensChanged();

protected:
    /// Set authentication token.
    void setToken(const QString &v);

    /// Set authentication token secret.
    void setTokenSecret(const QString &v);

    /// Set the linked state
    void setLinked(bool v);

    /// Set extra tokens found in OAuth response
    void setExtraTokens(QVariantMap extraTokens);

protected:
    QString clientId_;
    QString clientSecret_;
    QString redirectUri_;
    QString requestToken_;
    QString requestTokenSecret_;
    QUrl requestTokenUrl_;
    QUrl authorizeUrl_;
    QUrl accessTokenUrl_;
    quint16 localPort_;
    O2AbstractStore *store_;
    QVariantMap extraTokens_;
};

#endif // OAUTHBASE_H
