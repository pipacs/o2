#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QMetaEnum>
#include <QDebug>
#include <QRegExp>

#include "msgraphdemo.h"
#include "o0globals.h"
#include "o0settingsstore.h"
#include "o2requestor.h"

const char MSGRAPH_APP_ID[] = "YOUR_MSGRAPH_APP_ID";
const char MSGRAPH_APP_SECRET[] = "YOUR_MSGRAPH_APP_SECRET";
const char MSGRAPH_SCOPE[] = "User.Read";
const char MSGRAPH_USER_INFO_URL[] = "https://graph.microsoft.com/v1.0/me";

const int localPort = 8888;

#define QENUM_NAME(o,e,v) (o::staticMetaObject.enumerator(o::staticMetaObject.indexOfEnumerator(#e)).valueToKey((v)))
#define GRANTFLOW_STR(v) QString(QENUM_NAME(O2, GrantFlow, v))

MsgraphDemo::MsgraphDemo(QObject *parent) :
    QObject(parent), requestId_(0) {
    o2Msgraph_ = new O2Msgraph(this);

    o2Msgraph_->setClientId(MSGRAPH_APP_ID);
    o2Msgraph_->setClientSecret(MSGRAPH_APP_SECRET);
    o2Msgraph_->setLocalPort(localPort);
    o2Msgraph_->setScope(MSGRAPH_SCOPE);

    // Create a store object for writing the received tokens
    O0SettingsStore *store = new O0SettingsStore(O2_ENCRYPTION_KEY);
    store->setGroupKey("msgraph");
    o2Msgraph_->setStore(store);

    connect(o2Msgraph_, SIGNAL(linkedChanged()), this, SLOT(onLinkedChanged()));
    connect(o2Msgraph_, SIGNAL(linkingFailed()), this, SIGNAL(linkingFailed()));
    connect(o2Msgraph_, SIGNAL(linkingSucceeded()), this, SLOT(onLinkingSucceeded()));
    connect(o2Msgraph_, SIGNAL(openBrowser(QUrl)), this, SLOT(onOpenBrowser(QUrl)));
    connect(o2Msgraph_, SIGNAL(closeBrowser()), this, SLOT(onCloseBrowser()));
}

void MsgraphDemo::doOAuth(O2::GrantFlow grantFlowType) {
    qDebug() << "Starting OAuth 2 with grant flow type" << GRANTFLOW_STR(grantFlowType) << "...";
    o2Msgraph_->setGrantFlow(grantFlowType);
    o2Msgraph_->unlink();
    o2Msgraph_->link();
}

void MsgraphDemo::getUserPrincipalName() {
    if (!o2Msgraph_->linked()) {
        qWarning() << "ERROR: Application is not linked!";
        emit linkingFailed();
        return;
    }

    QString userInfoURL = QString(MSGRAPH_USER_INFO_URL);
    QNetworkRequest request = QNetworkRequest(QUrl(userInfoURL));
    QNetworkAccessManager *mgr = new QNetworkAccessManager(this);
    O2Requestor *requestor = new O2Requestor(mgr, o2Msgraph_, this);
    requestor->setAddAccessTokenInQuery(false);
    requestor->setAccessTokenInAuthenticationHTTPHeaderFormat("Bearer %1");
    requestId_ = requestor->get(request);
    connect(requestor, SIGNAL(finished(int, QNetworkReply::NetworkError, QByteArray)),
        this, SLOT(onFinished(int, QNetworkReply::NetworkError, QByteArray))
    );
    qDebug() << "Getting user info... Please wait.";
}

void MsgraphDemo::onOpenBrowser(const QUrl &url) {
    QDesktopServices::openUrl(url);
}

void MsgraphDemo::onCloseBrowser() {
}

void MsgraphDemo::onLinkedChanged() {
    qDebug() << "Link changed!";
}

void MsgraphDemo::onLinkingSucceeded() {
    O2Msgraph *o2t = qobject_cast<O2Msgraph *>(sender());
    if (!o2t->linked()) {
        return;
    }
    QVariantMap extraTokens = o2t->extraTokens();
    if (!extraTokens.isEmpty()) {
        emit extraTokensReady(extraTokens);
        qDebug() << "Extra tokens in response:";
        foreach (QString key, extraTokens.keys()) {
            qDebug() << "\t" << key << ":" << (extraTokens.value(key).toString().left(3) + "...");
        }
    }
    emit linkingSucceeded();
}

void MsgraphDemo::onFinished(int requestId, QNetworkReply::NetworkError error, QByteArray replyData) {
    if (requestId != requestId_)
        return;

    if (error != QNetworkReply::NoError) {
        qWarning() << "Reply error:" << error;
        emit userPrincipalNameFailed();
        return;
    }

    QString reply(replyData);
    bool errorFound = reply.contains("error");
    if (errorFound) {
        qDebug() << "Request failed";
        emit userPrincipalNameFailed();
        return;
    }

    QRegExp userPrincipalNameRE("\"userPrincipalName\":\"([^\"]+)\"");
    if (userPrincipalNameRE.indexIn(reply) == -1) {
        qDebug() << "Can not parse reply:" << reply;
        emit userPrincipalNameFailed();
        return;
    }

    qInfo() << "userPrincipalName: " << userPrincipalNameRE.cap(1);
    emit userPrincipalNameReceived();
}
