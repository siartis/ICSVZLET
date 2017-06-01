#include "httpserver.h"
#include "httprequest.h"
#include <time.h>
#include "cookie.h"
#include "option.h"
#include "httpthread.h"
#include "httpsession.h"
#include <QMutex>

namespace HTTPSESSION {
    extern QMutex cookieMutex;
}

HttpServer::HttpServer(QObject *parent) : QTcpServer(parent) {
    qsrand(time(NULL));
    setMaxPendingConnections(OPTION::maxThread*3);
    P=new QSemaphore(OPTION::maxThread-1);
    for(int i=0;i<OPTION::maxThread;i++)
        HTTPTHREAD::ThreadPtr[i]=new HttpRequest(P);
    connect(this,SIGNAL(newConnection()),this,SLOT(Incomming()));
    timer=this->startTimer(OPTION::cookieLive);
    timeout=0;
}

void HttpServer::timerEvent(QTimerEvent *event){
    //return;
    this->killTimer(timer);
    HTTPSESSION::cookieMutex.lock();
    for(int i=0;i<OPTION::maxThread;i++)
        if(HTTPTHREAD::ThreadPtr[i]->isRunning())HTTPTHREAD::ThreadPtr[i]->wait(100);
    time_t ti=time(NULL);
    QStringList list=HTTPSESSION::cookie.keys();
    foreach(QString s,list)
        if(ti-HTTPSESSION::cookie.value(s).LastTime>OPTION::cookieLive) {
            HTTPSESSION::cookie.remove(s);
            HTTPSESSION::lastCode.remove(s);
            HTTPSESSION::envSession.remove(s);
            HTTPSESSION::envAll.remove(s);
            foreach(QString x,HTTPSESSION::envParent.value(s))
                HTTPSESSION::envAll.remove(x);
            HTTPSESSION::envParent.remove(s);
        }
    HTTPSESSION::cookieMutex.unlock();
    timer=this->startTimer(OPTION::cookieLive);
    return;
}

int HttpServer::getThread() {
    for(int i=0;i<OPTION::maxThread;i++)
        if(!HTTPTHREAD::ThreadPtr[i]->isRunning())return i;
    if(OPTION::pauseCOOKIE==0)return -1;
    QTimer t;
    QEventLoop q;
    connect(&t,SIGNAL(timeout()),&q,SLOT(quit()));
    t.start(OPTION::pauseCOOKIE);
    q.exec();
    for(int i=0;i<OPTION::maxThread;i++)
        if(!HTTPTHREAD::ThreadPtr[i]->isRunning())return i;
    return -1;
}

void HttpServer::Incomming(void) {
    P->acquire();
    if(!hasPendingConnections()) {
        P->release();
        return;
    }
    QTcpSocket *client=nextPendingConnection();
    if(client==NULL) {
        P->release();
        return;
    }
    client->setSocketOption(QAbstractSocket::KeepAliveOption,1);
    if(OPTION::maxCOOKIE>0)
        if(HTTPSESSION::cookie.size()>OPTION::maxCOOKIE) {
            client->write("HTTP/1.1 404\r\n"
                          "Content-Type: text\r\n\r\n"
                          "big busy");
            client->waitForBytesWritten();
            client->close();
            client->deleteLater();
            P->release();
            return;
        }
    int i=getThread();
    if(i==-1) {
        puts("no treads");
        client->write("HTTP/1.1 201 OK\r\n"
                      "Content-Type: text/html\r\n"
                      "charset=windows-1251\r\n"
                      "\r\n"
                      "no thread");
        client->waitForBytesWritten();
        client->close();
        client->deleteLater();
        P->release();
        return;
    }
    HTTPTHREAD::ThreadCode[i]=qrand();
    client->setParent(NULL);
    client->moveToThread(HTTPTHREAD::ThreadPtr[i]);
    HTTPTHREAD::ThreadPtr[i]->start(i,client);
    return;
}
