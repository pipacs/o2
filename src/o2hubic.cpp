#include "o2hubic.h"
#include "o2globals.h"
#include "o2replyserver.h"
#include <QHostAddress>

static const char *HubicScope = "usage.r,account.r,getAllLinks.r,credentials.r,activate.w,links.drw";
static const char *HubicEndpoint = "https://api.hubic.com/oauth/auth/";
static const char *HubicTokenUrl = "https://api.hubic.com/oauth/token/";
static const char *HubicRefreshUrl = "https://api.hubic.com/oauth/token/";

O2Hubic::O2Hubic(QObject *parent): O2(parent) {
    setRequestUrl(HubicEndpoint);
    setTokenUrl(HubicTokenUrl);
    setRefreshTokenUrl(HubicRefreshUrl);
    setScope(HubicScope);
}

void O2Hubic::link()
{
    qDebug() << "O2::link";
    if (linked()) {
        qDebug() << " Linked already";
        emit linkingSucceeded();
        return;
    }

    // Start listening to authentication replies
    replyServer_->listen(QHostAddress::Any, localPort_);

    // Save redirect URI, as we have to reuse it when requesting the access token
    redirectUri_ = QString("http://localhost:%1/").arg(replyServer_->serverPort());

    // Assemble intial authentication URL
    QList<QPair<QString, QString> > parameters;
    parameters.append(qMakePair(QString(O2_OAUTH2_RESPONSE_TYPE), (grantFlow_ == GrantFlowAuthorizationCode) ? QString(O2_OAUTH2_CODE) : QString(O2_OAUTH2_TOKEN)));
    parameters.append(qMakePair(QString(O2_OAUTH2_CLIENT_ID), clientId_));
    parameters.append(qMakePair(QString(O2_OAUTH2_REDIRECT_URI), redirectUri_));
    // parameters.append(qMakePair(QString(OAUTH2_REDIRECT_URI), QString(QUrl::toPercentEncoding(redirectUri_))));
    parameters.append(qMakePair(QString(O2_OAUTH2_SCOPE), scope_));

    // Show authentication URL with a web browser
    QUrl url(requestUrl_);
    #if QT_VERSION < 0x050000
    url.setQueryItems(parameters);
    #else
    QUrlQuery query(url);
    query.setQueryItems(parameters);
    url.setQuery(query);
    #endif

    qDebug() << "Emit openBrowser" << url.toString();
    emit openBrowser(url);
}
