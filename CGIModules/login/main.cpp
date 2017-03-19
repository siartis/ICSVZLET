#include <QCoreApplication>

#include "stdlib.h"
#include "stdio.h"
#include <QByteArray>
#include <QFile>
#include <QTextCodec>
#include <fcntl.h>
#include "io.h"
#include <QStringList>
#include <QSqlDatabase>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QRegExp>
#include "math.h"
#include <QSqlRecord>
#include <QDate>
#include <QTime>

char *personID=getenv("personID");


QByteArray Input(void) {

    QFile f;
    if(!f.open(fileno(stdin),QIODevice::ReadOnly)) {
        qApp->quit();
        exit(404);
    }
    return f.readAll();
}



void END(const int code, const char *anser) {
    if(anser!=NULL) {
        QFile f;
        if(!f.open(fileno(stdout),QIODevice::WriteOnly)) {
            qApp->quit();
            exit(404);
        }
        f.write(anser);
        f.close();
    }
    qApp->quit();
    exit(code);
}

QSqlDatabase db;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    setmode(fileno(stdout),O_BINARY);
    setmode(fileno(stdin ),O_BINARY);

    QString ps = Input();
    QStringList params = ps.split("&");

    if (params.at(0) == "user") {
        int userID = QString("%1").arg(personID).toInt();

        if (QString("%1").arg(userID).toInt() != 1) {
            db = QSqlDatabase::addDatabase("QMYSQL", "db");
            db.setDatabaseName("vzletbase");
            db.setHostName("");
            db.setUserName("root");
            db.setPassword("");
            db.setPort(3306);
            db.open();
            if (db.isOpen()) {
                QSqlQuery q(db);
                QString query = "";
                QString result;

                //Проверка - не администратор ли?
                if (userID != 3) {

                    query = QString("SELECT userID, userFIO, userStatusOrganizationSubdivision.statusID, statusName, "
                                    "userStatusOrganizationSubdivision.organizationID, organizationName, userStatusOrganizationSubdivision.subdivisionLevel1ID, "
                                    "subdivisionLevel1Name "
                                    "FROM vzletbase.userStatusOrganizationSubdivision, vzletbase.Dov, vzletbase.Status, vzletbase.Organization, vzletbase.SubdivisionLevel1 "
                                    "WHERE "
                                    "userID = %1 AND "
                                    "userStatusOrganizationSubdivision.userID = dov.dovID AND "
                                    "userStatusOrganizationSubdivision.statusID = status.statusID AND "
                                    "userStatusOrganizationSubdivision.organizationID = organization.organizationID AND "
                                    "userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID ")
                            .arg(QString("%1").arg(userID));

                    if (q.exec(query)) {
                        while (q.next()) {
                            result.append("{");
                            result.append(QString("\"userCode\":%1,").arg(q.value(0).toString()));
                            result.append(QString("\"userFIO\":\"%1\",").arg(q.value(1).toString()));
                            result.append(QString("\"userStatusCode\":%1,").arg(q.value(2).toString()));
                            result.append(QString("\"userStatusName\":\"%1\",").arg(q.value(3).toString()));

                            result.append(QString("\"userOrganizationCode\":%1,").arg(q.value(4).toString()));
                            result.append(QString("\"userOrganizationName\":\"%1\",").arg(q.value(5).toString()));

                            result.append(QString("\"userSubdivisionCode\":%1,").arg(q.value(6).toString()));
                            result.append(QString("\"userSubdivisionName\":\"%1\",").arg(q.value(7).toString()));
                        }


                        //Теперь выведем тип организации и его код
                        query = QString("SELECT organizationTypeID, organizationTypeName "
                                        "FROM vzletbase.organizationType "
                                        "WHERE "
                                        "organizationTypeID IN "
                                        "( "
                                        "SELECT organizationTypeID "
                                        "FROM vzletbase.organization "
                                        "WHERE "
                                        "organizationID IN ( "
                                        "SELECT organizationID "
                                        "FROM vzletbase.userStatusOrganizationSubdivision "
                                        "WHERE userID = %1 "
                                        ") "
                                        ")")
                                .arg(QString("%1").arg(userID));

                        if (q.exec(query)) {
                            while (q.next()) {
                                result.append(QString("\"userOrganizationTypeCode\":%1,").arg(q.value(0).toString()));
                                result.append(QString("\"userOrganizationTypeName\":\"%1\"}").arg(q.value(1).toString()));
                            }
                        }
                        else {
                            printf("[error]");
                            a.quit();
                            return (200);
                        }

                        db.close();
                        printf(result.toLocal8Bit());

                        a.quit();
                        return (200);
                    }
                    else {
                        printf("[error]");
                        a.quit();
                        return (200);
                    }
                }
                else {
                    //Это администратор
                    QString result = "{\"userCode\":3,\"userFIO\":\"Администратор\",\"userStatusCode\":100,\"userStatusName\":\"Администратор\"}";
                    printf(result.toLocal8Bit());
                    a.quit();
                    return (200);
                }
            }
            else {
                printf("[error]");
                a.quit();
                return (200);
            }
        }
        else {
            QString result = "{\"userCode\":1,\"userFIO\":\"Гость\",\"userStatusCode\":0,\"userStatusName\":\"Гость\"}";
            printf(result.toLocal8Bit());
            a.quit();
            return (200);
        }

    }


    return (200);
}
