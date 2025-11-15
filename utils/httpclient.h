#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <functional>

/**
 * @brief The HttpClient class
 *
 *  Usage:
 *
 *  HttpClient client;
 *  QString response = client.get("https://orz.ai/dailynews/?platform=baidu");
 *  qDebug() << "Response:" << response;
 */
class HttpClient {
public:
    explicit HttpClient();
    ~HttpClient();

    // HTTP GET 请求
    QString get(const QString &url, const QMap<QString, QString> &headers = {});

    // HTTP POST 请求 (JSON)
    QString post(const QString &url, const QJsonObject &payload, const QMap<QString, QString> &headers = {});

    // HTTP POST 请求 (表单数据)
    QString postForm(const QString &url, const QMap<QString, QString> &formData, const QMap<QString, QString> &headers = {});

    // HTTP 请求（回调方式处理结果）
    void asyncGet(const QString &url, const QMap<QString, QString> &headers,
                  std::function<void(const QString &, const QString &)> callback);

    void asyncPost(const QString &url, const QJsonObject &payload, const QMap<QString, QString> &headers,
                   std::function<void(const QString &, const QString &)> callback);

private:
    QNetworkAccessManager *networkManager;

    QString waitForReply(QNetworkReply *reply); // 内部同步等待方法
};

#endif // HTTPCLIENT_H
