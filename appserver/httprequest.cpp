#include "httprequest.h"
#include "../include/table.h"
#include <time.h>
#include "option.h"
#include "httpthread.h"
#include "httpsession.h"
#include "tb.h"
#include <QFile>
#include <zlib.h>

bool HTTPRead(QTcpSocket *client,
              QUrl *url, QHash <QString,QString> *env, QByteArray *param);

namespace API {
    extern QMutex F;
    long API(const COOKIE& cookie,
             TABLE *anser,
             const QString &path,const QString &param);
    long READ(const QString &sql, TABLE *res);
}

namespace AUTHORIZATION {
    extern COOKIE Empty;
    int Authorization(const QString &author, COOKIE *cookie);
}

QHash <QString, COOKIE> hashAuthor;

HttpRequest::HttpRequest(QSemaphore *P,
                         QObject *parent):QThread(parent),I(0),client(NULL) {
    this->P=P;
    count=0;
    code=0;
    //cookie=COOKIE::Empty();
    cookie=AUTHORIZATION::Empty;
}

void HttpRequest::start(const int I,
                        QTcpSocket *client, QThread::Priority priority) {
    count++;
    qsrand(I);
    this->I=HTTPTHREAD::ThreadCode[I];
    this->client=client;
    code=0;
    anser.clear();
    type.clear();
    QThread::start(priority);
}

