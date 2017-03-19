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

//Для всех типов строк
QString v=" 0123456789qwertyuiopasdfghjklzxcvbnm.,йцукенгшщзхъфывапролджэячсмитьбю@№;%?()-+_";

//Выборка из текста только определенных разрешенных символов
QString TEXT(const QString& s, const QString& x) {
    QString n=x;
    QString p;
    for(int i=0;i<s.length();i++)
        if(n.contains(s.at(i),Qt::CaseInsensitive))p.append(s.at(i));

    removeTagsSQL rts;
    return rts.removeTSQL(p);
}


QSqlDatabase db;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    setmode(fileno(stdout),O_BINARY);
    setmode(fileno(stdin ),O_BINARY);


    QString ps = Input();
    QStringList params = ps.split("&");


    //Вывод на экран списка тем (фильтр)
    if (params.at(0) == "getThemes") {

        int userCode = QString("%1").arg(personID).toInt();
        //Если вошел администратор
        if (userCode == 3) {
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


                query = "SELECT Theme.themeID, Dov.userFIO, Theme.themeName, Theme.themeComment, Theme.themeDateTime, "
                        "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_1) AS SCIENCEDIRECTIONNAME1, "
                        "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_2) AS SCIENCEDIRECTIONNAME2, "
                        "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = Theme.SCIENCEDIRECTIONID_3) AS SCIENCEDIRECTIONNAME3, "
                        "vzletbase.Organization.organizationName, vzletbase.Post.postName, vzletbase.StageExecute.StageExecuteName, vzletbase.StatusTheme.statusThemeName, vzletbase.Theme.themeID, "
                        "vzletbase.PriorityDirection.priorityDirectionName, vzletbase.Section.sectionName, vzletbase.ScienceIndustrialSphere.scienceIndustrialSphereName "
                        "FROM vzletbase.ScienceIndustrialSphere INNER JOIN (vzletbase.PriorityDirection INNER JOIN (Section INNER JOIN ((vzletbase.StatusTheme INNER JOIN (vzletbase.StageExecute INNER JOIN Theme ON vzletbase.StageExecute.stageExecuteID = vzletbase.Theme.stageExecuteID) ON vzletbase.StatusTheme.statusThemeID = Theme.statusThemeID) INNER JOIN (Post INNER JOIN (Organization INNER JOIN ((Dov INNER JOIN userStatusOrganizationSubdivision ON Dov.dovID = userStatusOrganizationSubdivision.userID) INNER JOIN UserTheme ON Dov.dovID = UserTheme.userID) ON Organization.organizationID = userStatusOrganizationSubdivision.organizationID) ON Post.postID = userStatusOrganizationSubdivision.postID) ON Theme.themeID = UserTheme.themeID) ON Section.sectionID = Theme.sectionID) ON PriorityDirection.priorityDirectionID = Theme.priorityDirectionID) ON ScienceIndustrialSphere.scienceIndustrialSphereID = Theme.scienceIndustrialSphereID "
                        "WHERE ((userStatusOrganizationSubdivision.statusID = 4) OR (userStatusOrganizationSubdivision.statusID = 3)) ";
                //направления науки
                if (params.at(1).toInt() >= 1) {
                    query.append(QString("AND ((SCIENCEDIRECTIONID_1 = %1) OR (SCIENCEDIRECTIONID_2 = %1) OR (SCIENCEDIRECTIONID_3 = %1)) ").arg(params.at(1)));
                }

                //Статус темы
                if (params.at(3).toInt() >= 1) {
                    query.append(QString("AND (THEME.STATUSTHEMEID = %1) ").arg(params.at(3)));
                    //query.append("AND (THEME.STATUSTHEMEID = STATUSTHEME.STATUSTHEMEID)) ");
                }

                if (q.exec(query)) {
                    while (q.next()) {
                        result.append("{");
                        result.append(QString("\"themeCode\":%1,"
                                              "\"consultantFIO\":\"%2\","
                                              "\"themeName\":\"%3\","
                                              "\"themeComment\":\"%4\","
                                              "\"themeDate\":\"%5\","
                                              "\"themeScienceDirectionName1\":\"%6\","
                                              "\"themeScienceDirectionName2\":\"%7\","
                                              "\"themeScienceDirectionName3\":\"%8\","
                                              "\"organizationName\":\"%9\","
                                              "\"postName\":\"%10\","
                                              "\"stageExecuteName\":\"%11\","
                                              "\"statusThemeName\":\"%12\","
                                              "\"themeCode\":\"%13\","
                                              "\"priorityDirectionName\":\"%14\","
                                              "\"sectionName\":\"%15\","
                                              "\"scienceIndustrialSphereName\":\"%16\"},")
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
                                      .arg(q.value(14).toString())
                                      .arg(q.value(15).toString()));
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
        else {
            a.quit();
            return (401);
        }
    }



    //Удаление темы
    if (params.at(0) == "removeTheme") {
        int userCode = QString("%1").arg(personID).toInt();
        if (userCode == 3) {
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
                int themeCode = params.at(1).toInt();

                query = QString("DELETE FROM vzletbase.Theme WHERE themeID = %1 ").arg(themeCode);
                if (q.exec(query)) {
                    db.close();
                    printf("ok");
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


    //Вывод на экран таблицу статистики
    if (params.at(0) == "statistics") {
        int userCode = QString("%1").arg(personID).toInt();

        userCode = 3;
        //Если администратор
        if (userCode == 3) {
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

                int i = 1;
                QString result = "<table border=\"1\">";
                result.append("<tr>");
                result.append("<td>№ п/п</td>");
                result.append("<td>Количество одновременно открытых сессий</td>");
                result.append("<td>Дата</td>");
                result.append("<td>Время</td>");
                result.append("</tr>");

                query = "SELECT countStatCount, countStatDate, countStatTime FROM countStat ";
                if (q.exec(query)) {

                    while (q.next()) {
                        result.append("<tr>");

                        result.append(QString("<td>%1</td>").arg(i));
                        result.append(QString("<td>%1</td>").arg(q.value(0).toString()));
                        result.append(QString("<td>%1</td>").arg(q.value(1).toString()));
                        result.append(QString("<td>%1</td>").arg(q.value(2).toString()));

                        result.append("</tr>");
                    }

                    result.append("</table>");
                    result.append("<br>");

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
        else {
            a.quit();
            return (401);
        }
    }





    //Список пользователей с их данными
    if (params.at(0) == "getUsersTable") {
        int userCode = QString("%1").arg(personID).toInt();

        if (userCode == 3) {
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

                int organizationCode = params.at(1).toInt();
                int statusCode = params.at(2).toInt();


                query = "SELECT dovID, userFIO, userPhone, userMobilePhone, userEMail, dovPassword, userRegDateTime, "
                        "statusName, organizationName, subdivisionLevel1Name "
                        "FROM vzletbase.dov, vzletbase.status, vzletbase.organization, vzletbase.subdivisionLevel1, vzletbase.userStatusOrganizationSubdivision "
                        "WHERE "
                        "(dovID <> 24) AND "
                        "(userStatusOrganizationSubdivision.userID = dov.dovID) AND "
                        "(userStatusOrganizationSubdivision.statusID = status.statusID) AND "
                        "(userStatusOrganizationSubdivision.organizationID = organization.organizationID) AND "
                        "(userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID)  ";

                if (organizationCode != 0) {
                    query.append(QString("AND (userStatusOrganizationSubdivision.organizationID = %1) ").arg(organizationCode));
                }

                if (statusCode != 0) {
                    query.append(QString("AND (userStatusOrganizationSubdivision.statusID = %1) ").arg(statusCode));
                }

                if (q.exec(query)) {
                    while (q.next()) {

                        result.append("{");

                        result.append(QString("\"userCode\":\"%1\",").arg(q.value(0).toString()));
                        result.append(QString("\"userLogin\":\"%1@dov\",").arg(q.value(0).toString()));
                        result.append(QString("\"userFIO\":\"%1\",").arg(q.value(1).toString()).replace("\r\n", "").replace("\n",""));
                        result.append(QString("\"userPhoneNumber\":\"%1\",").arg(q.value(2).toString()));
                        result.append(QString("\"userMobilePhoneNumber\":\"%1\",").arg(q.value(3).toString()));
                        result.append(QString("\"userEMail\":\"%1\",").arg(q.value(4).toString()));
                        result.append(QString("\"userPassword\":\"%1\",").arg(q.value(5).toString()));
                        result.append(QString("\"userDateTime\":\"%1\",").arg(q.value(6).toString()));
                        result.append(QString("\"userStatus\":\"%1\",").arg(q.value(7).toString()));
                        result.append(QString("\"userOrganization\":\"%1\",").arg(q.value(8).toString()));
                        result.append(QString("\"userSubdivision\":\"%1\"").arg(q.value(9).toString()));

                        result.append("},");
                    }

                    db.close();

                    result.chop(1);
                    result.append("]");

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



    //Вывод на экран списка статусов в системе (для остальных)
    if (params.at(0) == "getStatus") {
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

            query = "SELECT STATUSID, STATUSNAME "
                    "FROM VZLETBASE.STATUS "
                    "ORDER BY STATUSNAME ASC ";

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"statusCode\":%1,"
                                          "\"statusName\":\"%2\"},")
                                  .arg(q.value(0).toString())
                                  .arg(q.value(1).toString()));
                }
                db.close();

                result.chop(1);
                result.append("]");

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




    //Изменение ФИО пользователя
    if (params.at(0) == "changeUserFIO") {
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

            int user = QString("%1").arg(personID).toInt();

            //Если администратор
            if (user == 3) {

                int userCode = params.at(1).toInt();

                QByteArray b;
                b.clear();
                b.append(params.at(2));

                QString userFIO = TEXT(QByteArray::fromBase64(b), v);
                b.clear();

                query = QString("UPDATE vzletbase.Dov SET userFIO = '%1' WHERE dovID = %2 ")
                        .arg(userFIO)
                        .arg(userCode);

                if (q.exec(query)) {
                    db.close();

                    printf(QString("Успех").toLocal8Bit());
                    a.quit();
                    return (200);
                }
                else {
                    printf(QString("Ошибка").toLocal8Bit());
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


    //Изменение номера телефона пользователя
    if (params.at(0) == "changeUserPhone") {
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

            int user = QString("%1").arg(personID).toInt();

            //Если администратор
            if (user == 3) {

                int userCode = params.at(1).toInt();

                QByteArray b;
                b.clear();
                b.append(params.at(2));

                QString userPhone = TEXT(QByteArray::fromBase64(b), v);
                b.clear();

                query = QString("UPDATE vzletbase.Dov SET userPhone = '%1' WHERE dovID = %2 ")
                        .arg(userPhone)
                        .arg(userCode);

                if (q.exec(query)) {
                    db.close();

                    printf(QString("Успех").toLocal8Bit());
                    a.quit();
                    return (200);
                }
                else {
                    printf(QString("Ошибка").toLocal8Bit());
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



    //Изменение номера мобильного телефона пользователя
    if (params.at(0) == "changeUserMobilePhone") {
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

            int user = QString("%1").arg(personID).toInt();

            //Если администратор
            if (user == 3) {

                int userCode = params.at(1).toInt();

                QByteArray b;
                b.clear();
                b.append(params.at(2));

                QString userMobilePhone = TEXT(QByteArray::fromBase64(b), v);
                b.clear();

                query = QString("UPDATE vzletbase.Dov SET userMobilePhone = '%1' WHERE dovID = %2 ")
                        .arg(userMobilePhone)
                        .arg(userCode);

                if (q.exec(query)) {
                    db.close();

                    printf(QString("Успех").toLocal8Bit());
                    a.quit();
                    return (200);
                }
                else {
                    printf(QString("Ошибка").toLocal8Bit());
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


    //Изменение адреса электронной почты пользователя
    if (params.at(0) == "changeUserEMail") {
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

            int user = QString("%1").arg(personID).toInt();

            //Если администратор
            if (user == 3) {

                int userCode = params.at(1).toInt();

                QByteArray b;
                b.clear();
                b.append(params.at(2));

                QString userEMail = TEXT(QByteArray::fromBase64(b), v);
                b.clear();

                query = QString("UPDATE vzletbase.Dov SET userEMail = '%1' WHERE dovID = %2 ")
                        .arg(userEMail.trimmed())
                        .arg(userCode);

                if (q.exec(query)) {
                    db.close();

                    printf(QString("Успех").toLocal8Bit());
                    a.quit();
                    return (200);
                }
                else {
                    printf(QString("Ошибка").toLocal8Bit());
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



    //Изменение адреса электронной почты пользователя
    if (params.at(0) == "removeUser") {
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

            int user = QString("%1").arg(personID).toInt();

            //Если администратор
            if (user == 3) {

                int userCode = params.at(1).toInt();

                query = QString("DELETE FROM vzletbase.Dov "
                                "WHERE dovID = %1 ")
                        .arg(userCode);

                if (q.exec(query)) {
                    db.close();

                    printf(QString("Успех").toLocal8Bit());
                    a.quit();
                    return (200);
                }
                else {
                    db.close();

                    printf(QString("Ошибка").toLocal8Bit());
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




    //Вывод на экран списка школьников
    if (params.at(0) == "getChildsTable") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {
            int userCode = QString("%1").arg(personID).toInt();

            if (userCode == 3) {
                QSqlQuery q(db);
                QString query = "";
                QString result = "[";

                query = "SELECT userID, userFIO, organizationName, subdivisionLevel1Name  "
                        "FROM vzletbase.userStatusOrganizationSubdivision, vzletbase.dov, vzletbase.organization, vzletbase.subdivisionLevel1 "
                        "WHERE "
                        "(userStatusOrganizationSubdivision.userID = dov.dovID) AND "
                        "(userStatusOrganizationSubdivision.organizationID = organization.organizationID) AND "
                        "(userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID) "
                        "AND (userStatusOrganizationSubdivision.statusID = 2) "
                        "ORDER BY userFIO ASC ";

                if (q.exec(query)) {
                    while (q.next()) {
                        result.append("{");
                        result.append(QString("\"childCode\":%1,"
                                              "\"childFIO\":\"%2\","
                                              "\"childOrganization\":\"%3\","
                                              "\"childSubdivision\":\"%4\"},")
                                      .arg(q.value(0).toString())
                                      .arg(q.value(1).toString())
                                      .arg(q.value(2).toString())
                                      .arg(q.value(3).toString()));
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


    //Добавить школьнику баллы к рейтингу за - победитель школьного этапа Конкурса
    if (params.at(0) == "setWinnerSchoolStateCompetition") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {
            int userCode = QString("%1").arg(personID).toInt();

            if (userCode == 3) {
                QSqlQuery q(db);
                QString query = "";
                int childCode = params.at(1).toInt();

                QDate currentDate;
                QTime currentTime;

                query = QString("UPDATE vzletbase.userStatusOrganizationSubdivision "
                                "SET childRatingWinnerSchoolDistrictStage = childRatingWinnerSchoolDistrictStage + 3, "
                                "childPortfolioWinnerSchoolDistrictStage = childPortfolioWinnerSchoolDistrictStage + '+3 балла за победу в школьном этапе Конкурса (%2 %3); ' "
                                "WHERE userID = %1 ")
                        .arg(childCode)
                        .arg(currentDate.currentDate().toString("dd.MM.yyyy"))
                        .arg(currentTime.currentTime().toString("hh:mm:ss"));

                if (q.exec(query)) {
                    printf("ok");
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

    //Добавить школьнику баллы к рейтингу за - призер школьного этапа Конкурса
    if (params.at(0) == "setMedalistSchoolStateCompetition") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {
            int userCode = QString("%1").arg(personID).toInt();

            if (userCode == 3) {
                QSqlQuery q(db);
                QString query = "";
                int childCode = params.at(1).toInt();

                QDate currentDate;
                QTime currentTime;

                query = QString("UPDATE vzletbase.userStatusOrganizationSubdivision "
                                "SET childRatingWinnerSchoolDistrictStage = childRatingWinnerSchoolDistrictStage + 2, "
                                "childPortfolioWinnerSchoolDistrictStage = childPortfolioWinnerSchoolDistrictStage + '+2 балла за призовое место в школьном этапе Конкурса (%2 %3); ' "
                                "WHERE userID = %1 ")
                        .arg(childCode)
                        .arg(currentDate.currentDate().toString("dd.MM.yyyy"))
                        .arg(currentTime.currentTime().toString("hh:mm:ss"));

                if (q.exec(query)) {
                    printf("ok");
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

    //Добавить школьнику баллы к рейтингу за - победа в окружном этапе Конкурса
    if (params.at(0) == "setWinnerRegionalStageCompetition") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {
            int userCode = QString("%1").arg(personID).toInt();

            if (userCode == 3) {
                QSqlQuery q(db);
                QString query = "";
                int childCode = params.at(1).toInt();

                QDate currentDate;
                QTime currentTime;

                query = QString("UPDATE vzletbase.userStatusOrganizationSubdivision "
                                "SET childRatingWinnerSchoolDistrictStage = childRatingWinnerSchoolDistrictStage + 5, "
                                "childPortfolioWinnerSchoolDistrictStage = childPortfolioWinnerSchoolDistrictStage + '+5 баллов за победу в окружном этапе Конкурса (%2 %3); ' "
                                "WHERE userID = %1 ")
                        .arg(childCode)
                        .arg(currentDate.currentDate().toString("dd.MM.yyyy"))
                        .arg(currentTime.currentTime().toString("hh:mm:ss"));

                if (q.exec(query)) {
                    printf("ok");
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

    //Добавить школьнику баллы к рейтингу за - призер окружного этапа Конкурса
    if (params.at(0) == "setMedalistRegionalStateCompetition") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {
            int userCode = QString("%1").arg(personID).toInt();

            if (userCode == 3) {
                QSqlQuery q(db);
                QString query = "";
                int childCode = params.at(1).toInt();

                QDate currentDate;
                QTime currentTime;

                query = QString("UPDATE vzletbase.userStatusOrganizationSubdivision "
                                "SET childRatingWinnerSchoolDistrictStage = childRatingWinnerSchoolDistrictStage + 4, "
                                "childPortfolioWinnerSchoolDistrictStage = childPortfolioWinnerSchoolDistrictStage + '+4 балла за призовое место в окружном этапе Конкурса (%2 %3); ' "
                                "WHERE userID = %1 ")
                        .arg(childCode)
                        .arg(currentDate.currentDate().toString("dd.MM.yyyy"))
                        .arg(currentTime.currentTime().toString("hh:mm:ss"));

                if (q.exec(query)) {
                    printf("ok");
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


    return 200;
}
