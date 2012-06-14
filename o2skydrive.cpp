#include <QDebug>
#include <QDateTime>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QDesktopServices>

#include "o2skydrive.h"

#define trace() if (1) qDebug()
// define trace() if (0) qDebug()

O2Skydrive::O2Skydrive(QObject *parent): O2(parent) {
    setRequestUrl("https://login.live.com/oauth20_authorize.srf");
    setTokenUrl("https://login.live.com/oauth20_token.srf");
}

void O2Skydrive::link() {
    trace() << "O2::link";
    if (linked()) {
        trace() << "Linked already";
        return;
    }

    redirectUri_ = QString("https://login.live.com/oauth20_desktop.srf");

    // Assemble intial authentication URL
    QList<QPair<QString, QString> > parameters;
    parameters.append(qMakePair(QString("response_type"), QString("token")));
    parameters.append(qMakePair(QString("client_id"), clientId_));
    parameters.append(qMakePair(QString("redirect_uri"), redirectUri_));
    parameters.append(qMakePair(QString("scope"), scope_));

    // Show authentication URL with a web browser
    QUrl url(requestUrl_);
    url.setQueryItems(parameters);
    emit openBrowser(url);
}

void O2Skydrive::redirected(const QUrl &url) {
    emit closeBrowser();
    QString token = "";
    int expiresIn = 0;

    QStringList parts = url.toString().split("#");
    if (parts.length() > 1) {
        foreach (QString item, parts[1].split("&")) {
            int index = item.indexOf("=");
            if (index == -1) {
                continue;
            }
            QString key = item.left(index);
            QString value = item.mid(index + 1);
            trace() << "" << key;
            if (key == "access_token") {
                token = value;
            } else if (key == "expires_in") {
                expiresIn = value.toInt();
            } else if (key == "authentication_token") {
                // FIXME: Is this the refresh token?
            }
        }
    }

    setToken(token);
    setExpires(QDateTime::currentMSecsSinceEpoch() / 1000 + expiresIn);
    if (token.isEmpty()) {
        emit linkingFailed();
    } else {
        emit linkedChanged();
        emit linkingSucceeded();
    }
}
