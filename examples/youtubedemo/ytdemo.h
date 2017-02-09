#ifndef YTDEMO_H
#define YTDEMO_H

#include <QObject>

#include "o2google.h"

class YTDemo : public QObject
{
    Q_OBJECT

public:
    explicit YTDemo(QObject *parent = 0);

signals:
    void extraTokensReady(const QVariantMap &extraTokens);
    void linkingFailed();
    void linkingSucceeded();
    void channelInfoReceived();
    void channelInfoFailed();

public slots:
    void doOAuth(O2::GrantFlow grantFlowType);
    void getUserChannelInfo();

private slots:
    void onLinkedChanged();
    void onLinkingSucceeded();
    void onOpenBrowser(const QUrl &url);
    void onCloseBrowser();
    void onFinished(int, QNetworkReply::NetworkError, QByteArray);

private:
    O2Google *o2Google_;
    int requestId_;
};

#endif // YTDEMO_H
