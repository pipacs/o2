#ifndef TWEETER_H
#define TWEETER_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QVariantMap>

#include "o1twitter.h"
#include "oxtwitter.h"

class Tweeter : public QObject
{
    Q_OBJECT
public:
    explicit Tweeter(QObject *parent = 0);

signals:
    void extraTokensReady(const QVariantMap &extraTokens);

public slots:
    void doOAuth();
    void doXAuth(const QString &username, const QString &password);
    void postStatusUpdate(const QString &message);

private slots:
    void onLinkedChanged();
    void onLinkingFailed();
    void onLinkingSucceeded();
    void onOpenBrowser(const QUrl &url);
    void onCloseBrowser();
    void tweetReplyDone();

private:
    O1Twitter* o1Twitter_;
    OXTwitter* oxTwitter_;
};

#endif // TWEETER_H