void HttpRequest::run() {
    QByteArray param;
    if(!HTTPRead(client,&url,&env,&param)) {
        if(!client->isValid())
            if(client->state()==QAbstractSocket::ConnectedState)client->close();
        client->deleteLater();
        if(P!=NULL)P->release();
        return;
    }
    {
        QString z="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäå¸¨æçèéêëìíîïðñòóôõö÷øùúûüýþÿ _!#$*+-.,;?=<>(){}[]¹:&/@";
        QString s=param;
        QString e;
        for(int j=0;j<s.length();j++)
            e.append(z.contains(s.at(j))?s.at(j):' ');
        //param=e.toAscii();
        param=e.toLocal8Bit();
    }

    QString path=url.path();
    path.replace("jsscript","js.exe");
    if(path.endsWith(".exe") && !path.startsWith("script/"))path.prepend("script/");
    url.setPath(path);
    if(path=="count") {
        client->write("HTTP/1.1 200 OK\r\n"
                      "text/html;charset=windows-1251\r\n\r\n");
        //client->write(QString::number(HTTPSESSION::cookie.size()).toAscii());
        client->write(QString::number(HTTPSESSION::cookie.size()).toLocal8Bit());
        if(client->state()==QAbstractSocket::ConnectedState)client->waitForBytesWritten();
        client->close();
        client->deleteLater();
        if(P!=NULL)P->release();
        return;
    }

//    if(path.compare("Help",Qt::CaseInsensitive)==0) {
//        path="htdocs/Help.html";
//        url.setPath(path);
//    }
    if(path.isEmpty()||path=="/") {
        path="htdocs/index.html";
        url.setPath(path);
    }
    QString new_session=HTTPSESSION::getCookie(&cookie,env.value("Cookie"));
    int last_code=HTTPSESSION::lastCode.value(cookie.Name);
    //QString author=QByteArray::fromBase64(env.value("Authorization").mid(6).toAscii());
    QString author=QByteArray::fromBase64(env.value("Authorization").mid(6).toLocal8Bit());
    int f=0;
    if(last_code==401 || last_code==-401)f=Authorization(author,&cookie);
    if(path=="pause") {
        code=200;
        type="text/html;charset=windows-1251";
        QTimer t;
        QEventLoop q;
        connect(&t,SIGNAL(timeout()),&q,SLOT(quit()));
        t.start((param.isEmpty())?10:param.toInt());
        q.exec();
        anser.append(QString("Ïàóçà %1").arg((param.isEmpty())?10:param.toInt()));
    }
    else if(path=="active") {
        code=200;
        type="text/html;charset=windows-1251";
        cookie.LastTime=time(NULL);
        anser.append(QString("<html>%1</html>").arg(cookie.Name));
        HTTPSESSION::updateCookie(cookie);
    }
    else if(path=="bay") {
        new_session.clear();
        code=200;
        type="text/html;charset=windows-1251";
        anser.append("<html>bay ...</html>");
    }
    else if(path=="exit") {
        COOKIE t=AUTHORIZATION::Empty;
        t.Name=cookie.Name;
        t.Autor=cookie.Autor;
        HTTPSESSION::updateCookie(t);
        new_session.clear();
        anser.clear();
        code=301;
    }
    else if(path=="login" || path=="json.login") {
        type="text/html;charset=windows-1251";
        QStringList x=QString(param).split("&");
        if(!x.at(0).isEmpty()) {
            //f=Authorization(param,&cookie);
            f=Authorization(x.at(0),&cookie);
            //if(f==0)f=cookie.personID; ????????????????????
        }
        if(f==0) {
            if(x.at(0).isEmpty()) {
                cookie.Autor="";
                HTTPSESSION::updateCookie(cookie);
                code=401;
            }
            else {
                code=404;
                anser.append("\r\n\r\n").append(OPTION::HTTP::spCODE.value(404));
            }
        }
        else if(f>0){
            HTTPSESSION::updateCookie(cookie);
            code=200;
            if(x.length()==2) {
                code=-301;
                anser=QString("HTTP/1.1 301 Moved Permanently\r\n"
                              "Location: %1\r\n\r\n").arg(x.at(1)).toLocal8Bit();
                              //"Location: %1\r\n\r\n").arg(x.at(1)).toAscii();
            }
            else if(path=="login")anser.append(cookie.xxx);
                                  //anser.append("<html>")
                                  //     .append(cookie.toString("<br>"))
                                  //     .append("</html>");
            else anser.append("({")
                      .append("cookie:'").append(cookie.Name)
                      .append("',personID:'").append(QString::number(cookie.personID))
                      .append("',personName:'").append(cookie.personName)
                      .append("',roleID:'").append(QString::number(cookie.roleID))
                      .append("',roleName:'").append(cookie.roleName)
                      .append("',lastTime:'").append(QString::number(cookie.LastTime))
                      .append("'})");
        }
        else {
            code=500;
            anser.append("\r\n\r\n").append(OPTION::HTTP::spCODE.value(500));
        }
    }
    else if(path=="logstat") {
            type="text/html;charset=windows-1251";
            anser=cookie.xxx.toLocal8Bit();
            code=200;
    }
    else if(path=="user")code=user(path,cookie);
    else if(path=="status" || path=="json.status")code=status(path,cookie);
    else if(path.startsWith("api."))code=API(cookie,path.mid(4),param,false);
    else if(path.startsWith("json.api."))code=API(cookie,path.mid(5+4),param,true);
    //else if(path.startsWith("htdocs/"))code=htdocs(url,param,cookie);
    else if(path.startsWith("script/"))code=script(url,param,cookie,false);
    else if(path.startsWith("api/"))code=script(url,param,cookie,true);
    else if(path.startsWith("env.all.")||path.startsWith("env.")||
            path.startsWith("json.env.all.")||path.startsWith("json.env.")) {
        type="text/html;charset=windows-1251";
        code=HTTPSESSION::env(&anser,path,param,cookie);
    }
    else if(path.startsWith("help"))code=HELP(path.mid(4));
    else code=htdocs(url);
//    if(HTTPSESSION::lastCode.contains(cookie.Name))
//        HTTPSESSION::lastCode[cookie.Name]=this->code;
//    else
//        HTTPSESSION::lastCode.insert(cookie.Name,code);
//    if(HTTPSESSION::lastCode.contains(cookie.Name))
//        HTTPSESSION::lastCode.remove(cookie.Name);
    HTTPSESSION::lastCode.insert(cookie.Name,code);
    if(code==0)code=404;
    int i=0;
    QByteArray head;
    if(OPTION::HTTP::spCODE.contains(code)) {
        type="text/html;charset=windows-1251";
        anser=OPTION::HTTP::spCODE.value(code);
    }
    if(code<0) {
        type.clear();
        i=anser.indexOf("\r\n\r\n");
        if(i==-1) {
            client->close();
            client->deleteLater();
            if(P!=NULL)P->release();
            return;
        }
        head=anser.left(i);
        i+=4;
    }
    else head=OPTION::HTTP::CODE.value(code);
    if(!new_session.isEmpty()) {
        int j=head.indexOf("\r\n");
        head.insert((j==-1)?head.size():j,
                    QString("\r\nSet-Cookie: %1=%2==; path=/;").arg(OPTION::UID)
                                                               .arg(new_session));
    }
    client->write(head);
    if(!type.isEmpty())
        //client->write(QString("\r\nContent-Type: %1").arg(type).toAscii());
        client->write(QString("\r\nContent-Type: %1").arg(type).toLocal8Bit());
    client->write("\r\nServer: fist");
    client->write("\r\naccess-control-allow-origin: *");
    client->write("\r\Cache-Control: no-cache, must-revalidate");
    client->write("\r\Pragma: no-cache");
    //putCookie(client,cookie);
    if(OPTION::zipMode>0 && env.value("Accept-Encoding").indexOf("gzip")!=-1) {
        ulong isize=anser.size()-i;
        //ulong crc=crc32(crc32(0, NULL, 0),(const Bytef *)anser.data(),isize);
        int y=anser.size()-i;
        anser=qCompress((const uchar*)anser.data()+i,anser.size()-i,OPTION::zipMode);
        int a1=10+anser.size()-4-2-4+sizeof(ulong)+sizeof(ulong);
        //client->write(QString("\r\nContent-Length: %1").arg(y).toAscii());
        //client->write(QString("\r\nContent-Length: %1").arg(10+anser.size()-4-2-4+sizeof(ulong)+sizeof(ulong)).toAscii());
        //client->write(QString("\r\nContent-Length: %1").arg(y).toAscii());
        client->write("\r\nContent-Encoding: gzip");
        client->write("\r\n\r\n");

        //const char gzipheader[]={0x1f, 0x8b, 0x08,0x01,0,0,0,0,2,255};
        //client->write(gzipheader,10);

        if(anser.size()-4-2-4>0)client->write(anser.data()+6,anser.size()-4-2-4);
        //client->write((char *)&crc,sizeof(ulong));
        client->write((char *)&isize,sizeof(ulong));
    }
    else {
        //client->write(QString("\r\nContent-Length: %1").arg(anser.size()-i).toAscii());
        client->write(QString("\r\nContent-Length: %1").arg(anser.size()-i).toLocal8Bit());
        client->write("\r\n\r\n");
        if(anser.size()-i>0)client->write(anser.data()+i,anser.size()-i);
    }
    QString a1=client->peerAddress().toString();
    QString a2=client->peerName();
    QString a3=QString::number(client->peerPort());
    //int a3=client->bytesToWrite();
    //if(client->state()==QAbstractSocket::ConnectedState)client->waitForBytesWritten();
    client->waitForBytesWritten();
    client->close();
    client->deleteLater();
    if(P!=NULL)P->release();
//    "Content-Language: ru\r\n"
//    "Content-Type: text/html;charset=utf-8\r\n");
//client->write("<head><meta content=\"text/html; charset=utf-8\"></head>");
    /*{
        QProcess p;
        p.setWorkingDirectory(OPTION::PATH+"htdocs");
        p.start(OPTION::PATH+"/App/e.exe");
        if(p.waitForStarted() || 1) {
            QString s;
            s
           //.append("Host="+env.value("Host")).append("\r\n")
             .append("UserPort="+a3).append("\r\n")
             .append("UserIP="+a1).append("\r\n")
             .append("UserIPName="+a2).append("\r\n")
             .append("UserAgent="+env.value("User-Agent")).append("\r\n")
             .append("cookie="+cookie.Name).append("\r\n")
             .append("personID="+QString::number(cookie.personID)).append("\r\n")
             .append("personRole="+cookie.roleName).append("\r\n")
             .append("personName="+cookie.personName).append("\r\n")
             .append("url="+url.toString(QUrl::RemoveAuthority|QUrl::RemoveScheme|QUrl::RemoveQuery|QUrl::RemoveFragment));
            p.write(s.toAscii());
            p.waitForBytesWritten();
            p.closeWriteChannel();
            p.waitForFinished();
        }
    }*/
    return;
}

