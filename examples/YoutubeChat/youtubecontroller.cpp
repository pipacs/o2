#include "youtubecontroller.h"
#include "o2youtube.h"
#include <QDesktopServices>
#include "youtubeapi.h"
#include "o2youtube.h"
#include <QApplication>

namespace youtube
{
const int DEFAULT_YOUTUBE_UPDATE_MESSAGES_INTERVAL_MS = 3000;
YoutubeController::YoutubeController(QObject *parent): QObject(parent),
    commentsTimerId_(0),
    updateMessagesIntervalMs_(DEFAULT_YOUTUBE_UPDATE_MESSAGES_INTERVAL_MS),
    liveChatId_("")
{
    o2youtube_ = new O2Youtube(this);

    o2youtube_->setClientId("463300085238-11h8th4cbl9uoi4rd8pcl0e19sspn1s8.apps.googleusercontent.com");
    o2youtube_->setClientSecret("PZE0m3Ym4lZRKWDWk6KQ9c7T");

    
    connect(o2youtube_, SIGNAL(linkedChanged()), this, SLOT(onLinkedChanged()));
    connect(o2youtube_, SIGNAL(linkingFailed()), this, SLOT(onLinkingFailed()));
    connect(o2youtube_, SIGNAL(linkingSucceeded()), this, SLOT(onLinkingSucceeded()));
    connect(o2youtube_, SIGNAL(openBrowser(QUrl)), this, SLOT(onOpenBrowser(QUrl)));
    connect(o2youtube_, SIGNAL(closeBrowser()), this, SLOT(onCloseBrowser()));

    api_ = new YoutubeApi(this);
    connect(api_, SIGNAL(broadcastModelChanged()),this, SLOT(broadcastModelChanged()));
    connect(api_, SIGNAL(commentModelChanged()),this, SLOT(commentModelChanged()));
}

void YoutubeController::doAuth()
{
    //o2youtube_->unlink();
    o2youtube_->link();
}
void YoutubeController::delogin()
{
    //o2youtube_->unlink();
    stopPolling();
    o2youtube_->unlink();
}
void YoutubeController::onLinkedChanged()
{  
}

void YoutubeController::onLinkingFailed()
{
    emit error(tr("Cannot login to youtube services.")) ;
}


void YoutubeController::onLinkingSucceeded()
{    
   //bug?
   //o2 emit linkingSucceed in unlink() too
    if (!o2youtube_->linked())
        return;

    emit message(QApplication::tr("Connecting to youtube channel..."),"","");    
    api_->setAuthenticator(o2youtube_);
    api_->requestOwnLiveBroadcasts();
}

void YoutubeController::onOpenBrowser(const QUrl &url)
{
    QDesktopServices::openUrl(url);

}

void YoutubeController::onCloseBrowser()
{

}

void YoutubeController::onFinished()
{

}


//#Broadcast region
void YoutubeController::broadcastModelChanged()
{
    const auto& broadcastModel = api_->broadcastModel();
    if (broadcastModel->rowCount() ==0)
    {
        emit error(tr("No broadcasts found"));
        return;
    }

    QString liveChatId = broadcastModel->data(broadcastModel->index(0,0)).toString();

    if (liveChatId.size()==0)
    {
        emit error(tr("Broadcast has no live chats attached"));
        return;
    }

    emit message(QApplication::tr("Connected to youtube channel."),"","");
    liveChatId_ = liveChatId;
    startPolling();

}

void YoutubeController::timerEvent( QTimerEvent * event )
{
    (void)event;
    api_->requestComments(liveChatId_);
}

void YoutubeController::commentModelChanged()
{
    const auto& commentModel = api_->commentModel();

    for(int k = 0; k!=commentModel->rowCount();k++)
    {
        auto& rawMessage =  commentModel->data(commentModel->index(k,0),CommentModel::Roles::RoleCommentMessage).toMap();
        auto& rawAuthor = commentModel->data(commentModel->index(k,0),CommentModel::Roles::RoleCommentAuthor).toMap();
        CommentMessage msg = getCommentMessageFromRawData(rawMessage);
        CommentAuthor author = getCommentAuthorFromRawData(rawAuthor);
        switch (msg.messageType)
        {
        case CommentType::TextMessage:
        {
            if (!msg.canBeDisplayed)
                return;

            QString messageText = rawMessage["textMessageDetails"].toMap()["messageText"].toString();
            QString authorName = author.displayName;
            emit message(messageText,authorName,author.profileImageUrl,
                         author.isChatModerator, author.isChatOwner,author.isChatSponsor);
        }
            break;
        case CommentType::FanFunding:
        {
            auto fundsDetails = getCommentFundraisingDetailsFromRawData(rawMessage);
            emit fundraising(fundsDetails.userComment,author.displayName,author.profileImageUrl
                             ,fundsDetails.amountMicros,fundsDetails.currency,fundsDetails.amountDisplayString);
        }
            break;
        case CommentType::NewSponsor:
        {
            emit newSponsor(author.displayName);
        }
            break;
        // ignore rest of message types.
        default: break;
        }
    }
}

//filling only few CommentMessage fields
CommentMessage YoutubeController::getCommentMessageFromRawData(const QVariantMap& data)
{
    CommentMessage msg;
    QString messageType = data["type"].toString();

    if (messageType.startsWith("textMessageEvent"))
        msg.messageType = CommentType::TextMessage;
    else
    if (messageType.startsWith("fanFundingEvent"))
        msg.messageType = CommentType::FanFunding;
    else
    if (messageType.startsWith("newSponsorEvent"))
        msg.messageType = CommentType::NewSponsor;
    else
    if (messageType.startsWith("messageDeletedEvent"))
        msg.messageType = CommentType::MessageDeleted;
    else
    if (messageType.startsWith("userBannedEvent"))
        msg.messageType = CommentType::UserBanned;
    else
        msg.messageType = CommentType::NotImplemented;

    msg.canBeDisplayed = data["hasDisplayContent"].toBool();


    return msg;
}
CommentAuthor YoutubeController::getCommentAuthorFromRawData(const QVariantMap& data)
{
    CommentAuthor msg;
    msg.displayName = data["displayName"].toString();
    msg.isChatModerator = data["isChatModerator"].toBool();
    msg.isChatOwner = data["isChatOwner"].toBool();
    msg.isChatSponsor = data["isChatSponsor"].toBool();
    msg.profileImageUrl = data["profileImageUrl"].toString();
    return msg;
}

CommentFundraisingDetails YoutubeController::getCommentFundraisingDetailsFromRawData(const QVariantMap& data)
{
    CommentFundraisingDetails fundraisingDetails;
    const auto rawDetails =  data["fanFundingEventDetails"].toMap();

    fundraisingDetails.amountMicros = rawDetails["amountMicros"].toUInt();
    fundraisingDetails.amountDisplayString = rawDetails["amountDisplayString"].toString();
    fundraisingDetails.currency = rawDetails["currency"].toString();
    fundraisingDetails.userComment = rawDetails["userComment"].toString();

    return fundraisingDetails;
}

void YoutubeController::startPolling()
{
    commentsTimerId_ = startTimer(updateMessagesIntervalMs_);
}

void YoutubeController::stopPolling()
{
    if (commentsTimerId_==0)
        return;

    killTimer(commentsTimerId_);
    commentsTimerId_ = 0;
}

}
