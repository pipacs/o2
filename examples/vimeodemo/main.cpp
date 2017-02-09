#include <QApplication>
#include <QStringList>
#include <QTimer>
#include <QDebug>

#include <QUrl>
#include <QDesktopServices>

#include "vimeodemo.h"

const char OPT_OAUTH_CODE[] = "-o";

class Helper : public QObject {
    Q_OBJECT

public:
    Helper() : QObject(), demo_(this), waitForMsg_(false), msg_(QString()) {}

public slots:
    void run() {
        connect(&demo_, SIGNAL(linkingFailed()), this, SLOT(onLinkingFailed()));
        connect(&demo_, SIGNAL(linkingSucceeded()), this, SLOT(onLinkingSucceeded()));
        connect(&demo_, SIGNAL(userNameReceived()), this, SLOT(onUserNameReceived()));
        connect(&demo_, SIGNAL(userNameFailed()), this, SLOT(onUserNameFailed()));

        demo_.doOAuth(O2::GrantFlowAuthorizationCode);
    }

    void onLinkingFailed() {
        qDebug() << "Linking failed!";
        qApp->exit(1);
    }

    void onLinkingSucceeded() {
        qDebug() << "Linking succeeded!";
        demo_.getUserName();
    }

    void onUserNameFailed() {
        qDebug() << "Error getting user name!";
        qApp->exit(1);
    }

    void onUserNameReceived() {
        qDebug() << "User namereceived!";
        qApp->quit();
    }

private:
    VimeoDemo demo_;
    bool waitForMsg_;
    QString msg_;
};

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("O2");
    QCoreApplication::setApplicationName("Vimeo Test");
    Helper helper;
    QTimer::singleShot(0, &helper, SLOT(run()));
    return a.exec();
}

#include "main.moc"
