#include <QApplication>
#include <QStringList>
#include <QTimer>
#include <QDebug>

#include "fbdemo.h"

const char OPT_OAUTH_CODE[] = "-o";
const char OPT_IMPLICIT_GRANT[] = "-i";
const char OPT_USERNAME[] = "-u";
const char OPT_PASSWORD[] = "-p";

const char USAGE[] = "\n"
                     "Usage: facebookdemo [OPTION]...\n"
                     "Get OAuth2 access tokens from Facebook's OAuth service\n"
                     "\nOptions:\n"
                     "  %1\t\tLink with Facebook OAuth2 service using Authorization Code\n"
                     "  %2\t\tLink with Facebook OAuth2 service using Implicit Grant\n";


class Helper : public QObject
{
    Q_OBJECT

public:
    Helper() : QObject(), fbdemo_(this), waitForMsg_(false), msg_(QString()) {}

public slots:

    void processArgs() {

        QStringList argList = qApp->arguments();

        QByteArray help = QString(USAGE).arg(OPT_OAUTH_CODE,
                                             OPT_IMPLICIT_GRANT).toLatin1();

        const char* helpText = help.constData();

        connect(&fbdemo_, SIGNAL(linkingFailed()),
                this, SLOT(onLinkingFailed()));
        connect(&fbdemo_, SIGNAL(linkingSucceeded()),
                this, SLOT(onLinkingSucceeded()));

        if (argList.contains(OPT_OAUTH_CODE)) {
            // Start OAuth
            fbdemo_.doOAuth(O2::GrantFlowAuthorizationCode);
        } else if (argList.contains(OPT_IMPLICIT_GRANT)) {
            fbdemo_.doOAuth(O2::GrantFlowImplicit);
        } else {
            qDebug() << helpText;
            qApp->exit(1);
        }
    }

    void onLinkingFailed() {
        qDebug() << "Linking failed!";
        qApp->exit(1);
    }

    void onLinkingSucceeded() {
        qDebug() << "Linking succeeded!";
        if (waitForMsg_) {
            //postStatusUpdate(msg_);
        } else {
            qApp->quit();
        }
    }

private:
    FBDemo fbdemo_;
    bool waitForMsg_;
    QString msg_;
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Helper helper;
    QTimer::singleShot(0, &helper, SLOT(processArgs()));

    return a.exec();
}

#include "main.moc"
