#ifndef FBDEMO_H
#define FBDEMO_H

#include <QObject>

#include "o2facebook.h"

class FBDemo : public QObject
{
    Q_OBJECT

public:
    explicit FBDemo(QObject *parent = 0);

Q_SIGNALS:
    void extraTokensReady(const QVariantMap &extraTokens);
    void linkingFailed();
    void linkingSucceeded();

public Q_SLOTS:
    void doOAuth(O2::GrantFlow grantFlowType);
    void validateToken();

private Q_SLOTS:
    void onLinkedChanged();
    void onLinkingSucceeded();
    void onOpenBrowser(const QUrl &url);
    void onCloseBrowser();
    void onFinished();

private:
    O2Facebook *o2Facebook_;
};

#endif // FBDEMO_H
