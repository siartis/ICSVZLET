#include <QCoreApplication>
#include "ctime"
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
#include <QSqlRecord>

char *personID=getenv("personID");

QByteArray Input(void) {

    QFile f;
    if(!f.open(fileno(stdin),QIODevice::ReadOnly)) {
        qApp->quit();
        exit(404);
    }
    return f.readAll();
}


int checkUserStatus(QSqlDatabase *db, const int userID) {
    int userStatusCode = 0;
    QSqlQuery q(*db);
    if (q.exec(QString("SELECT statusID "
                       "FROM userStatusOrganizationSubdivision "
                       "WHERE "
                       "userID = %1").arg(userID))) {
        while (q.next()) {
            userStatusCode = q.value(0).toInt();
        }
    }
    return userStatusCode;
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

    if (params.at(0) == "part") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {

            int userCode = QString("%1").arg(personID).toInt();
            int currentUserStatus = checkUserStatus(&db, userCode);

            //Если ученик
            if (currentUserStatus == 2) {
                QSqlQuery q(db);

                srand(time(0));
                int x = rand()%30;
                if (!q.exec(QString("INSERT INTO vzletbase.Part ([date], [x], [userID]) VALUES (Now(), %1, %2) ")
                            .arg(x)
                            .arg(userCode))) {
                    printf("[error]");
                    a.quit();
                    return (200);
                }

                db.commit();
                db.close();
                db.open();
                if (!q.exec(QString("SELECT [count] FROM vzletbase.Part WHERE (x = %1) AND (userID = %2) ORDER BY [date] DESC ")
                            .arg(x)
                            .arg(userCode))) {
                    printf("[error]");
                    a.quit();
                    return (200);
                }
                q.next();
                int count=q.value(0).toInt();
                QFile f(QString("c://vzlet//PARTS/%1").arg(count));
                f.open(QIODevice::WriteOnly);
                f.write(params.at(1).toLocal8Bit());
                f.close();
                printf("%li",count);
                a.quit();
            }
            else {
                a.quit();
                return (401);
            }
        }
    }

    return 200;
}
