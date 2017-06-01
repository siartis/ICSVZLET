#include <QtSql>
#include "tb.h"
#include "cookie.h"

#include <QFile>

namespace API {
   extern QSqlDatabase db;
   extern QMutex F;
}

namespace AUTHORIZATION {
    COOKIE Empty;

    QString jsonSqlRecord(const QString& table,const QSqlRecord& r) {
        QString s="{";
        for(int i=0;i<r.count();i++) {
            QSqlField t=r.field(i);
            if(t.type()==QMetaType::Int)s.append(QString("\"%1\":%2,").arg(t.name()).arg(t.value().toInt()));
            else s.append(QString("\"%1\":\"%2\",").arg(t.name()).arg(t.value().toString()));
        }
        if(!table.isEmpty())s.append(QString("\"table\":\"%1\"").arg(table));
        if(s.endsWith(","))s.chop(1);
        s.append("}");
        return s;
    }

    QString jsonSqlRecord(const QString& table,const QString& sql) {
        QSqlQuery q(API::db);
        if(!q.exec(sql))return QString();
        if(!q.next())return QString("{}");
        return jsonSqlRecord(table,q.record());
    }


    int ID(const QString& s) {
        if(s=="{}")return 0;
        if(s.isEmpty())return -1;
        if(s.startsWith("{\"table\":"))return -1;
        int i=s.indexOf(":");
        if(i==-1)return -1;
        int j=s.indexOf(",");
        if(j==-1)j=s.length();
        QString a1=s.mid(i+1,j-i-1);
        return s.mid(i+1,j-i-1).toInt();

    }

//    int Authorization_Simple_otv(const QString &author, COOKIE *cookie) {
//        cookie->xxx=jsonSqlRecord("otv",QString("SELECT otvID,univID "
//                                                "FROM Otv "
//                                                "WHERE otvLogin&'@otv:'&otvPassword='%1'")
//                                                .arg(author));
//        return ID(cookie->xxx);
//    }


//    int Authorization_Simple_univ(const QString &author, COOKIE *cookie) {
//        cookie->xxx=jsonSqlRecord("univ",QString("SELECT univID,univName,regionID "
//                                                 "FROM univ "
//                                                 "WHERE univLogin&'@univ:'&univPassword='%1'")
//                                                .arg(author));
//        return ID(cookie->xxx);
//    }

//    int Authorization_Simple_child(const QString &author, COOKIE *cookie) {
//        cookie->xxx=jsonSqlRecord("child",QString("SELECT child.childID, child.childFIO, child.schoolID, School.regionID "
//                                                  "FROM School INNER JOIN child ON School.schoolID = child.schoolID "
//                                                  "WHERE child.childID&'@child:'&child.childPassword='%1'")
//                                                 .arg(author));
//        return ID(cookie->xxx);
//    }

    int Authorization_Simple_adm(const QString &author, COOKIE *cookie) {

        //3@adm:ki63ev
        QString login = author.split("@").at(0);
        QString password = author.split(":").at(1);

        cookie->xxx=jsonSqlRecord("adm", QString("SELECT admID, admFIO FROM vzletbase.Adm "
                                                 "WHERE admLogin = '%1' AND admPassword = '%2' ")
                                  .arg(login)
                                  .arg(password));



//        cookie->xxx=jsonSqlRecord("adm",QString("SELECT admID,admFIO "
//                                                 "FROM adm "
//                                                 "WHERE admLogin&'@adm:'&admPassword='%1'")
//                                                .arg(author));

        QFile f;
        f.setFileName("qqq.txt");
        f.open(QIODevice::WriteOnly);
//        f.write(QString("SELECT admID,admFIO "
//                        "FROM adm "
//                        "WHERE admLogin&'@adm:'&admPassword='%1'")
        //                       .arg(author).toLocal8Bit());
        f.write(QString("SELECT admID, admFIO FROM Adm "
                        "WHERE admLogin = '%1' AND admPassword = '%2'")
                .arg(login)
                .arg(password).toLocal8Bit());
        f.close();

        //Win8Pro-ПК\SQLEXPRESS




        return ID(cookie->xxx);
    }

//    int Authorization_Simple_reg(const QString &author, COOKIE *cookie) {
//        cookie->xxx=jsonSqlRecord("reg",QString("SELECT regID,regFIO,regionID "
//                                                "FROM reg "
//                                                "WHERE regLogin&'@reg:'&regPassword='%1'")
//                                                .arg(author));
//        return ID(cookie->xxx);
//    }

//    int Authorization_Simple_school(const QString &author, COOKIE *cookie) {
//        cookie->xxx=jsonSqlRecord("school",QString("SELECT schoolID,schoolChifFIO,schoolName,regionID "
//                                                   "FROM school "
//                                                   "WHERE schoolLogin&'@school:'&schoolPassword='%1'")
//                                                   .arg(author));
//        return ID(cookie->xxx);
//    }

