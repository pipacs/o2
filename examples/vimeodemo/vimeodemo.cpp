#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QMetaEnum>
#include <QDebug>
#include <QRegExp>

#include "vimeodemo.h"
#include "o0globals.h"
#include "o0settingsstore.h"
#include "o2requestor.h"

const char VIMEO_APP_KEY[] = "YOUR_VIMEO_APP_KEY";
const char VIMEO_APP_SECRET[] = "YOUT_VIMEO_APP_SECRET";
const char VIMEO_SCOPE[] = "public";
const char VIMEO_USER_INFO_URL[] = "https://api.vimeo.com/me";

const int localPort = 8888;

#define QENUM_NAME(o,e,v) (o::staticMetaObject.enumerator(o::staticMetaObject.indexOfEnumerator(#e)).valueToKey((v)))
#define GRANTFLOW_STR(v) QString(QENUM_NAME(O2, GrantFlow, v))

VimeoDemo::VimeoDemo(QObject *parent) :
    QObject(parent), requestId_(0) {
    o2Vimeo_ = new O2Vimeo(this);

    o2Vimeo_->setClientId(VIMEO_APP_KEY);
    o2Vimeo_->setClientSecret(VIMEO_APP_SECRET);
    o2Vimeo_->setLocalPort(localPort);
    o2Vimeo_->setScope(VIMEO_SCOPE);

    // Create a store object for writing the received tokens
    O0SettingsStore *store = new O0SettingsStore(O2_ENCRYPTION_KEY);
    store->setGroupKey("google");
    o2Vimeo_->setStore(store);

    connect(o2Vimeo_, SIGNAL(linkedChanged()), this, SLOT(onLinkedChanged()));
    connect(o2Vimeo_, SIGNAL(linkingFailed()), this, SIGNAL(linkingFailed()));
    connect(o2Vimeo_, SIGNAL(linkingSucceeded()), this, SLOT(onLinkingSucceeded()));
    connect(o2Vimeo_, SIGNAL(openBrowser(QUrl)), this, SLOT(onOpenBrowser(QUrl)));
    connect(o2Vimeo_, SIGNAL(closeBrowser()), this, SLOT(onCloseBrowser()));
}

void VimeoDemo::doOAuth(O2::GrantFlow grantFlowType) {
    qDebug() << "Starting OAuth 2 with grant flow type" << GRANTFLOW_STR(grantFlowType) << "...";
    o2Vimeo_->setGrantFlow(grantFlowType);
    o2Vimeo_->unlink();
    o2Vimeo_->link();
}

void VimeoDemo::getUserName() {
    if (!o2Vimeo_->linked()) {
        qWarning() << "ERROR: Application is not linked!";
        emit linkingFailed();
        return;
    }

    QString userInfoURL = QString(VIMEO_USER_INFO_URL);
    QNetworkRequest request = QNetworkRequest(QUrl(userInfoURL));
    QNetworkAccessManager *mgr = new QNetworkAccessManager(this);
    O2Requestor *requestor = new O2Requestor(mgr, o2Vimeo_, this);
    requestId_ = requestor->get(request);
    connect(requestor, SIGNAL(finished(int, QNetworkReply::NetworkError, QByteArray)),
        this, SLOT(onFinished(int, QNetworkReply::NetworkError, QByteArray))
    );
    qDebug() << "Getting user channel info... Please wait.";
}

void VimeoDemo::onOpenBrowser(const QUrl &url) {
    QDesktopServices::openUrl(url);
}

void VimeoDemo::onCloseBrowser() {
}

void VimeoDemo::onLinkedChanged() {
    qDebug() << "Link changed!";
}

void VimeoDemo::onLinkingSucceeded() {
    if (!o2Vimeo_->linked()) {
        return;
    }
    QVariantMap extraTokens = o2Vimeo_->extraTokens();
    if (!extraTokens.isEmpty()) {
        emit extraTokensReady(extraTokens);
        qDebug() << "Extra tokens in response:";
        foreach (QString key, extraTokens.keys()) {
            qDebug() << "\t" << key << ":" << (extraTokens.value(key).toString().left(3) + "...");
        }
    }
    emit linkingSucceeded();
}

void VimeoDemo::onFinished(int requestId, QNetworkReply::NetworkError error, QByteArray replyData) {
    if (requestId != requestId_)
        return;

    if (error != QNetworkReply::NoError) {
        qWarning() << "Reply error:" << error;
        emit userNameFailed();
        return;
    }

    QString reply(replyData);
    bool errorFound = reply.contains("error");
    if (errorFound) {
        qDebug() << "Request failed";
        emit userNameFailed();
        return;
    }

    QRegExp nameRE("\"name\":\"([^\"]+)\"");
    if (nameRE.indexIn(reply) == -1) {
        qDebug() << "Can not parse reply:" << reply;
        emit userNameFailed();
        return;
    }

    qInfo() << "User name: " << nameRE.cap(1);
    emit userNameReceived();
}
