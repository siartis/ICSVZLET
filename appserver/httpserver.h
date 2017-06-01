#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QtNetwork>
#include <QSemaphore>
//#include <QMutex>

class HttpServer : public QTcpServer
{
    Q_OBJECT

    public:
        int timeout;
        explicit HttpServer(QObject *parent = 0);

    protected:
        QSemaphore *P;
        int timer;

        void timerEvent(QTimerEvent *event);
        int getThread(void);

    signals:
    
    public slots:
        void Incomming(void);

};

#endif // HTTPSERVER_H
