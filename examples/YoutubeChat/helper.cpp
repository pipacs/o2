#include "helper.h"

using namespace youtube;
Helper::Helper(YoutubeController& controller,MessageReceiver& receiver,QObject * parent ):
    youtube_(controller),
    receiver_(receiver),
    QObject(parent)
{
       connect(&youtube_,SIGNAL(error(const QString )),&receiver_,SLOT(onError(const QString)));

       connect(&youtube_,SIGNAL(message(const QString ,const QString ,const QString,bool , bool ,bool)),
               &receiver_,SLOT(onNewMessage(const QString ,const QString ,const QString,bool , bool ,bool)));



       connect(&youtube_,SIGNAL(fundraising(const QString ,const QString,const QString,ulong ,QString ,QString )),
               &receiver_,SLOT(onFundraising(const QString ,const QString,const QString,ulong ,QString ,QString )));

       connect(&youtube_,SIGNAL(newSponsor(const QString )),&receiver_,SLOT(onNewSponsor(const QString)));
}
