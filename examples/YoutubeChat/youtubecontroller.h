#ifndef YOTUBEAUTHENTIFICATOR_H
#define YOTUBEAUTHENTIFICATOR_H
#include <QObject>

#include "commentmodel.h"
namespace youtube
{
class O2Youtube;
class YoutubeApi;
class YoutubeController:public QObject
{
Q_OBJECT
public:
    YoutubeController(QObject *parent = nullptr);   
    void delogin();

signals:
    void error(const QString errorMessage);
    void message(const QString message,const QString authorName,const QString authorImageLogo,
                 bool isChatModerator = false, bool isChatOwner = false,bool isChatSponsor = false);
    void fundraising(const QString userComment,const QString authorName,const QString authorImageLogo
                     ,ulong amountMicros,QString currency,QString amountDisplayString);
    void newSponsor(const QString authorName);

public slots:
     void doAuth();
protected:
    virtual void timerEvent( QTimerEvent * event );
private slots:
    void onLinkedChanged();
    void onLinkingSucceeded();
    void onLinkingFailed();
    void onOpenBrowser(const QUrl &url);
    void onCloseBrowser();
    void onFinished();
    void broadcastModelChanged();
    void commentModelChanged();

private:
    static CommentMessage getCommentMessageFromRawData(const QVariantMap& data);
    static CommentAuthor getCommentAuthorFromRawData(const QVariantMap& data);
    static CommentFundraisingDetails getCommentFundraisingDetailsFromRawData(const QVariantMap& data);

    void startPolling();
    void stopPolling();

    O2Youtube* o2youtube_;
    YoutubeApi * api_;

    QString liveChatId_;

    //timer id for comments polling loop
    int commentsTimerId_;

    int updateMessagesIntervalMs_;


};
}
#endif // YOTUBEAUTHENTIFICATOR_H
