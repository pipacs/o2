#include <QApplication>
#include <QStringList>
#include <QTimer>
#include <QDebug>

#include "msgraphdemo.h"

class Helper : public QObject {
    Q_OBJECT

public:
    Helper() : QObject(), demo_(this) {}

public slots:
    void run() {
        connect(&demo_, SIGNAL(linkingFailed()), this, SLOT(onLinkingFailed()));
        connect(&demo_, SIGNAL(linkingSucceeded()), this, SLOT(onLinkingSucceeded()));
        connect(&demo_, SIGNAL(userPrincipalNameReceived()), this, SLOT(onUserPrincipalNameReceived()));
        connect(&demo_, SIGNAL(userPrincipalNameFailed()), this, SLOT(onUserPrincipalNameFailed()));

        // Start OAuth
        demo_.doOAuth(O2::GrantFlowAuthorizationCode);
    }

    void onLinkingFailed() {
        qDebug() << "Linking failed!";
        qApp->exit(1);
    }

    void onLinkingSucceeded() {
        qDebug() << "Linking succeeded!";
        demo_.getUserPrincipalName();
    }

    void onUserPrincipalNameFailed() {
        qDebug() << "Error getting userPrincipalName!";
        qApp->exit(1);
    }

    void onUserPrincipalNameReceived() {
        qDebug() << "UserPrincipalName received!";
        qApp->quit();
    }

private:
    MsgraphDemo demo_;
};

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("O2");
    QCoreApplication::setApplicationName("Msgraph Example");
    Helper helper;
    QTimer::singleShot(0, &helper, SLOT(run()));
    return a.exec();
}

#include "main.moc"
