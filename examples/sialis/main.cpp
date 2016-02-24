#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include "o1.h"
#include "o1twitter.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<O1Twitter>("com.pipacs.o2", 1, 0, "O1Twitter");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