int HttpRequest::API(const COOKIE& cookie, const QString &path,
                     const QString &param, const bool json) {
    type="text/html;charset=windows-1251";
    TABLE table;
    int code=API::API(cookie,&table,path,param);
    if(code<0) {
        anser.append("\r\n\r\nerror");
        return 404;
    }
    if(json) {
        if(path.endsWith("Get")) {
            PARSER n(param);
            n.Index("&");
            n.replace('=','\0');
            const char *t=n.SimpleItem(1);
            n.RemoveAt(1);
            anser.append("([");
            for(int r=1;r<=table.RowCount();r++) {
                anser.append('{');
                for(int c=1;c<=table.ColunmCount();c++)
                    if(isdigit(t[c-1]))anser.append(n.SimpleItem(t[c-1]-'0'))
                                            .append(":\'")
                                            .append(table.Item(r,c))
                                            .append("\',");
                    else anser.append(t[c-1])
                              .append(":\'")
                              .append(table.Item(r,c))
                              .append("\',");
                anser.chop(1);
                anser.append("},");
            }
            if(anser.endsWith(','))anser.chop(1);
            anser.append("])");
        }
        else anser.append("({id:\'").append(table.Item(1,1)).append("\'})");
    }
    else {
        for(int r=1;r<=table.RowCount();r++) {
            for(int c=1;c<=table.ColunmCount();c++) {
                anser.append(table.Item(r,c));
                anser.append("&");
            }
            anser.chop(1);
            anser.append("\r\n");
        }
        anser.chop(2);
    }
    return 200;
}

