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
#include "ctime"
#include <QSqlRecord>

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
QString v=" 0123456789qwertyuiopasdfghjklzxcvbnm.,йцукенгшщзхъфывапролджэячсмитьбю@№;%?()-+=_";

//Выборка из текста только определенных разрешенных символов
QString TEXT(const QString& s, const QString& x) {
    QString n=x;
    QString p;
    for(int i=0;i<s.length();i++)
        if(n.contains(s.at(i),Qt::CaseInsensitive))p.append(s.at(i));

    removeTagsSQL rts;
    return rts.removeTSQL(p);
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

//Получение статуса пользователя
QString getUserStatus(QSqlDatabase *db, const int userID) {
    QString userStatusName;
    QSqlQuery q(*db);

    if (q.exec(QString("SELECT statusName FROM status WHERE statusID IN "
                       "(SELECT vzletbase.statusID "
                       "FROM userStatusOrganizationSubdivision "
                       "WHERE "
                       "userID = %1) ").arg(userID))) {
        while (q.next()) {
            userStatusName = q.value(0).toInt();
        }

    }
    return userStatusName;
}


//Проверка - ученик ли это
bool isChild(QSqlDatabase *db, const int userID) {
    int childID = 0;
    QSqlQuery q(*db);

    if (q.exec(QString("SELECT userID FROM vzletbase.userStatusOrganizationSubdivision WHERE (statusID = 2) AND (userID = %1) ").arg(userID))) {
        while (q.next()) {
            childID = q.value(0).toInt();
        }

        if (childID != 0) {
            return true;
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
}



//Проверка - ученик из той же школы, что и координатор от школы
bool isChildSchool(QSqlDatabase *db, const int tutorID, const int childID) {
    int childCode = 0;
    int schoolID = 0;
    QSqlQuery q(*db);

    //Получаем код школы
    if (q.exec(QString("SELECT subdivisionLevel1ID FROM vzletbase.userStatusOrganizationSubdivision WHERE userID = %1 ").arg(tutorID))) {
        while (q.next()) {
            schoolID = q.value(0).toInt();
        }

        //Проверка
        if (q.exec(QString("SELECT userID FROM vzletbase.userStatusOrganizationSubdivision WHERE (statusID = 2) AND (subdivisionLevel1ID = %1) AND (userID = %2) ")
                   .arg(schoolID)
                   .arg(childID))) {
            while (q.next()) {
                childCode = q.value(0).toInt();
            }

            if (childCode != 0) {
                return true;
            }
            else {
                return false;
            }

        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
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


    //Вывод на экран списка учеников конкретной школы (в зависимости от координатора от школы)
    if (params.at(0) == "getChild") {
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

            int userCode = QString("%1").arg(personID).toInt();

            //Получаем код школы вошедшего координатора
            query = QString("SELECT subdivisionLevel1ID FROM vzletbase.userStatusOrganizationSubdivision WHERE userID = %1 ")
                    .arg(userCode);

            if (q.exec(query)) {
                int schoolCode = 0;
                while (q.next()) {
                    schoolCode = q.value(0).toInt();
                }

                query = QString("SELECT dovID, userFIO FROM dov "
                                "WHERE dovID IN ( "
                                "SELECT userID FROM vzletbase.userStatusOrganizationSubdivision "
                                "WHERE (statusID = 2) "
                                ") "
                                "ORDER BY userFIO ASC ")
                        .arg(schoolCode);


                if (q.exec(query)) {
                    while (q.next()) {
                        result.append("{");
                        result.append(QString("\"childCode\":%1,"
                                              "\"childFIO\":\"%2\"},")
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
    }



    //Вывод на экран полного списка конференций
    if (params.at(0) == "getConference") {
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

            int userCode = QString("%1").arg(personID).toInt();
            int currentUserStatus = checkUserStatus(&db, userCode);

            //Если это координатор от школы
            if (currentUserStatus == 6) {

                //Получаем полный список учителей
                query = "SELECT conferenceID, conferenceName "
                        "FROM valetbase.conference ";

                if (q.exec(query)) {
                    while (q.next()) {
                        result.append("{");
                        result.append(QString("\"conferenceCode\":%1,"
                                              "\"conferenceName\":\"%2\"},")
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
                printf("error");
                a.quit();
                return (401);
            }
        }
    }







    //Вывод на экран полного уровней конференции
    if (params.at(0) == "getLevelConference") {
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

            int userCode = QString("%1").arg(personID).toInt();
            int currentUserStatus = checkUserStatus(&db, userCode);

            //Если это координатор от школы
            if (currentUserStatus == 6) {

                //Получаем полный список учителей
                query = "SELECT levelConferenceID, levelConferenceName "
                        "FROM vzletbase.levelConference "
                        "ORDER BY levelConferenceMark ASC ";

                if (q.exec(query)) {
                    while (q.next()) {
                        result.append("{");
                        result.append(QString("\"levelConferenceCode\":%1,"
                                              "\"levelConferenceName\":\"%2\"},")
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
                printf("error");
                a.quit();
                return (401);
            }
        }
    }





    //Вывод на экран полного списка учителей (руководителей)
    if (params.at(0) == "getTutor") {
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
            query = "SELECT dovID, userFIO FROM vzletbase.dov "
                    "WHERE dovID IN ( "
                    "SELECT userID FROM valetbase.userStatusOrganizationSubdivision "
                    "WHERE statusID = 3 "
                    ") "
                    "ORDER BY userFIO ASC ";

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"tutorCode\":%1,"
                                          "\"tutorFIO\":\"%2\"},")
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


    //Вывод на экран полного списка научных консультантов
    if (params.at(0) == "getConsultant") {
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
            query = "SELECT dovID, userFIO FROM vzletbase.dov "
                    "WHERE dovID IN ( "
                    "SELECT userID FROM vzletbase.userStatusOrganizationSubdivision "
                    "WHERE statusID = 4 "
                    ") "
                    "ORDER BY userFIO ASC ";

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"consultantCode\":%1,"
                                          "\"consultantFIO\":\"%2\"},")
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


    //Вывод на экран полного списка городов (местонахождение)
    if (params.at(0) == "getConferenceLocation") {
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

            int userCode = QString("%1").arg(personID).toInt();
            int currentUserStatus = checkUserStatus(&db, userCode);

            //Если это координатор от школы
            if (currentUserStatus == 6) {

                //Получаем полный список учителей
                query = "SELECT conferenceLocationID, conferenceLocationName "
                        "FROM vzletbase.conferenceLocation "
                        "WHERE conferenceLocationID <> 1 "
                        "ORDER BY conferenceLocationName ASC ";

                if (q.exec(query)) {
                    while (q.next()) {
                        result.append("{");
                        result.append(QString("\"conferenceLocationCode\":%1,"
                                              "\"conferenceLocationName\":\"%2\"},")
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
                printf("error");
                a.quit();
                return (401);
            }
        }
    }


    //Вывод на экран полного списка результатов участия (видов достижения)
    if (params.at(0) == "getAchievementView") {
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

            int userCode = QString("%1").arg(personID).toInt();
            int currentUserStatus = checkUserStatus(&db, userCode);

            //Если это координатор от школы
            if (currentUserStatus == 6) {

                //Получаем полный список учителей
                query = "SELECT achievementViewID, achievementViewName FROM vzletbase.achievementView ORDER BY achievementViewMark ";

                if (q.exec(query)) {
                    while (q.next()) {
                        result.append("{");
                        result.append(QString("\"achievementViewCode\":%1,"
                                              "\"achievementViewName\":\"%2\"},")
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
                printf("error");
                a.quit();
                return (401);
            }
        }
    }


    //Вывод на экран полного списка Учебных годов совершения достижения
    if (params.at(0) == "getConferenceYear") {
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

            int userCode = QString("%1").arg(personID).toInt();
            int currentUserStatus = checkUserStatus(&db, userCode);

            //Если это координатор от школы
            if (currentUserStatus == 6) {

                //Получаем полный список учителей
                query = "SELECT conferenceYearID, conferenceYearName "
                        "FROM vzletbase.conferenceYear "
                        "WHERE conferenceYearID <> 1 "
                        "ORDER BY conferenceYearName ASC ";

                if (q.exec(query)) {
                    while (q.next()) {
                        result.append("{");
                        result.append(QString("\"conferenceYearCode\":%1,"
                                              "\"conferenceYearName\":\"%2\"},")
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
                printf("error");
                a.quit();
                return (401);
            }
        }
    }



    //Вывод на экран полного списка образовательных округов
    if (params.at(0) == "getEducationalDistrict") {
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

            query = "SELECT organizationID, organizationName "
                    "FROM vzletbase.organization "
                    "WHERE organizationTypeID = 2 "
                    "ORDER BY organizationName ASC ";

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"educationalDistrictCode\":%1,"
                                          "\"educationalDistrictName\":\"%2\"},")
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


    //Вывод на экран полного списка школ по выбранному образовательному округу
    if (params.at(0) == "getSchool") {
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

            int educationalDistrict = params.at(1).toInt();

            query = QString("SELECT subdivisionLevel1ID, subdivisionLevel1Name "
                            "FROM vzletbase.subdivisionLevel1 "
                            "WHERE organizationID = %1 "
                            "ORDER BY subdivisionLevel1Name ASC ")
                    .arg(educationalDistrict);

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"schoolCode\":%1,"
                                          "\"schoolName\":\"%2\"},")
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




    //Вывод на экран полного списка школ
    if (params.at(0) == "listAchievement") {
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

            int childCode, tutorCode, consultantCode, educationalDistrict, schoolCode = 0;

            childCode = params.at(1).toInt();
            tutorCode = params.at(2).toInt();
            consultantCode = params.at(3).toInt();
            educationalDistrict = params.at(4).toInt();
            schoolCode = params.at(5).toInt();

            query = "SELECT achievementID, "
                    "(SELECT userFIO FROM vzletbase.dov WHERE dovID = childID) AS childFIO, achievementViewName, achievementThemeName, conferenceName, conferenceLocationName, conferenceYearName, "
                    "(SELECT userFIO FROM vzletbase.dov WHERE dovID = tutorID) AS tutorFIO, "
                    "(SELECT userFIO FROM vzletbase.dov WHERE dovID = consultantID) AS consultantFIO, organizationName, subdivisionLevel1Name, levelConferenceName "
                    "FROM vzletbase.achievement, vzletbase.achievementView, vzletbase.conference, vzletbase.conferenceLocation, vzletbase.conferenceYear, "
                    "vzletbase.userStatusOrganizationSubdivision, vzletbase.organization, vzletbase.subdivisionLevel1, vzletbase.levelConference "
                    "WHERE (achievementView.achievementViewID = achievement.achievementViewID) AND "
                    "(conference.conferenceID = achievement.conferenceID) AND "
                    "(conferenceLocation.conferenceLocationID = achievement.conferenceLocationID) AND "
                    "(conferenceYear.conferenceYearID = achievement.conferenceYearID) AND "
                    "(achievement.levelConferenceID = levelConference.levelConferenceID)"
                    "AND (userStatusOrganizationSubdivision.userID = childID) "
                    "AND (organization.organizationID = userStatusOrganizationSubdivision.organizationID) "
                    "AND (userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID)";


            if (childCode != 0) {
                query.append(QString(" AND (childID = %1) ").arg(childCode));
            }

            if (tutorCode != 0) {
                query.append(QString(" AND (tutorID = %1) ").arg(tutorCode));
            }

            if (consultantCode != 0) {
                query.append(QString(" AND (consultantID = %1) ").arg(consultantCode));
            }


            if (educationalDistrict != 0) {
                query.append(QString(" AND (userStatusOrganizationSubdivision.organizationID = %1) ").arg(educationalDistrict));
            }

            if (schoolCode != 0) {
                query.append(QString(" AND (userStatusOrganizationSubdivision.subdivisionLevel1ID = %1) ").arg(schoolCode));
            }


            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"emptyCode\":0,"
                                          "\"achievementCode\":%1,"
                                          "\"achievementChildFIO\":\"%2\","
                                          "\"achievementViewName\":\"%3\","
                                          "\"achievementThemeName\":\"%4\","
                                          "\"achievementConferenceName\":\"%5\","
                                          "\"achievementConferenceLocationName\":\"%6\","
                                          "\"achievementConferenceYearName\":\"%7\","
                                          "\"achievementTutorFIO\":\"%8\","
                                          "\"achievementConsultantFIO\":\"%9\","
                                          "\"achievementOrganizationName\":\"%10\","
                                          "\"achievementSubdivisionLevel1Name\":\"%11\","
                                          "\"achievementLevelConference\":\"%12\"},")
                                  .arg(q.value(0).toString())
                                  .arg(q.value(1).toString())
                                  .arg(q.value(2).toString())
                                  .arg(q.value(3).toString())
                                  .arg(q.value(4).toString())
                                  .arg(q.value(5).toString())
                                  .arg(q.value(6).toString())
                                  .arg(q.value(7).toString())
                                  .arg(q.value(8).toString())
                                  .arg(q.value(9).toString())
                                  .arg(q.value(10).toString())
                                  .arg(q.value(11).toString()));
                }

                if (result == "[") {
                    db.close();
                    result.append("{\"emptyCode\":1}]");
                    printf(result.toLocal8Bit());
                    a.quit();
                    return (200);

                }
                else {
                    result.chop(1);
                    result.append("]");
                    db.close();
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
    }





    //Ввод нового достижения
    if (params.at(0) == "setAchievement") {
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

            int userCode = QString("%1").arg(personID).toInt();
            int currentUserStatus = checkUserStatus(&db, userCode);

            //Если это координатор от кафедры/школы
            if (currentUserStatus == 6) {

                int childCode, tutorCode, consultantCode, conferenceCode, conferenceLocationCode, conferenceYearCode, achievementViewCode = 0;
                QString themeName;

                bool ok1, ok2, ok3, ok4, ok5, ok6, ok7, ok8 = true; //проверки на число

                childCode = params.at(1).toInt(&ok1);
                conferenceCode = params.at(2).toInt(&ok2);
                tutorCode = params.at(3).toInt(&ok3);
                consultantCode = params.at(4).toInt(&ok4);
                conferenceLocationCode = params.at(5).toInt(&ok5);
                conferenceYearCode = params.at(6).toInt(&ok6);
                achievementViewCode = params.at(8).toInt(&ok7);

                QByteArray b;
                b.append(params.at(7));
                themeName = TEXT(QByteArray::fromBase64(b), v);

                int levelConference = params.at(9).toInt(&ok8);

                if (ok1 && ok2 && ok3 && ok4 && ok5 && ok6 && ok7 && ok8) {
                    //Проверка - ученика ли код указан
                    if (isChild(&db, childCode)) {
                        //Проверка - относится ли ученик к той же школе, что и координатор от школы
                        if (isChildSchool(&db, userCode, childCode)) {

                            //Начало работы
                            if (consultantCode != 0) {
                                if (tutorCode == 0) {
                                    query = QString("INSERT INTO vzletbase.achievement "
                                                    "(userID, childID, achievementViewID, achievementThemeName, conferenceID, conferenceLocationID, tutorID, consultantID, achievementDateTime, conferenceYearID, levelConferenceID) "
                                                    "VALUES "
                                                    "(%1, %2, %3, '%4', %5, %6, NULL, %7, NOW(), %8, %9) ")
                                            .arg(userCode)
                                            .arg(childCode)
                                            .arg(achievementViewCode)
                                            .arg(themeName)
                                            .arg(conferenceCode)
                                            .arg(conferenceLocationCode)
                                            .arg(consultantCode)
                                            .arg(conferenceYearCode)
                                            .arg(levelConference);
                                }
                                else {
                                    query = QString("INSERT INTO vzletbase.achievement "
                                                    "(userID, childID, achievementViewID, achievementThemeName, conferenceID, conferenceLocationID, tutorID, consultantID, achievementDateTime, conferenceYearID, levelConferenceID) "
                                                    "VALUES "
                                                    "(%1, %2, %3, '%4', %5, %6, %7, %8, NOW(), %9, %10) ")
                                            .arg(userCode)
                                            .arg(childCode)
                                            .arg(achievementViewCode)
                                            .arg(themeName)
                                            .arg(conferenceCode)
                                            .arg(conferenceLocationCode)
                                            .arg(tutorCode)
                                            .arg(consultantCode)
                                            .arg(conferenceYearCode)
                                            .arg(levelConference);
                                }
                            }
                            else {
                                if (tutorCode == 0) {
                                    query = QString("INSERT INTO vzletbase.achievement "
                                                    "(userID, childID, achievementViewID, achievementThemeName, conferenceID, conferenceLocationID, tutorID, consultantID, achievementDateTime, conferenceYearID, levelConferenceID) "
                                                    "VALUES "
                                                    "(%1, %2, %3, '%4', %5, %6, NULL, NULL, NOW(), %7, %8) ")
                                            .arg(userCode)
                                            .arg(childCode)
                                            .arg(achievementViewCode)
                                            .arg(themeName)
                                            .arg(conferenceCode)
                                            .arg(conferenceLocationCode)
                                            .arg(conferenceYearCode)
                                            .arg(levelConference);
                                }
                                else {
                                    query = QString("INSERT INTO vzletbase.achievement "
                                                    "(userID, childID, achievementViewID, achievementThemeName, conferenceID, conferenceLocationID, tutorID, consultantID, achievementDateTime, conferenceYearID, levelConferenceID) "
                                                    "VALUES "
                                                    "(%1, %2, %3, '%4', %5, %6, %7, NULL, NOW(), %8, %9) ")
                                            .arg(userCode)
                                            .arg(childCode)
                                            .arg(achievementViewCode)
                                            .arg(themeName)
                                            .arg(conferenceCode)
                                            .arg(conferenceLocationCode)
                                            .arg(tutorCode)
                                            .arg(conferenceYearCode)
                                            .arg(levelConference);
                                }
                            }

                            if (q.exec(query)) {

                                //Добавляем баллы к рейтингу за достижения
                                double levelConferencePoint = 0; //Балл за уровень конференции
                                double achievementViewPoint = 0; //Балл за достигнутый на конференции результат
                                double resultPoint = 0; //Общий (итоговый) балл за достижение

                                //Получаем балл за уровень конференции
                                query = QString("SELECT levelConferencePoint FROM vzletbase.levelConference WHERE levelConferenceID = %1 ").arg(levelConference);

                                if (q.exec(query)) {
                                    while (q.next()) {
                                        levelConferencePoint = q.value(0).toDouble();
                                    }

                                    //Получаем балл за достигнутый на конференции результат
                                    query = QString("SELECT achievementViewPointOut FROM vzletbase.achievementView WHERE achievementViewID = %1 ").arg(achievementViewCode);

                                    if (q.exec(query)) {
                                        while (q.next()) {
                                            achievementViewPoint = q.value(0).toDouble();
                                        }

                                        resultPoint = levelConferencePoint * achievementViewPoint;

                                        //Добавляем балл ученику
                                        query = QString("UPDATE vzletbase.userStatusOrganizationSubdivision "
                                                        "SET childRatingAchievementOut = childRatingAchievementOut + %1 "
                                                        "WHERE userID = %2 ")
                                                .arg(resultPoint)
                                                .arg(childCode);

                                        if (q.exec(query)) {

                                            //Добавляем строчку в портфолио
                                            query = QString("UPDATE vzletbase.userStatusOrganizationSubdivision "
                                                            "SET childPortfolioAchievementOut = childPortfolioAchievementOut + ' +%1 балл(-ов) за участие в конференции с темой (%2) - (%3, %4)' "
                                                            "WHERE userID = %5 ")
                                                    .arg(resultPoint)
                                                    .arg(themeName)
                                                    .arg(QDate::currentDate().toString("dd.MM.yyyy"))
                                                    .arg(QTime::currentTime().toString("hh:mm:ss"))
                                                    .arg(childCode);

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
                            else {
                                db.close();
                                printf("error");
                                a.quit();
                                return (200);
                            }
                        }
                        else {
                            printf(QString("Проверьте правильность указания кода ученика! ").toLocal8Bit());
                            a.quit();
                            return (200);
                        }
                    }
                    else {
                        printf(QString("Проверьте, выбрали ли вы ученика! ").toLocal8Bit());
                        a.quit();
                        return (200);
                    }
                }
                else {
                    printf(QString("Проверьте правильность выбора данных! ").toLocal8Bit());
                    a.quit();
                    return (200);
                }
            }
            else {
                printf("error");
                a.quit();
                return (401);
            }
        }
    }


    a.quit();
    return 200;
}
