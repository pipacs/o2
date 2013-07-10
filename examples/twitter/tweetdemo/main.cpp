#include <QApplication>
#include <QStringList>
#include <QDebug>

#include "tweeter.h"

const char OPT_MODE_GUI[] = "-g";
const char OPT_MODE_CLI[] = "-c";
const char OPT_OAUTH[] = "-l";
const char OPT_XAUTH[] = "-x";
const char OPT_USERNAME[] = "-u";
const char OPT_PASSWORD[] = "-p";
const char OPT_STATUS[] = "-m";

const char USAGE[] = "\n"
                     "Usage: tweetdemo [OPTION]...\n"
                     "Get OAuth access tokens from Twitter's OAuth service and "
                     "(optionally) post a status update on a user's timeline\n\n"
                     "  %1\t\tRun application in GUI mode (Qt Quick Application)\n"
                     "  %2\t\tRun application in CLI mode (command line interface)\n"
                     "\nCLI mode options:\n"
                     "  %3\t\tLink with Twitter OAuth service, i.e get access tokens\n"
                     "  %4\t\tLink with Twitter XAuth service, i.e get access tokens using the XAuth protocol\n"
                     "  %5 <username>\tTwitter username to be used when using XAuth (-x option)\n"
                     "  %6 <password>\tTwitter password to be used when using XAuth (-x option)\n"
                     "  %7\t\tStatus update message, enclosed in double quotes\n";


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QStringList argList = a.arguments();

    QByteArray help = QString(USAGE).arg(OPT_MODE_GUI,
                                         OPT_MODE_CLI,
                                         OPT_OAUTH,
                                         OPT_XAUTH,
                                         OPT_USERNAME,
                                         OPT_PASSWORD,
                                         OPT_STATUS).toLatin1();
    const char* helpText = help.constData();

    if (argList.size() == 1) {
        qDebug() << helpText;
        exit(1);
    }

    if (!(argList.contains(OPT_MODE_GUI) || argList.contains(OPT_MODE_CLI))) {
        qDebug() << helpText;
        exit(1);
    } else if (argList.contains(OPT_MODE_GUI)) {
        qDebug() << "\nGUI mode is currently not not supported."
                    " Please use CLI mode (-c option).\n";
        exit(1);
    }

    Tweeter tweeter;

    if (argList.contains(OPT_OAUTH)) {
        // Do OAuth
        tweeter.doOAuth();
    } else if (argList.contains(OPT_XAUTH)) {
        if(!(argList.contains(OPT_USERNAME) && argList.contains(OPT_PASSWORD))) {
            qDebug() << "\nError: Username or Password missing!";
            qDebug() << helpText;
            exit(1);
        }
        QString username = argList.at(argList.indexOf(OPT_USERNAME) + 1);
        QString password = argList.at(argList.indexOf(OPT_PASSWORD) + 1);
        tweeter.doXAuth(username, password);
    } else if (argList.contains(OPT_STATUS)) {
        QString statusMessage = argList.at(argList.indexOf(OPT_STATUS) + 1);
        tweeter.postStatusUpdate(statusMessage);
    } else {
        qDebug() << helpText;
        exit(1);
    }

    return a.exec();
}
