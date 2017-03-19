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
#include <QProcess>

#include "D:/newOdarmol/server/modules/sources/Classes/removetagssql.h";

char *personID=getenv("personID");

QByteArray Input(void) {

    QFile f;
    if(!f.open(fileno(stdin),QIODevice::ReadOnly)) {
        qApp->quit();
        exit(404);
    }
    return f.readAll();
}


//Для всех типов строк
QString v = " 0123456789qwertyuiopasdfghjklzxcvbnm.,йцукенгшщзхъфывапролджэячсмитьбю@№;:%?()-+_";

//Выборка из текста только определенных разрешенных символов
QString TEXT(const QString& s, const QString& x) {
    QString n=x;
    QString p;
    for(int i=0;i<s.length();i++)
        if(n.contains(s.at(i),Qt::CaseInsensitive))p.append(s.at(i));

    removeTagsSQL rts;
    return rts.removeTSQL(p);
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

int checkUserStatus(QSqlDatabase *db, const int userID) {
    int userStatusCode = 0;
    QSqlQuery q(*db);
    if (q.exec(QString("SELECT statusID "
                       "FROM uvzletbase.serStatusOrganizationSubdivision "
                       "WHERE "
                       "userID = %1").arg(userID))) {
        while (q.next()) {
            userStatusCode = q.value(0).toInt();
        }

    }
    return userStatusCode;
}


//Проверка прав пользователя - координатора от организации-партнера на то, может ли он видеть информацию по выбранному мероприятию или нет
//Тоесть - принадлежит ли выбранное мероприятие ему или нет
bool checkEventRight(QSqlDatabase *db, const int userID, const int eventID) {
    int result = 0;
    QSqlQuery q(*db);
    if (q.exec(QString("SELECT count(userID) FROM vzletbase.userTheme "
                       "WHERE (userID = %1) AND (themeID = %2)")
               .arg(userID)
               .arg(eventID))) {
        while (q.next()) {
            result = q.value(0).toInt();
        }

    }

    if (result == 0) {
        return false;
    }
    else if (result >= 1) {
        return true;
    }
    else {
        return false;
    }
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    setmode(fileno(stdout),O_BINARY);
    setmode(fileno(stdin ),O_BINARY);

    QString ps = Input();
    QStringList params = ps.split("&");


    //Получить информацию о мероприятии по его коду
    if (params.at(0) == "getEventInfo") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {

            int coordinatorCode = QString("%1").arg(personID).toInt();
            int currentUserStatus = checkUserStatus(&db, coordinatorCode); //Статус текущего пользователя

            //Если координатор от организации-партнера
            if (currentUserStatus == 17) {
                QSqlQuery q(db);
                QString query = "";
                QString result;

                int eventCode = params.at(1).toInt(); //Код темы

                //Проверка прав доступа к выбранному мероприятию
                if (!checkEventRight(&db, coordinatorCode, eventCode)) {
                    a.quit();
                    return (401);
                }
                else {
                    query = QString("SELECT THEMEID, THEMENAME, THEMECOMMENT, THEMEDATETIME, (SELECT SCIENCEDIRECTIONNAME "
                                    "FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_1) AS SCIENCEDIRECTIONNAME1, "
                                    "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_2) AS SCIENCEDIRECTIONNAME2, "
                                    "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_3) AS SCIENCEDIRECTIONNAME3, "
                                    "STATUSTHEMENAME,  PRIORITYDIRECTIONNAME, SCIENCEINDUSTRIALSPHERENAME "
                                    "FROM VZLETBASE.THEME, VZLETBASE.STATUSTHEME, VZLETBASE.PRIORITYDIRECTION, VZLETBASE.SCIENCEINDUSTRIALSPHERE WHERE (THEMEID = %1) AND (THEME.STATUSTHEMEID = STATUSTHEME.STATUSTHEMEID) AND (THEME.PRIORITYDIRECTIONID = PRIORITYDIRECTION.PRIORITYDIRECTIONID) AND (THEME.SCIENCEINDUSTRIALSPHEREID = SCIENCEINDUSTRIALSPHERE.SCIENCEINDUSTRIALSPHEREID)  ")
                            .arg(eventCode);

                    if (q.exec(query)) {
                        while (q.next()) {
                            result.append("{");
                            result.append(QString("\"eventCode\":%1,"
                                                  "\"eventName\":\"%2\","
                                                  "\"eventComment\":\"%3\","
                                                  "\"eventDateTime\":\"%4\","
                                                  "\"eventScienceDirectionName1\":\"%5\","
                                                  "\"eventScienceDirectionName2\":\"%6\","
                                                  "\"eventScienceDirectionName3\":\"%7\","
                                                  "\"eventStatusThemeName\":\"%8\","
                                                  "\"priorityDirectionName\":\"%9\","
                                                  "\"scienceIndustrialSphereName\":\"%10\"}")
                                          .arg(q.value(0).toString())
                                          .arg(q.value(1).toString())
                                          .arg(q.value(2).toString())
                                          .arg(q.value(3).toString())
                                          .arg(q.value(4).toString())
                                          .arg(q.value(5).toString())
                                          .arg(q.value(6).toString())
                                          .arg(q.value(7).toString())
                                          .arg(q.value(8).toString())
                                          .arg(q.value(9).toString()));

                        }

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
            }
            else {
                a.quit();
                return (401);
            }
        }
        else {
            printf("[error]");
            a.quit();
            return (200);
        }
    }




    //Получить список (с подробной информацией) по участникам, кто привязан к мероприятию
    if (params.at(0) == "getUserInfoByConnectedEvent") {
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
            QString result = "[";
            QSqlQuery q1(db);

            int themeCode = params.at(1).toInt();

            query = QString("SELECT USERID, USERFIO, ORGANIZATIONNAME, SUBDIVISIONLEVEL1NAME, POSTNAME, STATUSNAME  "
                            "FROM VZLETBASE.USERSTATUSORGANIZATIONSUBDIVISION, VZLETBASE.DOV, VZLETBASE.ORGANIZATION, VZLETBASE.SUBDIVISIONLEVEL1, VZLETBASE.STATUS "
                            "WHERE "
                            "(USERSTATUSORGANIZATIONSUBDIVISION.USERID IN (SELECT USERID FROM VZLETBASE.USERTHEME WHERE THEMEID = %1)) AND "
                            "(USERSTATUSORGANIZATIONSUBDIVISION.USERID = DOV.DOVID) AND "
                            "(USERSTATUSORGANIZATIONSUBDIVISION.ORGANIZATIONID = ORGANIZATION.ORGANIZATIONID) AND "
                            "(USERSTATUSORGANIZATIONSUBDIVISION.SUBDIVISIONLEVEL1ID = SUBDIVISIONLEVEL1.SUBDIVISIONLEVEL1ID) AND "
                            "(USERSTATUSORGANIZATIONSUBDIVISION.STATUSID = STATUS.STATUSID) ")
                    .arg(themeCode);

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"statusListCode\":%1,"
                                          "\"statusListFIO\":\"%2\","
                                          "\"statusListOrganizationName\":\"%3\","
                                          "\"statusListSubdivisionLevel1Name\":\"%4\","
                                          "\"statusListPostName\":\"%5\","
                                          "\"statusListStatusName\":\"%6\",")
                                  .arg(q.value(0).toString())
                                  .arg(q.value(1).toString())
                                  .arg(q.value(2).toString())
                                  .arg(q.value(3).toString())
                                  .arg(q.value(4).toString())
                                  .arg(q.value(5).toString()));

                    query = QString("SELECT userThemeMarkPresence, userThemeMarkActive, eventComment FROM vzletbase.userTheme WHERE (userID = %1) AND (themeID = %2) ")
                            .arg(q.value(0).toString())
                            .arg(themeCode);

                    if (q1.exec(query)) {
                        while (q1.next()) {
                            result.append(QString("\"statusListMarkPresence\":\"%1\",").arg(q1.value(0).toString()));
                            result.append(QString("\"statusListMarkActive\":\"%1\",").arg(q1.value(1).toString()));
                            result.append(QString("\"statusListEventComment\":\"%1\"},").arg(q1.value(2).toString()));
                        }
                    }
                    else {
                        printf("[error]");
                        a.quit();
                        return (200);
                    }
                }

                result.chop(1);
                result.append("]");
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
            printf("[error]");
            a.quit();
            return (200);
        }
    }





    //Проставить информацию об участии в мероприятии - присутствовал или нет
    if (params.at(0) == "setMarkPresence") {
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

            int coordinatorCode = QString("%1").arg(personID).toInt(); //Текущий пользователь
            int currentUserStatus = checkUserStatus(&db, coordinatorCode); //Статус текущего пользователя

            //Если координатор от организации-партнера
            if (!currentUserStatus == 17) {
                a.quit();
                return (401);
            }
            else {
                int userCode = params.at(1).toInt(); //Код участника
                int eventCode = params.at(2).toInt(); //Код мероприятия

                int ball = 1; //Балл за участие в мероприятии

                if (!checkEventRight(&db, coordinatorCode, eventCode)) {
                    a.quit();
                    return (401);
                }
                else {
                    query = QString("UPDATE userTheme "
                                    "SET vzletbase.userThemeMarkPresence = TRUE "
                                    "WHERE (userID = %1) AND (themeID = %2) ")
                            .arg(userCode)
                            .arg(eventCode);

                    if (q.exec(query)) {

                        query = QString("UPDATE vzletbase.userStatusOrganizationSubdivision "
                                        "SET childRating = childRating + %1 "
                                        "WHERE userID = %2 ")
                                .arg(ball)
                                .arg(userCode);

                        if (q.exec(query)) {

                            query = QString("UPDATE vzletbase.userStatusOrganizationSubdivision "
                                            "SET childPortfolio = childPortfolio + '+1 балл за участие в мероприятии (%2 %3); ' "
                                            "WHERE userID = %1 ")
                                    .arg(userCode)
                                    .arg(QDate::currentDate().toString("dd.MM.yyyy"))
                                    .arg(QTime::currentTime().toString("hh:mm:ss"));

                            if (q.exec(query)) {
                                printf(QString("ok").toLocal8Bit());
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
                            printf("[error]");
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
            }
        }
        else {
            printf("[error]");
            a.quit();
            return (200);
        }
    }





    //Проставить информацию об активном участии в мероприятии с начислением дополнительного балла
    if (params.at(0) == "setMarkActive") {
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

            int coordinatorCode = QString("%1").arg(personID).toInt(); //Текущий пользователь
            int currentUserStatus = checkUserStatus(&db, coordinatorCode); //Статус текущего пользователя

            //Если координатор от организации-партнера
            if (!currentUserStatus == 17) {
                a.quit();
                return (401);
            }
            else {
                int userCode = params.at(1).toInt(); //Код участника
                int eventCode = params.at(2).toInt(); //Код мероприятия

                int ball = 1; //Балл за активное участие в мероприятии

                if (!checkEventRight(&db, coordinatorCode, eventCode)) {
                    a.quit();
                    return (401);
                }
                else {
                    query = QString("UPDATE vzletbase.userTheme "
                                    "SET userThemeMarkActive = TRUE "
                                    "WHERE (userID = %1) AND (themeID = %2) ")
                            .arg(userCode)
                            .arg(eventCode);

                    if (q.exec(query)) {

                        query = QString("UPDATE vzletbase.userStatusOrganizationSubdivision "
                                        "SET childRating = childRating + %1 "
                                        "WHERE userID = %2 ")
                                .arg(ball)
                                .arg(userCode);

                        if (q.exec(query)) {

                            query = QString("UPDATE vzldtbase.userStatusOrganizationSubdivision "
                                            "SET childPortfolio = childPortfolio + '+1 балл за активное участие в мероприятии (%2 %3); ' "
                                            "WHERE userID = %1 ")
                                    .arg(userCode)
                                    .arg(QDate::currentDate().toString("dd.MM.yyyy"))
                                    .arg(QTime::currentTime().toString("hh:mm:ss"));

                            if (q.exec(query)) {
                                printf(QString("ok").toLocal8Bit());
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
                            printf("[error]");
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
            }
        }
        else {
            printf("[error]");
            a.quit();
            return (200);
        }
    }



    return 200;
}
