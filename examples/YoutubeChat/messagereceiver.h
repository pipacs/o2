#ifndef MESSAGERECEIVER_H
#define MESSAGERECEIVER_H

#include "youtubecontroller.h"
class MessageReceiver:public QObject
{
    Q_OBJECT
public:
    MessageReceiver(QObject * parent = nullptr);
public slots:


    void onError(const QString error);

    void onNewMessage(const QString message,const QString authorName,const QString authorImageLogo,
                 bool isChatModerator, bool isChatOwner,bool isChatSponsor);

    void onFundraising(const QString userComment,const QString authorName,const QString authorImageLogo
                     ,ulong amountMicros,QString currency,QString amountDisplayString);

    void onNewSponsor(const QString authorName);
};

#endif // MESSAGERECEIVER_H
