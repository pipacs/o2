#include <QDesktopServices>
#include <QDebug>

#include "fbdemo.h"
#include "o2globals.h"
#include "o2settingsstore.h"

const char FB_APP_KEY[] = "227896037359072";
const char FB_APP_SECRET[] = "3d35b063872579cf7213e09e76b65ceb";

const char FB_REQUEST_URL[] = "https://www.facebook.com/dialog/oauth";
const char FB_TOKEN_URL[] = "https://graph.facebook.com/oauth/access_token";

const int localPort = 8888;

FBDemo::FBDemo(QObject *parent) :
    QObject(parent) {
    o2Facebook_ = new O2Facebook(this);

    o2Facebook_->setClientId(FB_APP_KEY);
    o2Facebook_->setClientSecret(FB_APP_SECRET);
    o2Facebook_->setLocalPort(localPort);
    o2Facebook_->setRequestUrl(FB_REQUEST_URL);
    o2Facebook_->setTokenUrl(FB_TOKEN_URL);
//    o2Facebook_->setGrantFlow(O2::GrantFlowImplicit);

    // Create a store object for writing the received tokens
    O2SettingsStore *store = new O2SettingsStore(O2_ENCRYPTION_KEY);
    store->setGroupKey("facebook");
    o2Facebook_->setStore(store);

    connect(o2Facebook_, SIGNAL(linkedChanged()),
            this, SLOT(onLinkedChanged()));
    connect(o2Facebook_, SIGNAL(linkingFailed()),
            this, SIGNAL(linkingFailed()));
    connect(o2Facebook_, SIGNAL(linkingSucceeded()),
            this, SLOT(onLinkingSucceeded()));
    connect(o2Facebook_, SIGNAL(openBrowser(QUrl)),
            this, SLOT(onOpenBrowser(QUrl)));
    connect(o2Facebook_, SIGNAL(closeBrowser()),
            this, SLOT(onCloseBrowser()));
}

void FBDemo::doOAuth() {
    qDebug() << "Starting OAuth 2...";
    o2Facebook_->link();
}

void FBDemo::onOpenBrowser(const QUrl &url) {
    QDesktopServices::openUrl(url);
}

void FBDemo::onCloseBrowser() {
}

void FBDemo::onLinkedChanged() {
    qDebug() << "Link changed!";
}

void FBDemo::onLinkingSucceeded() {
    O2Facebook* o1t = qobject_cast<O2Facebook *>(sender());

     QVariantMap extraTokens = o1t->extraTokens();

    if (!extraTokens.isEmpty()) {
        emit extraTokensReady(extraTokens);

        qDebug() << "Extra tokens in response:";
        foreach (QString key, extraTokens.keys()) {
            qDebug() << "\t" << key << ":" << extraTokens.value(key).toString();
        }
    }
    emit linkingSucceeded();
}
