#ifndef VMDEMO_H
#define VMDEMO_H

#include <QObject>

#include "o2vimeo.h"

class VimeoDemo : public QObject
{
    Q_OBJECT

public:
    explicit VimeoDemo(QObject *parent = 0);

signals:
    void extraTokensReady(const QVariantMap &extraTokens);
    void linkingFailed();
    void linkingSucceeded();
    void userNameReceived();
    void userNameFailed();

public slots:
    void doOAuth(O2::GrantFlow grantFlowType);
    void getUserName();

private slots:
    void onLinkedChanged();
    void onLinkingSucceeded();
    void onOpenBrowser(const QUrl &url);
    void onCloseBrowser();
    void onFinished(int, QNetworkReply::NetworkError, QByteArray);

private:
    O2Vimeo *o2Vimeo_;
    int requestId_;
};

#endif // VMDEMO_H
