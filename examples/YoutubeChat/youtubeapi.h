#ifndef YOUTUBEAPI_H
#define YOUTUBEAPI_H

#include <QObject>
#include <QNetworkReply>
#include "livebroadcastmodel.h"

class QNetworkAccessManager;

namespace youtube
{

//ignore messages older than 10 minutes
const unsigned int IGNORE_MESSAGES_OLDER_THAN_SEC = 10*60;

class CommentModel;
class O2Youtube;


class YoutubeApi : public QObject
{
    Q_OBJECT
public:
    /// List of comments
    Q_PROPERTY(CommentModel *commentModel READ commentModel NOTIFY commentModelChanged)
    CommentModel *commentModel() ;
    Q_PROPERTY(LiveBroadcastModel *commentModel READ broadcastModel NOTIFY broadcastModelChanged)
    LiveBroadcastModel *broadcastModel() ;

    /// OAuth authenticator
    Q_PROPERTY(O2Youtube *authenticator READ authenticator WRITE setAuthenticator)
    O2Youtube *authenticator() const;
    void setAuthenticator(O2Youtube *v) ;

    explicit YoutubeApi(QObject *parent = 0);
    virtual ~YoutubeApi();

    //empty broadcast/comments models,
    //wipe page token.
    void cleanState();
    //request livebroadcast id that behold user's account
    void requestOwnLiveBroadcasts();
    void requestComments(QString liveCommentsId,QString lastPageToken="");
protected:

    O2Youtube *authenticator_;
    CommentModel *commentModel_;
    LiveBroadcastModel *broadcastModel_;

    QNetworkAccessManager *manager_;  
private:

    //token that keeps which comments parts we need
    QString lastPageToken_;
    QDateTime lastMessageTimestamp_;



signals:
    void commentModelChanged();
    void broadcastModelChanged();
   // void comments()


protected slots:
    //obsolete in O2Requestor?
    void requestFailed(QNetworkReply::NetworkError error,QByteArray data);
    Q_INVOKABLE virtual void broadcastRequestFinished(int id, QNetworkReply::NetworkError error, QByteArray data);
    Q_INVOKABLE virtual void commentRequestFinished(int id, QNetworkReply::NetworkError error, QByteArray data);

};
}
#endif // YOUTUBEAPI_H
