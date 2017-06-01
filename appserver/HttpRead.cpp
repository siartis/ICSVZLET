#include <QtCore>
#include <QTcpSocket>

int PageDecode(QByteArray *source, const int page);
void UrlDecode(QByteArray *source);
bool HttpSplint(QUrl *url, QHash <QString,QString> *env,
                QByteArray *param, const QByteArray& in);


bool HTTPRead(QTcpSocket *client,
              QUrl *url, QHash <QString,QString> *env, QByteArray *param) {
    url->clear();
    env->clear();
    param->clear();
    if(!client->isValid())return false;
    if(client->state()!=QAbstractSocket::ConnectedState)return false;
    if(!client->waitForReadyRead())return false;
    if(client->bytesAvailable()>5000000L)return false;
    char h[32];
    client->peek(h,31);
    if(strncmp(h,"GET /favicon.ico",11)==0)return false;
    //if(strncmp(h,"GET /",5)!=0)return false;
    QByteArray in;
    do {
        if(in.size()+client->bytesAvailable()>5000000L)return false;
        in.append(client->readAll());
        if(!in.contains("\r\n\r\n"))
            if(!client->waitForReadyRead())return false;
    } while(!in.contains("\r\n\r\n"));
    int i=in.indexOf("\r\n\r\n");
    param->append(in.data()+i+4);
    in.truncate(i+4);
    UrlDecode(&in);
    int page=8;
    if(in.contains("charset=UTF-8"))page=8;
    PageDecode(&in,page);
    bool f=HttpSplint(url,env,(in.startsWith("GET /"))?param:NULL,in);
    if(in.startsWith("GET /"))return f;
    if(!in.startsWith("POST"))return false;
    int k=env->value("Content-Length").toLong();
    if(k!=param->length()) {
        client->write("HTTP/1.1 100 Continue\r\n\r\n");
        client->waitForBytesWritten();
        while(k>param->length()) {
            if(param->length()+client->bytesAvailable()>5000000L)return false;
            param->append(client->readAll());
            if(k>param->length())if(!client->waitForReadyRead())return false;
        }
    }
    PageDecode(&in,page);
    return true;
}
/*
bool HTTPRead(QTcpSocket *client,
              QUrl *url, QHash <QString,QString> *env, QByteArray *param) {
    if(!_HTTPRead(client,url, QHash <QString,QString> *env, QByteArray *param) {
    return true;
}
*/

