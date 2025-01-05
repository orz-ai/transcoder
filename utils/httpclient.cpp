#include "HttpClient.h"
#include <QEventLoop>

HttpClient::HttpClient() {
    networkManager = new QNetworkAccessManager();
}

HttpClient::~HttpClient() {
    delete networkManager;
}

QString HttpClient::get(const QString &url, const QMap<QString, QString> &headers) {
    QNetworkRequest request((QUrl(url)));
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
    }
    QNetworkReply *reply = networkManager->get(request);
    return waitForReply(reply);
}

QString HttpClient::post(const QString &url, const QJsonObject &payload, const QMap<QString, QString> &headers) {
    QNetworkRequest request((QUrl(url)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
    }
    QByteArray data = QJsonDocument(payload).toJson();
    QNetworkReply *reply = networkManager->post(request, data);
    return waitForReply(reply);
}

QString HttpClient::postForm(const QString &url, const QMap<QString, QString> &formData, const QMap<QString, QString> &headers) {
    QNetworkRequest request((QUrl(url)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
    }

    QByteArray data;
    for (auto it = formData.begin(); it != formData.end(); ++it) {
        if (!data.isEmpty()) {
            data.append("&");
        }
        data.append(QUrl::toPercentEncoding(it.key()) + "=" + QUrl::toPercentEncoding(it.value()));
    }
    QNetworkReply *reply = networkManager->post(request, data);
    return waitForReply(reply);
}

void HttpClient::asyncGet(const QString &url, const QMap<QString, QString> &headers,
                          std::function<void(const QString &, const QString &)> callback) {
    QNetworkRequest request((QUrl(url)));
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
    }
    QNetworkReply *reply = networkManager->get(request);
    QAbstractSocket::connect(reply, &QNetworkReply::finished, [reply, callback]() {
        if (reply->error() == QNetworkReply::NoError) {
            callback(QString::fromUtf8(reply->readAll()), QString());
        } else {
            callback(QString(), reply->errorString());
        }
        reply->deleteLater();
    });
}

void HttpClient::asyncPost(const QString &url, const QJsonObject &payload, const QMap<QString, QString> &headers,
                           std::function<void(const QString &, const QString &)> callback) {
    QNetworkRequest request((QUrl(url)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
    }
    QByteArray data = QJsonDocument(payload).toJson();
    QNetworkReply *reply = networkManager->post(request, data);
    QAbstractSocket::connect(reply, &QNetworkReply::finished, [reply, callback]() {
        if (reply->error() == QNetworkReply::NoError) {
            callback(QString::fromUtf8(reply->readAll()), QString());
        } else {
            callback(QString(), reply->errorString());
        }
        reply->deleteLater();
    });
}

QString HttpClient::waitForReply(QNetworkReply *reply) {
    QEventLoop loop;
    QAbstractSocket::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    if (reply->error() == QNetworkReply::NoError) {
        QString response = QString::fromUtf8(reply->readAll());
        reply->deleteLater();
        return response;
    } else {
        QString error = reply->errorString();
        reply->deleteLater();
        return QString("Error: %1").arg(error);
    }
}