int HttpRequest::user(const QString &path, const COOKIE &cookie) {
    long regionID=1;
    QString regionName="-";
    TABLE res;
    if(API::READ(QString("SELECT personID, Person.regionID, regionName "
                    "FROM Region INNER JOIN Person "
                    "ON Region.regionID = Person.regionID "
                    "where personID=%1").arg(cookie.personID),&res)>0) {
         regionID=res.ItemLong(1,2);
         regionName=res.Item(1,3);
    }
    type="text/html;charset=windows-1251";
    anser.append("{")
              .append("\"cookie\":\"").append(cookie.Name)
              .append("\",\"personID\":\"").append(QString::number(cookie.personID))
              .append("\",\"personName\":\"").append(cookie.personName)
              .append("\",\"roleID\":\"").append(QString::number(cookie.roleID))
              .append("\",\"roleName\":\"").append(cookie.roleName)
              .append("\",\"lastTime\":\"").append(QString::number(cookie.LastTime))
              .append("\",\"regionID\":\"").append(QString::number(regionID))
              .append("\",\"regionName\":\"").append(regionName)
              .append("\"}");
    return 200;
}

int HttpRequest::status(const QString &path, const COOKIE &cookie) {
    long regionID=1;
    QString regionName="-";
    TABLE res;
    if(API::READ(QString("SELECT personID, Person.regionID, regionName "
                    "FROM Region INNER JOIN Person "
                    "ON Region.regionID = Person.regionID "
                    "where personID=%1").arg(cookie.personID),&res)>0) {
         regionID=res.ItemLong(1,2);
         regionName=res.Item(1,3);
    }
    type="text/html;charset=windows-1251";
    if(path=="status")anser.append("<html>")
                           .append(cookie.toString("<br>"))
                           .append("<br>").append(QString::number(regionID))
                           .append("<br>").append(regionName)
                           .append("</html>");
    else anser.append("({")
              .append("cookie:'").append(cookie.Name)
              .append("',personID:'").append(QString::number(cookie.personID))
              .append("',personName:'").append(cookie.personName)
              .append("',roleID:'").append(QString::number(cookie.roleID))
              .append("',roleName:'").append(cookie.roleName)
              .append("',lastTime:'").append(QString::number(cookie.LastTime))
              .append("',regionID:'").append(QString::number(regionID))
              .append("',regionName:'").append(regionName)
              .append("'})");
    return 200;
}

