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
QString v = " 0123456789qwertyuiopasdfghjklzxcvbnm.,йцукенгшщзхъфывапролджэячсмитьбю@№;%?()-+_!#";

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
                       "FROM vzletbase.userStatusOrganizationSubdivision "
                       "WHERE "
                       "userID = %1").arg(userID))) {
        while (q.next()) {
            userStatusCode = q.value(0).toInt();
        }

    }
    return userStatusCode;
}



int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    setmode(fileno(stdout),O_BINARY);
    setmode(fileno(stdin ),O_BINARY);


    QString ps = Input();
    QStringList params = ps.split("&");


    //Вывод на экран списка наук (научных направлений)
    if (params.at(0) == "getScienceDirection") {
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

            query = "SELECT scienceDirectionID, scienceDirectionName FROM vzletbase.ScienceDirection ";

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"scienceDirectionCode\":%1,"
                                          "\"scienceDirectionName\":\"%2\"},")
                                  .arg(q.value(0).toString())
                                  .arg(q.value(1).toString()));
                }
                result.chop(1);
                result.append("]");
                db.close();
                printf(result.toLocal8Bit());
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


    //Вывод на экран полного списка городов (местонахождение)
    if (params.at(0) == "getEventLocation") {
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

            //Получаем полный список учителей
            query = "SELECT conferenceLocationID, conferenceLocationName "
                    "FROM vzletbase.conferenceLocation "
                    "WHERE conferenceLocationID <> 1 "
                    "ORDER BY conferenceLocationName ASC ";

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"eventLocationCode\":%1,"
                                          "\"eventLocationName\":\"%2\"},")
                                  .arg(q.value(0).toString())
                                  .arg(q.value(1).toString()));
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
    }


    //Вывод на экран списка приоритетных направлений науки и техники
    if (params.at(0) == "getPriorityDirection") {
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

            query = "SELECT priorityDirectionID, priorityDirectionName FROM vzletbase.PriorityDirection ORDER BY priorityDirectionName ASC ";

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"priorityDirectionCode\":%1,"
                                          "\"priorityDirectionName\":\"%2\"},")
                                  .arg(q.value(0).toString())
                                  .arg(q.value(1).toString()));
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



    //Вывод на экран списка научно-промышленных сфер
    if (params.at(0) == "getScienceIndustrialSphere") {
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

            query = "SELECT scienceIndustrialSphereID, scienceIndustrialSphereName FROM vzletbase.scienceIndustrialSphere ";

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"scienceIndustrialSphereCode\":%1,"
                                          "\"scienceIndustrialSphereName\":\"%2\"},")
                                  .arg(q.value(0).toString())
                                  .arg(q.value(1).toString()));
                }
                result.chop(1);
                result.append("]");
                db.close();
                printf(result.toLocal8Bit());
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



    //Вывод на экран списка организаций-партнеров
    if (params.at(0) == "getOrganization") {
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

            query = "SELECT organizationID, organizationName FROM vzletbase.organization WHERE organizationTypeID = 14 ";

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"organizationCode\":%1,"
                                          "\"organizationName\":\"%2\"},")
                                  .arg(q.value(0).toString())
                                  .arg(q.value(1).toString()));
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



    //Вывод на экран списка мероприятий (фильтр) - для участников
    if (params.at(0) == "chooseEvent") {
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

            //Может войти только администратор, ученик, учитель, консультант, координатор от организации-партнера, аналитик (только подтвержденные)
            if (userCode == 3 || currentUserStatus == 2 || currentUserStatus == 3 || currentUserStatus == 4 || currentUserStatus == 17 || currentUserStatus == 14) {

                QSqlQuery q(db);
                QString query = "";
                QString result = "[";

                query = "SELECT Dov.userFIO, Theme.themeName, Theme.themeComment, Theme.eventDate, "
                        "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_1) AS SCIENCEDIRECTIONNAME1, "
                        "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_2) AS SCIENCEDIRECTIONNAME2, "
                        "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = Theme.SCIENCEDIRECTIONID_3) AS SCIENCEDIRECTIONNAME3, "
                        "Organization.organizationName, userStatusOrganizationSubdivision.postName, StatusTheme.statusThemeName, Theme.themeID, "
                        "PriorityDirection.priorityDirectionName, ScienceIndustrialSphere.scienceIndustrialSphereName, ConferenceLocation.conferenceLocationName, "
                        "Theme.eventRemotely "
                        "FROM vzletbase.ConferenceLocation INNER JOIN ((StatusTheme INNER JOIN (ScienceIndustrialSphere INNER JOIN (PriorityDirection INNER JOIN Theme ON PriorityDirection.priorityDirectionID = Theme.priorityDirectionID) ON ScienceIndustrialSphere.scienceIndustrialSphereID = Theme.scienceIndustrialSphereID) ON StatusTheme.statusThemeID = Theme.statusThemeID) INNER JOIN (Organization INNER JOIN ((Dov INNER JOIN userStatusOrganizationSubdivision ON Dov.dovID = userStatusOrganizationSubdivision.userID) INNER JOIN UserTheme ON Dov.dovID = UserTheme.userID) ON Organization.organizationID = userStatusOrganizationSubdivision.organizationID) ON Theme.themeID = UserTheme.themeID) ON ConferenceLocation.conferenceLocationID = Theme.conferenceLocationID "
                        "WHERE ((userStatusOrganizationSubdivision.statusID)=17) AND ((Theme.statusThemeID)=10) AND ((Theme.eventModerateMark)=True) ";


                //Организации
                if (params.at(2).toInt() < 1) {
                    query.append("AND THEME.THEMEID IN (SELECT THEMEID FROM USERTHEME WHERE USERID IN (SELECT USERID FROM USERSTATUSORGANIZATIONSUBDIVISION)) ");
                }
                else {
                    query.append(QString("AND THEME.THEMEID IN (SELECT THEMEID FROM USERTHEME WHERE USERID IN (SELECT USERID FROM USERSTATUSORGANIZATIONSUBDIVISION WHERE ORGANIZATIONID = %1)) ").arg(params.at(2)));
                }

                //Направления науки
                if (params.at(1).toInt() >= 1) {
                    query.append(QString("AND ((SCIENCEDIRECTIONID_1 = %1) OR (SCIENCEDIRECTIONID_2 = %1) OR (SCIENCEDIRECTIONID_3 = %1)) ").arg(params.at(1)));
                }


                //Город проведения
                if (params.at(3).toInt() >= 1) {
                    query.append(QString("AND (Theme.conferenceLocationID = %1)  ").arg(params.at(3)));
                }

                //Признак того, что мероприятие проводится дистанционно
                if (params.at(4).toInt() == 1) {
                    query.append(QString(" AND ((Theme.eventRemotely=TRUE)) "));
                }

                query.append("ORDER BY Theme.eventDate ASC "); //Сортировка по дате


                if (q.exec(query)) {
                    while (q.next()) {
                        result.append("{");
                        result.append(QString("\"coordinatorFIO\":\"%1\","
                                              "\"eventName\":\"%2\","
                                              "\"eventComment\":\"%3\","
                                              "\"eventDate\":\"%4\","
                                              "\"eventScienceDirectionName1\":\"%5\","
                                              "\"eventScienceDirectionName2\":\"%6\","
                                              "\"eventScienceDirectionName3\":\"%7\","
                                              "\"organizationName\":\"%8\","
                                              "\"postName\":\"%9\","
                                              "\"statusEventName\":\"%10\","
                                              "\"eventCode\":\"%11\","
                                              "\"priorityDirectionName\":\"%12\","
                                              "\"scienceIndustrialSphereName\":\"%13\","
                                              "\"eventLocationName\":\"%14\"},")
                                      .arg(q.value(0).toString())
                                      .arg(q.value(1).toString())
                                      .arg(q.value(2).toString())
                                      .arg(QDate::fromString(q.value(3).toString().left(10),"yyyy-MM-dd").toString("dd.MM.yyyy"))
                                      .arg(q.value(4).toString())
                                      .arg(q.value(5).toString())
                                      .arg(q.value(6).toString())
                                      .arg(q.value(7).toString())
                                      .arg(q.value(8).toString())
                                      .arg(q.value(9).toString())
                                      .arg(q.value(10).toString())
                                      .arg(q.value(11).toString())
                                      .arg(q.value(12).toString())
                                      .arg(q.value(13).toString()));
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





    //Вывод на экран списка мероприятий (фильтр) - для модератора (аналитика) и администратора
    if (params.at(0) == "chooseEventModerate") {
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

            //Если модератор (аналитик) или администратор
            if (currentUserStatus == 14 || userCode == 3) {

                QSqlQuery q(db);
                QString query = "";
                QString result = "[";

                query = "SELECT Dov.userFIO, Theme.themeName, Theme.themeComment, Theme.eventDate, "
                        "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_1) AS SCIENCEDIRECTIONNAME1, "
                        "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_2) AS SCIENCEDIRECTIONNAME2, "
                        "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = Theme.SCIENCEDIRECTIONID_3) AS SCIENCEDIRECTIONNAME3, "
                        "Organization.organizationName, userStatusOrganizationSubdivision.postName, StatusTheme.statusThemeName, Theme.themeID, "
                        "PriorityDirection.priorityDirectionName, ScienceIndustrialSphere.scienceIndustrialSphereName, Theme.eventModerateMark, Theme.eventTextEMail "
                        "FROM (StatusTheme INNER JOIN (ScienceIndustrialSphere INNER JOIN (PriorityDirection INNER JOIN Theme ON PriorityDirection.priorityDirectionID = Theme.priorityDirectionID) ON ScienceIndustrialSphere.scienceIndustrialSphereID = Theme.scienceIndustrialSphereID) ON StatusTheme.statusThemeID = Theme.statusThemeID) INNER JOIN (Organization INNER JOIN ((Dov INNER JOIN userStatusOrganizationSubdivision ON Dov.dovID = userStatusOrganizationSubdivision.userID) INNER JOIN UserTheme ON Dov.dovID = UserTheme.userID) ON Organization.organizationID = userStatusOrganizationSubdivision.organizationID) ON Theme.themeID = UserTheme.themeID "
                        "WHERE (((userStatusOrganizationSubdivision.statusID)=17) AND ((Theme.statusThemeID)=10)) ";

                //Организации
                if (params.at(2).toInt() < 1) {
                    query.append("AND THEME.THEMEID IN (SELECT THEMEID FROM USERTHEME WHERE USERID IN (SELECT USERID FROM USERSTATUSORGANIZATIONSUBDIVISION)) ");
                }
                else {
                    query.append(QString("AND THEME.THEMEID IN (SELECT THEMEID FROM USERTHEME WHERE USERID IN (SELECT USERID FROM USERSTATUSORGANIZATIONSUBDIVISION WHERE ORGANIZATIONID = %1)) ").arg(params.at(2)));
                }

                //Направления науки
                if (params.at(1).toInt() >= 1) {
                    query.append(QString("AND ((SCIENCEDIRECTIONID_1 = %1) OR (SCIENCEDIRECTIONID_2 = %1) OR (SCIENCEDIRECTIONID_3 = %1)) ").arg(params.at(1)));
                }


                if (q.exec(query)) {
                    while (q.next()) {
                        result.append("{");
                        result.append(QString("\"coordinatorFIO\":\"%1\","
                                              "\"eventName\":\"%2\","
                                              "\"eventComment\":\"%3\","
                                              "\"eventDate\":\"%4\","
                                              "\"eventScienceDirectionName1\":\"%5\","
                                              "\"eventScienceDirectionName2\":\"%6\","
                                              "\"eventScienceDirectionName3\":\"%7\","
                                              "\"organizationName\":\"%8\","
                                              "\"postName\":\"%9\","
                                              "\"statusEventName\":\"%10\","
                                              "\"eventCode\":\"%11\","
                                              "\"priorityDirectionName\":\"%12\","
                                              "\"scienceIndustrialSphereName\":\"%13\","
                                              "\"eventMarkModerate\":\"%14\","
                                              "\"eventTextToEMail\":\"%15\"},")
                                      .arg(q.value(0).toString())
                                      .arg(q.value(1).toString())
                                      .arg(q.value(2).toString())
                                      .arg(QDate::fromString(q.value(3).toString().left(10),"yyyy-MM-dd").toString("dd.MM.yyyy"))
                                      .arg(q.value(4).toString())
                                      .arg(q.value(5).toString())
                                      .arg(q.value(6).toString())
                                      .arg(q.value(7).toString())
                                      .arg(q.value(8).toString())
                                      .arg(q.value(9).toString())
                                      .arg(q.value(10).toString())
                                      .arg(q.value(11).toString())
                                      .arg(q.value(12).toString())
                                      .arg(q.value(13).toInt())
                                      .arg(q.value(14).toString()));
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





    //Одобрить мероприятие
    if (params.at(0) == "eventApprove") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {

            int userCode = QString("%1").arg(personID).toInt();
            int currentuserStatus = checkUserStatus(&db, userCode);

            //Если модератор (аналитик) или администратор
            if (currentuserStatus == 14 || userCode == 3) {

                QSqlQuery q(db);
                QString query;

                bool ok = true;
                int eventCode = params.at(1).toInt(&ok);

                if (ok) {
                    query = QString("UPDATE vzletbase.theme "
                                    "SET eventModerateMark = TRUE, eventModerateDateTime = NOW() "
                                    "WHERE themeID = %1 ")
                            .arg(eventCode);

                    if (q.exec(query)) {
                        db.close();

                        printf("{\"errorCode\":0}");
                        a.quit();
                        return (200);
                    }
                    else {
                        printf("{\"errorCode\":1}");
                        a.quit();
                        return (200);
                    }
                }
                else {
                    printf(QString("Проверьте код мероприятия! ").toLocal8Bit());
                    a.quit();
                    return (200);
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



    //Запретить к показу мероприятие
    if (params.at(0) == "eventBanShow") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {

            int userCode = QString("%1").arg(personID).toInt();
            int currentuserStatus = checkUserStatus(&db, userCode);

            //Если модератор (аналитик) или администратор
            if (currentuserStatus == 14 || userCode == 3) {

                QSqlQuery q(db);
                QString query;

                bool ok = true;
                int eventCode = params.at(1).toInt(&ok);

                if (ok) {
                    query = QString("UPDATE vzletbase.theme "
                                    "SET eventModerateMark = FALSE, eventModerateDateTime = NOW() "
                                    "WHERE themeID = %1 ")
                            .arg(eventCode);

                    if (q.exec(query)) {
                        db.close();

                        printf("{\"errorCode\":0}");
                        a.quit();
                        return (200);
                    }
                    else {
                        printf("{\"errorCode\":1}");
                        a.quit();
                        return (200);
                    }
                }
                else {
                    printf(QString("Проверьте код мероприятия! ").toLocal8Bit());
                    a.quit();
                    return (200);
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




    //Удалить мероприятие
    if (params.at(0) == "eventRemove") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {

            int userCode = QString("%1").arg(personID).toInt();
            int currentuserStatus = checkUserStatus(&db, userCode);

            //Если модератор (аналитик) или администратор
            if (currentuserStatus == 14 || userCode == 3) {

                QSqlQuery q(db);
                QString query;

                bool ok = true;
                int eventCode = params.at(1).toInt(&ok);

                if (ok) {
                    query = QString("DELETE FROM vzletbase.theme "
                                    "WHERE themeID = %1 ")
                            .arg(eventCode);

                    if (q.exec(query)) {
                        db.close();

                        printf("{\"errorCode\":0}");
                        a.quit();
                        return (200);
                    }
                    else {
                        printf("{\"errorCode\":1}");
                        a.quit();
                        return (200);
                    }
                }
                else {
                    printf(QString("Проверьте код мероприятия! ").toLocal8Bit());
                    a.quit();
                    return (200);
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


    return 200;
}
