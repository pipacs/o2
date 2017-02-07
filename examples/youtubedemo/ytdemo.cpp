#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QMetaEnum>
#include <QDebug>

#include "ytdemo.h"
#include "o0globals.h"
#include "o0settingsstore.h"
#include "o2requestor.h"

const char GOOGLE_APP_KEY[]    = "YOUR_YOUTUBE_APP_KEY";
const char GOOGLE_APP_SECRET[] = "YOUR_YOUTUBE_APP_SECRET";
const char YOUTUBE_SCOPE[] = "https://www.googleapis.com/auth/youtube.upload https://www.googleapis.com/auth/youtube";
const char YOUTUBE_CHANNELS_LIST_URL[] = "https://www.googleapis.com/youtube/v3/channels?part=status&mine=true";

const int localPort = 8888;

#define QENUM_NAME(o,e,v) (o::staticMetaObject.enumerator(o::staticMetaObject.indexOfEnumerator(#e)).valueToKey((v)))
#define GRANTFLOW_STR(v) QString(QENUM_NAME(O2, GrantFlow, v))

YTDemo::YTDemo(QObject *parent) :
    QObject(parent), requestId_(0) {
    o2Google_ = new O2Google(this);

    o2Google_->setClientId(GOOGLE_APP_KEY);
    o2Google_->setClientSecret(GOOGLE_APP_SECRET);
    o2Google_->setLocalPort(localPort);
    o2Google_->setScope(YOUTUBE_SCOPE);

    // Create a store object for writing the received tokens
    O0SettingsStore *store = new O0SettingsStore(O2_ENCRYPTION_KEY);
    store->setGroupKey("google");
    o2Google_->setStore(store);

    connect(o2Google_, SIGNAL(linkedChanged()), this, SLOT(onLinkedChanged()));
    connect(o2Google_, SIGNAL(linkingFailed()), this, SIGNAL(linkingFailed()));
    connect(o2Google_, SIGNAL(linkingSucceeded()), this, SLOT(onLinkingSucceeded()));
    connect(o2Google_, SIGNAL(openBrowser(QUrl)), this, SLOT(onOpenBrowser(QUrl)));
    connect(o2Google_, SIGNAL(closeBrowser()), this, SLOT(onCloseBrowser()));
}

void YTDemo::doOAuth(O2::GrantFlow grantFlowType) {
    qDebug() << "Starting OAuth 2 with grant flow type" << GRANTFLOW_STR(grantFlowType) << "...";
    o2Google_->setGrantFlow(grantFlowType);
    o2Google_->unlink();
    o2Google_->link();
}

void YTDemo::getUserChannelInfo() {
    if (!o2Google_->linked()) {
        qWarning() << "ERROR: Application is not linked!";
        emit linkingFailed();
        return;
    }

    QString channelsListUrl = QString(YOUTUBE_CHANNELS_LIST_URL);
    QNetworkRequest request = QNetworkRequest(QUrl(channelsListUrl));
    QNetworkAccessManager *mgr = new QNetworkAccessManager(this);
    O2Requestor *requestor = new O2Requestor(mgr, o2Google_, this);
    requestId_ = requestor->get(request);
    connect(requestor, SIGNAL(finished(int, QNetworkReply::NetworkError, QByteArray)),
        this, SLOT(onFinished(int, QNetworkReply::NetworkError, QByteArray))
    );
    qDebug() << "Getting user channel info... Please wait.";
}

void YTDemo::onOpenBrowser(const QUrl &url) {
    QDesktopServices::openUrl(url);
}

void YTDemo::onCloseBrowser() {
}

void YTDemo::onLinkedChanged() {
    qDebug() << "Link changed!";
}

void YTDemo::onLinkingSucceeded() {
    O2Google *o2t = qobject_cast<O2Google *>(sender());
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

void YTDemo::onFinished(int requestId, QNetworkReply::NetworkError error, QByteArray replyData) {
    if (requestId != requestId_)
        return;

    if (error != QNetworkReply::NoError) {
        qWarning() << "Reply error:" << error;
        emit channelInfoFailed();
        return;
    }

    QString reply(replyData);
    bool errorFound = reply.contains("error");
    if (errorFound) {
        qDebug() << "Request failed";
        emit channelInfoFailed();
        return;
    }

    qInfo() << "Channel info: " << reply;
    emit channelInfoReceived();
}
