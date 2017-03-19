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

#include <QProcess>

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


//Проверка кода темы
int checkThemCode(QSqlDatabase *db, const int themeID) {
    int userThemeID = 0;
    QSqlQuery q(*db);
    if (q.exec(QString("SELECT themeID "
                       "FROM vzletbase.Theme "
                       "WHERE "
                       "themeID = %1").arg(themeID))) {
        while (q.next()) {
            userThemeID = q.value(0).toInt();
        }

    }
    return userThemeID;
}

//Проверка, входит ли пользователь в тройку (по теме)
int checkTrio(QSqlDatabase *db, const int themeID, const int userID) {
    int userCode = 0;
    QSqlQuery q(*db);
    if (q.exec(QString("SELECT userID "
                       "FROM vzletbase.userTheme "
                       "WHERE "
                       "(themeID = %1) AND (userID = %2) ")
               .arg(themeID)
               .arg(userID))) {

        while (q.next()) {
            userCode = q.value(0).toInt();
        }
    }
    return userCode;
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


    //Получить оценочный лист
    if (params.at(0) == "getAssessmentList") {
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
            QString result = "[";

            query = "SELECT criteriaDistQuestionID, criteriaDistQuestionName "
                    "FROM vzletbase.СriteriaDistQuestion ";

            if (q.exec(query)) {
                while(q.next()) {
                    result.append(QString("{\"criteriaDistQuestionCode\":%1,"
                                          "\"criteriaDistQuestionName\":\"%2\"},")
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
        else {
            printf("[error]");
            a.quit();
            return (200);
        }
    }



    if (params.at(0) == "setAssessmentList") {
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

            int userCode = QString("%1").arg(personID).toInt();

            if (userCode == 1) {
                printf("Вы не имеете доступа к этому разделу!");
                a.quit();
                return (401);
            }
            else {
                int userStatusCode = checkUserStatus(&db, userCode);

                //Если ученик, учитель или консультант
                if ((userStatusCode == 2) || (userStatusCode == 3) || (userStatusCode == 4)) {

                    int themeCode = params.at(1).toInt(); //Код темы

                    //Проверяем, может ли пользователь по этой теме вводить какие-то данные
                    if (checkTrio(&db, themeCode, userCode) == 0) {
                        a.quit();
                        return (401);
                    }
                    else {
                        //Проверяем код темы на то, что такая тема есть
                        if (checkThemCode(&db, themeCode) == 0) {
                            //Такой темы нет
                            printf("{\"errorCode\":2}");
                            a.quit();
                            return (200);
                        }
                        else {
                            //Такая тема есть, продолжаем работать

                            int numberCriteria = 0; //Номер критерия
                            QByteArray b;
                            b.clear();
                            QString userComment;
                            int j = 1; //Номер критерия

                            for(int i = 2; i < params.length(); i+=2) {

                                numberCriteria = params.at(i).toInt();
                                b.clear();
                                b.append(params.at(i+1));
                                userComment = QByteArray::fromBase64(b);

                                query = QString("INSERT INTO vzletbase.SheetsRatingsTheme "
                                                "(userID, themeID, criteriaDistQuestionID, sheetsRatingsThemeEvaluationCriteria, sheetsRatingsThemeComment, sheetsRatingsThemeDateTime) "
                                                "VALUES "
                                                "(%1, %2, %3, %4, '%5', NOW()) ")
                                        .arg(userCode)
                                        .arg(themeCode)
                                        .arg(j)
                                        .arg(numberCriteria)
                                        .arg(userComment);

                                j++;

                                if (!q.exec(query)) {
                                    printf("[error]");

                                    a.quit();
                                    return (200);
                                }
                            }

                            printf("{\"errorCode\":0}");

                            a.quit();
                            return (200);
                        }
                    }
                }
                else {
                    printf("Вы не имеете доступа к этому разделу!");
                    a.quit();
                    return (401);
                }
            }
        }
        else {
            printf("[error]");
            a.quit();
            return (200);
        }
    }





    //Получить список школьников-участников с их творческими рейтингами
    if (params.at(0) == "getChildsWithRating") {
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
            QString result = "[";

            query = "SELECT Theme.themeName, Dov.userFIO, Organization.organizationName, SubdivisionLevel1.subdivisionLevel1Name, "
                    "Theme.themePlanned, Theme.themeActivity, Theme.themeRemoteConkurs, Theme.themeAccomodation, Theme.themePrelimiRating, "
                    "Theme.themeIntramuralConkurs, Theme.themeCurrentRating, Theme.themeLastRating, Theme.themeOverallRating, Theme.themeLevelProvideIncentive "
                    "FROM (Organization INNER JOIN ((Dov INNER JOIN (Theme INNER JOIN UserTheme ON Theme.themeID = UserTheme.themeID) ON Dov.dovID = UserTheme.userID) INNER JOIN userStatusOrganizationSubdivision ON Dov.dovID = userStatusOrganizationSubdivision.userID) ON Organization.organizationID = userStatusOrganizationSubdivision.organizationID) INNER JOIN SubdivisionLevel1 ON (SubdivisionLevel1.subdivisionLevel1ID = userStatusOrganizationSubdivision.subdivisionLevel1ID) AND (Organization.organizationID = SubdivisionLevel1.organizationID) "
                    "WHERE (((userStatusOrganizationSubdivision.statusID)=2) AND ((Theme.stageExecuteID)>1)) ";

            if (q.exec(query)) {
                while (q.next()) {
                    result.append(QString("{\"themeName\":\"%1\","
                                          "\"childName\":\"%2\","
                                          "\"organizationName\":\"%3\","
                                          "\"subdivisionName\":\"%4\","
                                          "\"themePlanned\":%5,"
                                          "\"themeActivity\":%6,"
                                          "\"themeRemoteConkurs\":%7,"
                                          "\"themeAccomodation\":%8,"
                                          "\"themePrelimiRating\":%9,"
                                          "\"themeIntramuralConkurs\":%10,"
                                          "\"themeCurrentRating\":%11,"
                                          "\"themeLastRating\":%12,"
                                          "\"themeOverallRating\":%13,"
                                          "\"themeLevelProvideIncentive\":%14},")
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
                                  .arg(q.value(11).toString())
                                  .arg(q.value(12).toString())
                                  .arg(q.value(13).toString()));
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



    return 200;
}