    int Authorization_Simple_dov(const QString &author, COOKIE *cookie) {

        QString login = author.split("@").at(0);
        QString password = author.split(":").at(1);

        cookie->xxx=jsonSqlRecord("dov", QString("SELECT dovID, dovFIO FROM vzletbase.dov "
                                                 "WHERE dovID = %1 AND dovPassword = '%2' ")
                                  .arg(login)
                                  .arg(password));





//        cookie->xxx=jsonSqlRecord("dov",QString("SELECT dovID,dovFIO "
//                                                "FROM dov "
//                                                "WHERE dovID&'@dov:'&dovPassword='%1'")
//                                                .arg(author));
        return ID(cookie->xxx);
    }

//    int Authorization_Simple_jury(const QString &author, COOKIE *cookie) {

//        cookie->xxx=jsonSqlRecord("jury",QString("SELECT juryID,juryFIO "
//                                                "FROM jury "
//                                                "WHERE juryID&'@jury:'&juryPas='%1'")
//                                                .arg(author));
//        return ID(cookie->xxx);
//    }

    QString Authorization_Role(const QString &author, COOKIE *cookie) {
        int i=author.indexOf(":");
        if(i==-1)return QString();
        QSqlQuery query(API::db);


        QString login = author.split("@").at(0);
        QString password = author.split(":").at(1);



        QFile f;
        f.setFileName("qqq1.txt");
        f.open(QIODevice::WriteOnly);
        f.write(QString("SELECT personID, vip.roleID, roleName, tabel "
                        "FROM vzletbase.VIP "
                        "WHERE personLogin = '%1' AND personPAS = '%2' ")
                .arg(login.replace(":",""))
                .arg(password).toLocal8Bit());
        f.close();



        if(!query.exec(QString("SELECT personID, vip.roleID, roleName, tabel "
                               "FROM vzletbase.VIP "
                               "WHERE personLogin = '%1' AND personPAS = '%2' ")
                       .arg(login.replace(":",""))
                       .arg(password))) {
            puts(query.lastError().text().toLocal8Bit());
        }





        if(!query.next())return QString("777");
        cookie->personID=query.value(0).toInt();
        cookie->roleID=query.value(1).toInt();
        cookie->personName=query.value(2).toString();
        QString table=query.value(3).toString().trimmed();
        return (table.isEmpty())?"777":table;


//        if(!query.exec(QString("SELECT personID,vip.roleID,roleName,table "
//                               "FROM VIP "
//                               "WHERE personLogin&':'&personPAS='%1'").arg(author)))

//        if(!query.next())return QString("777");
//        cookie->personID=query.value(0).toInt();
//        cookie->roleID=query.value(1).toInt();
//        cookie->personName=query.value(2).toString();
//        QString table=query.value(3).toString();
//        return (table.isEmpty())?"777":table;
    }

    int Authorization_Simple(const QString &author, COOKIE *cookie) {
        QString x=author;
        cookie->xxx="{}";
        cookie->roleID=0;
        cookie->roleName="";
        if(!author.contains("@")) {
            QString a1=Authorization_Role(x,cookie);
            if(a1.isEmpty())return -1;
            if(a1=="777")return 0;
            if(a1=="Anonimus")return cookie->personID;
            x.replace(QString(":"),QString("@%1:").arg(a1));
            //x.replace(QString(":"),QString("@%1:").arg(a1));
        }
        if(false);
//        else if(x.contains("@otv"))cookie->personID=Authorization_Simple_otv(x,cookie);
//        else if(x.contains("@univ"))cookie->personID=Authorization_Simple_univ(x,cookie);
//        else if(x.contains("@child"))cookie->personID=Authorization_Simple_child(x,cookie);
        else if(x.contains("@adm"))cookie->personID=Authorization_Simple_adm(x,cookie);
//        else if(x.contains("@reg"))cookie->personID=Authorization_Simple_reg(x,cookie);
//        else if(x.contains("@school"))cookie->personID=Authorization_Simple_school(x,cookie);
        else if(x.contains("@dov"))cookie->personID=Authorization_Simple_dov(x,cookie);
//        else if(x.contains("@jury"))cookie->personID=Authorization_Simple_jury(x,cookie);
        //else if(x.contains("@dov"))cookie->personID=Authorization_Simple_otv(x,cookie);
        else return -1;
        return cookie->personID;
    }

    int Authorization(const QString &author, COOKIE *cookie) {
        API::F.lock();
        int x=Authorization_Simple(author,cookie);
        API::F.unlock();
        return x;
    }
}

