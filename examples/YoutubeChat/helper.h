#ifndef HELPER_H
#define HELPER_H

#include "youtubecontroller.h"
#include "messagereceiver.h"
class Helper:public QObject
{
    Q_OBJECT
public:
    Helper(youtube::YoutubeController& controller,MessageReceiver& receiver,QObject * parent );
private:
    youtube::YoutubeController& youtube_;
    MessageReceiver& receiver_;
};

#endif // HELPER_H
