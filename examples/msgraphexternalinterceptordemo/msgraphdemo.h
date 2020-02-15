#ifndef MSGRAPHDEMO_H
#define MSGRAPHDEMO_H

#include <QObject>

#include "o2msgraph.h"
#include "webwindow.h"

class MsgraphDemo : public QObject
{
    Q_OBJECT

public:
    explicit MsgraphDemo(QObject *parent = 0);

signals:
    void extraTokensReady(const QVariantMap &extraTokens);
    void linkingFailed();
    void linkingSucceeded();
    void userPrincipalNameReceived();
    void userPrincipalNameFailed();

public slots:
    void doOAuth(O2::GrantFlow grantFlowType);
    void getUserPrincipalName();

private slots:
    void onLinkedChanged();
    void onLinkingSucceeded();
    void onOpenBrowser(const QUrl &url);
    void onAuthWindowCallbackCalled(const QString &inURLString);
    void onAuthWindowClosed();
    void onCloseBrowser();
    void onFinished(int, QNetworkReply::NetworkError, QByteArray);

private:
    O2Msgraph *o2Msgraph_;
    WebWindow* authDialog;
    int requestId_;
};

#endif // MSGRAPHDEMO_H
