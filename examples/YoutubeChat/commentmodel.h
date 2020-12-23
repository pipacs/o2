#ifndef COMMENTMODEL_H
#define COMMENTMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QVariantMap>

#include <QObject>

namespace youtube
{
//docs can be found here: https://developers.google.com/youtube/v3/live/docs/liveChatMessages
enum class CommentType
{
    TextMessage,
    FanFunding,
    MessageDeleted,
    NewSponsor,
    UserBanned,
    //you can extend this class and define more types
    //as youtube defines much more event types.
    NotImplemented
};

//you can rewrite this class as it contain only fields author need.
//data example
/*
 * "authorDetails": {
    "channelId": "UCgCuI4JV0Imukb3ZcmC24kA",
    "channelUrl": "http://www.youtube.com/channel/UCgCuI4JV0Imukb3ZcmC24kA",
    "displayName": "Stas Artemjev",
    "profileImageUrl": "https://yt3.ggpht.com/--esiXXT-93U/AAAAAAAAAAI/AAAAAAAAAAA/MReYWijclIg/s88-c-k-no-mo-rj-c0xffffff/photo.jpg",
    "isVerified": false,
    "isChatOwner": true,
    "isChatSponsor": false,
    "isChatModerator": false
   }
 * */
struct CommentAuthor
{
    QString displayName;
    QString profileImageUrl;
    bool isChatOwner;
    bool isChatSponsor;
    bool isChatModerator;
};

//data example
/*
 * "snippet": {
    "type": "textMessageEvent",
    "liveChatId": "EiEKGFVDZ0N1STRKVjBJbXVrYjNaY21DMjRrQRIFL2xpdmU",
    "authorChannelId": "UCgCuI4JV0Imukb3ZcmC24kA",
    "publishedAt": "2016-11-16T20:50:29.047Z",
    "hasDisplayContent": true,
    "displayMessage": "тест 3",
    "textMessageDetails": {
     "messageText": "тест 3"
    }
 * */
struct CommentMessage
{
    QString text;
    CommentType messageType;
    bool canBeDisplayed;
};

struct CommentFundraisingDetails
{
    unsigned long amountMicros;
    QString currency;
    QString amountDisplayString;
    QString userComment;
};


class CommentModel:public QAbstractListModel
{
public:
    enum Roles {
        //message text
        RoleCommentMessage = Qt::UserRole + 1,
        //Type of message: standart message, fanfunding event, new sponsor event etc
        RoleCommentAuthor = Qt::UserRole +2,
    };

    CommentModel(QObject *parent = 0);

    /// Clear all comments
    void clearComments();

    /// Add a comment
    void addComment(QVariantMap comment);

    /// Get number of comments
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    /// Access a comment
    QVariant data(const QModelIndex & index, int role) const;

    /// Get role names
    QHash<int, QByteArray>roleNames() const;


protected:
    QList<QVariantMap> comments;


};
}
#endif // COMMENTMODEL_H
