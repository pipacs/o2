#ifndef O2REPLYSERVER_H
#define O2REPLYSERVER_H

#include <QTcpServer>
#include <QMap>
#include <QByteArray>
#include <QString>

/// HTTP server to process authentication response.
class O2ReplyServer: public QTcpServer {
    Q_OBJECT

public:
    explicit O2ReplyServer(QObject *parent = 0);
    ~O2ReplyServer();

    /// Page content on local host after successful oauth - in case you do not want to close the browser, but display something
        Q_PROPERTY(QByteArray replyContent READ replyContent WRITE setReplyContent)
    QByteArray replyContent();
    void setReplyContent(const QByteArray &value);

signals:
    void verificationReceived(QMap<QString, QString>);

public slots:
    void onIncomingConnection();
    void onBytesReady();
    QMap<QString, QString> parseQueryParams(QByteArray *data);

protected:
    QByteArray replyContent_;
};

#endif // O2REPLYSERVER_H
