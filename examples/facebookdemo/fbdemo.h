#ifndef FBDEMO_H
#define FBDEMO_H

#include <QObject>

#include "o2facebook.h"

class FBDemo : public QObject
{
    Q_OBJECT

public:
    explicit FBDemo(QObject *parent = 0);

signals:
    void extraTokensReady(const QVariantMap &extraTokens);
    void linkingFailed();
    void linkingSucceeded();

public slots:
    void doOAuth(O2::GrantFlow grantFlowType);

private slots:
    void onLinkedChanged();
    void onLinkingSucceeded();
    void onOpenBrowser(const QUrl &url);
    void onCloseBrowser();
    void onUrlChanged(const QUrl &url);

private:
    O2Facebook *o2Facebook_;
};

#endif // FBDEMO_H
