//#include <QtCore/QCoreApplication>

//#include <QApplicationStateChangeEvent>
//#include <QGuiApplication>
#include <QApplication>
#include <QTextCodec>
#include <QtSql>
#include "httpserver.h"
#include "httprequest.h"
#include "../include/table.h"
#include "tb.h"
#include "cookie.h"
#include "option.h"
#include "httpthread.h"
#include "httpadmin.h"

#include <QMessageBox>
#include <QtSql>

namespace API {
   extern QSqlDatabase db;
   extern QMutex F;
}

namespace AUTHORIZATION {
    extern COOKIE Empty;
    int Authorization(const QString &author, COOKIE *cookie);
}

QByteArray FileRead(const QString& name) {
    QFile f(name);
    if(!f.open(QIODevice::ReadOnly)) {
        //printf("file error %s",name.toAscii().data());
        printf("file error %s",name.toLocal8Bit().data());
    }
    return(f.readAll());
}

int Load(TABLE *t, const QString& name, const QString& sep, const long e=0) {
    *t=FileRead(name);
    t->insert(0,"\r\n");
    t->append("\r\n");
    int i,j;
    while((i=t->indexOf("\r\n#"))!=-1) {
        j=t->indexOf("\r\n",i+2);
        t->remove(i+2,j-i);
    }
    while(t->indexOf("\r\n\r\n")!=-1)t->replace("\r\n\r\n","\r\n");
    if(t->endsWith("\r\n"))t->chop(2);
    if(t->startsWith("\r\n"))t->remove(0,2);
    //int r=t->Index((sep+"\r").toAscii().data());
    int r=t->Index((sep+"\r").toLocal8Bit().data());
    if(e!=0 && e!=r) {
        qApp->quit();
        //printf("%s line %i\r\n",name.toAscii().data(),r);
        printf("%s line %i\r\n",name.toLocal8Bit().data(),r);
        exit(0);
    }
    return r;
}

