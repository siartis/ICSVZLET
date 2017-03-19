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



int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    setmode(fileno(stdout),O_BINARY);
    setmode(fileno(stdin ),O_BINARY);

    QString ps = Input();
    QStringList params = ps.split("&");


    //Вывод на экран типа организации
    if (params.at(0) == "getOrganizationType") {
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

            query = "SELECT organizationTypeID, organizationTypeName FROM vzletbase.organizationType ";

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"organizationTypeCode\":%1,"
                                          "\"organizationTypeName\":\"%2\"},")
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


    //Вывод на экран списка стадий выполнения проекта
    if (params.at(0) == "getStageExecute") {
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

            query = "SELECT stageExecuteID, stageExecuteName "
                    "FROM vzletbase.stageExecute "
                    "ORDER BY stageExecuteName ASC  ";

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"stageExecuteCode\":%1,"
                                          "\"stageExecuteName\":\"%2\"},")
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




    //Вывод на экран списка названий организаций по коду типа организации
    if (params.at(0) == "getOrganizationByOrganizationType") {
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

            query = QString("SELECT ORGANIZATIONID, ORGANIZATIONNAME FROM VZLETBASE.ORGANIZATION WHERE (ORGANIZATIONTYPEID = %1) OR (ORGANIZATIONTYPEID IS NULL) ")
                    .arg(params.at(1));

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





    //Вывод на экран списка тем (фильтр)
    if (params.at(0) == "getIndividualMonitoring") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {

            int userCode = QString("%1").arg(personID).toInt();
            int currentUserStatus = checkUserStatus(&db, userCode); //Статус текущего пользователя

            //Если вошел Администратор или Аналитик
            if (userCode == 3 || currentUserStatus == 14) {

                QSqlQuery q(db);
                QSqlQuery q1(db);
                int themeCode = 0;
                QString consultFIO, consultantAcademicDirection, consultantDegree, consultantOrganization, consultSubdivision; //Данные консультанта
                QString teacherFIO, teacherOrganization, teacherSubdivision; //Данные учителя
                QString childFIO, childOrganization, childSubdivision, childGradeName; //Данные ученика


                QString query = "";
                QString result = "[";

                query = "SELECT THEMEID, THEMENAME, STAGEEXECUTENAME, THEMEDATETIME, "
                        "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_1) AS SCIENCEDIRECTIONNAME1, "
                        "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_2) AS SCIENCEDIRECTIONNAME2, "
                        "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_3) AS SCIENCEDIRECTIONNAME3, "
                        "STATUSTHEMENAME, SECTIONNAME, "
                        "THEME.STAGEEXECUTEID, "
                        "THEME.themeCreativeRatingRemoteReviewer1, "
                        "THEME.themeCreativeRatingRemoteReviewer2, "
                        "THEME.themeCreativeRatingRemoteChild, "
                        "THEME.themeCreativeRatingRemoteTeacher, "
                        "THEME.themeCreativeRatingRemoteConsultant, "
                        "THEME.themeAdmissionStage, "
                        "THEME.themeRatingAdmission "
                        "FROM VZLETBASE.THEME, VZLETBASE.STAGEEXECUTE, VZLETBASE.STATUSTHEME, VZLETBASE.SECTION "
                        "WHERE (THEME.STATUSTHEMEID = STATUSTHEME.STATUSTHEMEID) "
                        "AND (THEME.STAGEEXECUTEID = STAGEEXECUTE.STAGEEXECUTEID) "
                        "AND (THEME.SECTIONID = SECTION.SECTIONID) AND ";


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

                //Секция
                if (params.at(4).toInt() >= 1) {
                    query.append(QString("AND (THEME.SECTIONID = %1) ").arg(params.at(4)));
                }

                bool childGrade = false;

                //Класс
                if (params.at(5).toInt() >= 1) {
                    childGrade = true;
                }

                if (q.exec(query)) {
                    while (q.next()) {

                        consultFIO.clear();
                        consultantAcademicDirection.clear();
                        consultantDegree.clear();
                        consultantOrganization.clear();
                        consultSubdivision.clear();

                        teacherFIO.clear();
                        teacherOrganization.clear();
                        teacherSubdivision.clear();


                        childFIO.clear();
                        childOrganization.clear();
                        childSubdivision.clear();
                        childGradeName.clear();


                        themeCode = q.value(0).toInt();

                        query = QString("SELECT userID, userFIO, degreeName, academicDirectionName, organizationName, subdivisionLevel1Name "
                                        "FROM vzletbase.userStatusOrganizationSubdivision, vzletbase.dov, vzletbase.academicDirection, vzletbase.degree, vzletbase.organization, vzletbase.subdivisionLevel1 "
                                        "WHERE (userStatusOrganizationSubdivision.userID IN (SELECT userID FROM userTheme WHERE themeID = %1)) AND "
                                        "(userStatusOrganizationSubdivision.statusID = 4) AND "
                                        "(userStatusOrganizationSubdivision.userID = dov.dovID) AND "
                                        "(dov.degreeID = degree.degreeID) AND "
                                        "(dov.academicDirectionID = academicDirection.academicDirectionID) AND "
                                        "(userStatusOrganizationSubdivision.organizationID = organization.organizationID) AND "
                                        "(userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID)  ")
                                .arg(themeCode);


                        if (q1.exec(query)) {
                            while (q1.next()) {
                                consultFIO = q1.value(1).toString();
                                consultantDegree = q1.value(2).toString();
                                consultantAcademicDirection = q1.value(3).toString();
                                consultantOrganization = q1.value(4).toString();
                                consultSubdivision = q1.value(5).toString();
                            }


                            //Теперь получаем ФИО учителя и его подразделение (школу)
                            query = QString("SELECT userID, userFIO, organizationName, subdivisionLevel1Name "
                                            "FROM vzletbase.userStatusOrganizationSubdivision, vzletbase.dov, vzletbase.organization, vzletbase.subdivisionLevel1 "
                                            "WHERE (userStatusOrganizationSubdivision.userID IN (SELECT userID FROM userTheme WHERE themeID = %1)) AND "
                                            "(userStatusOrganizationSubdivision.statusID = 3) AND "
                                            "(userStatusOrganizationSubdivision.userID = dov.dovID) AND "
                                            "(userStatusOrganizationSubdivision.organizationID = organization.organizationID) AND "
                                            "(userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID) ")
                                    .arg(themeCode);

                            if (q1.exec(query)) {
                                while (q1.next()) {
                                    teacherFIO = q1.value(1).toString();
                                    teacherOrganization = q1.value(2).toString();
                                    teacherSubdivision = q1.value(3).toString();
                                }

                                if (!childGrade) {
                                    //Теперь получаем ФИО ученика и его подразделение (школу) и его класс
                                    query = QString("SELECT userID, userFIO, organizationName, subdivisionLevel1Name, childGradeName "
                                                    "FROM vzletbase.userStatusOrganizationSubdivision, vzletbase.dov, vzletbase.organization, vzletbase.subdivisionLevel1, vzletbase.childGrade "
                                                    "WHERE (userStatusOrganizationSubdivision.userID IN (SELECT userID FROM userTheme WHERE themeID = %1)) AND "
                                                    "(userStatusOrganizationSubdivision.statusID = 2) AND "
                                                    "(userStatusOrganizationSubdivision.userID = dov.dovID) AND "
                                                    "(userStatusOrganizationSubdivision.organizationID = organization.organizationID) AND "
                                                    "(userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID) AND "
                                                    "(userStatusOrganizationSubdivision.childGradeID = childGrade.childGradeID) ")
                                            .arg(themeCode);

                                }
                                else {
                                    //Теперь получаем ФИО ученика и его подразделение (школу) и его класс
                                    query = QString("SELECT userID, userFIO, organizationName, subdivisionLevel1Name, childGradeName "
                                                    "FROM vzletbase.userStatusOrganizationSubdivision, vzletbase.dov, vzletbase.organization, vzletbase.subdivisionLevel1, vzletbase.childGrade "
                                                    "WHERE (userStatusOrganizationSubdivision.userID IN (SELECT userID FROM userTheme WHERE themeID = %1)) AND "
                                                    "(userStatusOrganizationSubdivision.statusID = 2) AND "
                                                    "(userStatusOrganizationSubdivision.userID = dov.dovID) AND "
                                                    "(userStatusOrganizationSubdivision.organizationID = organization.organizationID) AND "
                                                    "(userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID) AND "
                                                    "(userStatusOrganizationSubdivision.childGradeID = childGrade.childGradeID) AND "
                                                    "(userStatusOrganizationSubdivision.childGradeID = %2) ")
                                            .arg(themeCode)
                                            .arg(params.at(5).toInt());
                                }

                                if (q1.exec(query)) {
                                    while (q1.next()) {
                                        childFIO = q1.value(1).toString();
                                        childOrganization = q1.value(2).toString();
                                        childSubdivision = q1.value(3).toString();

                                        childGradeName = q1.value(4).toString();
                                    }

                                    if (!childGradeName.isEmpty() || !childFIO.isEmpty()) {

                                        result.append("{");
                                        result.append(QString("\"themeCode\":%1,"
                                                              "\"themeName\":\"%2\","
                                                              "\"themeStageExecuteName\":\"%3\","
                                                              "\"themeDateTime\":\"%4\","
                                                              "\"themeScienceDirectionName1\":\"%5\","
                                                              "\"themeScienceDirectionName2\":\"%6\","
                                                              "\"themeScienceDirectionName3\":\"%7\","
                                                              "\"themeStatusThemeName\":\"%8\","
                                                              "\"themeSectionName\":\"%9\","
                                                              "\"themeConsultantFIO\":\"%10\","
                                                              "\"themeConsultantDegree\":\"%11\","
                                                              "\"themeConsultantAcademicDirection\":\"%12\","
                                                              "\"themeConsultantOrganization\":\"%13\","
                                                              "\"themeConsultantSubdivision\":\"%14\","
                                                              "\"themeTeacherFIO\":\"%15\","
                                                              "\"themeTeacherOrganization\":\"%16\","
                                                              "\"themeTeacherSubdivision\":\"%17\","
                                                              "\"themeChildFIO\":\"%18\","
                                                              "\"themeChildOrganization\":\"%19\","
                                                              "\"themeChildSubdivision\":\"%20\","
                                                              "\"themeStageExecuteCode\":%21,"
                                                              "\"themeRatingReviewer1\":%22,"
                                                              "\"themeRatingReviewer2\":%23,"
                                                              "\"themeRatingChild\":%24,"
                                                              "\"themeRatingTeacher\":%25,"
                                                              "\"themeRatingConsultant\":%26,"
                                                              "\"themeChildGradeName\":\"%27\","
                                                              "\"themeAdmissionStage\":%28,"
                                                              "\"themeRatingAdmission\":\%29},")
                                                      .arg(q.value(0).toString())
                                                      .arg(q.value(1).toString())
                                                      .arg(q.value(2).toString())
                                                      .arg(QDate::fromString(q.value(3).toString().mid(0,10), "yyyy-MM-dd").toString("dd.MM.yyyy"))
                                                      .arg(q.value(4).toString())
                                                      .arg(q.value(5).toString())
                                                      .arg(q.value(6).toString())
                                                      .arg(q.value(7).toString())
                                                      .arg(q.value(8).toString())
                                                      .arg(consultFIO)
                                                      .arg(consultantDegree)
                                                      .arg(consultantAcademicDirection)
                                                      .arg(consultantOrganization)
                                                      .arg(consultSubdivision)
                                                      .arg(teacherFIO)
                                                      .arg(teacherOrganization)
                                                      .arg(teacherSubdivision)
                                                      .arg(childFIO)
                                                      .arg(childOrganization)
                                                      .arg(childSubdivision)
                                                      .arg(q.value(9).toString())
                                                      .arg(q.value(10).toInt())
                                                      .arg(q.value(11).toInt())
                                                      .arg(q.value(12).toInt())
                                                      .arg(q.value(13).toInt())
                                                      .arg(q.value(14).toInt())
                                                      .arg(childGradeName)
                                                      .arg(q.value(15).toInt())
                                                      .arg(q.value(16).toInt()));
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


    //Вывод на экран списка тем (фильтр)
    if (params.at(0) == "getIndividualMonitoring1") {
        db = QSqlDatabase::addDatabase("QODBC", "db");
        db.setDatabaseName(QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1").arg(dbName));
        db.open();
        if (db.isOpen()) {

            QSqlQuery q(db);
            QSqlQuery q1(db);
            int themeCode = 0;
            QString consultFIO, consultantAcademicDirection, consultantDegree, consultantOrganization, consultSubdivision; //Данные консультанта
            QString teacherFIO, teacherOrganization, teacherSubdivision; //Данные учителя
            QString childFIO, childOrganization, childSubdivision, childGradeName; //Данные ученика

            int countRecords = 0;
            QString query = "";
            QString result = "[";

            query = "SELECT THEMEID, THEMENAME, STAGEEXECUTENAME, THEMEDATETIME, "
                    "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_1) AS SCIENCEDIRECTIONNAME1, "
                    "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_2) AS SCIENCEDIRECTIONNAME2, "
                    "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_3) AS SCIENCEDIRECTIONNAME3, "
                    "STATUSTHEMENAME, SECTIONNAME, "
                    "THEME.STAGEEXECUTEID, "
                    "THEME.themeCreativeRatingRemoteReviewer1, "
                    "THEME.themeCreativeRatingRemoteReviewer2, "
                    "THEME.themeCreativeRatingRemoteChild, "
                    "THEME.themeCreativeRatingRemoteTeacher, "
                    "THEME.themeCreativeRatingRemoteConsultant, "
                    "THEME.themeAdmissionStage, "
                    "THEME.themeRatingAdmission "
                    "FROM VZLETBASE.THEME, VZLETBASE.STAGEEXECUTE, VZLETBASE.STATUSTHEME, VZLETBASE.SECTION "
                    "WHERE (THEME.STATUSTHEMEID = STATUSTHEME.STATUSTHEMEID) "
                    "AND (THEME.STAGEEXECUTEID = STAGEEXECUTE.STAGEEXECUTEID) "
                    "AND (THEME.SECTIONID = SECTION.SECTIONID) AND ";


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

            //Секция
            if (params.at(4).toInt() >= 1) {
                query.append(QString("AND (THEME.SECTIONID = %1) ").arg(params.at(4)));
            }

            bool childGrade = false;

            //Класс
            if (params.at(5).toInt() >= 1) {
                childGrade = true;
            }

            //Стадия выполнения проекта
            if (params.at(6).toInt() >= 1) {
                query.append(QString("AND (THEME.STAGEEXECUTEID = %1) ").arg(params.at(6)));
            }

            if (q.exec(query)) {
                while (q.next()) {

                    consultFIO.clear();
                    consultantAcademicDirection.clear();
                    consultantDegree.clear();
                    consultantOrganization.clear();
                    consultSubdivision.clear();

                    teacherFIO.clear();
                    teacherOrganization.clear();
                    teacherSubdivision.clear();


                    childFIO.clear();
                    childOrganization.clear();
                    childSubdivision.clear();
                    childGradeName.clear();


                    themeCode = q.value(0).toInt();

                    //Сначала получаем ФИО консультанта с его подразделением (кафедрой)
                    query = QString("SELECT userID, userFIO, degreeName, academicDirectionName, organizationName, subdivisionLevel1Name "
                                    "FROM vzletbase.userStatusOrganizationSubdivision, vzletbase.dov, vzletbase.academicDirection, vzletbase.degree, vzletbase.organization, vzletbase.subdivisionLevel1 "
                                    "WHERE (userStatusOrganizationSubdivision.userID IN (SELECT userID FROM userTheme WHERE themeID = %1)) AND "
                                    "(userStatusOrganizationSubdivision.statusID = 4) AND "
                                    "(userStatusOrganizationSubdivision.userID = dov.dovID) AND "
                                    "(dov.degreeID = degree.degreeID) AND "
                                    "(dov.academicDirectionID = academicDirection.academicDirectionID) AND "
                                    "(userStatusOrganizationSubdivision.organizationID = organization.organizationID) AND "
                                    "(userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID)  ")
                            .arg(themeCode);

                    if (q1.exec(query)) {
                        while (q1.next()) {
                            consultFIO = q1.value(1).toString();
                            consultantDegree = q1.value(2).toString();
                            consultantAcademicDirection = q1.value(3).toString();
                            consultantOrganization = q1.value(4).toString();
                            consultSubdivision = q1.value(5).toString();
                        }


                        //Теперь получаем ФИО учителя и его подразделение (школу)
                        query = QString("SELECT userID, userFIO, organizationName, subdivisionLevel1Name "
                                        "FROM vzletbase.userStatusOrganizationSubdivision, vzletbase.dov, vzletbase.organization, vzletbase.subdivisionLevel1 "
                                        "WHERE (userStatusOrganizationSubdivision.userID IN (SELECT userID FROM userTheme WHERE themeID = %1)) AND "
                                        "(userStatusOrganizationSubdivision.statusID = 3) AND "
                                        "(userStatusOrganizationSubdivision.userID = dov.dovID) AND "
                                        "(userStatusOrganizationSubdivision.organizationID = organization.organizationID) AND "
                                        "(userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID) ")
                                .arg(themeCode);

                        if (q1.exec(query)) {
                            while (q1.next()) {
                                teacherFIO = q1.value(1).toString();
                                teacherOrganization = q1.value(2).toString();
                                teacherSubdivision = q1.value(3).toString();
                            }

                            if (!childGrade) {
                                //Теперь получаем ФИО ученика и его подразделение (школу) и его класс
                                query = QString("SELECT userID, userFIO, organizationName, subdivisionLevel1Name, childGradeName "
                                                "FROM vzletbase.userStatusOrganizationSubdivision, vzletbase.dov, vzletbase.organization, vzletbase.subdivisionLevel1, vzletbase.childGrade "
                                                "WHERE (userStatusOrganizationSubdivision.userID IN (SELECT userID FROM userTheme WHERE themeID = %1)) AND "
                                                "(userStatusOrganizationSubdivision.statusID = 2) AND "
                                                "(userStatusOrganizationSubdivision.userID = dov.dovID) AND "
                                                "(userStatusOrganizationSubdivision.organizationID = organization.organizationID) AND "
                                                "(userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID) AND "
                                                "(userStatusOrganizationSubdivision.childGradeID = childGrade.childGradeID) ")
                                        .arg(themeCode);

                            }
                            else {
                                //Теперь получаем ФИО ученика и его подразделение (школу) и его класс
                                query = QString("SELECT userID, userFIO, organizationName, subdivisionLevel1Name, childGradeName "
                                                "FROM vzletbase.userStatusOrganizationSubdivision, vzletbase.dov, vzletbase.organization, vzletbase.subdivisionLevel1, vzletbase.childGrade "
                                                "WHERE (userStatusOrganizationSubdivision.userID IN (SELECT userID FROM userTheme WHERE themeID = %1)) AND "
                                                "(userStatusOrganizationSubdivision.statusID = 2) AND "
                                                "(userStatusOrganizationSubdivision.userID = dov.dovID) AND "
                                                "(userStatusOrganizationSubdivision.organizationID = organization.organizationID) AND "
                                                "(userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID) AND "
                                                "(userStatusOrganizationSubdivision.childGradeID = childGrade.childGradeID) AND "
                                                "(userStatusOrganizationSubdivision.childGradeID = %2) ")
                                        .arg(themeCode)
                                        .arg(params.at(5).toInt());
                            }

                            if (q1.exec(query)) {

                                countRecords = 0;
                                childFIO.clear();
                                childOrganization.clear();
                                childSubdivision.clear();
                                childGradeName.clear();

                                while (q1.next()) {
                                    childFIO = q1.value(1).toString();
                                    childOrganization = q1.value(2).toString();
                                    childSubdivision = q1.value(3).toString();

                                    childGradeName = q1.value(4).toString();
                                    countRecords++;
                                }

                                if (countRecords > 0) {

                                    result.append("{");
                                    result.append(QString("\"themeCode\":%1,"
                                                          "\"themeName\":\"%2\","
                                                          "\"themeStageExecuteName\":\"%3\","
                                                          "\"themeDateTime\":\"%4\","
                                                          "\"themeScienceDirectionName1\":\"%5\","
                                                          "\"themeScienceDirectionName2\":\"%6\","
                                                          "\"themeScienceDirectionName3\":\"%7\","
                                                          "\"themeStatusThemeName\":\"%8\","
                                                          "\"themeSectionName\":\"%9\","
                                                          "\"themeConsultantFIO\":\"%10\","
                                                          "\"themeConsultantDegree\":\"%11\","
                                                          "\"themeConsultantAcademicDirection\":\"%12\","
                                                          "\"themeConsultantOrganization\":\"%13\","
                                                          "\"themeConsultantSubdivision\":\"%14\","
                                                          "\"themeTeacherFIO\":\"%15\","
                                                          "\"themeTeacherOrganization\":\"%16\","
                                                          "\"themeTeacherSubdivision\":\"%17\","
                                                          "\"themeChildFIO\":\"%18\","
                                                          "\"themeChildOrganization\":\"%19\","
                                                          "\"themeChildSubdivision\":\"%20\","
                                                          "\"themeStageExecuteCode\":%21,"
                                                          "\"themeRatingReviewer1\":%22,"
                                                          "\"themeRatingReviewer2\":%23,"
                                                          "\"themeRatingChild\":%24,"
                                                          "\"themeRatingTeacher\":%25,"
                                                          "\"themeRatingConsultant\":%26,"
                                                          "\"themeChildGradeName\":\"%27\","
                                                          "\"themeAdmissionStage\":%28,"
                                                          "\"themeRatingAdmission\":\%29},")
                                                  .arg(q.value(0).toString())
                                                  .arg(q.value(1).toString())
                                                  .arg(q.value(2).toString())
                                                  .arg(QDate::fromString(q.value(3).toString().mid(0,10), "yyyy-MM-dd").toString("dd.MM.yyyy"))
                                                  .arg(q.value(4).toString())
                                                  .arg(q.value(5).toString())
                                                  .arg(q.value(6).toString())
                                                  .arg(q.value(7).toString())
                                                  .arg(q.value(8).toString())
                                                  .arg(consultFIO)
                                                  .arg(consultantDegree)
                                                  .arg(consultantAcademicDirection)
                                                  .arg(consultantOrganization)
                                                  .arg(consultSubdivision)
                                                  .arg(teacherFIO)
                                                  .arg(teacherOrganization)
                                                  .arg(teacherSubdivision)
                                                  .arg(childFIO)
                                                  .arg(childOrganization)
                                                  .arg(childSubdivision)
                                                  .arg(q.value(9).toString())
                                                  .arg(q.value(10).toInt())
                                                  .arg(q.value(11).toInt())
                                                  .arg(q.value(12).toInt())
                                                  .arg(q.value(13).toInt())
                                                  .arg(q.value(14).toInt())
                                                  .arg(childGradeName)
                                                  .arg(q.value(15).toInt())
                                                  .arg(q.value(16).toInt()));
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
    if (params.at(0) == "getMonitoringLevelInteraction") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {
            int userCode = QString("%1").arg(personID).toInt();
            int currentUserStatus = checkUserStatus(&db, userCode); //Статус текущего пользователя

            //Если вошел Аналитик или Научный руководитель Программы или Администратор
            if (currentUserStatus == 14 || currentUserStatus == 15 || userCode == 3) {
                QSqlQuery q(db);
                QSqlQuery q1(db);
                QString query = "";
                QString result = "[";

                query = "SELECT THEMEID, THEMENAME, STAGEEXECUTENAME, THEMEDATETIME, "
                        "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_1) AS SCIENCEDIRECTIONNAME1, "
                        "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_2) AS SCIENCEDIRECTIONNAME2, "
                        "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_3) AS SCIENCEDIRECTIONNAME3, "
                        "STATUSTHEMENAME, SECTIONNAME "
                        "FROM VZLETBASE.THEME, VZLETBASE.STAGEEXECUTE, VZLETBASE.STATUSTHEME, VZLETBASE.SECTION "
                        "WHERE (THEME.STATUSTHEMEID = STATUSTHEME.STATUSTHEMEID) AND (THEME.STAGEEXECUTEID = STAGEEXECUTE.STAGEEXECUTEID) "
                        "AND (THEME.SECTIONID = SECTION.SECTIONID) AND ";


                //Организации
                if (params.at(2).toInt() < 1) {
                    query.append("THEMEID IN (SELECT THEMEID FROM VZLETBASE.USERTHEME WHERE USERID IN (SELECT USERID FROM VZLETBASE.USERSTATUSORGANIZATIONSUBDIVISION)) ");
                }
                else {
                    query.append(QString("THEMEID IN (SELECT THEMEID FROM VZLETBASE.USERTHEME WHERE USERID IN (SELECT USERID FROM USERSTATUSORGANIZATIONSUBDIVISION WHERE ORGANIZATIONID = %1)) ").arg(params.at(2)));
                }

                //направления науки
                if (params.at(1).toInt() >= 1) {
                    query.append(QString("AND ((SCIENCEDIRECTIONID_1 = %1) OR (SCIENCEDIRECTIONID_2 = %1) OR (SCIENCEDIRECTIONID_3 = %1)) ").arg(params.at(1)));
                }

                //Статус темы
                if (params.at(3).toInt() >= 1) {
                    query.append(QString("AND (THEME.STATUSTHEMEID = %1) ").arg(params.at(3)));
                }

                //Секция
                if (params.at(4).toInt() >= 1) {
                    query.append(QString("AND (THEME.SECTIONID = %1) ").arg(params.at(4)));
                }

                if (q.exec(query)) {
                    while (q.next()) {
                        result.append("{");
                        result.append(QString("\"themeCode\":%1,"
                                              "\"themeName\":\"%2\","
                                              "\"themeStageExecuteName\":\"%3\","
                                              "\"themeDateTime\":\"%4\","
                                              "\"themeScienceDirectionName1\":\"%5\","
                                              "\"themeScienceDirectionName2\":\"%6\","
                                              "\"themeScienceDirectionName3\":\"%7\","
                                              "\"themeStatusThemeName\":\"%8\","
                                              "\"themeSectionName\":\"%9\",")
                                      .arg(q.value(0).toString())
                                      .arg(q.value(1).toString())
                                      .arg(q.value(2).toString())
                                      .arg(q.value(3).toString())
                                      .arg(q.value(4).toString())
                                      .arg(q.value(5).toString())
                                      .arg(q.value(6).toString())
                                      .arg(q.value(7).toString())
                                      .arg(q.value(8).toString()));

                        QDate currentDate = QDate::currentDate(); //Текущая серверная дата

                        query = QString("SELECT ConsultantTutor.levelInteractionName1, ConsultantChild.levelInteractionName2, "
                                        "TutorConsultant.levelinteractionName3, TutorChild.levelInteractionName4, "
                                        "ChildTutor.levelinteractionName5, ChildConsultant.levelInteractionName6, "

                                        "ConsultantTutor.levelInteractionDateTime1, ConsultantChild.levelInteractionDateTime2,"
                                        "TutorConsultant.levelInteractionDateTime3, TutorChild.levelInteractionDateTime4, "
                                        "ChildTutor.levelInteractionDateTime5, ChildConsultant.levelInteractionDateTime6 "

                                        "FROM "
                                        "("
                                        "SELECT LAST(InteractionID), LAST(levelInteractionName) AS levelInteractionName1,  LAST(interactionDateTime) AS levelInteractionDateTime1 "
                                        "FROM Interaction, levelInteraction "
                                        "WHERE (userIDWho IN (SELECT userID from vzletbase.userStatusOrganizationSubdivision WHERE statusID = 4)) AND "
                                        "(userIDWhom IN (SELECT userID from vzletbase.userStatusOrganizationSubdivision WHERE statusID = 3)) AND "
                                        "(Interaction.levelInteractionID = levelInteraction.levelInteractionID) AND "
                                        "(themeID = %1)"
                                        ") AS ConsultantTutor, "
                                        "("
                                        "SELECT LAST(InteractionID), LAST(levelInteractionName) AS levelInteractionName2,  LAST(interactionDateTime) AS levelInteractionDateTime2 "
                                        "FROM Interaction, levelInteraction "
                                        "WHERE (userIDWho IN (SELECT userID from vzletbase.userStatusOrganizationSubdivision WHERE statusID = 4)) AND "
                                        "(userIDWhom IN (SELECT userID from vzletbase.userStatusOrganizationSubdivision WHERE statusID = 2)) AND "
                                        "(Interaction.levelInteractionID = levelInteraction.levelInteractionID) AND "
                                        "(themeID = %1)"
                                        ") AS ConsultantChild, "
                                        "("
                                        "SELECT LAST(InteractionID), LAST(levelInteractionName) AS levelInteractionName3,  LAST(interactionDateTime) AS levelInteractionDateTime3 "
                                        "FROM Interaction, levelInteraction "
                                        "WHERE (userIDWho IN (SELECT userID from vzletbase.userStatusOrganizationSubdivision WHERE statusID = 3)) AND "
                                        "(userIDWhom IN (SELECT userID from vzletbase.userStatusOrganizationSubdivision WHERE statusID = 4)) AND "
                                        "(Interaction.levelInteractionID = levelInteraction.levelInteractionID) AND "
                                        "(themeID = %1)"
                                        ") AS TutorConsultant, "
                                        "("
                                        "SELECT LAST(InteractionID), LAST(levelInteractionName) AS levelInteractionName4,  LAST(interactionDateTime) AS levelInteractionDateTime4 "
                                        "FROM Interaction, levelInteraction "
                                        "WHERE (userIDWho IN (SELECT userID from vzletbase.userStatusOrganizationSubdivision WHERE statusID = 3)) AND "
                                        "(userIDWhom IN (SELECT userID from vzletbase.userStatusOrganizationSubdivision WHERE statusID = 2)) AND "
                                        "(Interaction.levelInteractionID = levelInteraction.levelInteractionID) AND "
                                        "(themeID = %1)"
                                        ") AS TutorChild, "
                                        "("
                                        "SELECT LAST(InteractionID), LAST(levelInteractionName) AS levelInteractionName5,  LAST(interactionDateTime) AS levelInteractionDateTime5 "
                                        "FROM Interaction, levelInteraction "
                                        "WHERE (userIDWho IN (SELECT userID from vzletbase.userStatusOrganizationSubdivision WHERE statusID = 2)) AND "
                                        "(userIDWhom IN (SELECT userID from vzletbase.userStatusOrganizationSubdivision WHERE statusID = 3)) AND "
                                        "(Interaction.levelInteractionID = levelInteraction.levelInteractionID) AND "
                                        "(themeID = %1)"
                                        ") AS ChildTutor, "
                                        "("
                                        "SELECT LAST(InteractionID), LAST(levelInteractionName) AS levelInteractionName6,  LAST(interactionDateTime) AS levelInteractionDateTime6 "
                                        "FROM Interaction, levelInteraction  "
                                        "WHERE (userIDWho IN (SELECT userID from vzletbase.userStatusOrganizationSubdivision WHERE statusID = 2)) AND "
                                        "(userIDWhom IN (SELECT userID from vzletbase.userStatusOrganizationSubdivision WHERE statusID = 4)) AND "
                                        "(Interaction.levelInteractionID = levelInteraction.levelInteractionID) AND "
                                        "(themeID = %1)"
                                        ") AS ChildConsultant ")
                                .arg(q.value(0).toString());

                        if (q1.exec(query)) {
                            while (q1.next()) {
                                result.append(QString("\"ConsultantTutorAssName\":\"%1\","
                                                      "\"ConsultantChildAssName\":\"%2\","
                                                      "\"TutorConsultantAssName\":\"%3\","
                                                      "\"TutorChildAssName\":\"%4\","
                                                      "\"ChildTutorAssName\":\"%5\","
                                                      "\"ChildConsultantAssName\":\"%6\","
                                                      "\"countDaysConsultantTutor\":%7,"
                                                      "\"countDaysConsultantChild\":%8,"
                                                      "\"countDaysTutorConsultant\":%9,"
                                                      "\"countDaysTutorChild\":%10,"
                                                      "\"countDaysChildTutor\":%11,"
                                                      "\"countDaysChildConsultant\":%12},")
                                              .arg(q1.value(0).toString())
                                              .arg(q1.value(1).toString())
                                              .arg(q1.value(2).toString())
                                              .arg(q1.value(3).toString())
                                              .arg(q1.value(4).toString())
                                              .arg(q1.value(5).toString())
                                              .arg(currentDate.daysTo(QDate::fromString(q1.value(6).toString().mid(0, 10), "yyyy-MM-dd"))*(-1))
                                              .arg(currentDate.daysTo(QDate::fromString(q1.value(7).toString().mid(0, 10), "yyyy-MM-dd"))*(-1))
                                              .arg(currentDate.daysTo(QDate::fromString(q1.value(8).toString().mid(0, 10), "yyyy-MM-dd"))*(-1))
                                              .arg(currentDate.daysTo(QDate::fromString(q1.value(9).toString().mid(0, 10), "yyyy-MM-dd"))*(-1))
                                              .arg(currentDate.daysTo(QDate::fromString(q1.value(10).toString().mid(0, 10), "yyyy-MM-dd"))*(-1))
                                              .arg(currentDate.daysTo(QDate::fromString(q1.value(11).toString().mid(0, 10), "yyyy-MM-dd"))*(-1)));
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
                }
            }
            else {
                a.quit();
                return (401);
            }
        }
        else {
            printf("[error]");
        }
    }



    //Статистика - общая статистика
    if (params.at(0) == "getGeneralStatistics") {
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


            query = "SELECT organizationName, offered, executed, archives "
                    "FROM "
                    "vzletbase.Organization, vzletbase.GeneralStatistic "
                    "WHERE "
                    "(GeneralStatistic.organizationID = Organization.organizationID) ";

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"organizationName\":\"%1\","
                                          "\"offered\":%2,"
                                          "\"executed\":%3,"
                                          "\"archives\":%4},")
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
            printf("[error]");
            a.quit();
            return (200);
        }
    }





    //Статистика - общая статистика
    if (params.at(0) == "getQWERTY") {
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
            QStringList organizationCode;
            QStringList organizationName;

            query = "SELECT organizationID, organizationName FROM vzletbase.Organization WHERE organizationID > 1";
            organizationCode.clear();
            organizationName.clear();

            if (q.exec(query)) {
                while (q.next()) {
                    organizationCode.append(q.value(0).toString());
                    organizationName.append(q.value(1).toString());
                }

                for(int i = 0; i < organizationCode.length(); i++) {
                    query = QString("SELECT userCount, consultantCount, tutorCount, childCount "
                                    "FROM "
                                    "( "
                                    "SELECT COUNT(userID) AS userCount "
                                    "FROM "
                                    "vzletbase.UserStatusOrganizationSubdivision "
                                    "WHERE "
                                    "organizationID = %1 "
                                    ") AS P1, "
                                    "( "
                                    "SELECT COUNT(userID) AS consultantCount "
                                    "FROM "
                                    "vzletbase.UserStatusOrganizationSubdivision "
                                    "WHERE "
                                    "(organizationID = %1) AND (statusID = 4) "
                                    ") AS Consultant, "
                                    "( "
                                    "SELECT COUNT(userID) AS tutorCount "
                                    "FROM "
                                    "vzletbase.UserStatusOrganizationSubdivision "
                                    "WHERE "
                                    "(organizationID = %1) AND (statusID = 3) "
                                    ") AS Tutor,  "
                                    "( "
                                    "SELECT COUNT(userID) AS childCount "
                                    "FROM "
                                    "vzletbase.UserStatusOrganizationSubdivision "
                                    "WHERE "
                                    "(organizationID = %1) AND (statusID = 2) "
                                    ") AS Child ")
                            .arg(organizationCode.at(i));
                    if (q.exec(query)) {
                        while (q.next()) {
                            result.append("{");
                            result.append(QString("\"organizationName\":\"%1\","
                                                  "\"generalCount\":%2,"
                                                  "\"consultantCount\":%3,"
                                                  "\"tutorCount\":%4,"
                                                  "\"childCount\":%5},")
                                          .arg(organizationName.at(i))
                                          .arg(q.value(0).toString())
                                          .arg(q.value(1).toString())
                                          .arg(q.value(2).toString())
                                          .arg(q.value(3).toString()));
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




    //Статистика - общая статистика (по организациям) - Развернутая статистика по университетам, образовательным округам и крупным городам
    if (params.at(0) == "getMonitoringGeneralStatisticsByUniversity") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {

            int userCode = QString("%1").arg(personID).toInt();

            //Если вошел аналитик или Администратор или координатор от вуза/школы
            if (checkUserStatus(&db, userCode) == 14 || userCode == 3 || userCode == 5 || userCode == 6) {
                QSqlQuery q(db);
                QString query = "";
                QString result = "[";
                QStringList organizationCode; //Код организации
                QStringList organizationName; //Название организации
                QStringList organizationNumberPPS; //Численность ППС, НС, учителей

                organizationCode.clear();
                organizationName.clear();
                organizationNumberPPS.clear();

                query = "SELECT organizationID, organizationName, organizationNumberPPS "
                        "FROM "
                        "vzletbase.organization "
                        "WHERE "
                        "(organizationTypeID = 3) AND (universityObserver = FALSE) "
                        "ORDER BY organizationName ASC ";

                if (q.exec(query)) {
                    while (q.next()) {
                        organizationCode.append(q.value(0).toString());
                        organizationName.append(q.value(1).toString());
                        organizationNumberPPS.append(q.value(2).toString());
                    }

                    for(int i = 0; i < organizationCode.length(); i++) {

                        result.append("{");

                        //Сразу запишем на вывод код и название оранизации
                        result.append(QString("\"organizationCode\":%1,").arg(organizationCode.at(i)));
                        result.append(QString("\"organizationName\":\"%1\",").arg(organizationName.at(i)));
                        result.append(QString("\"organizationNumberPPS\":\"%1\",").arg(organizationNumberPPS.at(i)));

                        //Вывод общего количество участников, количества консультантов, учителей и учеников по конкретной организации
                        query = QString("SELECT P1.countAll, P2.countConsultants, P3.countTutors, P4.countChilds "
                                        "FROM "
                                        "( "
                                        "SELECT count(userID) AS countAll FROM vzletbase.userStatusOrganizationSubdivision WHERE organizationID = %1 "
                                        ") AS P1, "
                                        "( "
                                        "SELECT count(userID) AS countConsultants FROM vzltebase.userStatusOrganizationSubdivision WHERE (organizationID = %1) AND (statusID = 4) "
                                        ") AS P2, "
                                        "( "
                                        "SELECT count(userID) AS countTutors FROM vzletbase.userStatusOrganizationSubdivision WHERE (organizationID = %1) AND (statusID = 3) "
                                        ") AS P3, "
                                        "( "
                                        "SELECT count(userID) AS countChilds FROM vzletbase.userStatusOrganizationSubdivision WHERE (organizationID = %1) AND (statusID = 2) "
                                        ") AS P4 ")
                                .arg(organizationCode.at(i));

                        if (q.exec(query)) {
                            while (q.next()) {
                                result.append(QString("\"countAll\":%1,"
                                                      "\"countConsultants\":%2,"
                                                      "\"countTutors\":%3,"
                                                      "\"countChilds\":%4,")
                                              .arg(q.value(0).toString())
                                              .arg(q.value(1).toString())
                                              .arg(q.value(2).toString())
                                              .arg(q.value(3).toString()));
                            }


                            //Теперь по организации получаем количество тем
                            query = QString("SELECT Count(P.themeID) FROM "
                                            "(SELECT DISTINCT themeID "
                                            "FROM vzletbase.userTheme "
                                            "WHERE userID IN "
                                            "( "
                                            "SELECT userID "
                                            "FROM vzletbase.userStatusOrganizationSubdivision "
                                            "WHERE (organizationID = %1) AND "
                                            "( "
                                            "(statusID = 4) OR (statusID = 3) OR (statusID = 2) "
                                            ") "
                                            ")) AS P ")
                                    .arg(organizationCode.at(i));
                            if (q.exec(query)) {
                                while (q.next()) {
                                    result.append(QString("\"countTheme\":%1,").arg(q.value(0).toString()));
                                }


                                //Теперь получаем количество тем, у которых разные этапы выполнения работы
                                for(int j = 1; j <= 20; j++) {
                                    if (j != 13) {
                                        query = QString("SELECT Count(stageExecuteID) AS countStageExecute "
                                                        "FROM vzletbase.theme "
                                                        "WHERE stageExecuteID = %1 AND themeID IN "
                                                        "( "
                                                        "SELECT DISTINCT themeID "
                                                        "FROM vzletbase.userTheme "
                                                        "WHERE userID IN "
                                                        "( "
                                                        "SELECT userID "
                                                        "FROM vzletbase.userStatusOrganizationSubdivision "
                                                        "WHERE (organizationID = %2) AND "
                                                        "( "
                                                        "(statusID = 4) OR (statusID = 3) OR (statusID = 2) "
                                                        ") "
                                                        ") "
                                                        ") ")
                                                .arg(j)
                                                .arg(organizationCode.at(i));

                                        if (q.exec(query)) {
                                            q.next();
                                            result.append(QString("\"countStageExecute%1\":%2,").arg(j).arg(q.value(0).toInt()));
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
                        else {
                            printf("[error]");
                            a.quit();
                            return (200);
                        }

                        result.chop(1);
                        result.append("},");
                    }

                    result.chop(1);
                    result.append("]");
                    printf(result.toLocal8Bit());
                    a.quit();
                    return (200);
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
        else {
            printf("[error]");
            a.quit();
            return (200);
        }
    }









    //Статистика - общая статистика (по организациям) - Развернутая статистика по территориальным управлениям
    if (params.at(0) == "getMonitoringGeneralStatisticsByEducationalDistrict") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {

            int userCode = QString("%1").arg(personID).toInt();

            //Если вошел аналитик или Администратор или координатор от вуза/школы
            if (checkUserStatus(&db, userCode) == 14 || userCode == 3 || checkUserStatus(&db, userCode) == 5) {
                QSqlQuery q(db);
                QString query = "";
                QString result = "[";
                QStringList organizationCode; //Код организации
                QStringList organizationName; //Название организации
                QStringList organizationNumberPPS; //Численность ППС, НС, учителей

                organizationCode.clear();
                organizationName.clear();
                organizationNumberPPS.clear();

                query = "SELECT organizationID, organizationName, organizationNumberPPS "
                        "FROM "
                        "vzletbase.organization "
                        "WHERE "
                        "organizationTypeID = 2 "
                        "ORDER BY organizationName ASC ";

                if (q.exec(query)) {
                    while (q.next()) {
                        organizationCode.append(q.value(0).toString());
                        organizationName.append(q.value(1).toString());
                        organizationNumberPPS.append(q.value(2).toString());
                    }

                    for(int i = 0; i < organizationCode.length(); i++) {

                        result.append("{");

                        //Сразу запишем на вывод код и название оранизации
                        result.append(QString("\"organizationCode\":%1,").arg(organizationCode.at(i)));
                        result.append(QString("\"organizationName\":\"%1\",").arg(organizationName.at(i)));
                        result.append(QString("\"organizationNumberPPS\":\"%1\",").arg(organizationNumberPPS.at(i)));

                        //Вывод общего количество участников, количества консультантов, учителей и учеников по конкретной организации
                        query = QString("SELECT P1.countAll, P2.countConsultants, P3.countTutors, P4.countChilds "
                                        "FROM "
                                        "( "
                                        "SELECT count(userID) AS countAll FROM vzletbase.userStatusOrganizationSubdivision WHERE organizationID = %1 "
                                        ") AS P1, "
                                        "( "
                                        "SELECT count(userID) AS countConsultants FROM vzletbase.userStatusOrganizationSubdivision WHERE (organizationID = %1) AND (statusID = 4) "
                                        ") AS P2, "
                                        "( "
                                        "SELECT count(userID) AS countTutors FROM vzletbase.userStatusOrganizationSubdivision WHERE (organizationID = %1) AND (statusID = 3) "
                                        ") AS P3, "
                                        "( "
                                        "SELECT count(userID) AS countChilds FROM vzletbase.userStatusOrganizationSubdivision WHERE (organizationID = %1) AND (statusID = 2) "
                                        ") AS P4 ")
                                .arg(organizationCode.at(i));

                        if (q.exec(query)) {
                            while (q.next()) {
                                result.append(QString("\"countAll\":%1,"
                                                      "\"countConsultants\":%2,"
                                                      "\"countTutors\":%3,"
                                                      "\"countChilds\":%4,")
                                              .arg(q.value(0).toString())
                                              .arg(q.value(1).toString())
                                              .arg(q.value(2).toString())
                                              .arg(q.value(3).toString()));
                            }


                            //Теперь по организации получаем количество тем
                            query = QString("SELECT Count(P.themeID) FROM "
                                            "(SELECT DISTINCT themeID "
                                            "FROM vzletbase.userTheme "
                                            "WHERE userID IN "
                                            "( "
                                            "SELECT userID "
                                            "FROM vzletbase.userStatusOrganizationSubdivision "
                                            "WHERE (organizationID = %1) AND "
                                            "( "
                                            "(statusID = 4) OR (statusID = 3) OR (statusID = 2) "
                                            ") "
                                            ")) AS P ")
                                    .arg(organizationCode.at(i));
                            if (q.exec(query)) {
                                while (q.next()) {
                                    result.append(QString("\"countTheme\":%1,").arg(q.value(0).toString()));
                                }


                                //Теперь получаем количество тем, у которых разные этапы выполнения работы
                                for(int j = 1; j <= 22; j++) {
                                    if (j != 13 || j!= 21) {
                                        query = QString("SELECT Count(themeID) AS countThemeExecute "
                                                        "FROM vzletbase.theme "
                                                        "WHERE (stageExecuteID = %1) AND (statusThemeID = 2) AND themeID IN "
                                                        "( "
                                                        "SELECT DISTINCT themeID "
                                                        "FROM vzletbase.userTheme "
                                                        "WHERE userID IN "
                                                        "( "
                                                        "SELECT userID "
                                                        "FROM vzletbase.userStatusOrganizationSubdivision "
                                                        "WHERE (organizationID = %2) AND "
                                                        "( "
                                                        "(statusID = 3) "
                                                        ") "
                                                        ") "
                                                        ") ")
                                                .arg(j)
                                                .arg(organizationCode.at(i));

                                        if (q.exec(query)) {
                                            while (q.next()) {
                                                result.append(QString("\"countStageExecute%1\":%2,").arg(j).arg(q.value(0).toInt()));
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
                        else {
                            printf("[error]");
                            a.quit();
                            return (200);
                        }

                        result.chop(1);
                        result.append("},");
                    }

                    result.chop(1);
                    result.append("]");
                    printf(result.toLocal8Bit());
                    a.quit();
                    return (200);
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
        else {
            printf("[error]");
            a.quit();
            return (200);
        }
    }




    //Статистика - Развернутая статистика по школам
    if (params.at(0) == "getMonitoringGeneralStatisticsBySchool") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {

            int userCode = QString("%1").arg(personID).toInt();
            int currentUserStatus = checkUserStatus(&db, userCode); //Статус текущего пользователя

            //Если вошел аналитик или Администратор или координатор от кафедры/школы
            if (currentUserStatus == 14 || userCode == 3 || currentUserStatus == 6) {

                QSqlQuery q(db);
                QString query = "";
                QString result = "[";
                QStringList subdivisionCode; //Код подразделения
                QStringList subdivisionName; //Название подразделения
                int areaCode = params.at(1).toInt(); //Код федерального района области
                int currentStageID = params.at(2).toInt(); //Код выбранной пользователем стадии выполнения проекта

                subdivisionCode.clear();
                subdivisionName.clear();

                query = QString("SELECT subdivisionLevel1ID, subdivisionLevel1Name, subdivisionLevel1NumberPPS FROM vzletbase.subdivisionLevel1 "
                                "WHERE (organizationID = %1) AND (subdivisionLevel1ID <> 1) "
                                "ORDER BY subdivisionLevel1Name ASC ")
                        .arg(areaCode);

                if (q.exec(query)) {
                    while (q.next()) {
                        subdivisionCode.append(q.value(0).toString());
                        subdivisionName.append(q.value(1).toString());
                    }


                    for(int i = 0; i < subdivisionCode.length(); i++) {

                        result.append("{");

                        //Сразу запишем на вывод код и название оранизации
                        result.append(QString("\"subdivisionCode\":%1,").arg(subdivisionCode.at(i)));
                        result.append(QString("\"subdivisionName\":\"%1\",").arg(subdivisionName.at(i)));

                        query = QString("SELECT Count(stageExecuteID) AS countStageExecute "
                                        "FROM vzletbase.theme "
                                        "WHERE stageExecuteID = %1 AND themeID IN "
                                        "( "
                                        "SELECT DISTINCT themeID "
                                        "FROM vzletbase.userTheme "
                                        "WHERE userID IN "
                                        "( "
                                        "SELECT userID "
                                        "FROM vzletbase.userStatusOrganizationSubdivision "
                                        "WHERE (subdivisionLevel1ID = %2) AND (organizationID = %3) AND "
                                        "( "
                                        "(statusID = 4) OR (statusID = 3) OR (statusID = 2) "
                                        ") "
                                        ") "
                                        ") ")
                                .arg(currentStageID)
                                .arg(subdivisionCode.at(i))
                                .arg(areaCode);

                        if (q.exec(query)) {
                            while (q.next()) {
                                result.append(QString("\"countStageExecute\":%1,").arg(q.value(0).toInt()));
                            }
                        }
                        else {
                            printf("[error]");
                            a.quit();
                            return (200);
                        }

                        result.chop(1);
                        result.append("},");
                    }

                    result.chop(1);
                    result.append("]");
                    printf(result.toLocal8Bit());
                    a.quit();
                    return (200);
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
        else {
            printf("[error]");
            a.quit();
            return (200);
        }
    }






    //Статистика - Развернутая статистика по школам
    if (params.at(0) == "getMonitoringGeneralStatisticsBySchool1") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {

            int userCode = QString("%1").arg(personID).toInt();
            int currentUserStatus = checkUserStatus(&db, userCode); //Статус текущего пользователя

            //Если вошел аналитик или Администратор или координатор от кафедры/школы
            if (currentUserStatus == 14 || userCode == 3 || currentUserStatus == 6) {

                QSqlQuery q(db);
                QString query = "";
                QString result = "[";
                QStringList subdivisionCode; //Код подразделения
                QStringList subdivisionName; //Название подразделения
                int areaCode = params.at(1).toInt(); //Код федерального района области
                int currentStageID = params.at(2).toInt(); //Код выбранной пользователем стадии выполнения проекта

                subdivisionCode.clear();
                subdivisionName.clear();

                query = QString("SELECT subdivisionLevel1ID, subdivisionLevel1Name FROM vzletbase.subdivisionLevel1 "
                                "WHERE (organizationID = %1) AND (subdivisionLevel1ID <> 1) "
                                "ORDER BY subdivisionLevel1Name ASC ")
                        .arg(areaCode);

                if (q.exec(query)) {
                    while (q.next()) {
                        subdivisionCode.append(q.value(0).toString());
                        subdivisionName.append(q.value(1).toString());
                    }

                    for(int i = 0; i < subdivisionCode.count(); i++) {

                        result.append("{");

                        //Сразу запишем на вывод код и название оранизации
                        result.append(QString("\"subdivisionCode\":%1,").arg(subdivisionCode.at(i)));
                        result.append(QString("\"subdivisionName\":\"%1\",").arg(subdivisionName.at(i)));

                        query = QString("SELECT Count(stageExecuteID) AS countStageExecute "
                                        "FROM vzletbase.theme "
                                        "WHERE stageExecuteID = %1 AND themeID IN "
                                        "( "
                                        "SELECT DISTINCT themeID "
                                        "FROM vzletbase.userTheme "
                                        "WHERE userID IN "
                                        "( "
                                        "SELECT userID "
                                        "FROM vzletbase.userStatusOrganizationSubdivision "
                                        "WHERE (subdivisionLevel1ID = %2) AND (organizationID = %3) AND "
                                        "( "
                                        "(statusID = 4) OR (statusID = 3) OR (statusID = 2) "
                                        ") "
                                        ") "
                                        ") ")
                                .arg(currentStageID)
                                .arg(subdivisionCode.at(i).toInt())
                                .arg(areaCode);

                        if (q.exec(query)) {
                            while (q.next()) {
                                result.append(QString("\"countStageExecute\":%1},").arg(q.value(0).toInt()));
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
                    printf(result.toLocal8Bit());
                    a.quit();
                    return (200);
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
        else {
            printf("[error]");
            a.quit();
            return (200);
        }
    }




    //Статистика - Развернутая статистика по школам
    if (params.at(0) == "getMonitoringGeneralStatisticsBySubdivision") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {

            int userCode = QString("%1").arg(personID).toInt();
            int currentUserStatus = checkUserStatus(&db, userCode); //Статус текущего пользователя

            //Если вошел аналитик (или Администратор)
            if ((currentUserStatus == 5) || (currentUserStatus == 14) || (currentUserStatus == 6) || (userCode == 3)) {
                QSqlQuery q(db);
                QString query = "";
                QString result = "[";
                QStringList subdivisionCode; //Код подразделения
                QStringList subdivisionName; //Название подразделения
                QStringList subdivisionNumberPPS; //Численность ППС, НС, учителей
                int universityCode = params.at(1).toInt(); //Код университета

                subdivisionCode.clear();
                subdivisionName.clear();
                subdivisionNumberPPS.clear();

                query = QString("SELECT subdivisionLevel1ID, subdivisionLevel1Name, subdivisionLevel1NumberPPS FROM vzletbase.subdivisionLevel1 "
                                "WHERE (organizationID = %1) AND (subdivisionLevel1ID <> 1) "
                                "ORDER BY subdivisionLevel1Name ASC ")
                        .arg(universityCode);

                if (q.exec(query)) {
                    while (q.next()) {
                        subdivisionCode.append(q.value(0).toString());
                        subdivisionName.append(q.value(1).toString());
                        subdivisionNumberPPS.append(q.value(2).toString());
                    }


                    for(int i = 0; i < subdivisionCode.length(); i++) {

                        result.append("{");

                        //Сразу запишем на вывод код и название оранизации
                        result.append(QString("\"subdivisionCode\":%1,").arg(subdivisionCode.at(i)));
                        result.append(QString("\"subdivisionName\":\"%1\",").arg(subdivisionName.at(i)));
                        result.append(QString("\"subdivisionNumberPPS\":\"%1\",").arg(subdivisionNumberPPS.at(i)));

                        //Вывод общего количество участников, количества консультантов, учителей и учеников по конкретной организации
                        query = QString("SELECT P1.countAll, P2.countConsultants, P3.countTutors, P4.countChilds "
                                        "FROM "
                                        "( "
                                        "SELECT count(userID) AS countAll FROM vzletbase.userStatusOrganizationSubdivision WHERE (subdivisionLevel1ID = %1) "
                                        ") AS P1, "
                                        "( "
                                        "SELECT count(userID) AS countConsultants FROM vzletbase.userStatusOrganizationSubdivision WHERE (subdivisionLevel1ID = %1) AND (organizationID = %2) AND (statusID = 4) "
                                        ") AS P2, "
                                        "( "
                                        "SELECT count(userID) AS countTutors FROM vzletbase.userStatusOrganizationSubdivision WHERE (subdivisionLevel1ID = %1) AND (organizationID = %2) AND (statusID = 3) "
                                        ") AS P3, "
                                        "( "
                                        "SELECT count(userID) AS countChilds FROM vzletbase.userStatusOrganizationSubdivision WHERE (subdivisionLevel1ID = %1) AND (organizationID = %2) AND (statusID = 2) "
                                        ") AS P4 ")
                                .arg(subdivisionCode.at(i))
                                .arg(universityCode);

                        if (q.exec(query)) {
                            while (q.next()) {
                                result.append(QString("\"countAll\":%1,"
                                                      "\"countConsultants\":%2,"
                                                      "\"countTutors\":%3,"
                                                      "\"countChilds\":%4,")
                                              .arg(q.value(0).toString())
                                              .arg(q.value(1).toString())
                                              .arg(q.value(2).toString())
                                              .arg(q.value(3).toString()));
                            }


                            //Теперь по организации получаем количество тем
                            query = QString("SELECT Count(P.themeID) FROM "
                                            "(SELECT DISTINCT themeID "
                                            "FROM vzletbase.userTheme "
                                            "WHERE userID IN "
                                            "( "
                                            "SELECT userID "
                                            "FROM vzletbase.userStatusOrganizationSubdivision "
                                            "WHERE (subdivisionLevel1ID = %1) AND (organizationID = %2) AND "
                                            "( "
                                            "(statusID = 4) OR (statusID = 3) OR (statusID = 2) "
                                            ") "
                                            ")) AS P ")
                                    .arg(subdivisionCode.at(i))
                                    .arg(universityCode);

                            if (q.exec(query)) {
                                while (q.next()) {
                                    result.append(QString("\"countTheme\":%1,").arg(q.value(0).toString()));
                                }


                                //Теперь получаем количество тем, у которых разные этапы выполнения работы
                                for(int j = 1; j <= 20; j++) {
                                    if (j != 13) {
                                        query = QString("SELECT Count(stageExecuteID) AS countStageExecute "
                                                        "FROM vzletbase.theme "
                                                        "WHERE stageExecuteID = %1 AND themeID IN "
                                                        "( "
                                                        "SELECT DISTINCT themeID "
                                                        "FROM vzletbase.userTheme "
                                                        "WHERE userID IN "
                                                        "( "
                                                        "SELECT userID "
                                                        "FROM vzletbase.userStatusOrganizationSubdivision "
                                                        "WHERE (subdivisionLevel1ID = %2) AND (organizationID = %3) AND "
                                                        "( "
                                                        "(statusID = 4) OR (statusID = 3) OR (statusID = 2) "
                                                        ") "
                                                        ") "
                                                        ") ")
                                                .arg(j)
                                                .arg(subdivisionCode.at(i))
                                                .arg(universityCode);

                                        if (q.exec(query)) {
                                            q.next();
                                            result.append(QString("\"countStageExecute%1\":%2,").arg(j).arg(q.value(0).toInt()));
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
                        else {
                            printf("[error]");
                            a.quit();
                            return (200);
                        }

                        result.chop(1);
                        result.append("},");
                    }

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






    //Статистика - общая статистика (только по по университетам)
    if (params.at(0) == "getOverallStatisticByUniversity") {
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
            QStringList organizationCode; //Код организации
            QStringList organizationName; //Название организации
            QStringList organizationNumberPPS; //численность ППС, НС, учителей

            organizationCode.clear();
            organizationName.clear();
            organizationNumberPPS.clear();

            query = "SELECT organizationID, organizationName, organizationNumberPPS "
                    "FROM "
                    "vzletbase.organization "
                    "WHERE "
                    "(organizationID <> 1) AND (organizationTypeID = 3) AND (universityObserver = FALSE) "
                    "ORDER BY organizationName ASC ";

            if (q.exec(query)) {
                while (q.next()) {
                    organizationCode.append(q.value(0).toString());
                    organizationName.append(q.value(1).toString());
                    organizationNumberPPS.append(q.value(2).toString());
                }

                for(int i = 0; i < organizationCode.length(); i++) {

                    result.append("{");

                    //Сразу запишем на вывод код и название оранизации
                    result.append(QString("\"organizationCode\":%1,").arg(organizationCode.at(i)));
                    result.append(QString("\"organizationName\":\"%1\",").arg(organizationName.at(i)));
                    result.append(QString("\"organizationNumberPPS\":\"%1\",").arg(organizationNumberPPS.at(i)));

                    //Вывод общего количество участников, количества консультантов, учителей и учеников по конкретной организации
                    query = QString("SELECT P1.countAll, P2.countConsultants, P3.countTutors, P4.countChilds "
                                    "FROM "
                                    "( "
                                    "SELECT count(userID) AS countAll FROM vzletbase.userStatusOrganizationSubdivision WHERE organizationID = %1 "
                                    ") AS P1, "
                                    "( "
                                    "SELECT count(userID) AS countConsultants FROM vzletbase.userStatusOrganizationSubdivision WHERE (organizationID = %1) AND (statusID = 4) "
                                    ") AS P2, "
                                    "( "
                                    "SELECT count(userID) AS countTutors FROM vzletbase.userStatusOrganizationSubdivision WHERE (organizationID = %1) AND (statusID = 3) "
                                    ") AS P3, "
                                    "( "
                                    "SELECT count(userID) AS countChilds FROM vzletbase.userStatusOrganizationSubdivision WHERE (organizationID = %1) AND (statusID = 2) "
                                    ") AS P4 ")
                            .arg(organizationCode.at(i));

                    if (q.exec(query)) {
                        while (q.next()) {
                            result.append(QString("\"countAll\":%1,"
                                                  "\"countConsultants\":%2,"
                                                  "\"countTutors\":%3,"
                                                  "\"countChilds\":%4,")
                                          .arg(q.value(0).toString())
                                          .arg(q.value(1).toString())
                                          .arg(q.value(2).toString())
                                          .arg(q.value(3).toString()));
                        }


                        //Теперь по организации получаем количество тем
                        query = QString("SELECT Count(P.themeID) FROM "
                                        "(SELECT DISTINCT themeID "
                                        "FROM vzletbase.userTheme "
                                        "WHERE userID IN "
                                        "( "
                                        "SELECT userID "
                                        "FROM vzletbase.userStatusOrganizationSubdivision "
                                        "WHERE (organizationID = %1) AND "
                                        "( "
                                        "(statusID = 4) OR (statusID = 3) OR (statusID = 2) "
                                        ") "
                                        ")) AS P ")
                                .arg(organizationCode.at(i));
                        if (q.exec(query)) {
                            while (q.next()) {
                                result.append(QString("\"countTheme\":%1,").arg(q.value(0).toString()));
                            }


                            //Теперь получаем количество тем, у которых разные этапы выполнения работы
                            for(int j = 2; j <= 20; j++) {
                                if ((j != 13) || (j > 15)) {
                                    query = QString("SELECT Count(stageExecuteID) AS countStageExecute "
                                                    "FROM vzletbase.theme "
                                                    "WHERE stageExecuteID = %1 AND themeID IN "
                                                    "( "
                                                    "SELECT DISTINCT themeID "
                                                    "FROM vzletbase.userTheme "
                                                    "WHERE userID IN "
                                                    "( "
                                                    "SELECT userID "
                                                    "FROM vzletbase.userStatusOrganizationSubdivision "
                                                    "WHERE (organizationID = %2) AND "
                                                    "( "
                                                    "(statusID = 4) OR (statusID = 3) OR (statusID = 2) "
                                                    ") "
                                                    ") "
                                                    ") ")
                                            .arg(j)
                                            .arg(organizationCode.at(i));

                                    if (q.exec(query)) {
                                        q.next();
                                        result.append(QString("\"countStageExecute%1\":%2,").arg(j).arg(q.value(0).toInt()));
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
                    else {
                        printf("[error]");
                        a.quit();
                        return (200);
                    }

                    result.chop(1);
                    result.append("},");
                }

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
        else {
            printf("[error]");
            a.quit();
            return (200);
        }
    }






    //Статистика - общая статистика (только по по университетам)
    if (params.at(0) == "getOverallStatisticByEdDistrict") {
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
            QStringList organizationCode; //Код организации
            QStringList organizationName; //Название организации
            QStringList organizationNumberPPS; //численность ППС, НС, учителей

            organizationCode.clear();
            organizationName.clear();
            organizationNumberPPS.clear();

            query = "SELECT organizationID, organizationName, organizationNumberPPS "
                    "FROM "
                    "vzletbase.organization "
                    "WHERE "
                    "(organizationID <> 1) AND (organizationTypeID = 2) "
                    "ORDER BY organizationName ASC ";

            if (q.exec(query)) {
                while (q.next()) {
                    organizationCode.append(q.value(0).toString());
                    organizationName.append(q.value(1).toString());
                    organizationNumberPPS.append(q.value(2).toString());
                }

                for(int i = 0; i < organizationCode.length(); i++) {

                    result.append("{");

                    //Сразу запишем на вывод код и название оранизации
                    result.append(QString("\"organizationCode\":%1,").arg(organizationCode.at(i)));
                    result.append(QString("\"organizationName\":\"%1\",").arg(organizationName.at(i)));
                    result.append(QString("\"organizationNumberPPS\":\"%1\",").arg(organizationNumberPPS.at(i)));

                    //Вывод общего количество участников, количества консультантов, учителей и учеников по конкретной организации
                    query = QString("SELECT P1.countAll, P2.countConsultants, P3.countTutors, P4.countChilds "
                                    "FROM "
                                    "( "
                                    "SELECT count(userID) AS countAll FROM vzletbase.userStatusOrganizationSubdivision WHERE organizationID = %1 "
                                    ") AS P1, "
                                    "( "
                                    "SELECT count(userID) AS countConsultants FROM vzletbase.userStatusOrganizationSubdivision WHERE (organizationID = %1) AND (statusID = 4) "
                                    ") AS P2, "
                                    "( "
                                    "SELECT count(userID) AS countTutors FROM vzletbase.userStatusOrganizationSubdivision WHERE (organizationID = %1) AND (statusID = 3) "
                                    ") AS P3, "
                                    "( "
                                    "SELECT count(userID) AS countChilds FROM vzletbase.userStatusOrganizationSubdivision WHERE (organizationID = %1) AND (statusID = 2) "
                                    ") AS P4 ")
                            .arg(organizationCode.at(i));

                    if (q.exec(query)) {
                        while (q.next()) {
                            result.append(QString("\"countAll\":%1,"
                                                  "\"countConsultants\":%2,"
                                                  "\"countTutors\":%3,"
                                                  "\"countChilds\":%4,")
                                          .arg(q.value(0).toString())
                                          .arg(q.value(1).toString())
                                          .arg(q.value(2).toString())
                                          .arg(q.value(3).toString()));
                        }


                        //Теперь по организации получаем количество тем
                        query = QString("SELECT Count(P.themeID) FROM "
                                        "(SELECT DISTINCT themeID "
                                        "FROM vzletbase.userTheme "
                                        "WHERE userID IN "
                                        "( "
                                        "SELECT userID "
                                        "FROM vzletbase.userStatusOrganizationSubdivision "
                                        "WHERE (organizationID = %1) AND "
                                        "( "
                                        "(statusID = 4) OR (statusID = 3) OR (statusID = 2) "
                                        ") "
                                        ")) AS P ")
                                .arg(organizationCode.at(i));
                        if (q.exec(query)) {
                            while (q.next()) {
                                result.append(QString("\"countTheme\":%1,").arg(q.value(0).toString()));
                            }


                            //Теперь получаем количество тем, у которых разные этапы выполнения работы
                            for(int j = 2; j <= 20; j++) {
                                if ((j != 13) || (j > 15)) {
                                    query = QString("SELECT Count(stageExecuteID) AS countStageExecute "
                                                    "FROM vzletbase.theme "
                                                    "WHERE stageExecuteID = %1 AND themeID IN "
                                                    "( "
                                                    "SELECT DISTINCT themeID "
                                                    "FROM vzletbase.userTheme "
                                                    "WHERE userID IN "
                                                    "( "
                                                    "SELECT userID "
                                                    "FROM vzletbase.userStatusOrganizationSubdivision "
                                                    "WHERE (organizationID = %2) AND "
                                                    "( "
                                                    "(statusID = 4) OR (statusID = 3) OR (statusID = 2) "
                                                    ") "
                                                    ") "
                                                    ") ")
                                            .arg(j)
                                            .arg(organizationCode.at(i));

                                    if (q.exec(query)) {
                                        q.next();
                                        result.append(QString("\"countStageExecute%1\":%2,").arg(j).arg(q.value(0).toInt()));
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
                    else {
                        printf("[error]");
                        a.quit();
                        return (200);
                    }

                    result.chop(1);
                    result.append("},");
                }

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
        else {
            printf("[error]");
            a.quit();
            return (200);
        }
    }






    //Список университетов (организаций)
    if (params.at(0) == "getUniversities") {
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

            query = "SELECT organizationID, organizationName FROM vzletbase.organization "
                    "WHERE (organizationID <> 1) AND (organizationTypeID = 3) AND (universityObserver = FALSE) ";
            if(q.exec(query)) {
                while (q.next()) {
                    result.append(QString("{\"universityCode\":%1,"
                                          "\"universityName\":\"%2\"},")
                                  .arg(q.value(0).toString())
                                  .arg(q.value(1).toString()));
                }

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




    //Список образовательных округов (организаций)
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

            query = "SELECT organizationID, organizationName FROM vzletbase.organization "
                    "WHERE organizationID <> 1 AND organizationTypeID = 2 ";
            if(q.exec(query)) {
                while (q.next()) {
                    result.append(QString("{\"educationalDistrictCode\":%1,"
                                          "\"educationalDistrictName\":\"%2\"},")
                                  .arg(q.value(0).toString())
                                  .arg(q.value(1).toString()));
                }

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



    //Список Федеральных районов области
    if (params.at(0) == "getAreas") {
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

            query = "SELECT organizationID, organizationName FROM vzletbase.organization "
                    "WHERE organizationID <> 1 AND organizationTypeID = 2 ";
            if(q.exec(query)) {
                while (q.next()) {
                    result.append(QString("{\"areaCode\":%1,"
                                          "\"areaName\":\"%2\"},")
                                  .arg(q.value(0).toString())
                                  .arg(q.value(1).toString()));
                }

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






    //Статистика - Общая статистика по кафедрам университетов
    if (params.at(0) == "getOverallStatisticByUniversity") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {

            int userCode = QString("%1").arg(personID).toInt();
            int currentUserStatus = checkUserStatus(&db, userCode); //Статус текущего пользователя

            if ((currentUserStatus == 5) || (currentUserStatus == 14) || (currentUserStatus == 6)) {
                QSqlQuery q(db);
                QString query = "";
                QString result = "[";
                QStringList subdivisionCode; //Код подразделения
                QStringList subdivisionName; //Название подразделения
                QStringList subdivisionNumberPPS; //численность ППС, НС, учителей
                int organizationCode = params.at(1).toInt(); //Код организации

                subdivisionCode.clear();
                subdivisionName.clear();
                subdivisionNumberPPS.clear();

                query = QString("SELECT subdivisionLevel1ID, subdivisionLevel1Name, subdivisionLevel1NumberPPS FROM vzletbase.subdivisionLevel1 "
                                "WHERE organizationID = %1 "
                                "ORDER BY subdivisionLevel1Name ASC ")
                        .arg(organizationCode);

                if (q.exec(query)) {
                    while (q.next()) {
                        subdivisionCode.append(q.value(0).toString());
                        subdivisionName.append(q.value(1).toString());
                        subdivisionNumberPPS.append(q.value(2).toString());
                    }

                    for(int i = 0; i < subdivisionCode.length(); i++) {

                        result.append("{");

                        //Сразу запишем на вывод код и название оранизации
                        result.append(QString("\"subdivisionCode\":%1,").arg(subdivisionCode.at(i)));
                        result.append(QString("\"subdivisionName\":\"%1\",").arg(subdivisionName.at(i)));
                        result.append(QString("\"subdivisionNumberPPS\":\"%1\",").arg(subdivisionNumberPPS.at(i)));

                        //Вывод общего количество участников, количества консультантов, учителей и учеников по конкретной организации
                        query = QString("SELECT P1.countAll, P2.countConsultants, P3.countTutors, P4.countChilds "
                                        "FROM "
                                        "( "
                                        "SELECT count(userID) AS countAll FROM vzltebase.userStatusOrganizationSubdivision WHERE (organizationID = %1) AND (subdivisionLevel1ID = %2) "
                                        ") AS P1, "
                                        "( "
                                        "SELECT count(userID) AS countConsultants FROM vzletbase.userStatusOrganizationSubdivision WHERE (organizationID = %1) AND (subdivisionLevel1ID = %2) AND (statusID = 4) "
                                        ") AS P2, "
                                        "( "
                                        "SELECT count(userID) AS countTutors FROM vzletbase.userStatusOrganizationSubdivision WHERE (organizationID = %1) AND (subdivisionLevel1ID = %2) AND (statusID = 3) "
                                        ") AS P3, "
                                        "( "
                                        "SELECT count(userID) AS countChilds FROM vzletbase.userStatusOrganizationSubdivision WHERE (organizationID = %1) AND (subdivisionLevel1ID = %2) AND (statusID = 2) "
                                        ") AS P4 ")
                                .arg(organizationCode)
                                .arg(subdivisionCode.at(i));

                        if (q.exec(query)) {
                            while (q.next()) {
                                result.append(QString("\"countAll\":%1,"
                                                      "\"countConsultants\":%2,"
                                                      "\"countTutors\":%3,"
                                                      "\"countChilds\":%4,")
                                              .arg(q.value(0).toString())
                                              .arg(q.value(1).toString())
                                              .arg(q.value(2).toString())
                                              .arg(q.value(3).toString()));
                            }


                            //Теперь по организации получаем количество тем
                            query = QString("SELECT Count(P.themeID) FROM "
                                            "(SELECT DISTINCT themeID "
                                            "FROM vzletbase.userTheme "
                                            "WHERE userID IN "
                                            "( "
                                            "SELECT userID "
                                            "FROM vzletbase.userStatusOrganizationSubdivision "
                                            "WHERE (organizationID = %1) AND (subdivisionLevel1ID = %2) AND "
                                            "( "
                                            "(statusID = 4) OR (statusID = 3) OR (statusID = 2) "
                                            ") "
                                            ")) AS P ")
                                    .arg(organizationCode)
                                    .arg(subdivisionCode.at(i));

                            if (q.exec(query)) {
                                while (q.next()) {
                                    result.append(QString("\"countTheme\":%1,").arg(q.value(0).toString()));
                                }


                                //Теперь получаем количество тем, у которых разные этапы выполнения работы
                                for(int j = 2; j <= 20; j++) {
                                    if ((j != 13) || (j > 15)) {
                                        query = QString("SELECT Count(stageExecuteID) AS countStageExecute "
                                                        "FROM vzletbase.theme "
                                                        "WHERE stageExecuteID = %1 AND themeID IN "
                                                        "( "
                                                        "SELECT DISTINCT themeID "
                                                        "FROM userTheme "
                                                        "WHERE userID IN "
                                                        "( "
                                                        "SELECT userID "
                                                        "FROM userStatusOrganizationSubdivision "
                                                        "WHERE (organizationID = %2) AND (subdivisionLevel1ID = %3) AND "
                                                        "( "
                                                        "(statusID = 4) OR (statusID = 3) OR (statusID = 2) "
                                                        ") "
                                                        ") "
                                                        ") ")
                                                .arg(j)
                                                .arg(organizationCode)
                                                .arg(subdivisionCode.at(i));

                                        if (q.exec(query)) {
                                            q.next();
                                            result.append(QString("\"countStageExecute%1\":%2,").arg(j).arg(q.value(0).toInt()));
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
                        else {
                            printf("[error]");
                            a.quit();
                            return (200);
                        }

                        result.chop(1);
                        result.append("},");
                    }

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






    //Статистика - Общая статистика по кафедрам университетов
    if (params.at(0) == "getOverallStatisticBySchool") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {

            int userCode = QString("%1").arg(personID).toInt();
            int currentUserStatus = checkUserStatus(&db, userCode); //Статус текущего пользователя

            if (currentUserStatus == 5 || currentUserStatus == 14 || userCode == 3) {
                QSqlQuery q(db);
                QString query = "";
                QString result = "[";
                QStringList subdivisionCode; //Код подразделения
                QStringList subdivisionName; //Название подразделения
                QStringList subdivisionNumberPPS; //численность ППС, НС, учителей
                int organizationCode = params.at(1).toInt(); //Код организации

                subdivisionCode.clear();
                subdivisionName.clear();
                subdivisionNumberPPS.clear();

                query = QString("SELECT subdivisionLevel1ID, subdivisionLevel1Name, subdivisionLevel1NumberPPS FROM vzletbase.subdivisionLevel1 "
                                "WHERE organizationID = %1 "
                                "ORDER BY subdivisionLevel1Name ASC ")
                        .arg(organizationCode);

                if (q.exec(query)) {
                    while (q.next()) {
                        subdivisionCode.append(q.value(0).toString());
                        subdivisionName.append(q.value(1).toString());
                        subdivisionNumberPPS.append(q.value(2).toString());
                    }

                    for(int i = 0; i < subdivisionCode.length(); i++) {

                        result.append("{");

                        //Сразу запишем на вывод код и название оранизации
                        result.append(QString("\"subdivisionCode\":%1,").arg(subdivisionCode.at(i)));
                        result.append(QString("\"subdivisionName\":\"%1\",").arg(subdivisionName.at(i)));
                        result.append(QString("\"subdivisionNumberPPS\":\"%1\",").arg(subdivisionNumberPPS.at(i)));

                        //Вывод общего количество участников, количества консультантов, учителей и учеников по конкретной организации
                        query = QString("SELECT P1.countAll, P2.countConsultants, P3.countTutors, P4.countChilds "
                                        "FROM "
                                        "( "
                                        "SELECT count(userID) AS countAll FROM vzletbase.userStatusOrganizationSubdivision WHERE (organizationID = %1) AND (subdivisionLevel1ID = %2) "
                                        ") AS P1, "
                                        "( "
                                        "SELECT count(userID) AS countConsultants FROM vzletbase.userStatusOrganizationSubdivision WHERE (organizationID = %1) AND (subdivisionLevel1ID = %2) AND (statusID = 4) "
                                        ") AS P2, "
                                        "( "
                                        "SELECT count(userID) AS countTutors FROM vzletbase.userStatusOrganizationSubdivision WHERE (organizationID = %1) AND (subdivisionLevel1ID = %2) AND (statusID = 3) "
                                        ") AS P3, "
                                        "( "
                                        "SELECT count(userID) AS countChilds FROM vzletbase.userStatusOrganizationSubdivision WHERE (organizationID = %1) AND (subdivisionLevel1ID = %2) AND (statusID = 2) "
                                        ") AS P4 ")
                                .arg(organizationCode)
                                .arg(subdivisionCode.at(i));

                        if (q.exec(query)) {
                            while (q.next()) {
                                result.append(QString("\"countAll\":%1,"
                                                      "\"countConsultants\":%2,"
                                                      "\"countTutors\":%3,"
                                                      "\"countChilds\":%4,")
                                              .arg(q.value(0).toString())
                                              .arg(q.value(1).toString())
                                              .arg(q.value(2).toString())
                                              .arg(q.value(3).toString()));
                            }


                            //Теперь по организации получаем количество тем
                            query = QString("SELECT Count(P.themeID) FROM "
                                            "(SELECT DISTINCT themeID "
                                            "FROM vzletbase.userTheme "
                                            "WHERE userID IN "
                                            "( "
                                            "SELECT userID "
                                            "FROM vzletbase.userStatusOrganizationSubdivision "
                                            "WHERE (organizationID = %1) AND (subdivisionLevel1ID = %2) AND "
                                            "( "
                                            "(statusID = 4) OR (statusID = 3) OR (statusID = 2) "
                                            ") "
                                            ")) AS P ")
                                    .arg(organizationCode)
                                    .arg(subdivisionCode.at(i));

                            if (q.exec(query)) {
                                while (q.next()) {
                                    result.append(QString("\"countTheme\":%1,").arg(q.value(0).toString()));
                                }


                                //Теперь получаем количество тем, у которых разные этапы выполнения работы
                                for(int j = 2; j <= 20; j++) {
                                    if ((j != 13) || (j > 15)) {
                                        query = QString("SELECT Count(stageExecuteID) AS countStageExecute "
                                                        "FROM vzletbase.theme "
                                                        "WHERE stageExecuteID = %1 AND themeID IN "
                                                        "( "
                                                        "SELECT DISTINCT themeID "
                                                        "FROM vzletbase.userTheme "
                                                        "WHERE userID IN "
                                                        "( "
                                                        "SELECT userID "
                                                        "FROM vzletbase.userStatusOrganizationSubdivision "
                                                        "WHERE (organizationID = %2) AND (subdivisionLevel1ID = %3) AND "
                                                        "( "
                                                        "(statusID = 4) OR (statusID = 3) OR (statusID = 2) "
                                                        ") "
                                                        ") "
                                                        ") ")
                                                .arg(j)
                                                .arg(organizationCode)
                                                .arg(subdivisionCode.at(i));

                                        if (q.exec(query)) {
                                            q.next();
                                            result.append(QString("\"countStageExecute%1\":%2,").arg(j).arg(q.value(0).toInt()));
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
                        else {
                            printf("[error]");
                            a.quit();
                            return (200);
                        }

                        result.chop(1);
                        result.append("},");
                    }

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









    //Статистика - общая статистика (по подразделениям)
    if (params.at(0) == "getOverallStatisticsBySubdivision") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {

            int userCode = QString("%1").arg(personID).toInt();
            int currentUserStatus = checkUserStatus(&db, userCode); //Статус текущего пользователя

            //Если вошел аналитик или Администратор или координатор по кафедрам/школам
            if (currentUserStatus == 14 || userCode == 3 || userCode == 6) {
                QSqlQuery q(db);
                QString query = "";
                QString result = "[";
                QStringList subdivisionCode; //Код организации
                QStringList subdivisionName; //Название организации

                subdivisionCode.clear();
                subdivisionName.clear();

                query = QString("SELECT subdivisionLevel1ID, subdivisionLevel1Name "
                                "FROM vzletbase.subdivisionLevel1  "
                                "WHERE "
                                "organizationID = %1 ")
                        .arg(params.at(1));

                if (q.exec(query)) {
                    while (q.next()) {
                        subdivisionCode.append(q.value(0).toString());
                        subdivisionName.append(q.value(1).toString());
                    }


                    for(int i = 0; i < subdivisionCode.length(); i++) {

                        result.append("{");

                        //Сразу запишем на вывод код и название оранизации
                        result.append(QString("\"subdivisionCode\":%1,").arg(subdivisionCode.at(i)));
                        result.append(QString("\"subdivisionName\":\"%1\",").arg(subdivisionName.at(i)));

                        //Вывод общего количество участников, количества консультантов, учителей и учеников по конкретной организации
                        query = QString("SELECT P1.countAll, P2.countConsultants, P3.countTutors, P4.countChilds "
                                        "FROM "
                                        "( "
                                        "SELECT count(userID) AS countAll FROM vzletbase.userStatusOrganizationSubdivision WHERE subdivisionLevel1ID = %1 "
                                        ") AS P1, "
                                        "( "
                                        "SELECT count(userID) AS countConsultants FROM vzletbase.userStatusOrganizationSubdivision WHERE (subdivisionLevel1ID = %1) AND (statusID = 4) "
                                        ") AS P2, "
                                        "( "
                                        "SELECT count(userID) AS countTutors FROM vzletbase.userStatusOrganizationSubdivision WHERE (subdivisionLevel1ID = %1) AND (statusID = 3) "
                                        ") AS P3, "
                                        "( "
                                        "SELECT count(userID) AS countChilds FROM vzletbase.userStatusOrganizationSubdivision WHERE (subdivisionLevel1ID = %1) AND (statusID = 2) "
                                        ") AS P4 ")
                                .arg(subdivisionCode.at(i));

                        if (q.exec(query)) {
                            while (q.next()) {
                                result.append(QString("\"countAll\":%1,"
                                                      "\"countConsultants\":%2,"
                                                      "\"countTutors\":%3,"
                                                      "\"countChilds\":%4,")
                                              .arg(q.value(0).toString())
                                              .arg(q.value(1).toString())
                                              .arg(q.value(2).toString())
                                              .arg(q.value(3).toString()));
                            }


                            //Теперь по организации получаем количество тем
                            query = QString("SELECT Count(P.themeID) FROM "
                                            "(SELECT DISTINCT themeID "
                                            "FROM vzletbase.userTheme "
                                            "WHERE userID IN "
                                            "( "
                                            "SELECT userID "
                                            "FROM vzletbase.userStatusOrganizationSubdivision "
                                            "WHERE (subdivisionLevel1ID = %1) AND "
                                            "( "
                                            "(statusID = 4) OR (statusID = 3) OR (statusID = 2) "
                                            ") "
                                            ")) AS P ")
                                    .arg(subdivisionCode.at(i));
                            if (q.exec(query)) {
                                while (q.next()) {
                                    result.append(QString("\"countTheme\":%1,").arg(q.value(0).toString()));
                                }


                                //Теперь получаем количество тем, у которых разные этапы выполнения работы
                                for(int j = 1; j <= 20; j++) {
                                    if (j != 13) {
                                        query = QString("SELECT Count(stageExecuteID) AS countStageExecute "
                                                        "FROM vzletbase.theme "
                                                        "WHERE stageExecuteID = %1 AND themeID IN "
                                                        "( "
                                                        "SELECT DISTINCT themeID "
                                                        "FROM vzletbase.userTheme "
                                                        "WHERE userID IN "
                                                        "( "
                                                        "SELECT userID "
                                                        "FROM vzletbase.userStatusOrganizationSubdivision "
                                                        "WHERE (subdivisionLevel1ID = %2) AND "
                                                        "( "
                                                        "(statusID = 4) OR (statusID = 3) OR (statusID = 2) "
                                                        ") "
                                                        ") "
                                                        ") ")
                                                .arg(j)
                                                .arg(subdivisionCode.at(i));

                                        if (q.exec(query)) {
                                            q.next();
                                            result.append(QString("\"countStageExecute%1\":%2,").arg(j).arg(q.value(0).toInt()));
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
                        else {
                            printf("[error]");
                            a.quit();
                            return (200);
                        }

                        result.chop(1);
                        result.append("},");
                    }

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


    //Вывод на экран текущей даты
    if (params.at(0) == "getServerCurrentDate") {
        printf(QString("{\"currentDate\":\"%1\"}").arg(QDate::currentDate().toString("dd.MM.yyyy")).toLocal8Bit());
        a.quit();
        return (200);
    }



    //Вывод на экран списка участников Программы
    if (params.at(0) == "getUsers") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {

            //Доступ только для Администратора
            if (QString("%1").arg(personID).toInt() != 3) {
                a.quit();
                return (401);
            }


            QSqlQuery q(db);
            QString query = "";
            QString result = "[";

            int statusCode = params.at(1).toInt();
            int organizationCode = params.at(2).toInt();
            int showAllUsers = params.at(3).toInt(); //Показать всех учеников или привязанных к теме?

            if (statusCode != 0) {
                //Статус пользователя

                //Если ученик
                if (statusCode == 2) {

                    //Если показать всех учеников
                    if (showAllUsers == 1) {

                        query = QString("SELECT userID, userFIO, statusName, organizationName, subdivisionLevel1Name,  childRating, childGradeName, childRatingStage, childRatingAchievementOut, userMobilePhone, userEMail, childMemberRegistry "
                                        "FROM vzletbase.userStatusOrganizationSubdivision, vzletbase.dov, vzletbase.status, vzletbase.organization, vzletbase.subdivisionLevel1, vzletbase.childGrade  "
                                        "WHERE "
                                        "(userStatusOrganizationSubdivision.userID = dov.dovID) AND "
                                        "(userStatusOrganizationSubdivision.statusID = status.statusID) AND "
                                        "(userStatusOrganizationSubdivision.organizationID = organization.organizationID) AND "
                                        "(userStatusOrganizationSubdivision.childGradeID = childGrade.childGradeID) AND "
                                        "(userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID) "
                                        "AND (userStatusOrganizationSubdivision.statusID = %1) $$$organizationFilter$$$ ")
                                .arg(statusCode);

                        //Фильтр по организации
                        if (organizationCode != 0) {
                            //Организация пользователя
                            query.replace("$$$organizationFilter$$$", QString("AND (userStatusOrganizationSubdivision.organizationID = %1) ").arg(organizationCode));
                        }
                        else {
                            query.replace("$$$organizationFilter$$$", "");
                        }


                        //Сортировка по ФИО (по возрастанию)
                        query.append(" ORDER BY userFIO ASC ");

                        if (q.exec(query)) {
                            while (q.next()) {

                                double childRatingAchievement = floor(q.value(8).toDouble()*100)/100;

                                //Вывод количества тем, в которых участвует ученик
                                QSqlQuery q1(db);
                                query.clear();
                                query = QString("SELECT COUNT(themeID) "
                                                "FROM vzletbase.userTheme "
                                                "WHERE userID = %1 ")
                                        .arg(q.value(0).toString());

                                if (q1.exec(query)) {

                                    int countThemes = 0;
                                    while(q1.next()) {
                                        countThemes = q1.value(0).toInt();
                                    }

                                    result.append(QString("{\"userCode\":%1,"
                                                          "\"userFIO\":\"%2\","
                                                          "\"userStatus\":\"%3\","
                                                          "\"userOrganization\":\"%4\","
                                                          "\"userSubdivision\":\"%5\","
                                                          "\"userRating\":\"%6\","
                                                          "\"childGrade\":\"%7\","
                                                          "\"childRatingStage\":\"%8\","
                                                          "\"childRatingAchievement\":\"%9\","
                                                          "\"childCountThemes\":%10,"
                                                          "\"childMobilePhone\":\"%11\","
                                                          "\"childEMail\":\"%12\","
                                                          "\"childMemberRegistry\":%13},")
                                                  .arg(q.value(0).toString())
                                                  .arg(q.value(1).toString())
                                                  .arg(q.value(2).toString())
                                                  .arg(q.value(3).toString())
                                                  .arg(q.value(4).toString())
                                                  .arg(q.value(5).toString())
                                                  .arg(q.value(6).toString())
                                                  .arg(q.value(7).toString())
                                                  .arg(childRatingAchievement)
                                                  .arg(countThemes)
                                                  .arg(q.value(9).toString())
                                                  .arg(q.value(10).toString())
                                                  .arg(q.value(11).toInt()));

                                }
                                else {
                                    printf("[error]");
                                    a.quit();
                                    return (200);
                                }
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
                    //Показать учеников с темами
                    if (showAllUsers == 0) {
                        query = QString("SELECT userStatusOrganizationSubdivision.userID, userFIO, academicDirectionName, degreeName, statusName, organizationName, subdivisionLevel1Name, postName, childRating, childGradeName, count(userThemeID), childRatingStage, childRatingAchievementOut "
                                        "FROM vzletbase.userStatusOrganizationSubdivision, vzletbase.dov, vzletbase.academicDirection, vzletbase.degree, vzletbase.status, vzletbase.organization, vzletbase.subdivisionLevel1, vzletbase.childGrade, vzletbase.userTheme "
                                        "WHERE "
                                        "(dov.academicDirectionID = academicDirection.academicDirectionID) AND "
                                        "(dov.degreeID = degree.degreeID) AND (userStatusOrganizationSubdivision.userID = dov.dovID) AND "
                                        "(userStatusOrganizationSubdivision.statusID = status.statusID) AND "
                                        "(userStatusOrganizationSubdivision.organizationID = organization.organizationID) AND "
                                        "(userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID) AND "
                                        "(userStatusOrganizationSubdivision.statusID = %1) AND "
                                        "(userStatusOrganizationSubdivision.childGradeID = childGrade.childGradeID) AND "
                                        "(userTheme.userID = userStatusOrganizationSubdivision.userID) $$$organizationFilter$$$ "
                                        "GROUP BY userStatusOrganizationSubdivision.userID, userFIO, academicDirectionName, degreeName, statusName, organizationName, subdivisionLevel1Name, postName, childRating, childGradeName, childRatingStage, childRatingAchievementOut ")
                                .arg(statusCode);

                        //Фильтр по организации
                        if (organizationCode != 0) {
                            //Организация пользователя
                            query.replace("$$$organizationFilter$$$", QString("AND (userStatusOrganizationSubdivision.organizationID = %1) ").arg(organizationCode));
                        }
                        else {
                            query.replace("$$$organizationFilter$$$", "");
                        }

                        //Сортировка по ФИО (по возрастанию)
                        query.append(" ORDER BY userFIO ASC ");

                        if (q.exec(query)) {
                            while (q.next()) {

                                result.append(QString("{\"userCode\":%1,"
                                                      "\"userFIO\":\"%2\","
                                                      "\"userStatus\":\"%3\","
                                                      "\"userOrganization\":\"%4\","
                                                      "\"userSubdivision\":\"%5\","
                                                      "\"userPost\":\"%6\","
                                                      "\"userRating\":\"%7\","
                                                      "\"childGrade\":\"%8\","
                                                      "\"userCountTheme\":%9,"
                                                      "\"childRatingStage\":\"%10\","
                                                      "\"childRatingAchievement\":\"%11\"},")
                                              .arg(q.value(0).toString())
                                              .arg(q.value(1).toString())
                                              .arg(q.value(4).toString())
                                              .arg(q.value(5).toString())
                                              .arg(q.value(6).toString())
                                              .arg(q.value(5).toString())
                                              .arg(q.value(8).toString())
                                              .arg(q.value(9).toString())
                                              .arg(q.value(10).toString())
                                              .arg(q.value(11).toString())
                                              .arg(floor(q.value(12).toDouble()*100)/100));
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


                    //Показать учеников без тем
                    if (showAllUsers == 2) {
                        query = QString("SELECT userStatusOrganizationSubdivision.userID, userFIO, academicDirectionName, degreeName, statusName, organizationName, subdivisionLevel1Name, postName, childRating, childGradeName, childRatingStage, childRatingAchievementOut "
                                        "FROM vzletbase.userStatusOrganizationSubdivision, vzletbase.dov, vzletbase.academicDirection, vzletbase.degree, vzletbase.status, vzletbase.organization, vzletbase.subdivisionLevel1, vzletbase.childGrade "
                                        "WHERE "
                                        "(dov.academicDirectionID = academicDirection.academicDirectionID) AND "
                                        "(dov.degreeID = degree.degreeID) AND (userStatusOrganizationSubdivision.userID = dov.dovID) AND "
                                        "(userStatusOrganizationSubdivision.statusID = status.statusID) AND "
                                        "(userStatusOrganizationSubdivision.organizationID = organization.organizationID) AND "
                                        "(userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID) AND "
                                        "(userStatusOrganizationSubdivision.statusID = %1) AND "
                                        "(userStatusOrganizationSubdivision.childGradeID = childGrade.childGradeID) "
                                        " $$$organizationFilter$$$ ")
                                .arg(statusCode);

                        //Фильтр по организации
                        if (organizationCode != 0) {
                            //Организация пользователя
                            query.replace("$$$organizationFilter$$$", QString("AND (userStatusOrganizationSubdivision.organizationID = %1) ").arg(organizationCode));
                        }
                        else {
                            query.replace("$$$organizationFilter$$$", "");
                        }

                        //Сортировка по ФИО (по возрастанию)
                        query.append(" ORDER BY userFIO ASC ");


                        QSqlQuery q1(db);
                        int countThemes = 0;

                        if (q.exec(query)) {
                            while (q.next()) {

                                query.clear();
                                countThemes = 0;
                                //Получаем количество тем, в которых участвует пользователь
                                query = QString("SELECT count(themeID) FROM vzletbase.userTheme WHERE userID = %1 ")
                                        .arg(q.value(0).toString());

                                if (q1.exec(query)){
                                    while (q1.next()) {
                                        countThemes = q1.value(0).toInt();
                                    }

                                    if (countThemes == 0) {
                                        result.append(QString("{\"userCode\":%1,"
                                                              "\"userFIO\":\"%2\","
                                                              "\"userStatus\":\"%3\","
                                                              "\"userOrganization\":\"%4\","
                                                              "\"userSubdivision\":\"%5\","
                                                              "\"userPost\":\"%6\","
                                                              "\"userRating\":\"%7\","
                                                              "\"childGrade\":\"%8\","
                                                              "\"userCountTheme\":%9,"
                                                              "\"childRatingStage\":\"%10\","
                                                              "\"childRatingAchievement\":\"%11\"},")
                                                      .arg(q.value(0).toString())
                                                      .arg(q.value(1).toString())
                                                      .arg(q.value(4).toString())
                                                      .arg(q.value(5).toString())
                                                      .arg(q.value(6).toString())
                                                      .arg(q.value(5).toString())
                                                      .arg(q.value(8).toString())
                                                      .arg(q.value(9).toString())
                                                      .arg(countThemes)
                                                      .arg(q.value(10).toString())
                                                      .arg(floor(q.value(11).toDouble()*100)/100));
                                    }
                                }
                                else {
                                    printf("[error]");
                                    a.quit();
                                    return (200);
                                }
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

                    //Показать учеников, участвующих в мероприятиях организаций-партнеров
                    if (showAllUsers == 3) {
                        query = QString("SELECT userStatusOrganizationSubdivision.userID, userFIO, academicDirectionName, degreeName, statusName, organizationName, subdivisionLevel1Name, postName, childRating, childGradeName, childRatingStage, childRatingAchievementOut "
                                        "FROM vzletbase.userStatusOrganizationSubdivision, vzletbase.dov, vzletbase.academicDirection, vzletbase.degree, vzletbase.status, vzletbase.organization, vzletbase.subdivisionLevel1, vzletbase.childGrade "
                                        "WHERE "
                                        "(dov.academicDirectionID = academicDirection.academicDirectionID) AND "
                                        "(dov.degreeID = degree.degreeID) AND (userStatusOrganizationSubdivision.userID = dov.dovID) AND "
                                        "(userStatusOrganizationSubdivision.statusID = status.statusID) AND "
                                        "(userStatusOrganizationSubdivision.organizationID = organization.organizationID) AND "
                                        "(userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID) AND "
                                        "(userStatusOrganizationSubdivision.statusID = %1) AND "
                                        "(userStatusOrganizationSubdivision.childGradeID = childGrade.childGradeID) "
                                        " $$$organizationFilter$$$ ")
                                .arg(statusCode);

                        //Фильтр по организации
                        if (organizationCode != 0) {
                            //Организация пользователя
                            query.replace("$$$organizationFilter$$$", QString("AND (userStatusOrganizationSubdivision.organizationID = %1) ").arg(organizationCode));
                        }
                        else {
                            query.replace("$$$organizationFilter$$$", "");
                        }

                        //Сортировка по ФИО (по возрастанию)
                        query.append(" ORDER BY userFIO ASC ");


                        QSqlQuery q1(db);
                        int countEvents = 0;

                        if (q.exec(query)) {
                            while (q.next()) {

                                query.clear();
                                countEvents = 0;
                                //Получаем количество мероприятий, в которых участвует пользователь
                                query = QString("SELECT count(themeID) "
                                                "FROM vzletbase.userTheme "
                                                "WHERE "
                                                "(userID = %1) AND "
                                                "themeID IN (SELECT themeID FROM theme WHERE eventMark = TRUE) ")
                                        .arg(q.value(0).toString());

                                if (q1.exec(query)){
                                    while (q1.next()) {
                                        countEvents = q1.value(0).toInt();
                                    }

                                    if (countEvents != 0) {
                                        result.append(QString("{\"userCode\":%1,"
                                                              "\"userFIO\":\"%2\","
                                                              "\"userStatus\":\"%3\","
                                                              "\"userOrganization\":\"%4\","
                                                              "\"userSubdivision\":\"%5\","
                                                              "\"userPost\":\"%6\","
                                                              "\"userRating\":\"%7\","
                                                              "\"childGrade\":\"%8\","
                                                              "\"childCountEvents\":%9,"
                                                              "\"childRatingStage\":\"%10\","
                                                              "\"childRatingAchievement\":\"%11\"},")
                                                      .arg(q.value(0).toString())
                                                      .arg(q.value(1).toString())
                                                      .arg(q.value(4).toString())
                                                      .arg(q.value(5).toString())
                                                      .arg(q.value(6).toString())
                                                      .arg(q.value(5).toString())
                                                      .arg(q.value(8).toString())
                                                      .arg(q.value(9).toString())
                                                      .arg(countEvents)
                                                      .arg(q.value(10).toString())
                                                      .arg(floor(q.value(11).toDouble()*100)/100));
                                    }
                                }
                                else {
                                    printf("[error]");
                                    a.quit();
                                    return (200);
                                }
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



                    //Показать всех учеников (без рейтингов, по-минимуму)
                    if (showAllUsers == 4) {
                        query = QString("SELECT userID, userFIO, statusName, organizationName, subdivisionLevel1Name, childGradeName, childMemberRegistry "
                                        "FROM vzletbase.userStatusOrganizationSubdivision, vzletbase.dov, vzletbase.status, vzletbase.organization, vzletbase.subdivisionLevel1, vzletbase.childGrade  "
                                        "WHERE "
                                        "(userStatusOrganizationSubdivision.userID = dov.dovID) AND "
                                        "(userStatusOrganizationSubdivision.statusID = status.statusID) AND "
                                        "(userStatusOrganizationSubdivision.organizationID = organization.organizationID) AND "
                                        "(userStatusOrganizationSubdivision.childGradeID = childGrade.childGradeID) AND "
                                        "(userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID) "
                                        "AND (userStatusOrganizationSubdivision.statusID = %1) $$$organizationFilter$$$ ")
                                .arg(statusCode);

                        //Фильтр по организации
                        if (organizationCode != 0) {
                            //Организация пользователя
                            query.replace("$$$organizationFilter$$$", QString("AND (userStatusOrganizationSubdivision.organizationID = %1) ").arg(organizationCode));
                        }
                        else {
                            query.replace("$$$organizationFilter$$$", "");
                        }

                        //Сортировка по ФИО (по возрастанию)
                        query.append(" ORDER BY userFIO ASC ");

                        if (q.exec(query)) {
                            while (q.next()) {
                                result.append(QString("{\"userCode\":%1,"
                                                      "\"userFIO\":\"%2\","
                                                      "\"userStatus\":\"%3\","
                                                      "\"userOrganization\":\"%4\","
                                                      "\"userSubdivision\":\"%5\","
                                                      "\"childGrade\":\"%6\","
                                                      "\"childMemberRegistry\":%7},")
                                              .arg(q.value(0).toString())
                                              .arg(q.value(1).toString())
                                              .arg(q.value(2).toString())
                                              .arg(q.value(3).toString())
                                              .arg(q.value(4).toString())
                                              .arg(q.value(5).toString())
                                              .arg(q.value(6).toInt()));
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


                //Если учитель или консультант
                if (statusCode == 3 || statusCode == 4) {

                    //Если показать всех учителей или консультантов (активных)
                    if (showAllUsers == 1) {

                        query = QString("SELECT userStatusOrganizationSubdivision.userID, userFIO, academicDirectionName, degreeName, statusName, organizationName, subdivisionLevel1Name, postName, userMobilePhone, userEMail "
                                        "FROM vzletbase.userStatusOrganizationSubdivision, vzletbase.dov, vzletbase.academicDirection, vzletbase.degree, vzletbase.status, vzletbase.organization, vzletbase.subdivisionLevel1  "
                                        "WHERE "
                                        "(dov.academicDirectionID = academicDirection.academicDirectionID) AND "
                                        "(dov.degreeID = degree.degreeID) AND (userStatusOrganizationSubdivision.userID = dov.dovID) AND "
                                        "(userStatusOrganizationSubdivision.statusID = status.statusID) AND "
                                        "(userStatusOrganizationSubdivision.organizationID = organization.organizationID) AND "
                                        "(userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID) AND "
                                        "(userStatusOrganizationSubdivision.statusID = %1) $$$organizationFilter$$$ ")
                                .arg(statusCode);

                        //Фильтр по организации
                        if (organizationCode != 0) {
                            //Организация пользователя
                            query.replace("$$$organizationFilter$$$", QString("AND (userStatusOrganizationSubdivision.organizationID = %1) ").arg(organizationCode));
                        }
                        else {
                            query.replace("$$$organizationFilter$$$", "");
                        }

                        //Сортировка по ФИО (по возрастанию)
                        query.append(" ORDER BY userFIO ASC ");


                        if (q.exec(query)) {
                            while (q.next()) {

                                result.append(QString("{\"userCode\":%1,"
                                                      "\"userFIO\":\"%2\","
                                                      "\"userAcademicDirection\":\"%3\","
                                                      "\"userDegree\":\"%4\","
                                                      "\"userStatus\":\"%5\","
                                                      "\"userOrganization\":\"%6\","
                                                      "\"userSubdivision\":\"%7\","
                                                      "\"userPost\":\"%8\","
                                                      "\"userMobilePhone\":\"%9\","
                                                      "\"userEMail\":\"%10\"},")
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
                                              .arg(q.value(10).toString()));
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




                    //Показать учителей/консультантов с темами
                    if (showAllUsers == 0) {
                        query = QString("SELECT userStatusOrganizationSubdivision.userID, userFIO, academicDirectionName, degreeName, statusName, organizationName, subdivisionLevel1Name, postName, count(userThemeID) "
                                        "FROM vzletbase.userStatusOrganizationSubdivision, vzletbase.dov, vzletbase.academicDirection, vzletbase.degree, vzletbase.status, vzletbase.organization, vzletbase.subdivisionLevel1, vzletbase.userTheme "
                                        "WHERE "
                                        "(dov.academicDirectionID = academicDirection.academicDirectionID) AND "
                                        "(dov.degreeID = degree.degreeID) AND (userStatusOrganizationSubdivision.userID = dov.dovID) AND "
                                        "(userStatusOrganizationSubdivision.statusID = status.statusID) AND "
                                        "(userStatusOrganizationSubdivision.organizationID = organization.organizationID) AND "
                                        "(userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID) AND "
                                        "(userStatusOrganizationSubdivision.statusID = %1) AND "
                                        "(userTheme.userID = userStatusOrganizationSubdivision.userID) $$$organizationFilter$$$ "
                                        "GROUP BY userStatusOrganizationSubdivision.userID, userFIO, academicDirectionName, degreeName, statusName, organizationName, subdivisionLevel1Name, postName ")
                                .arg(statusCode);

                        //Фильтр по организации
                        if (organizationCode != 0) {
                            //Организация пользователя
                            query.replace("$$$organizationFilter$$$", QString("AND (userStatusOrganizationSubdivision.organizationID = %1) ").arg(organizationCode));
                        }
                        else {
                            query.replace("$$$organizationFilter$$$", "");
                        }

                        //Сортировка по ФИО (по возрастанию)
                        query.append(" ORDER BY userFIO ASC ");

                        if (q.exec(query)) {
                            while (q.next()) {
                                result.append(QString("{\"userCode\":%1,"
                                                      "\"userFIO\":\"%2\","
                                                      "\"userAcademicDirection\":\"%3\","
                                                      "\"userDegree\":\"%4\","
                                                      "\"userStatus\":\"%5\","
                                                      "\"userOrganization\":\"%6\","
                                                      "\"userSubdivision\":\"%7\","
                                                      "\"userPost\":\"%8\","
                                                      "\"userCountTheme\":%9},")
                                              .arg(q.value(0).toString())
                                              .arg(q.value(1).toString())
                                              .arg(q.value(2).toString())
                                              .arg(q.value(3).toString())
                                              .arg(q.value(4).toString())
                                              .arg(q.value(5).toString())
                                              .arg(q.value(6).toString())
                                              .arg(q.value(7).toString())
                                              .arg(q.value(8).toString()));

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



                    //Показать учителей/консультантов без тем
                    if (showAllUsers == 2) {
                        query = QString("SELECT userStatusOrganizationSubdivision.userID, userFIO, academicDirectionName, degreeName, statusName, organizationName, subdivisionLevel1Name, postName "
                                        "FROM vzletbase.userStatusOrganizationSubdivision, vzletbase.dov, vzletbase.academicDirection, vzletbase.degree, vzletbase.status, vzletbase.organization, vzletbase.subdivisionLevel1 "
                                        "WHERE "
                                        "(dov.academicDirectionID = academicDirection.academicDirectionID) AND "
                                        "(dov.degreeID = degree.degreeID) AND (userStatusOrganizationSubdivision.userID = dov.dovID) AND "
                                        "(userStatusOrganizationSubdivision.statusID = status.statusID) AND "
                                        "(userStatusOrganizationSubdivision.organizationID = organization.organizationID) AND "
                                        "(userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID) AND "
                                        "(userStatusOrganizationSubdivision.statusID = %1) "
                                        " $$$organizationFilter$$$ ")
                                .arg(statusCode);

                        //Фильтр по организации
                        if (organizationCode != 0) {
                            //Организация пользователя
                            query.replace("$$$organizationFilter$$$", QString("AND (userStatusOrganizationSubdivision.organizationID = %1) ").arg(organizationCode));
                        }
                        else {
                            query.replace("$$$organizationFilter$$$", "");
                        }

                        //Сортировка по ФИО (по возрастанию)
                        query.append(" ORDER BY userFIO ASC ");


                        QSqlQuery q1(db);
                        int countThemes = 0;

                        if (q.exec(query)) {
                            while (q.next()) {

                                query.clear();
                                countThemes = 0;
                                //Получаем количество тем, в которых участвует пользователь
                                query = QString("SELECT count(themeID) FROM userTheme WHERE userID = %1 ")
                                        .arg(q.value(0).toString());

                                if (q1.exec(query)){
                                    while (q1.next()) {
                                        countThemes = q1.value(0).toInt();
                                    }

                                    if (countThemes == 0) {
                                        result.append(QString("{\"userCode\":%1,"
                                                              "\"userFIO\":\"%2\","
                                                              "\"userAcademicDirection\":\"%3\","
                                                              "\"userDegree\":\"%4\","
                                                              "\"userStatus\":\"%5\","
                                                              "\"userOrganization\":\"%6\","
                                                              "\"userSubdivision\":\"%7\","
                                                              "\"userPost\":\"%8\","
                                                              "\"userCountTheme\":%9},")
                                                      .arg(q.value(0).toString())
                                                      .arg(q.value(1).toString())
                                                      .arg(q.value(2).toString())
                                                      .arg(q.value(3).toString())
                                                      .arg(q.value(4).toString())
                                                      .arg(q.value(5).toString())
                                                      .arg(q.value(6).toString())
                                                      .arg(q.value(7).toString())
                                                      .arg(countThemes));
                                    }
                                }
                                else {
                                    printf("[error]");
                                    a.quit();
                                    return (200);
                                }
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

                //Если не ученик, не учитель или не консультант
                if (statusCode != 2 && statusCode != 3 && statusCode != 4) {
                    query.append(QString("AND (userStatusOrganizationSubdivision.statusID = %1) ").arg(statusCode));
                }

            }

            //Фильтр по организации
            if (organizationCode != 0) {
                //Организация пользователя
                query.append(QString("AND (userStatusOrganizationSubdivision.organizationID = %1) ").arg(organizationCode));
            }

            //Сортировка по ФИО (по возрастанию)
            query.append(" ORDER BY userFIO ASC ");

            //Если выбрана организация
            if (q.exec(query)) {
                while (q.next()) {
                    result.append(QString("{\"userCode\":%1,"
                                          "\"userFIO\":\"%2\","
                                          "\"userAcademicDirection\":\"%3\","
                                          "\"userDegree\":\"%4\","
                                          "\"userStatus\":\"%5\","
                                          "\"userOrganization\":\"%6\","
                                          "\"userSubdivision\":\"%7\","
                                          "\"userPost\":\"%8\","
                                          "\"userRating\":\"%9\","
                                          "\"userMobilePhone\":\"%10\","
                                          "\"userEMail\":\"%11\"},")
                                  .arg(q.value(0).toString())
                                  .arg(q.value(1).toString())
                                  .arg(q.value(2).toString())
                                  .arg(q.value(3).toString())
                                  .arg(q.value(4).toString())
                                  .arg(q.value(5).toString())
                                  .arg(q.value(6).toString())
                                  .arg(q.value(7).toString())
                                  .arg(q.value(8).toString())
                                  .arg(q.value(10).toString())
                                  .arg(q.value(11).toString()));
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




    //Общая статистика предлагаемой тематики по университетам
    if (params.at(0) == "getGeneralStatisticsProposedUniversity") {
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

            QStringList universityCode;
            QStringList universityName;
            QStringList universityFullName;

            int countTheme = 0; //Общее количество предложенных университетом тем
            int countConsultTheme = 0; //Из них консультируемое количество предложенных университетом тем
            int countExecuteTheme = 0; //Количество выполняемых тем
            int countFreeTheme = 0; //Количество свободных тем (статус - предложенная консультантом)

            query = "SELECT organizationID, organizationName, organizationFullName "
                    "FROM vzletbase.organization "
                    "WHERE (organizationID <> 1) AND (organizationTypeID  = 3) AND (universityObserver = FALSE) "
                    "ORDER BY organizationName ASC ";

            universityCode.clear();
            universityName.clear();
            universityFullName.clear();

            if(q.exec(query)) {
                while (q.next()) {
                    universityCode.append(q.value(0).toString());
                    universityName.append(q.value(1).toString());
                    universityFullName.append(q.value(2).toString());
                }


                for(int i = 0; i < universityCode.length(); i++) {

                    countTheme = 0;
                    countConsultTheme = 0;

                    result.append("{");
                    result.append(QString("\"universityName\":\"%1\",").arg(universityName.at(i)));
                    result.append(QString("\"universityFullName\":\"%1\",").arg(universityFullName.at(i)));

                    //Общее количество предложенных тем (по университетам)
                    query = QString("SELECT count(themeID) "
                                    "FROM vzletbase.userTheme "
                                    "WHERE userID IN ( "
                                    "SELECT userID FROM userStatusOrganizationSubdivision WHERE (statusID = 4 OR statusID = 21) AND (organizationID = %1) "
                                    ") ")
                            .arg(universityCode.at(i));


                    if(q.exec(query)) {
                        countTheme = 0;
                        while (q.next()) {
                            countTheme = q.value(0).toInt();
                        }

                        result.append(QString("\"countThemes\":%1,").arg(countTheme));

                        query = QString("SELECT count(themeID) FROM theme WHERE themeID IN ( "
                                        "SELECT themeID "
                                        "FROM vzletbase.userTheme "
                                        "WHERE userID IN ( "
                                        "SELECT userID FROM vzletbase.userStatusOrganizationSubdivision WHERE (statusID = 4 OR statusID = 21) AND (organizationID = %1) "
                                        ") "
                                        ") AND statusThemeID = 6 ")

                                .arg(universityCode.at(i));
                        if (q.exec(query)) {
                            while (q.next()) {
                                countConsultTheme = q.value(0).toInt();
                            }

                            result.append(QString("\"countConsultThemes\":%1,").arg(countConsultTheme));
                        }
                        else {
                            printf("[error]");
                            a.quit();
                            return (200);
                        }

                        //Из них - количество выполняемых тем
                        query = QString("SELECT count(themeID) FROM theme WHERE themeID IN ( "
                                        "SELECT themeID "
                                        "FROM vzletbase.userTheme "
                                        "WHERE userID IN ( "
                                        "SELECT userID FROM vzletbase.userStatusOrganizationSubdivision WHERE (statusID = 4) AND (organizationID = %1) "
                                        ") "
                                        ") AND statusThemeID = 2 ")
                                .arg(universityCode.at(i));
                        if (q.exec(query)) {
                            while (q.next()) {
                                countExecuteTheme = q.value(0).toInt();
                            }

                            result.append(QString("\"countExecuteThemes\":%1,").arg(countExecuteTheme));


                            //Количество свободных тем (статус - предложенная консультантом)
                            query = QString("SELECT count(themeID) FROM vzletbase.theme WHERE themeID IN ( "
                                            "SELECT themeID "
                                            "FROM userTheme "
                                            "WHERE userID IN ( "
                                            "SELECT userID FROM vzletbase.userStatusOrganizationSubdivision WHERE (statusID = 4) AND (organizationID = %1) "
                                            ") "
                                            ") AND statusThemeID = 1 ")
                                    .arg(universityCode.at(i));

                            if (q.exec(query)) {
                                while (q.next()) {
                                    countFreeTheme = q.value(0).toInt();
                                }

                                result.append(QString("\"countFreeThemes\":%1").arg(countFreeTheme));
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

                        result.append("},");
                    }
                    else {
                        printf("[error]");
                        a.quit();
                        return (200);
                    }
                }

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
        else {
            printf("[error]");
            a.quit();
            return (200);
        }
    }




    //Общая статистика предлагаемой тематики по образовталеьным округам и крупным городам
    if (params.at(0) == "getGeneralStatisticsProposedEdDistrict") {
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

            QStringList edDistrictCode;
            QStringList edDistrictName;
            QString edDistrictCoordFIO;
            QStringList organizationEdDistrictLocation;
            int countTheme = 0; //Общее количество предложенных на консультирование тем
            int countConsultTheme = 0; //Из них - количество принятых к консультированию тем
            int countAmountDirectedThemes = 0; //Количество тем, направленных на рассмотрение консультанту
            int countExecuteTheme = 0; //Количество принятых тем на консультацию (консультируемых)

            query = "SELECT organizationID, organizationName, organizationEdDistrictLocation "
                    "FROM vzletbase.organization "
                    "WHERE (organizationID <> 1) AND (organizationTypeID  = 2) "
                    "ORDER BY organizationName ASC ";

            edDistrictCode.clear();
            edDistrictName.clear();
            edDistrictCoordFIO.clear();
            organizationEdDistrictLocation.clear();

            if(q.exec(query)) {
                while (q.next()) {
                    edDistrictCode.append(q.value(0).toString());
                    edDistrictName.append(q.value(1).toString());
                    organizationEdDistrictLocation.append(q.value(2).toString());
                }

                for(int i = 0; i < edDistrictCode.length(); i++) {
                    result.append("{");
                    result.append(QString("\"edDistrictName\":\"%1\",").arg(edDistrictName.at(i)));
                    result.append(QString("\"organizationEdDistrictLocation\":\"%1\",").arg(organizationEdDistrictLocation.at(i)));

                    //Получаем ФИО координатора от соответствующего территориального управления
                    query = QString("SELECT userFIO "
                                    "FROM vzletbase.userStatusOrganizationSubdivision, dov "
                                    "WHERE "
                                    "(userStatusOrganizationSubdivision.organizationID = %1) AND (userStatusOrganizationSubdivision.statusID = 5) AND "
                                    "(userStatusOrganizationSubdivision.userID = dov.dovID) ")
                            .arg(edDistrictCode.at(i));

                    if (q.exec(query)) {
                        edDistrictCoordFIO.clear();
                        while (q.next()) {
                            edDistrictCoordFIO = q.value(0).toString();
                        }

                        result.append(QString("\"edDistrictCoordFIO\":\"%1\",").arg(edDistrictCoordFIO));

                        //Количество запросов на консультирование
                        query = QString("SELECT count(themeID) "
                                        "FROM vzletbase.userTheme "
                                        "WHERE userID IN ( "
                                        "SELECT userID FROM vzletbase.userStatusOrganizationSubdivision WHERE (statusID = 3) AND (organizationID = %1) "
                                        ") ")
                                .arg(edDistrictCode.at(i));


                        if(q.exec(query)) {
                            countTheme = 0;
                            while (q.next()) {
                                countTheme = q.value(0).toInt();
                            }

                            result.append(QString("\"countThemes\":%1,").arg(countTheme));

                            //Количество запросов на консультирование
                            query = QString("SELECT count(themeID) "
                                            "FROM vzletbase.userTheme "
                                            "WHERE userID IN ( "
                                            "SELECT userID FROM userStatusOrganizationSubdivision WHERE (statusID = 3) AND (organizationID = %1) "
                                            ") AND (themeID IN (SELECT themeID FROM theme WHERE statusThemeID = 9)) ")
                                    .arg(edDistrictCode.at(i));


                            if (q.exec(query)) {
                                while (q.next()) {
                                    countConsultTheme = q.value(0).toInt();
                                }

                                result.append(QString("\"countConsultThemes\":%1,").arg(countConsultTheme));

                                //Количество тем, направленных на рассмотрение консультанту
                                query = QString("SELECT count(themeID) "
                                                "FROM vzletbase.userTheme "
                                                "WHERE userID IN ( "
                                                "SELECT userID FROM userStatusOrganizationSubdivision WHERE (statusID = 3) AND (organizationID = %1) "
                                                ") AND (themeID IN (SELECT themeID FROM theme WHERE statusThemeID = 6)) ")
                                        .arg(edDistrictCode.at(i));

                                if(q.exec(query)) {
                                    while (q.next()) {
                                        countAmountDirectedThemes = q.value(0).toInt();
                                    }

                                    result.append(QString("\"countAmountDirectedThemes\":%1,").arg(countAmountDirectedThemes));

                                    //Количество выполняемых тем
                                    query = QString("SELECT count(themeID) "
                                                    "FROM vzletbase.userTheme "
                                                    "WHERE userID IN ( "
                                                    "SELECT userID FROM userStatusOrganizationSubdivision WHERE (statusID = 3) AND (organizationID = %1) "
                                                    ") AND (themeID IN (SELECT themeID FROM theme WHERE statusThemeID = 2)) ")
                                            .arg(edDistrictCode.at(i));

                                    if (q.exec(query)) {
                                        while (q.next()) {
                                            countExecuteTheme = q.value(0).toInt();
                                        }

                                        result.append(QString("\"countExecuteThemes\":%1").arg(countExecuteTheme));
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


                            result.append("},");
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
        else {
            printf("[error]");
            a.quit();
            return (200);
        }

    }



    //Общая статистика предлагаемой тематики по кафедрам университетов
    if (params.at(0) == "getGeneralStatisticsProposedUniversityDepartment") {
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

            QStringList subdivisionCode;
            QStringList subdivisionName;
            int universityCode = params.at(1).toInt();
            int countTheme = 0; //Общее количество предложенных кафедрами тем
            int countConsultTheme = 0; //Из них количество консультируемых кафедрами тем

            query = QString("SELECT subdivisionLevel1ID, subdivisionLevel1Name "
                            "FROM vzletbase.subdivisionLevel1 "
                            "WHERE organizationID = %1 "
                            "ORDER BY subdivisionLevel1Name ASC ")
                    .arg(universityCode);

            subdivisionCode.clear();
            subdivisionName.clear();

            if(q.exec(query)) {
                while (q.next()) {
                    subdivisionCode.append(q.value(0).toString());
                    subdivisionName.append(q.value(1).toString());
                }

                for(int i = 0; i < subdivisionCode.length(); i++) {
                    result.append("{");
                    result.append(QString("\"subdivisionName\":\"%1\",").arg(subdivisionName.at(i)));

                    query = QString("SELECT count(themeID) "
                                    "FROM vzletbase.userTheme "
                                    "WHERE userID IN ( "
                                    "SELECT userID FROM vzletbase.userStatusOrganizationSubdivision WHERE (statusID = 4) AND (organizationID = %1) AND (subdivisionLevel1ID = %2) "
                                    ") ")
                            .arg(universityCode)
                            .arg(subdivisionCode.at(i));

                    if(q.exec(query)) {
                        countTheme = 0;
                        while (q.next()) {
                            countTheme = q.value(0).toInt();
                        }

                        result.append(QString("\"countThemes\":%1,").arg(countTheme));

                        query = QString("SELECT count(themeID) "
                                        "FROM vzletbase.userTheme "
                                        "WHERE userID IN ( "
                                        "SELECT userID FROM vzletbase.userStatusOrganizationSubdivision WHERE (statusID = 4) AND (organizationID = %1) AND (subdivisionLevel1ID = %2) "
                                        ") AND (themeID IN (SELECT themeID FROM vzletbase.theme WHERE statusThemeID = 2)) ")
                                .arg(universityCode)
                                .arg(subdivisionCode.at(i));

                        if (q.exec(query)) {
                            while (q.next()) {
                                countConsultTheme = q.value(0).toInt();
                            }

                            result.append(QString("\"countConsultThemes\":%1").arg(countConsultTheme));
                        }
                        else {
                            printf("[error]");
                            a.quit();
                            return (200);
                        }

                        result.append("},");
                    }
                    else {
                        printf("[error]");
                        a.quit();
                        return (200);
                    }
                }

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
        else {
            printf("[error]");
            a.quit();
            return (200);
        }
    }


    //Общая статистика запросов на консультирование по школам
    if (params.at(0) == "getGeneralStatisticsProposedSchool") {
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

            QStringList subdivisionCode;
            QStringList subdivisionName;
            int universityCode = params.at(1).toInt();
            int countTheme = 0; //Общее количество запросов на консультирование
            int countConsultTheme = 0; //Из них - количество удовлетворенных запросов

            query = QString("SELECT subdivisionLevel1ID, subdivisionLevel1Name "
                            "FROM vzletbase.subdivisionLevel1 "
                            "WHERE organizationID = %1 "
                            "ORDER BY subdivisionLevel1Name ASC ")
                    .arg(universityCode);

            subdivisionCode.clear();
            subdivisionName.clear();

            if(q.exec(query)) {
                while (q.next()) {
                    subdivisionCode.append(q.value(0).toString());
                    subdivisionName.append(q.value(1).toString());
                }

                for(int i = 0; i < subdivisionCode.length(); i++) {
                    result.append("{");
                    result.append(QString("\"schoolName\":\"%1\",").arg(subdivisionName.at(i)));

                    query = QString("SELECT count(themeID) "
                                    "FROM vzletbase.userTheme "
                                    "WHERE userID IN ( "
                                    "SELECT userID FROM userStatusOrganizationSubdivision WHERE (statusID = 3) AND (organizationID = %1) AND (subdivisionLevel1ID = %2) "
                                    ") ")
                            .arg(universityCode)
                            .arg(subdivisionCode.at(i));

                    if(q.exec(query)) {
                        countTheme = 0;
                        while (q.next()) {
                            countTheme = q.value(0).toInt();
                        }

                        result.append(QString("\"countThemes\":%1,").arg(countTheme));

                        //Из них - количество удовлетворенных запросов
                        query = QString("SELECT count(themeID) FROM vzletbase.theme WHERE themeID IN ( "
                                        "SELECT themeID "
                                        "FROM vzletbase.userTheme "
                                        "WHERE userID IN ( "
                                        "SELECT userID FROM vzletbase.userStatusOrganizationSubdivision WHERE (statusID = 3) AND (organizationID = %1) AND (subdivisionLevel1ID = %2) "
                                        ") "
                                        ") AND (themeStaffing = TRUE) AND (statusThemeID = 2) ")
                                .arg(universityCode)
                                .arg(subdivisionCode.at(i));

                        if (q.exec(query)) {
                            while (q.next()) {
                                countConsultTheme = q.value(0).toInt();
                            }

                            result.append(QString("\"countConsultThemes\":%1").arg(countConsultTheme));
                        }
                        else {
                            printf("[error]");
                            a.quit();
                            return (200);
                        }

                        result.append("},");
                    }
                    else {
                        printf("[error]");
                        a.quit();
                        return (200);
                    }
                }

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
        else {
            printf("[error]");
            a.quit();
            return (200);
        }
    }



    if (params.at(0) == "QWERTY5") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {
            QSqlQuery q(db);
            QString query;

            QStringList childs;
            int stageExecutePlannedCode = 0; //Плановая стадия проекта
            int stageExecuteCurrentCode = 0; //Текущая стадия проекта (выбранная учеником)
            int fOld = 0; //Предыдущий рейтинг (хранящийся в базе)
            int fNew = 0; //Рейтинг школьника в текущем году
            QDate plannedDate; //Плановая дата (по графику)

            //Получаем ближайшую дату и соответствующий балл за стадию выполнения проекта - к текущей дате по плану-графику
            query = QString("SELECT TOP 1 timeSheduleID, timeSheduleDateExecute, stageExecutePoint "
                            "FROM vzletbase.timeShedule, stageExecute "
                            "WHERE (#%1# <= timeSheduleDateExecute) AND (timeShedule.stageExecuteID = stageExecute.stageExecuteID) ")
                    .arg(QDate::currentDate().toString("dd/MM/yyyy"));

            if (q.exec(query)) {
                while (q.next()) {
                    stageExecutePlannedCode = q.value(2).toInt();
                    plannedDate = QDate::fromString(q.value(1).toString().left(10), "yyyy-MM-dd");
                }

                //Получаем разницу дат между сегодняшней датой и датой в плане
                int datesDifferent = QDate::currentDate().daysTo(plannedDate);

                if (datesDifferent == 0) {
                    //Если эта разница равна 0, значит сегодня дата выполнения определенной стадии работы

                    //Получаем список учеников
                    query = "SELECT userID FROM vzletbase.userStatusOrganizationSubdivision WHERE statusID = 2 ";

                    if (q.exec(query)) {
                        childs.clear();
                        while (q.next()) {
                            childs.append(q.value(0).toString());
                        }
                    }

                    for(int i = 0; i < childs.length(); i++) {
                        //Получаем количество баллов за текущий этап, выбранный учеником
                        query = QString("SELECT stageExecutePoint FROM vzletbase.stageExecute WHERE stageExecuteID IN ( "
                                        "SELECT LAST(stageExecuteID) FROM theme "
                                        "WHERE themeID IN (SELECT themeID FROM vzletbase.userTheme WHERE userID = %1)  "
                                        ") ")
                                .arg(childs.at(i));
                        if (q.exec(query)) {
                            while (q.next()) {
                                stageExecuteCurrentCode = q.value(0).toInt();
                            }

                            //Получаем текущий рейтинг пользователя
                            query = QString("SELECT userCurrentYearRating FROM vzletbase.dov WHERE dovID = %1 ").arg(childs.at(i));
                            if (q.exec(query)) {
                                fOld = q.value(0).toInt();
                            }

                            fNew = fOld + ((stageExecuteCurrentCode - stageExecutePlannedCode) + 1);

                            //Теперь этот рейтинг запишем в базу
                            query = QString("UPDATE vzletbase.dov "
                                            "SET userCurrentYearRating = %2 "
                                            "WHERE dovID = %1 ").arg(childs.at(i)).arg(fNew);

                            if (q.exec(query)) {
                                printf("OK");
                            }
                            else {
                                printf("Error");
                            }

                            a.quit();
                            return (200);
                        }
                    }
                }
            }
            a.quit();
            return (200);
        }
    }



    //Статистика принятых к выполнению проектов по образовательным округам и городам
    if (params.at(0) == "statisticsCarriedByEducationalDistrict") {
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

            QStringList educationalDistrictCode;
            QStringList educationalDistrictName;
            int countExecuteTheme = 0; //Количество выполняемых тем

            query = "SELECT organizationID, organizationName "
                    "FROM vzletbase.organization "
                    "WHERE (organizationID <> 1) AND (organizationTypeID  = 2) "
                    "ORDER BY organizationName ASC ";

            educationalDistrictCode.clear();
            educationalDistrictName.clear();

            if(q.exec(query)) {
                while (q.next()) {
                    educationalDistrictCode.append(q.value(0).toString());
                    educationalDistrictName.append(q.value(1).toString());
                }

                for(int i = 0; i < educationalDistrictCode.length(); i++) {

                    countExecuteTheme = 0;

                    result.append("{");
                    result.append(QString("\"educationalDistrictName\":\"%1\",").arg(educationalDistrictName.at(i)));

                    //Из них - количество консультируемых тем
                    query = QString("SELECT count(themeID) FROM vzletbase.theme WHERE themeID IN ( "
                                    "SELECT DISTINCT themeID "
                                    "FROM vzletbase.userTheme "
                                    "WHERE userID IN ( "
                                    "SELECT userID FROM vzletbase.userStatusOrganizationSubdivision WHERE ((statusID = 3) OR (statusID = 2)) AND (organizationID = %1) "
                                    ") "
                                    ") AND themeStaffing = TRUE ")
                            .arg(educationalDistrictCode.at(i));

                    if(q.exec(query)) {
                        while (q.next()) {
                            countExecuteTheme = q.value(0).toInt();
                        }

                        result.append(QString("\"educationalDistrictCountExecuteThemes\":%1,").arg(countExecuteTheme));
                        result.append("},");
                    }
                    else {
                        printf("[error]");
                        a.quit();
                        return (200);
                    }
                }

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
        else {
            printf("[error]");
            a.quit();
            return (200);
        }
    }







    //Статистика принятых к выполнению проектов по школам
    if (params.at(0) == "statisticsCarriedBySchool") {
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

            QStringList schoolCode;
            QStringList schoolName;
            int educationDistrictCode = params.at(1).toInt();
            int countExecuteTheme = 0; //Количество выполняемых тем

            query = QString("SELECT subdivisionLevel1ID, subdivisionLevel1Name "
                            "FROM vzletbase.subdivisionLevel1 "
                            "WHERE organizationID = %1 "
                            "ORDER BY subdivisionLevel1Name ASC ")
                    .arg(educationDistrictCode);

            schoolCode.clear();
            schoolName.clear();

            if(q.exec(query)) {
                while (q.next()) {
                    schoolCode.append(q.value(0).toString());
                    schoolName.append(q.value(1).toString());
                }

                for(int i = 0; i < schoolCode.length(); i++) {
                    result.append("{");
                    result.append(QString("\"schoolName\":\"%1\",").arg(schoolName.at(i)));

                    query = QString("SELECT count(themeID) FROM vzletbase.theme WHERE themeID IN ( "
                                    "SELECT DISTINCT themeID "
                                    "FROM vzletbase.userTheme "
                                    "WHERE userID IN ( "
                                    "SELECT userID FROM vzletbase.userStatusOrganizationSubdivision WHERE ((statusID = 3) OR (statusID = 2)) AND (organizationID = %1) AND (subdivisionLevel1ID = %2) "
                                    ") "
                                    ") AND statusThemeID = 2 ")
                            .arg(educationDistrictCode)
                            .arg(schoolCode.at(i));

                    if (q.exec(query)) {
                        while (q.next()) {
                            countExecuteTheme = q.value(0).toInt();
                        }

                        result.append(QString("\"schoolCountExecuteThemes\":%1").arg(countExecuteTheme));
                        result.append("},");
                    }
                    else {
                        printf("[error]");
                        a.quit();
                        return (200);
                    }
                }

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

            query = "SELECT organizationID, organizationName FROM vzletbase.organization WHERE organizationID <> 1 ";

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








    //Вывод на экран общей статистики - кол-во консультантов, учеников и т.п., общее количество тем
    if (params.at(0) == "getGeneralStat") {
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

            query = "SELECT COUNT(*), "
                    "(SELECT COUNT(userID)  FROM vzletbase.userStatusOrganizationSubdivision WHERE statusID = 4) AS consultantCount, "
                    "(SELECT COUNT(userID) AS tutorCount FROM vzletbase.userStatusOrganizationSubdivision WHERE statusID = 3) AS tutorCount, "
                    "(SELECT COUNT(userID) AS tutorCount FROM vzletbase.userStatusOrganizationSubdivision WHERE statusID = 2) AS childCount, "
                    "(SELECT COUNT(themeID) FROM theme WHERE eventMark = FALSE) AS themeCount "
                    "FROM vzletbase.userStatusOrganizationSubdivision, Theme ";

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"consultantCount\":%1,").arg(q.value(1).toString()));
                    result.append(QString("\"tutorCount\":%1,").arg(q.value(2).toString()));
                    result.append(QString("\"childCount\":%1,").arg(q.value(3).toString()));
                    result.append(QString("\"themeCount\":%1").arg(q.value(4).toString()));
                    result.append("}");
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
            printf("[error]");
            a.quit();
            return (200);
        }
    }




    //Вывод на экран списка школьников для выбора
    if (params.at(0) == "getChildsForApplicationTheme") {
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

            query = "SELECT userID, userFIO, organizationName, subdivisionLevel1Name  "
                    "FROM vzletbase.userStatusOrganizationSubdivision, vzletbase.dov, vzletbase.organization, vzletbase.subdivisionLevel1 "
                    "WHERE "
                    "(userStatusOrganizationSubdivision.userID = dov.dovID) AND "
                    "(userStatusOrganizationSubdivision.organizationID = organization.organizationID) AND "
                    "(userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID) "
                    "AND (userStatusOrganizationSubdivision.statusID = 2) ";

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"userCode\":%1,"
                                          "\"userFIO\":\"%2\","
                                          "\"userOrganization\":\"%3\","
                                          "\"userSubdivision\":\"%4\"},")
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
            printf("[error]");
            a.quit();
            return (200);
        }
    }





    //Общая статистика предлагаемой и выполняемой тематики по секциям
    if (params.at(0) == "getMonitoringStatisticsSection") {
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

            QStringList sectionID, sectionName;
            int countOfferedThemes = 0; //Количество предлагаемых тем
            int countPerformedThemes = 0; //Количество выполняемых тем


            query = "SELECT sectionID, sectionName FROM vzletbase.section ";
            if (q.exec(query)) {
                sectionID.clear();
                sectionName.clear();
                while (q.next()) {
                    sectionID.append(q.value(0).toString());
                    sectionName.append(q.value(1).toString());
                }

                for(int i = 0; i < sectionID.length(); i++) {
                    //Количество предлагаемых тем
                    query = QString("SELECT COUNT(themeID) "
                                    "FROM vzletbase.theme "
                                    "WHERE statusThemeID = 1 AND "
                                    "sectionID = %1 ").arg(sectionID.at(i));
                    if (q.exec(query)) {
                        countOfferedThemes = 0;
                        while (q.next()) {
                            countOfferedThemes = q.value(0).toInt();
                        }

                        //Количество выполняемых тем
                        query = QString("SELECT COUNT(themeID) "
                                        "FROM vzletbase.theme "
                                        "WHERE statusThemeID = 2 AND "
                                        "sectionID = %1 ").arg(sectionID.at(i));
                        if (q.exec(query)) {
                            countPerformedThemes = 0;
                            while (q.next()) {
                                countPerformedThemes = q.value(0).toInt();
                            }

                            result.append(QString("{\"sectionName\":\"%1\",\"countOfferedThemes\":%2,\"countPerformedThemes\":%3},")
                                          .arg(sectionName.at(i))
                                          .arg(countOfferedThemes)
                                          .arg(countPerformedThemes));
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
        else {
            printf("[error]");
            a.quit();
            return (200);
        }
    }

    return (200);
}
