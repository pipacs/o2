#ifndef TWITTERAPI_H
#define TWITTERAPI_H

#include <QAbstractListModel>
#include <QObject>
#include <QNetworkAccessManager>

#include "o1twitter.h"

class TwitterApi : public QObject
{
    Q_OBJECT

public:
    explicit TwitterApi(QObject *parent = 0);

    Q_PROPERTY(QAbstractListModel tweets READ tweets WRITE setTweets NOTIFY tweetsChanged)
    QAbstractListModel tweets();

signals:
    void tweetsChanged();

public slots:
};

#endif // TWITTERAPI_H
