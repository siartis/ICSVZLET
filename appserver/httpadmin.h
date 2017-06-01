#ifndef HTTPADMIN_H
#define HTTPADMIN_H

#include <QTcpServer>
#include <QtNetwork>
#include "../INCLUDE/table.h"

class HttpServer;

class HttpAdmin : public QTcpServer
{
    Q_OBJECT

    protected:
        QByteArray anser;
        int option(void);
        int session(void);
        int thread(void);
        int put(const QString& path, const QByteArray& param);
        int _put(const QString& path, const QByteArray& param);
        int get(const QString& path, const QByteArray& param);
        int _get(const QString& path, const QByteArray& param,TABLE *table);
        int users(const QString& path, const QByteArray& param);
        int _users(TABLE *table);
        HttpServer *httpserver;

    public:
        explicit HttpAdmin(HttpServer *httpserver,QObject *parent = 0);

    signals:

    public slots:
        void Incomming(void);

};

#endif // HTTPADMIN_H
