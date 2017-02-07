#include <QApplication>
#include <QStringList>
#include <QTimer>
#include <QDebug>

#include "ytdemo.h"

const char OPT_OAUTH_CODE[] = "-o";

class Helper : public QObject {
    Q_OBJECT

public:
    Helper() : QObject(), ytdemo_(this), waitForMsg_(false), msg_(QString()) {}

public slots:
    void run() {
        connect(&ytdemo_, SIGNAL(linkingFailed()), this, SLOT(onLinkingFailed()));
        connect(&ytdemo_, SIGNAL(linkingSucceeded()), this, SLOT(onLinkingSucceeded()));
        connect(&ytdemo_, SIGNAL(channelInfoReceived()), this, SLOT(onChannelInfoReceived()));
        connect(&ytdemo_, SIGNAL(channelInfoFailed()), this, SLOT(onChannelInfoFailed()));

        ytdemo_.doOAuth(O2::GrantFlowAuthorizationCode);
    }

    void onLinkingFailed() {
        qDebug() << "Linking failed!";
        qApp->exit(1);
    }

    void onLinkingSucceeded() {
        qDebug() << "Linking succeeded!";
        ytdemo_.getUserChannelInfo();
    }

    void onChannelInfoFailed() {
        qDebug() << "Error getting channel info!";
        qApp->exit(1);
    }

    void onChannelInfoReceived() {
        qDebug() << "Channel info received!";
        qApp->quit();
    }

private:
    YTDemo ytdemo_;
    bool waitForMsg_;
    QString msg_;
};

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("O2");
    QCoreApplication::setApplicationName("YouTube Test");
    Helper helper;
    QTimer::singleShot(0, &helper, SLOT(run()));
    return a.exec();
}

#include "main.moc"