int HttpRequest::htdocs(const QUrl& url) {
    QString path=url.path();
    if(!path.startsWith("htdocs/") && !path.startsWith("script/"))path="htdocs/"+path;
    while(path.contains("//"))path.replace("//","/");
    //while(path.contains(" "))path.replace(" ","");
    if(path.contains(".."))return 404;
    if(!path.startsWith("htdocs/") && !path.startsWith("script/"))return 404;
    QFile f;
    if(f.exists(OPTION::PATH+path))
        f.setFileName(OPTION::PATH+path);
    else return 404;
    QDateTime t=QFileInfo(f).lastModified();
    if(env.contains("If-Modified-Since")) {
        QString h=env.value("If-Modified-Since");
        if(h.endsWith(" GMT"))h.chop(4);
        int i=h.indexOf(",");
        QDateTime g=QLocale(QLocale::English).toDateTime((i==-1)?h:h.mid(i+2),
                                                         "dd MMM yyyy hh:mm:ss");
//        if(g.isValid())
//            if(g.toTime_t()>=t.toTime_t())return 304;
    }
    if(!f.open(QIODevice::ReadOnly))return 404;
    anser=f.readAll();
    f.close();
    int i=path.lastIndexOf(".");
    if(i!=-1)type=OPTION::HTTP::MIME.value(path.mid(i+1));
    type.append("\r\nLast-Modified: ")
        .append(QLocale(QLocale::English).toString(t,"ddd, dd MMM yyyy hh:mm:ss"))
        .append(" GMT");
    return 200;
}

int HttpRequest::script(const QUrl& url, const QString& param,
                        const COOKIE& cookie, const bool DB) {
    QString path=url.path();
    if(!QFile::exists(OPTION::PATH+path))return(404);
    QProcess p;
    QStringList env=p.environment();
    env.append(QString("SystemRoot=%1").arg(getenv("SystemRoot")));
    env.append(QString("WINDIR=%1").arg(getenv("windir")));
    env.append(QString("TMP=%1").arg(getenv("TMP")));
    env.append(QString("TEMP=%1").arg(getenv("TEMP")));
    env.append(QString("TMP=%1").arg(getenv("TMP")));
    env.append(QString("QT=%1").arg(getenv("QT")));
    env.append(QString("QTDIR=%1").arg(getenv("QTDIR")));
    env.append(QString("IP=%1").arg(OPTION::IP));
    env.append(QString("PORT=%1").arg(OPTION::PORT));
    env.append(QString("URL=%1").arg(url.toString()));
    env.append(QString("cookie=%1").arg(cookie.Name));
    env.append(QString("personID=%1").arg(cookie.personID));
    env.append(QString("personName=%1").arg(cookie.personName));
    env.append(QString("personRole=%1").arg(cookie.roleName));
    //if(DB)env.append(QString("DB=%1").arg(OPTION::DB::CONNECT));
    env.append(QString("DBCONNECT=%1").arg(OPTION::DB::CONNECT));
    env.append(QString("DBDRIVER=%1").arg(OPTION::DB::DRIVER));
    //QString a1=QString("{\"table\":\"%1\",\"roleName\":\"%1\",\"personID\":\"%2\",\"personName\":\"%3\"}").arg(cookie.roleName).arg(cookie.personID).arg(cookie.roleName);
    QString a1=cookie.xxx;
    env.append(QString("Author=%1").arg(a1.toLocal8Bit().toBase64().data()));
    p.setEnvironment(env);
    p.setWorkingDirectory(OPTION::PATH+"htdocs");
    if(DB)API::F.lock();
    p.start(OPTION::PATH+path);
    if(!p.waitForStarted()) {
        if(DB)API::F.unlock();
        return 500;
    }
    //if(!param.isEmpty())p.write(param.toAscii());
    if(!param.isEmpty())p.write(param.toLocal8Bit());
    p.closeWriteChannel();
    if(!p.waitForFinished()) {
        if(DB)API::F.unlock();
        return 500;
    }
    if(DB)API::F.unlock();
    int code=p.exitCode();
    if(code==401)return 401;
    if(code!=0)anser=p.readAll();
    if(code<0)return code;
    type="text/html;charset=windows-1251";
    return (code==0)?200:code;
}

