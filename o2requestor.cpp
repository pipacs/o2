#include <QDebug>
#include <QTimer>

#include "o2requestor.h"
#include "o2.h"

O2Requestor::O2Requestor(QNetworkAccessManager *manager, O2 *authenticator, QObject *parent): QObject(parent), reply_(NULL), status_(Idle) {
    manager_ = manager;
    authenticator_ = authenticator;
    qRegisterMetaType<QNetworkReply::NetworkError>("QNetworkReply::NetworkError");
    connect(authenticator, SIGNAL(refreshFinished(QNetworkReply::NetworkError)), this, SLOT(onRefreshFinished(QNetworkReply::NetworkError)), Qt::QueuedConnection);
}

O2Requestor::~O2Requestor() {
}

int O2Requestor::get(const QNetworkRequest &req) {
    if (-1 == setup(req, QNetworkAccessManager::GetOperation)) {
        return -1;
    }
    reply_ = manager_->get(request_);
    timedReplies_.add(reply_);
    connect(reply_, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRequestError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
    connect(reply_, SIGNAL(finished()), this, SLOT(onRequestFinished()), Qt::QueuedConnection);
    return id_;
}

int O2Requestor::post(const QNetworkRequest &req, const QByteArray &data) {
    if (-1 == setup(req, QNetworkAccessManager::PostOperation)) {
        return -1;
    }
    data_ = data;
    reply_ = manager_->post(request_, data_);
    timedReplies_.add(reply_);
    connect(reply_, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRequestError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
    connect(reply_, SIGNAL(finished()), this, SLOT(onRequestFinished()), Qt::QueuedConnection);
    connect(reply_, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(onUploadProgress(qint64,qint64)));
    return id_;
}

void O2Requestor::onRefreshFinished(QNetworkReply::NetworkError error) {
    qDebug() << "O2Requestor::onRefreshFinished";
    if (status_ != Requesting) {
        qWarning() << "O2Reqestor::onRefreshFinished: No pending request";
        return;
    }
    if (QNetworkReply::NoError == error) {
        QTimer::singleShot(100, this, SLOT(retry()));
    } else {
        QTimer::singleShot(10, this, SLOT(finish()));
    }
}

void O2Requestor::onRequestFinished() {
    QNetworkReply *senderReply = qobject_cast<QNetworkReply *>(sender());
    QNetworkReply::NetworkError error = senderReply->error();
    qDebug() << "O2Requestor::onRequestFinished: Error" << (int)error;
    if (status_ == Idle) {
        qDebug() << "O2Requestor::onRequestFinished: No pending request";
        return;
    }
    if (reply_ != senderReply) {
        qDebug() << "O2Requestor::onRequestFinished: Not a pending request";
        return;
    }
    if (error == QNetworkReply::NoError) {
        QTimer::singleShot(10, this, SLOT(finish()));
    }
}

void O2Requestor::onRequestError(QNetworkReply::NetworkError error) {
    qDebug() << "O2Requestor::onRequestError: Error" << (int)error;
    if (status_ == Idle) {
        qWarning() << "O2Requestor::onRequestError: No pending request";
        return;
    }
    if (reply_ != qobject_cast<QNetworkReply *>(sender())) {
        qDebug() << "O2Requestor::onRequestFinished: Not a pending request";
        return;
    }
    int httpStatus = reply_->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "O2Requestor::onRequestError: HTTP status" << httpStatus << reply_->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    if (status_ == Requesting) {
        if (httpStatus == 401) {
            // Call O2::refresh. Note the O2 instance might live in a different thread
            qDebug() << "O2Requestor::onRequestError: Refreshing token";
            if (QMetaObject::invokeMethod(authenticator_, "refresh")) {
                return;
            }
            qCritical() << "O2Requestor::onRequestError: Invoking remote refresh failed";
        }
    } else {
        qDebug() << "O2Requestor::onRequestError: Reply" << (unsigned)(void *)reply_ << ", status" << (int)status_;
    }
    QTimer::singleShot(10, this, SLOT(finish()));
}

void O2Requestor::onUploadProgress(qint64 uploaded, qint64 total) {
    if (status_ == Idle) {
        qWarning() << "O2Requestor::onUploadProgress: No pending request";
        return;
    }
    if (reply_ != qobject_cast<QNetworkReply *>(sender())) {
        qDebug() << "O2Requestor::onUploadProgress: Not a pending request";
        return;
    }
    emit uploadProgress(id_, uploaded, total);
}

int O2Requestor::setup(const QNetworkRequest &req, QNetworkAccessManager::Operation operation) {
    static int currentId;
    QUrl url;

    if (status_ != Idle) {
        qWarning() << "O2Requestor::setup: Another request pending";
        return -1;
    }

    request_ = req;
    operation_ = operation;
    id_ = currentId++;
    url_ = url = req.url();
    url.addQueryItem("access_token", authenticator_->token());
    request_.setUrl(url);
    status_ = Requesting;
    return id_;
}

void O2Requestor::finish() {
    QByteArray data;
    if (status_ == Idle) {
        qWarning() << "O2Requestor::finish: No pending request";
        return;
    }
    QNetworkReply::NetworkError error = reply_->error();
    qDebug() << "O2Requestor::finish: Error" << (int)error;
    if (QNetworkReply::NoError == error) {
        data = reply_->readAll();
    }
    status_ = Idle;
    timedReplies_.remove(reply_);
    reply_->disconnect(this);
    reply_->deleteLater();
    emit finished(id_, error, data);
}

void O2Requestor::retry() {
    qDebug() << "O2Requestor::retry: Old reply" << (unsigned)(void *)reply_;
    if (status_ != Requesting) {
        qWarning() << "O2Requestor::retry: No pending request";
        return;
    }
    timedReplies_.remove(reply_);
    reply_->disconnect(this);
    reply_->deleteLater();
    QUrl url = url_;
    url.addQueryItem("access_token", authenticator_->token());
    request_.setUrl(url);
    status_ = ReRequesting;
    if (operation_ == QNetworkAccessManager::GetOperation) {
        reply_ = manager_->get(request_);
    } else {
        reply_ = manager_->post(request_, data_);
    }
    qDebug() << "O2Requestor::retry: New reply" << (unsigned)(void *)reply_;
    timedReplies_.add(reply_);
    connect(reply_, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onRequestError(QNetworkReply::NetworkError)), Qt::QueuedConnection);
    connect(reply_, SIGNAL(finished()), this, SLOT(onRequestFinished()), Qt::QueuedConnection);
    connect(reply_, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(onUploadProgress(qint64,qint64)));
}