int main(int argc, char *argv[])
{
    //return(0);
    //QCoreApplication a(argc, argv);
    QApplication a(argc, argv);
    QString path=a.applicationDirPath()+"/";
    //path="d://M//apache/appserver/";
    //path="d://apache/appserver/";


    //path="c://odarmol/appserver5/";


    path="D:/odarmolTest/odarmol/appServer/";


//    QTextCodec *cyrillicCodec = QTextCodec::codecForName("CP1251");
//    QTextCodec::setCodecForLocale(cyrillicCodec);
//    //QTextCodec::setCodecForCStrings(cyrillicCodec);

    //QStringList a1=a.libraryPaths();
    //a.setLibraryPaths(QStringList()<<"D:/SAIT/Apache/appserver");

    TABLE t;
    int M,M_PARENT;
    Load(&t, path+"metrika.txt",";",3);
    for(int i=1;i<=t.RowCount();i++) {
        TB::METRIKA::Name.insert(t.ItemInt(i,2),t.Item(i,1));
        TB::METRIKA::Code.insert(t.Item(i,1),t.ItemInt(i,2));
        TB::HELP::Metrika.insert(t.ItemInt(i,2),t.Item(i,3));
    }
    Load(&t, path+"source.txt",";",3);
    for(int i=1;i<=t.RowCount();i++) {
        M=TB::METRIKA::Code.value(t.Item(i,1));
        TB::SQL::Source.insert(TB::I(false,M,0),t.Item(i,2));
        TB::SQL::Source.insert(TB::I(true,M,0),t.Item(i,3));
    }
    Load(&t, path+"item.txt",";",5);
    for(int i=1;i<=t.RowCount();i++) {
        M=TB::METRIKA::Code.value(t.Item(i,1));
        TB::SQL::Item.insert(TB::I(false,M,t.ItemChar(i,2)),t.Item(i,3));
        TB::SQL::Item.insert(TB::I(true,M,t.ItemChar(i,2)),t.Item(i,4));
        TB::HELP::Item.insert(TB::I(true,M,t.ItemChar(i,2)),t.Item(i,5));
    }
    Load(&t, path+"group.txt",";",2);
    for(int i=1;i<=t.RowCount();i++) {
        M=TB::METRIKA::Code.value(t.Item(i,1));
        M_PARENT=TB::METRIKA::Code.value(t.Item(i,2));
        TB::METRIKA::Group.insert(M,M_PARENT);
    }
    Load(&t, path+"filter.txt",";",3);
    for(int i=1;i<=t.RowCount();i++) {
        M=TB::METRIKA::Code.value(t.Item(i,1));
        M_PARENT=TB::METRIKA::Code.value(t.Item(i,2));
        TB::SQL::Filter.insert(TB::I(false,M,M_PARENT),t.Item(i,3));
    }
    Load(&t, path+"mime.txt",";",2);
    for(int i=1;i<=t.RowCount();i++)
        OPTION::HTTP::MIME.insert(t.Item(i,1),t.Item(i,2));
    Load(&t, path+"option.txt","\t",2);
    for(int i=1;i<=t.RowCount();i++) {
        if(t.ItemString(i,1)=="DRIVER")OPTION::DB::DRIVER=t.ItemString(i,2);
        if(t.ItemString(i,1)=="CONNECT")OPTION::DB::CONNECT=t.ItemString(i,2);
        if(t.ItemString(i,1)=="PATH")OPTION::PATH=t.ItemString(i,2);
        if(t.ItemString(i,1)=="T")OPTION::cookieLive=t.ItemLong(i,2);
        if(t.ItemString(i,1)=="THREAD")OPTION::maxThread=t.ItemLong(i,2);
        if(t.ItemString(i,1)=="UID")OPTION::UID=t.ItemString(i,2);
        if(t.ItemString(i,1)=="IP")OPTION::IP=t.ItemString(i,2);
        if(t.ItemString(i,1)=="PORT")OPTION::PORT=t.ItemLong(i,2);
        if(t.ItemString(i,1)=="PORTADMIN")OPTION::PORTADMIN=t.ItemLong(i,2);
        if(t.ItemString(i,1)=="SESSION")OPTION::maxCOOKIE=t.ItemLong(i,2);
        if(t.ItemString(i,1)=="TIMEOUT")OPTION::pauseCOOKIE=t.ItemLong(i,2);
        if(t.ItemString(i,1)=="ZIP")OPTION::zipMode=t.ItemLong(i,2);
    }
    HTTPTHREAD::ThreadCode=new int[OPTION::maxThread];
    HTTPTHREAD::ThreadPtr=new HttpRequest *[OPTION::maxThread];
    foreach(QString s,
            QDir(path).entryList(QStringList()<<"post???.http",QDir::Files))
        OPTION::HTTP::CODE.insert(s.mid(4,3).toInt(),FileRead(path+s));
    foreach(QString s,
            QDir(path).entryList(QStringList()<<"sp???.html",QDir::Files))
        OPTION::HTTP::spCODE.insert(s.mid(2,3).toInt(),FileRead(path+s));
    API::db=QSqlDatabase::addDatabase(OPTION::DB::DRIVER, "my_sql_db");
    if(!API::db.isValid()) {
        puts("db driver error");
        qApp->quit();
        exit(0);
    }
    //API::db.setDatabaseName(OPTION::DB::CONNECT);

    //Строка подключения для Microsoft SQL Server
    //API::db.setDatabaseName("DRIVER={SQL Server};Database=VzletBase; Server=Win8Pro-ПК\\SQLEXPRESS;");

    API::db.setDatabaseName("vzletbase");
    API::db.setHostName("localhost");
    API::db.setUserName("root");
    API::db.setPassword("gettherefast");
    API::db.setPort(3306);


    if(!API::db.open()) {
        QString a1=API::db.lastError().text();
        puts("db open error");

        QMessageBox::information(NULL, "", API::db.lastError().text());

        qApp->quit();
        exit(0);
    }

    if(AUTHORIZATION::Authorization("1:", &AUTHORIZATION::Empty)!=1) {
        puts("db users error");
        qApp->quit();
        exit(0);
    }
    //OPTION::PORT=8080;
    HttpServer httpserver;
    if(!httpserver.listen(QHostAddress::Any,OPTION::PORT)) {
        puts("main http error");
        qApp->quit();
        exit(0);
    }
    HttpAdmin httpadmin(&httpserver);
    if(OPTION::PORTADMIN!=0)
        if(!httpadmin.listen(QHostAddress::Any,OPTION::PORTADMIN)) {
            puts("admin http error");
            qApp->quit();
            exit(0);
        }
    return a.exec();
}
