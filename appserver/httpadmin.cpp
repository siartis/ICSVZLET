#include "httpadmin.h"
#include "cookie.h"
#include "option.h"
#include "httpthread.h"
#include "httpsession.h"
#include "httpthread.h"
#include "httprequest.h"
#include "httpserver.h"
#include "tb.h"
#include "../INCLUDE/parser.h"
#include <QtSql>

bool HTTPRead(QTcpSocket *client,
              QUrl *url, QHash <QString,QString> *env, QByteArray *param);

namespace AUTHORIZATION {
    extern COOKIE Empty;
}

namespace API {
    extern QMutex F;
    extern QSqlDatabase db;
}

HttpAdmin::HttpAdmin(HttpServer *httpserver, QObject *parent) : QTcpServer(parent) {
    this->httpserver=httpserver;
    connect(this,SIGNAL(newConnection()),this,SLOT(Incomming()));
}

void HttpAdmin::Incomming() {
    if(!hasPendingConnections())return;
    QTcpSocket *client=nextPendingConnection();
    if(client==NULL)return;
    QUrl url;
    QHash <QString,QString> env;
    QByteArray param;
    if(!HTTPRead(client,&url,&env,&param)) {
        if(!client->isValid())
            if(client->state()==QAbstractSocket::ConnectedState)client->close();
        client->deleteLater();
        return;
    }
    int code=0;
    anser.clear();
    QString path=url.path();
    if(path=="session")code=session();
    else if(path.startsWith("thread"))code=thread();
    else if(path.startsWith("option"))code=option();
    else if(path.endsWith("Put"))code=put(path,param);
    else if(path.endsWith("Get"))code=get(path,param);
    else if(path.endsWith("Users"))code=users(path,param);
    if(code==0)code=404;
    if(code<0) {
        anser="error";
        code=200;
    }
    if(OPTION::HTTP::spCODE.contains(code))anser=OPTION::HTTP::spCODE.value(code);
    if(OPTION::HTTP::CODE.contains(code))client->write(OPTION::HTTP::CODE.value(code));
    client->write("\r\nContent-Type: text/html;charset=windows-1251");
    client->write("\r\naccess-control-allow-origin: *");
    client->write("\r\n\r\n");
    client->write(anser);
    if(client->state()==QAbstractSocket::ConnectedState)client->waitForBytesWritten();
    client->close();
    client->deleteLater();
    return;
}

int HttpAdmin::option() {
    anser.append(QString("PORT: %1<br>\r\n").arg(OPTION::PORT));
    anser.append(QString("DRIVER: %1<br>\r\n").arg(OPTION::DB::DRIVER));
    anser.append(QString("CONNECT: %1<br>\r\n").arg(OPTION::DB::CONNECT));
    anser.append(QString("PATH: %1<br>\r\n").arg(OPTION::PATH));
    anser.append(QString("SESSION UID: %1<br>\r\n").arg(OPTION::UID));
    anser.append(QString("SESSION TIME: %1<br>\r\n").arg(OPTION::cookieLive));
    anser.append(QString("THREAD: %1<br>").arg(OPTION::maxThread));
    return 200;
}

int HttpAdmin::session() {
    anser.append("<html><table border=1><tr>"
                 "<td>Cookie</td>"
                 "<td>StudID</td>"
                 "<td>StudName</td>"
                 "<td>StudGroupID</td>"
                 "<td>StudGroupName</td>"
                 "<td>StudGroups</td>"
                 "<td>StudGroupsName</td>"
                 "<td>Users</td>"
                 "<td>UsersName</td>"
                 "<td>Time</td></tr>");
    QStringList list=HTTPSESSION::cookie.keys();
    foreach(QString s,list)
        anser.append("<tr><td>")
             .append(HTTPSESSION::cookie.value(s,AUTHORIZATION::Empty).toString("</td><td>"))
             .append("</td></tr>)");
    anser.append("</table></html>");
    return 200;
}

int HttpAdmin::thread() {
    anser.append("<html>");
    anser.append(QString("Нарушений потоков %1<br>").arg(httpserver->timeout));
    anser.append("<table border=1><tr>"
                 "<td>№№</td>"
                 "<td>Runing</td>"
                 "<td>Count</td>"
                 "<td>Cookie</td>"
                 "<td>StudID</td>"
                 "<td>StudName</td>"
                 "<td>StudGroupName</td>"
                 "<td>Users</td>"
                 "<td>Url</td>"
                 "<td>Last code</td>"
                 "</tr>");
    for(int i=0;i<OPTION::maxThread;i++)
        anser.append(QString("<tr>"
                             "<td>%1</td><td>%2</td><td>%3</td>"
                             "<td>%4</td><td>%5</td><td>%6</td>"
                             "<td>%7</td><td>%8</td><td></td>"
                             "</tr>")
             .arg(i+1)
             .arg((HTTPTHREAD::ThreadPtr[i]->isRunning())?"+":"-")
             .arg(HTTPTHREAD::ThreadPtr[i]->count)
             .arg(HTTPTHREAD::ThreadPtr[i]->cookie.Name)
             .arg(HTTPTHREAD::ThreadPtr[i]->cookie.personID)
             .arg(HTTPTHREAD::ThreadPtr[i]->cookie.personName)
             .arg(HTTPTHREAD::ThreadPtr[i]->url.toString())
             .arg(HTTPTHREAD::ThreadPtr[i]->code));
    anser.append("</table></html>");
    return 200;
}

