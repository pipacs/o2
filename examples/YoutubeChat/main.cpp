#include <QApplication>
#include <QTimer>
#include <QDebug>
#include "youtubecontroller.h"
#include "helper.h"

using namespace youtube;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    YoutubeController youtube;
    MessageReceiver receiver;
    Helper helper(youtube,receiver,nullptr);

    QTimer::singleShot(0, &youtube, SLOT(doAuth()));

    return a.exec();
}
