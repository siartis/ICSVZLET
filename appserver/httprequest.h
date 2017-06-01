#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <QThread>
#include <QtNetwork>
#include <QSemaphore>
#include "cookie.h"

class HttpRequest : public QThread
{
    Q_OBJECT

    public:
        QSemaphore *P;
        QTcpSocket *client;
        int I;
        int code;
        QHash <QString,QString> env;
        QByteArray anser;
        QString type;
        QUrl url;
        COOKIE cookie;
        int count;

        explicit HttpRequest(QSemaphore *P,QObject *parent = 0);
        void start(const int I,
                   QTcpSocket *client, Priority priority=InheritPriority);

    protected:
        void run(void);
        int API(const COOKIE& cookie, const QString& path,
                const QString& param, const bool json);
        int HELP(const QString& path);
        int status(const QString &path, const COOKIE &cookie);
        int user(const QString &path, const COOKIE &cookie);
        int htdocs(const QUrl& url);
        int script(const QUrl& url, const QString& param,
                   const COOKIE& cookie, const bool DB);
        int Authorization(const QString &author, COOKIE *cookie);

    signals:

    public slots:
    
};

#endif // HTTPREQUEST_H