int HttpAdmin::put(const QString &path, const QByteArray &param) {
    QString m=path;
    m.chop(3);
    API::F.lock();
    long id=_put((m.startsWith("json."))?m.mid(5):m,param);
    API::F.unlock();
    if(id<=0)return 404;
    if(m.startsWith("json."))anser.append("({id:\'").append(id).append("\'})");
    else anser.append(QString::number(id));
    return 200;
}

int HttpAdmin::_put(const QString &path, const QByteArray &param) {
    int M=TB::METRIKA::Code.value(path,-1);
    if(M==-1)return -1;
    PARSER t=param;
    if(t.Index("&")!=4)return -1;
    int id=t.SimpleItemInt(1);
    int user=t.SimpleItemInt(2);
    int item=t.SimpleItemInt(3);
    int f=t.SimpleItemInt(4);
    QSqlQuery query(API::db);
    if(id==0) {
        if(!query.exec(QString("select id from Right%1 "
                               "where [User]=%2 and ITEM=%3").arg(path)
                                                             .arg(user)
                                                             .arg(item)))return -1;
        if(query.next())id=query.value(0).toInt();
    }
    if(f==0 && id==0)return id;
    QString sql=(f==0)?QString("DELETE id FROM Right%1 WHERE id=%2").arg(path).arg(id)
                      :(id>0)?QString("UPDATE Right%1 SET [User]=%2,ITEM=%3,FLAG=%4 "
                                      "WHERE ID=%5").arg(path).arg(user)
                                                    .arg(item).arg(f).arg(id)
                             :QString("INSERT INTO Right%1 ([USER],ITEM,FLAG) "
                                      "VALUES (%2,%3,%4)").arg(path).arg(user)
                                                          .arg(item).arg(f);
    if(!query.exec(sql))return -1;
    if(f==0)return id;
    if(id>0)return id;
    if(!query.exec(QString("SELECT Last(ID) FROM Right%1").arg(path)))return -1;
    if(!query.next())return -1;
    return query.value(0).toInt();
}

int HttpAdmin::get(const QString &path, const QByteArray &param) {
    QString m=path;
    m.chop(3);
    TABLE table;
    API::F.lock();
    long id=_get((m.startsWith("json."))?m.mid(5):m,param,&table);
    API::F.unlock();
    if(id<0)return 404;
    if(m.startsWith("json.")) {
        anser.append("([");
        for(int r=1;r<=table.RowCount();r++)
            anser.append('{')
                 .append("id:'").append(table.Item(r,1)).append("',")
                 .append("user:'").append(table.Item(r,2)).append("',")
                 .append("item:'").append(table.Item(r,3)).append("',")
                 .append("flag:'").append(table.Item(r,4)).append("'")
                 .append("},");
        if(anser.endsWith(','))anser.chop(1);
        anser.append("])");
    }
    else {
        for(int r=1;r<=table.RowCount();r++)
            anser.append(table.Item(r,1)).append(";")
                 .append(table.Item(r,2)).append(";")
                 .append(table.Item(r,3)).append(";")
                 .append(table.Item(r,4))
                 .append("\r\n");
        anser.chop(2);
    }
    return 200;
}

int HttpAdmin::_get(const QString &path, const QByteArray &param, TABLE *table) {
    int M=TB::METRIKA::Code.value(path,-1);
    if(M==-1)return -1;
    PARSER t=param;
    int k=t.Index("&");
    if(k<0 || k>3)return -1;
    QString s;
    QString filter;
    for(int i=1;i<=t.ItemCount();i++) {
        s=t.SimpleItem(i);
        s.replace("="," in (");
        s.append(")");
        filter.append(s).append(" and ");
    }
    filter.chop(5);
    QString sql=QString("select id,[user],item,flag from Right%1").arg(path);
    if(!filter.isEmpty())sql.append(" where ").append(filter);
    QSqlQuery query(API::db);
    query.setForwardOnly(true);
    if(!query.exec(sql))return -1;
    while(query.next())
        table->append(query.value(0).toString()).append(";")
              .append(query.value(1).toString()).append(";")
              .append(query.value(2).toString()).append(";")
              .append(query.value(3).toString()).append("\r\n");
    if(table->endsWith("\r\n"))table->chop(2);
    return table->Index(";");
}

int HttpAdmin::users(const QString &path, const QByteArray &param) {
    if(path!="Users" && path!="json.Users")return 404;
    TABLE table;
    API::F.lock();
    long id=_users(&table);
    API::F.unlock();
    if(id<0)return 404;
    if(path.startsWith("json.")) {
        anser.append("([");
        for(int r=1;r<=table.RowCount();r++)
            anser.append('{')
                 .append("id:'").append(table.Item(r,1)).append("',")
                 .append("name:'").append(table.Item(r,2)).append("'")
                 .append("},");
        if(anser.endsWith(','))anser.chop(1);
        anser.append("])");
    }
    else {
        for(int r=1;r<=table.RowCount();r++)
            anser.append(table.Item(r,1)).append(";")
                 .append(table.Item(r,2))
                 .append("\r\n");
        anser.chop(2);
    }
    return 200;
}


int HttpAdmin::_users(TABLE *table) {
    QString sql=QString("select userid,username from [user]");
    QSqlQuery query(API::db);
    query.setForwardOnly(true);
    if(!query.exec(sql))return -1;
    while(query.next())
        table->append(query.value(0).toString()).append(";")
              .append(query.value(1).toString()).append("\r\n");
    if(table->endsWith("\r\n"))table->chop(2);
    return table->Index(";");
}
