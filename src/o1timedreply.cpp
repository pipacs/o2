#include <QTimer>
#include <QNetworkReply>

#include "o1timedreply.h"

O1TimedReply::O1TimedReply(QNetworkReply *parent, int pTimeout): QTimer(parent) {
    setSingleShot(true);
    setInterval(pTimeout);
    connect(this, SIGNAL(timeout()), this, SLOT(onTimeout()));
    connect(parent, SIGNAL(finished()), this, SLOT(onFinished()));
}

void O1TimedReply::onFinished() {
    stop();
    emit finished();
}

void O1TimedReply::onTimeout() {
    emit error(QNetworkReply::TimeoutError);
}
