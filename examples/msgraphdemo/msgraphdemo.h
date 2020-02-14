#ifndef MSGRAPH_H
#define MSGRAPH_H

#include <QObject>

#include "o2msgraph.h"

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
    void onCloseBrowser();
    void onFinished(int, QNetworkReply::NetworkError, QByteArray);

private:
    O2Msgraph *o2Msgraph_;
    int requestId_;
};

#endif // MSGRAPHDEMO_H