int HttpRequest::Authorization(const QString &author, COOKIE *cookie) {
    if(author==cookie->Autor || author.isEmpty() //||
       /*author=="1:1" || author=="1:"*/)return 0;
    if(hashAuthor.contains(author)) {
        QString name=cookie->Name;
        *cookie=hashAuthor.value(author);
        cookie->Name=name;
        cookie->LastTime=time(NULL);
        return cookie->personID;
    }
    int i=AUTHORIZATION::Authorization(author,cookie);
    if(i<=0)return i;
    hashAuthor.insert(author,*cookie);
    return cookie->personID;
}

int HttpRequest::HELP(const QString &path) {
    type="text/html;charset=windows-1251";
    if(path.isEmpty())
        return htdocs(QUrl("help.html"));
    else if(path==".list") {
        anser.append("<html><table border=1>"
                     "<tr><td>Ìåòðèêà</td><td>Îïèñàíèå</td></tr>");
        foreach(int h,TB::METRIKA::Name.keys())
            anser.append("<tr><td>")
                 .append(TB::METRIKA::Name.value(h))
                 .append("</td><td>")
                 .append(TB::HELP::Metrika.value(h))
                 .append("</td></tr>");
        anser.append("</table></html>");
        return 200;
    }
    else if(path==".table") {
        QList <int> m_keys=TB::METRIKA::Name.keys();
        QList <int> i_keys=TB::HELP::Item.keys();
        anser.append("<html><table border=1><tr><td></td>");
        for(int i=0;i<m_keys.size();i++)
                anser.append("<td>")
                     .append(TB::METRIKA::Name.value(m_keys.at(i)))
                     .append("<br>")
                     .append(TB::HELP::Metrika.value(m_keys.at(i)))
                     .append("</td>");
        anser.append("</tr>");
        QString a1;
        int M,k;
        for(int j=0;j<i_keys.size();j++) {
            k=i_keys.at(j)%256;
            anser.append("<tr>");
            anser.append("<td>").append(TB::HELP::Item.value(i_keys.at(j)))
                                .append("</td>");
            for(int i=0;i<m_keys.size();i++) {
                M=m_keys.at(i);
                a1.clear();
                if(!TB::SQL::Item.contains(TB::I(false,M,k)) &&
                   !TB::SQL::Item.contains(TB::I(true,M,k)))a1.append("-");
                else if(!TB::SQL::Item.value(TB::I(false,M,k)).isEmpty())a1.append("<b>")
                                                                      .append(k)
                                                                      .append("</b>");
                else if(!TB::SQL::Item.value(TB::I(true,M,k)).isEmpty())a1.append(k);
                else a1.append("-");
                anser.append("<td>").append(a1).append("</td>");
            }
            anser.append("</tr>");
        }
        anser.append("</table></html>");
        return 200;
    }
    else if(path.startsWith(".")) {
        int M=TB::METRIKA::Code.value(path.mid(1),-1);
        if(M==-1)return 404;
        anser.append("<html>")
             .append(TB::HELP::Metrika.value(M))
             .append("<p>Äîñòóïíûå ôèëüòðû: ");
        foreach(int h,TB::METRIKA::Name.keys())
            if(TB::SQL::Filter.contains(TB::I(false,M,h)))
                anser.append(TB::METRIKA::Name.value(h)).append(", ");
        anser.chop(1);
        anser.append("<p>Äîñòóïíûå îïöèè<br>"
                     "<table border=1>"
                     "<tr><td>Item</td><td>write</td><td>read</td></tr>");
        for(int i=0;i<=255;i++)
            if(TB::HELP::Item.contains(TB::I(true ,M,i)))
                anser.append("<tr><td>")
                     .append(TB::HELP::Item.value(TB::I(true ,M,i)))
                     .append("</td><td>")
                     .append((TB::SQL::Item.value(TB::I(false,M,i)).isEmpty())?QChar()
                                                                              :QChar(i))
                     .append("</td><td>")
                     .append((TB::SQL::Item.value(TB::I(true,M,i)).isEmpty())?QChar()
                                                                             :QChar(i))
                     .append("</td></tr>");
        anser.append("</table></html>");
        return 200;
    }
    else return 404;
}
