#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

#include "o2requestor.h"
#include "o2youtube.h"
#include "youtubeapi.h"
#include "commentmodel.h"
#include "LiveBroadcastModel.h"

namespace youtube
{
static const char * YOUTUBE_LIVE_PART = "part";
static const char * YOUTUBE_LIVE_FILTER_BROADCAST_STATUS = "broadcastStatus";
static const char * YOUTUBE_LIVE_BROADCAST_TYPE = "broadcastType";
static const char * YOUTUBE_LIVE_FILTER_MINE = "mine";
static const char * YOUTUBE_LIVE_FILTER_ID = "id";
static const char * YOUTUBE_LIVE_OPTIONAL_MAXRESULTS = "maxResults";

static const char * YOUTUBE_LIVE_COMMENTS_PAGETOKEN = "pageToken";
static const char * YOUTUBE_LIVE_COMMENTS_LIVECHATID = "liveChatId";


YoutubeApi::YoutubeApi(QObject *parent): QObject(parent),
    authenticator_(0),
    lastPageToken_("")
{
    manager_ = new QNetworkAccessManager(this);
    commentModel_ = new CommentModel(this);
    broadcastModel_ = new LiveBroadcastModel(this);
    lastMessageTimestamp_ = QDateTime::currentDateTimeUtc().addDays(-2);
}

YoutubeApi::~YoutubeApi() {
}

void YoutubeApi::cleanState()
{
    broadcastModel_->clearBroadcasts();
    commentModel_->clearComments();

    //disable for debug purposes
    //lastPageToken_ = "";
}

CommentModel *YoutubeApi::commentModel() {
    return commentModel_;
}

LiveBroadcastModel * YoutubeApi::broadcastModel()
{
    return broadcastModel_;
}

O2Youtube *YoutubeApi::authenticator() const {
    return authenticator_;
}
void YoutubeApi::setAuthenticator(O2Youtube *v) {
    authenticator_ = v;
}


//request only our live broadcasts, single item
void YoutubeApi::requestOwnLiveBroadcasts()
{
    broadcastModel_->clearBroadcasts();

    if (!authenticator_ || !authenticator_->linked()) {
        emit broadcastModelChanged();
        return;
    }

     O2Requestor *requestor = new O2Requestor(manager_, authenticator_, this);
     QUrl url = QUrl("https://www.googleapis.com/youtube/v3/liveBroadcasts");

     QUrlQuery query(url);
     query.addQueryItem(YOUTUBE_LIVE_PART,"snippet");
     query.addQueryItem(YOUTUBE_LIVE_FILTER_MINE,"true");
     query.addQueryItem(YOUTUBE_LIVE_BROADCAST_TYPE,"persistent");
     query.addQueryItem(YOUTUBE_LIVE_OPTIONAL_MAXRESULTS,"1");

     url.setQuery(query);

     QNetworkRequest request(url);
     requestor->get(request);

     //connect(requestor, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(requestFailed(QNetworkReply::NetworkError)));//commentsReceived()));
     connect(requestor, SIGNAL(finished(int, QNetworkReply::NetworkError, QByteArray)),
             this, SLOT(broadcastRequestFinished(int, QNetworkReply::NetworkError, QByteArray)));//commentsReceived()));


}

void YoutubeApi::broadcastRequestFinished(int id, QNetworkReply::NetworkError error, QByteArray data)
{
    (void)(id);
    if (error != QNetworkReply::NetworkError::NoError)
    {
        emit requestFailed(error,data);
        broadcastModel_->clearBroadcasts();
        emit broadcastModelChanged();
        return;
    }
    qWarning() << "data " << data;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data);
    QVariantMap variantData = jsonResponse.toVariant().toMap();

    /*foreach (const QVariant &v, variantData.keys()) {
        qWarning() << v.toString() << " : " << variantData[v.toString()];
    }*/

    auto broadcastsData = variantData["items"].toList();

    foreach (const QVariant &v, broadcastsData) {
        auto broadcastData = v.toMap();
        broadcastModel_->addBroadcast(broadcastData);
    }

    emit broadcastModelChanged();

}



void YoutubeApi::requestComments(QString liveCommentsId,QString lastPageToken) {
    commentModel_->clearComments();

    if (!authenticator_ || !authenticator_->linked()) {
        emit commentModelChanged();
        return;
    }

    O2Requestor *requestor = new O2Requestor(manager_, authenticator_, this);
    QUrl url = QUrl("https://www.googleapis.com/youtube/v3/liveChat/messages");

    QUrlQuery query(url);
    query.addQueryItem(YOUTUBE_LIVE_PART,"snippet,authorDetails");

    //set page size to minimum of 200 as we will take one page each time we getting comments
    query.addQueryItem(YOUTUBE_LIVE_OPTIONAL_MAXRESULTS,"200");

    //retrieve *last* message page
    if (lastPageToken.size()>0)
        query.addQueryItem(YOUTUBE_LIVE_COMMENTS_PAGETOKEN,"lastPageToken");

    query.addQueryItem(YOUTUBE_LIVE_COMMENTS_LIVECHATID,liveCommentsId);

    url.setQuery(query);

    QNetworkRequest request(url);
    requestor->get(request);

    connect(requestor, SIGNAL(finished(int, QNetworkReply::NetworkError, QByteArray)), this, SLOT(commentRequestFinished(int, QNetworkReply::NetworkError, QByteArray)));//commentsReceived()));
    //connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(requestFailed(QNetworkReply::NetworkError)));
}



void YoutubeApi::commentRequestFinished(int id, QNetworkReply::NetworkError error, QByteArray data)
{
    (void)(id);
    if (error != QNetworkReply::NetworkError::NoError)
    {
        emit requestFailed(error,data);
        commentModel_->clearComments();
        emit commentModelChanged();
        return;
    }

    qWarning() << "data " << data;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data);
    QVariantMap variantData = jsonResponse.toVariant().toMap();

    /*foreach (const QVariant &v, variantData.keys()) {
        qWarning() << v.toString() << " : " << variantData[v.toString()];
    }*/

    //getting lastPageToken for future comment requests
    lastPageToken_ = variantData["nextPageToken"].toString();


    auto commentsData = variantData["items"].toList();

    foreach (const QVariant &v, commentsData) {                
        auto commentData = v.toMap();

        //skip every comment older than (lastMessageTimestamp_) or IGNORE_MESSAGES_OLDER_THAN_SEC

        //todo: should be optimized as we make commentData["snippet"].toMap in commentModel too
        QDateTime commentTimestamp = QDateTime::fromString(commentData["snippet"].toMap()["publishedAt"].toString(),Qt::ISODate);
        if (commentTimestamp.toUTC()<=lastMessageTimestamp_.toUTC())
            continue;

        if (commentTimestamp.toUTC().addSecs(IGNORE_MESSAGES_OLDER_THAN_SEC)<QDateTime::currentDateTimeUtc())
            continue;

        lastMessageTimestamp_ = commentTimestamp;

        commentModel_->addComment(commentData);
    }

    emit commentModelChanged();

}

//obsolete in O2Requestor?
void YoutubeApi::requestFailed(QNetworkReply::NetworkError error,QByteArray data) {

    qWarning() << "YoutubeApi::requestFailed:" << (int)error << error <<data;
}
}
