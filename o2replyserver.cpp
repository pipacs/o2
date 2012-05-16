#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QString>
#include <QMap>
#include <QPair>
#include <QStringList>
#include <QUrl>

#include "o2replyserver.h"

O2ReplyServer::O2ReplyServer(QObject *parent): QTcpServer(parent) {
    connect(this, SIGNAL(newConnection()), this, SLOT(onIncomingConnection()));
}

O2ReplyServer::~O2ReplyServer() {
}

void O2ReplyServer::onIncomingConnection() {
    socket = nextPendingConnection();
    connect(socket, SIGNAL(readyRead()), this, SLOT(onBytesReady()), Qt::UniqueConnection);
}

void O2ReplyServer::onBytesReady() {
    QByteArray reply;
    QByteArray content;
    content.append("<HTML></HTML>");
    reply.append("HTTP/1.0 200 OK \r\n");
    reply.append("Content-Type: text/html; charset=\"utf-8\"\r\n");
    reply.append(QString("Content-Length: %1\r\n\r\n").arg(content.size()));
    reply.append(content);
    socket->write(reply);

    QByteArray data = socket->readAll();
    QMap<QString, QString> queryParams = parseQueryParams(&data);
    socket->disconnectFromHost();
    close();
    emit verificationReceived(queryParams);
}

QMap<QString, QString> O2ReplyServer::parseQueryParams(QByteArray *data) {
    QString splitGetLine = QString(*data).split("\r\n").first();   // Retrieve the first line with query params.
    splitGetLine.remove("GET ");                                   // Clean the line from GET
    splitGetLine.remove("HTTP/1.1");                               // From HTTP
    splitGetLine.remove("\r\n");                                   // And from rest.
    splitGetLine.prepend("http://localhost");                      // Now, make it a URL
    QUrl getTokenUrl(splitGetLine);
    QList< QPair<QString, QString> > tokens = getTokenUrl.queryItems();
    QMultiMap<QString, QString> queryParams;
    QPair<QString, QString> tokenPair;
    foreach (tokenPair, tokens) {
        queryParams.insert(tokenPair.first.trimmed(), tokenPair.second.trimmed());
    }
    return queryParams;
}
