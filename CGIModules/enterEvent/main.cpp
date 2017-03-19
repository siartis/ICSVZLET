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
QString v = " 0123456789qwertyuiopasdfghjklzxcvbnm.,йцукенгшщзхъфывапролджэячсмитьбю@№;%?()-+_!#:/";

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

            int userCode = QString("%1").arg(personID).toInt();
            int currentUserStatus = checkUserStatus(&db, userCode);

            //Если это координатор от организации-партнера
            if (currentUserStatus == 17) {

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
            else {
                printf("error");
                a.quit();
                return (401);
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







    //Ввод нового мероприятия
    if (params.at(0) == "enterEvent") {
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

            //Если вошел Координатор от организации-партнера
            if (currentUserStatus != 17) {
                a.quit();
                return (401);
            }
            else {
                QSqlQuery q(db);
                QString query = "";

                QByteArray b;
                b.append(params.at(1));

                QString eventName = TEXT(QByteArray::fromBase64(b), v);
                b.clear();
                b.append(params.at(2));

                QString comment = TEXT(QByteArray::fromBase64(b), v);

                int priorityDirection = params.at(3).toInt();
                int scienceIndustrialSphere = params.at(4).toInt();

                int scienceDirection1 = params.at(5).toInt();
                int scienceDirection2 = params.at(6).toInt();
                int scienceDirection3 = params.at(7).toInt();
                int eventLocation = params.at(8).toInt(); //Город проведения мероприятия
                int eventRemotely = params.at(9).toInt(); //Признак того, что мероприятие будет проводиться дистанционно

                b.clear();
                b.append(params.at(10));
                QString dateEvent = TEXT(QByteArray::fromBase64(b), v); //Дата проведения мероприятия

                b.clear();
                b.append(params.at(11));
                QString textToEMail = TEXT(QByteArray::fromBase64(b), v);//Текст для отправки записавшемуся участнику мероприятия


                QString eventRemote = "FALSE";
                if (eventRemotely == 1) {
                    eventRemote = "TRUE";
                }

                query = QString("INSERT INTO vzletbase.Theme "
                                "(themeName, themeComment, themeDateTime, scienceDirectionID_1, "
                                "scienceDirectionID_2, scienceDirectionID_3, priorityDirectionID, sectionID, scienceIndustrialSphereID, stageExecuteID, "
                                "statusThemeID, eventMark, conferenceLocationID, eventRemotely, eventDate, eventTextEMail) "
                                "VALUES "
                                "('%1', '%2', NOW(), %3, %4, %5, %6, NULL, %7, NULL, 10, TRUE, %8, %9, '%10', '%11') ")
                        .arg(eventName)
                        .arg(comment)
                        .arg(scienceDirection1)
                        .arg(scienceDirection2)
                        .arg(scienceDirection3)
                        .arg(priorityDirection)
                        .arg(scienceIndustrialSphere)
                        .arg(eventLocation)
                        .arg(eventRemote)
                        .arg(dateEvent)
                        .arg(textToEMail);


                if (q.exec(query)) {
                    //Получаем код только что введенного мероприятия
                    query = QString("SELECT THEMEID FROM VZLETBASE.THEME WHERE "
                                    "(themeName = '%1') AND "
                                    "(themeComment = '%2') AND "
                                    "(scienceDirectionID_1 = %3) AND "
                                    "(scienceDirectionID_2 = %4) AND "
                                    "(scienceDirectionID_3 = %5) AND "
                                    "(priorityDirectionID = %6) AND "
                                    "(scienceIndustrialSphereID = %7) AND "
                                    "(conferenceLocationID = %8) AND "
                                    "(eventMark = TRUE) AND "
                                    "(eventRemotely = %9) ")
                            .arg(eventName)
                            .arg(comment)
                            .arg(scienceDirection1)
                            .arg(scienceDirection2)
                            .arg(scienceDirection3)
                            .arg(priorityDirection)
                            .arg(scienceIndustrialSphere)
                            .arg(eventLocation)
                            .arg(eventRemote);

                    if (q.exec(query)) {

                        int themeCode = 0;
                        while (q.next()) {
                            themeCode = q.value(0).toInt();
                        }

                        query = QString("INSERT INTO VZLETBASE.USERTHEME (userID, themeID, userThemeDateTime, statusParticipationID) VALUES "
                                        "(%1, %2, NOW(), NULL) ")
                                .arg(userCode)
                                .arg(themeCode);

                        if (q.exec(query)) {
                            db.close();
                            printf(QString("Новое мероприятие успешно введено! ").toLocal8Bit());
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
        else {
            printf("[error]");
            a.quit();
            return (200);
        }

    }







    //Присоединиться к выбранному мероприятию
    if (params.at(0) == "applyToEvent") {
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

            //Если ученик, учитель, консультант, аналитик, координатор от организации-партнера
            if (currentUserStatus == 2 || currentUserStatus == 3 || currentUserStatus == 4 || currentUserStatus == 14 || currentUserStatus == 17) {

                int eventCode = params.at(1).toInt(); //Код мероприятия

                QByteArray b;
                b.clear();
                b.append(params.at(2));
                QString eventComment = TEXT(QByteArray::fromBase64(b), v); //Краткий комментарий участника мероприятия, подавшего заявку

                //Проверим, не подал ли он заявку повторно?!
                query = QString("SELECT count(userID) FROM vzletbase.userTheme WHERE (userID = %1) AND (themeID = %2) ")
                        .arg(userCode)
                        .arg(eventCode);
                if (q.exec(query)) {

                    int countSelectedEvents = 0; //Количество выбранных учеником тем с этим кодом

                    while (q.next()) {
                        countSelectedEvents = q.value(0).toInt();
                    }

                    if (countSelectedEvents == 1) {
                        printf(QString("Вы не можете подать больше одной заявки на это мероприятие! ").toLocal8Bit());
                        a.quit();
                        return (200);
                    }
                    else {
                        query = QString("INSERT INTO vzletbase.userTheme (userID, themeID, userThemeDateTime, statusParticipationID, eventComment) "
                                        "VALUES (%1, %2, NOW(), NULL, '%3') ")
                                .arg(userCode)
                                .arg(eventCode)
                                .arg(eventComment);

                        if (q.exec(query)) {

                            //Отправка письма на адрес электронной почты зарегистрированного участника
                            query = QString("SELECT userFIO, userEMail FROM vzletbase.dov WHERE dovID = %1 ")
                                    .arg(userCode);
                            if (q.exec(query)) {

                                QString userFIO;
                                QString userEMail;

                                while (q.next()) {
                                    userFIO = q.value(0).toString();
                                    userEMail = q.value(1).toString();
                                }


                                //Получаем текст письма
                                query = QString("SELECT eventTextEMail FROM vzletbase.theme WHERE themeID = %1 ")
                                        .arg(eventCode);
                                if (q.exec(query)) {
                                    QString textMessage;

                                    while(q.next()) {
                                        textMessage = q.value(0).toString();
                                    }

                                    //Непосредственная отправка письма
                                    QByteArray b;
                                    b.clear();

                                    QProcess p;
                                    QStringList parametres;
                                    parametres.clear();
                                    parametres.append("applyToEvent");

                                    b.clear();
                                    b.append(userEMail);
                                    parametres.append(b.toBase64()); //Адрес электронной почты пользователя

                                    b.clear();
                                    b.append(userFIO);
                                    parametres.append(b.toBase64()); //ФИО пользователя

                                    b.clear();
                                    b.append(textMessage);
                                    parametres.append(b.toBase64()); //Текст письма

                                    p.start("C:/vzlet/modules/emailNotifierVZLET.exe", parametres);
                                    p.waitForReadyRead(10 * 1000);
                                    int result = p.readAll().toInt();

                                    switch (result) {
                                    case 1:
                                        printf(QString("Вы успешно подали заявку на мероприятие! ").toLocal8Bit());
                                        break;
                                    case -1:
                                        printf("[error]");
                                        break;
                                    case 2:
                                        //Недостаточно параметров передано
                                        printf("[error]");
                                        break;
                                    }

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
            QSqlQuery q(db);
            QString query = "";
            QString result;

            int statusThemeID = 0; //Статус выбранной темы
            int eventCode = params.at(1).toInt(); //Код темы


            query = QString("SELECT THEMEID, THEMENAME, THEMECOMMENT, THEMEDATETIME, (SELECT SCIENCEDIRECTIONNAME "
                            "FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_1) AS SCIENCEDIRECTIONNAME1, "
                            "(SELECT SCIENCEDIRECTIONNAME FROM SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_2) AS SCIENCEDIRECTIONNAME2, "
                            "(SELECT SCIENCEDIRECTIONNAME FROM SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_3) AS SCIENCEDIRECTIONNAME3, "
                            "STATUSTHEMENAME,  PRIORITYDIRECTIONNAME, SCIENCEINDUSTRIALSPHERENAME "
                            "FROM THEME, STATUSTHEME, PRIORITYDIRECTION, SCIENCEINDUSTRIALSPHERE WHERE (THEMEID = %1) AND (THEME.STATUSTHEMEID = STATUSTHEME.STATUSTHEMEID) AND (THEME.PRIORITYDIRECTIONID = PRIORITYDIRECTION.PRIORITYDIRECTIONID) AND (THEME.SCIENCEINDUSTRIALSPHEREID = SCIENCEINDUSTRIALSPHERE.SCIENCEINDUSTRIALSPHEREID)  ")
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

            query = QString("SELECT USERID, USERFIO, ORGANIZATIONNAME, SUBDIVISIONLEVEL1NAME, POSTNAME, STATUSNAME  "
                            "FROM VZLETBASE.USERSTATUSORGANIZATIONSUBDIVISION, VZLETBASE.DOV, VZLETBASE.ORGANIZATION, VZLETBASE.SUBDIVISIONLEVEL1, VZLETBASE.STATUS "
                            "WHERE "
                            "(USERSTATUSORGANIZATIONSUBDIVISION.USERID IN (SELECT USERID FROM VZLETBASE.USERTHEME WHERE THEMEID = %1)) AND "
                            "(USERSTATUSORGANIZATIONSUBDIVISION.USERID = DOV.DOVID) AND "
                            "(USERSTATUSORGANIZATIONSUBDIVISION.ORGANIZATIONID = ORGANIZATION.ORGANIZATIONID) AND "
                            "(USERSTATUSORGANIZATIONSUBDIVISION.SUBDIVISIONLEVEL1ID = SUBDIVISIONLEVEL1.SUBDIVISIONLEVEL1ID) AND "
                            "(USERSTATUSORGANIZATIONSUBDIVISION.STATUSID = STATUS.STATUSID) ")
                    .arg(params.at(1));


            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"statusListCode\":%1,"
                                          "\"statusListFIO\":\"%2\","
                                          "\"statusListOrganizationName\":\"%3\","
                                          "\"statusListSubdivisionLevel1Name\":\"%4\","
                                          "\"statusListPostName\":\"%5\","
                                          "\"statusListStatusName\":\"%6\"},")
                                  .arg(q.value(0).toString())
                                  .arg(q.value(1).toString())
                                  .arg(q.value(2).toString())
                                  .arg(q.value(3).toString())
                                  .arg(q.value(4).toString())
                                  .arg(q.value(5).toString()));
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


    return 200;
}
