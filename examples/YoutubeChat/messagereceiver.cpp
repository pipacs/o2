#include <QDebug>
#include "messagereceiver.h"

MessageReceiver::MessageReceiver(QObject * parent):QObject(parent)
{

}

void MessageReceiver::onError(const QString error)
{
    qDebug() << "Error: " << error ;
}

void MessageReceiver::onNewMessage(const QString message,const QString authorName,const QString authorImageLogo,
             bool isChatModerator, bool isChatOwner,bool isChatSponsor)
{
    qDebug() << authorName << ": " <<message;
}
void MessageReceiver::onFundraising(const QString userComment,const QString authorName,const QString authorImageLogo
                 ,ulong amountMicros,QString currency,QString amountDisplayString)
{
    qDebug() << "a gift incoming: "<< authorName << " sent you " << amountMicros << currency << "and said \"" <<userComment <<"\"";
}

void MessageReceiver::onNewSponsor(const QString authorName)
{
    qDebug() << "New sponsor: "<< authorName ;
}
