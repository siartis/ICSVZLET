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


//Проверка статуса пользователя
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





    //Вывод на экран списка секций
    if (params.at(0) == "getSection") {
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

            query = "SELECT sectionID, sectionName FROM vzletbase.Section ";

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"sectionCode\":%1,"
                                          "\"sectionName\":\"%2\"},")
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



    //Вывод на экран списка классов (категорий)
    if (params.at(0) == "getChildGrade") {
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

            query = "SELECT childGradeID, childGradeName FROM vzletbase.ChildGrade ";

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"childGradeCode\":%1,"
                                          "\"childGradeName\":\"%2\"},")
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







    //Вывод на экран списка организаций
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

            query = "SELECT organizationID, organizationName FROM vzletbase.organization ";

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





    //Вывод на экран списка статусов темы
    if (params.at(0) == "getStatusTheme") {
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

            query = "SELECT statusThemeID, statusThemeName FROM vzletbase.statusTheme ";

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"statusThemeCode\":%1,"
                                          "\"statusThemeName\":\"%2\"},")
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



    //Вывод на экран списка секций конкурса ВЗЛЕТ
    if (params.at(0) == "getVZLETSection") {
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

            query = "SELECT sectionID, sectionName FROM vzletbase.section WHERE sectionID <> 1 ";

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"sectionCode\":%1,"
                                          "\"sectionName\":\"%2\"},")
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



    //Вывод на экран списка тем (фильтр)
    if (params.at(0) == "chooseTheme") {
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


            query = "SELECT THEMEID, THEMENAME, THEMECOMMENT, THEMEDATETIME, (SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_1) AS SCIENCEDIRECTIONNAME1, (SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_2) AS SCIENCEDIRECTIONNAME2, (SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_3) AS SCIENCEDIRECTIONNAME3, STATUSTHEMENAME "
                    "FROM VZLETBASE.THEME, VZLETBASE.STATUSTHEME "
                    "WHERE (THEME.STATUSTHEMEID = STATUSTHEME.STATUSTHEMEID) AND ";

            //Организации
            if (params.at(2).toInt() < 1) {
                query.append("THEMEID IN (SELECT THEMEID FROM VZLETBASE.USERTHEME WHERE USERID IN (SELECT USERID FROM VZLETBASE.USERSTATUSORGANIZATIONSUBDIVISION)) ");
            }
            else {
                query.append(QString("THEMEID IN (SELECT THEMEID FROM VZLETBASE.USERTHEME WHERE USERID IN (SELECT USERID FROM VZLETBASE.USERSTATUSORGANIZATIONSUBDIVISION WHERE ORGANIZATIONID = %1)) ").arg(params.at(2)));
            }

            //направления науки
            if (params.at(1).toInt() >= 1) {
                query.append(QString("AND ((SCIENCEDIRECTIONID_1 = %1) OR (SCIENCEDIRECTIONID_2 = %1) OR (SCIENCEDIRECTIONID_3 = %1)) ").arg(params.at(1)));
            }

            //Статус темы
            if (params.at(3).toInt() >= 1) {
                query.append(QString("AND (THEME.STATUSTHEMEID = %1) ").arg(params.at(3)));
            }

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"themeCode\":%1,"
                                          "\"themeName\":\"%2\","
                                          "\"themeComment\":\"%3\","
                                          "\"themeDate\":\"%4\","
                                          "\"themeScienceDirectionName1\":\"%5\","
                                          "\"themeScienceDirectionName2\":\"%6\","
                                          "\"themeScienceDirectionName3\":\"%7\","
                                          "\"themeStatusThemeName\":\"%8\"},")
                                  .arg(q.value(0).toString())
                                  .arg(q.value(1).toString())
                                  .arg(q.value(2).toString())
                                  .arg(QDate::fromString(q.value(3).toString().left(10),"yyyy-MM-dd").toString("dd.MM.yyyy"))
                                  .arg(q.value(4).toString())
                                  .arg(q.value(5).toString())
                                  .arg(q.value(6).toString())
                                  .arg(q.value(7).toString()));
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




    //Вывод на экран списка тем (фильтр)
    if (params.at(0) == "chooseTheme1") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {
            int currentUserID = QString("%1").arg(personID).toInt(); //Польватель системы (его код)
            //Если вошел не пользователь системы
            if (QString("%1").arg(personID).toInt() != 3) {
                if ((checkUserStatus(&db, currentUserID) != 16) || (checkUserStatus(&db, currentUserID) != 17)) {
                    if ((checkUserStatus(&db, currentUserID) < 2) && (checkUserStatus(&db, currentUserID) > 6)) {
                        a.quit();
                        return (401);
                    }
                }
            }

            QSqlQuery q(db);
            QString query = "";
            QString result = "[";

            //Только предлагаемые темы
            if (params.at(4).toInt() == 1) {
                query = "SELECT userFIO, themeName, themeComment, theme.themeDateTime, "
                        "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_1) AS SCIENCEDIRECTIONNAME1, "
                        "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_2) AS SCIENCEDIRECTIONNAME2, "
                        "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_3) AS SCIENCEDIRECTIONNAME3, "
                        "organizationName, postName, stageExecuteName, statusThemeName, theme.themeID, priorityDirectionName, sectionName,  scienceIndustrialSphereName "
                        " "
                        "FROM vzletbase.theme, vzletbase.dov, vzletbase.userStatusOrganizationSubdivision, vzletbase.userTheme, vzletbase.organization, vzletbase.stageExecute, vzletbase.statusTheme, vzletbase.section, vzletbase.priorityDirection, vzletbase.scienceIndustrialSphere "
                        " "
                        "WHERE (theme.statusThemeID = 1) AND "
                        "userTheme.themeID = theme.themeID AND "
                        "dov.dovID = userTheme.userID AND "
                        "userStatusOrganizationSubdivision.userID = dov.dovID AND "
                        "organization.organizationID = userStatusOrganizationSubdivision.organizationID AND "
                        "stageExecute.stageExecuteID = theme.stageExecuteID AND "
                        "statusTheme.statusThemeID = theme.statusThemeID AND "
                        "section.sectionID = theme.sectionID AND "
                        "priorityDirection.priorityDirectionID = theme.priorityDirectionID AND "
                        "scienceIndustrialSphere.scienceIndustrialSphereID = theme.scienceIndustrialSphereID ";
            }


            //Только заявки от учителей с у темы
            if (params.at(4).toInt() == 2) {
                query = "SELECT Dov.userFIO, Theme.themeName, Theme.themeComment, Theme.themeDateTime, "
                        "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_1) AS SCIENCEDIRECTIONNAME1, "
                        "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_2) AS SCIENCEDIRECTIONNAME2, "
                        "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = Theme.SCIENCEDIRECTIONID_3) AS SCIENCEDIRECTIONNAME3, "
                        "Organization.organizationName, userStatusOrganizationSubdivision.postName, StageExecute.stageExecuteName, StatusTheme.statusThemeName, Theme.themeID, PriorityDirection.priorityDirectionName, "
                        "Section.sectionName, ScienceIndustrialSphere.scienceIndustrialSphereName "
                        "FROM (StatusTheme INNER JOIN (StageExecute INNER JOIN ([Section] INNER JOIN (ScienceIndustrialSphere INNER JOIN (PriorityDirection INNER JOIN Theme ON PriorityDirection.priorityDirectionID = Theme.priorityDirectionID) ON ScienceIndustrialSphere.scienceIndustrialSphereID = Theme.scienceIndustrialSphereID) ON Section.sectionID = Theme.sectionID) ON StageExecute.stageExecuteID = Theme.stageExecuteID) ON StatusTheme.statusThemeID = Theme.statusThemeID) INNER JOIN (Organization INNER JOIN ((Dov INNER JOIN userStatusOrganizationSubdivision ON Dov.dovID = userStatusOrganizationSubdivision.userID) INNER JOIN UserTheme ON Dov.dovID = UserTheme.userID) ON Organization.organizationID = userStatusOrganizationSubdivision.organizationID) ON Theme.themeID = UserTheme.themeID "
                        "WHERE (((userStatusOrganizationSubdivision.statusID)=4 Or (userStatusOrganizationSubdivision.statusID)=3) AND ((Theme.statusThemeID)=9)) ";
            }


            //Организации
            if (params.at(2).toInt() < 1) {
                query.append("AND THEME.THEMEID IN (SELECT THEMEID FROM VZLETBASE.USERTHEME WHERE USERID IN (SELECT USERID FROM VZLETBASE.USERSTATUSORGANIZATIONSUBDIVISION)) ");
            }
            else {
                query.append(QString("AND THEME.THEMEID IN (SELECT THEMEID FROM USERTHEME WHERE USERID IN (SELECT USERID FROM VZLETBASE.USERSTATUSORGANIZATIONSUBDIVISION WHERE ORGANIZATIONID = %1)) ").arg(params.at(2)));
            }

            //направления науки
            if (params.at(1).toInt() >= 1) {
                query.append(QString("AND ((SCIENCEDIRECTIONID_1 = %1) OR (SCIENCEDIRECTIONID_2 = %1) OR (SCIENCEDIRECTIONID_3 = %1)) ").arg(params.at(1)));
            }

            //Секция конкурса ВЗЛЕТ
            if (params.at(3).toInt() >= 1) {
                query.append(QString("AND (Theme.sectionID = %1) ").arg(params.at(3)));
            }


            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"consultantFIO\":\"%1\","
                                          "\"themeName\":\"%2\","
                                          "\"themeComment\":\"%3\","
                                          "\"themeDate\":\"%4\","
                                          "\"themeScienceDirectionName1\":\"%5\","
                                          "\"themeScienceDirectionName2\":\"%6\","
                                          "\"themeScienceDirectionName3\":\"%7\","
                                          "\"organizationName\":\"%8\","
                                          "\"postName\":\"%9\","
                                          "\"stageExecuteName\":\"%10\","
                                          "\"statusThemeName\":\"%11\","
                                          "\"themeCode\":\"%12\","
                                          "\"priorityDirectionName\":\"%13\","
                                          "\"sectionName\":\"%14\","
                                          "\"scienceIndustrialSphereName\":\"%15\"},")
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
                                  .arg(q.value(13).toString())
                                  .arg(q.value(14).toString()));
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



    a.quit();
    return (200);
}
