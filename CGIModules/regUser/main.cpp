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
#include <QRegExp>

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


//Получение всех необходимых для отправки на почту данных пользователя
QStringList getUserData(QSqlDatabase *db, const int userID) {
    QSqlQuery q(*db);

    QString query;
    QStringList userData;
    userData.clear();

    query = QString("SELECT userFIO, organizationName, subdivisionLevel1Name, statusName, userEMail "
                    "FROM vzletbase.dov, vzletbase.userStatusOrganizationSubdivision, vzletbase.organization, vzletbase.subdivisionLevel1, vzletbase.status "
                    "WHERE "
                    "dovID = %1 AND "
                    "userStatusOrganizationSubdivision.userID = dov.dovID AND "
                    "userStatusOrganizationSubdivision.organizationID = organization.organizationID  AND "
                    "userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID AND "
                    "userStatusOrganizationSubdivision.statusID = status.statusID ")
            .arg(userID);


    if (q.exec(query)) {
        while (q.next()) {
            userData.append(q.value(0).toString());
            userData.append(q.value(1).toString());
            userData.append(q.value(2).toString());
            userData.append(q.value(3).toString());
            userData.append(q.value(4).toString());
        }

    }
    return userData;
}


//Получение статуса пользователя
QString getUserStatus(QSqlDatabase *db, const int userID) {
    QString userStatusName;
    QSqlQuery q(*db);


    QString query = QString("SELECT statusName FROM status WHERE statusID IN "
                            "(SELECT statusID "
                            "FROM vzletbase.userStatusOrganizationSubdivision "
                            "WHERE "
                            "userID = %1) ").arg(userID);
    if (q.exec(query)) {
        while (q.next()) {
            userStatusName = q.value(0).toString();
        }

    }
    return userStatusName;
}

//Получение организации пользователя
QString getUserOrganization(QSqlDatabase *db, const int organizationID) {
    QString userOrganizationName;
    QSqlQuery q(*db);

    QString query = QString("SELECT organizationName FROM vzletbase.organization WHERE organizationID = %1 ").arg(organizationID);

    if (q.exec(query)) {
        while (q.next()) {
            userOrganizationName = q.value(0).toString();
        }
    }
    return userOrganizationName;
}


//Получение подразделения пользователя
QString getUserSubdivision(QSqlDatabase *db, const int subdivisionID) {
    QString userSubdivisionName;
    QSqlQuery q(*db);

    QString query = QString("SELECT subdivisionLevel1Name FROM vzletbase.subdivisionLevel1 WHERE subdivisionLevel1ID = %1 ").arg(subdivisionID);

    if (q.exec(query)) {
        while (q.next()) {
            userSubdivisionName = q.value(0).toString();
        }
    }
    return userSubdivisionName;
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

//Генерация пароля
QString randomGenerate(int countNumber) {
    QString pass;
    pass.clear();
    int j = 0;
    srand(time(0));
    for(int i = 1; i <= countNumber; i++) {
        j = rand() % 15 + 7;
        pass.append(QString::number(j));
    }
    return pass;
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    setmode(fileno(stdout),O_BINARY);
    setmode(fileno(stdin ),O_BINARY);

    QString ps = Input();
    QStringList params = ps.split("&");


    //Вывод на экран ученые степени
    if (params.at(0) == "getDegree") {
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

            query = "SELECT degreeID, degreeName FROM vzletbase.degree ";

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"degreeCode\":%1,"
                                          "\"degreeName\":\"%2\"},")
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



    //Вывод на экран списка классов/курсов ученика
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

            query = "SELECT childGradeID, childGradeName FROM vzletbase.childGrade ";

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



    //Вывод на экран ученые звания
    if (params.at(0) == "getAcademicDirection") {
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

            query = "SELECT academicDirectionID, academicDirectionName FROM vzletbase.academicDirection ";

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"academicDirectionCode\":%1,"
                                          "\"academicDirectionName\":\"%2\"},")
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
            }
            else {
                printf("[error]");
            }
        }
        else {
            printf("[error]");
        }
    }



    //Вывод на экран списка должностей
    if (params.at(0) == "getPostByOrganizationType") {
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

            int organizationTypeCode = params.at(1).toInt(); //Тип организации

            //Если это школа
            if (organizationTypeCode == 2) {
                query = QString("SELECT ORGANIZATIONPOST.POSTID, POSTNAME FROM VZLETBASE.ORGANIZATIONPOST, POST WHERE (ORGANIZATIONTYPEID = %1) AND (ORGANIZATIONPOST.POSTID = POST.POSTID) ")
                        .arg(organizationTypeCode);
            }
            else {
                query = QString("SELECT ORGANIZATIONPOST.POSTID, POSTNAME FROM VZLETBASE.ORGANIZATIONPOST, POST WHERE (ORGANIZATIONTYPEID = %1) AND (ORGANIZATIONPOST.POSTID = POST.POSTID) ")
                        .arg(organizationTypeCode);
            }

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"postCode\":%1,"
                                          "\"postName\":\"%2\"},")
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
            }
        }
        else {
            printf("[error]");
        }
    }





    //Вывод на экран списка должностей (для школ)
    if (params.at(0) == "getPostSchoolByOrganizationType") {
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

            int organizationTypeCode = params.at(1).toInt(); //Тип организации

            query = QString("SELECT ORGANIZATIONPOST.POSTID, POSTNAME FROM VZLETBASE.ORGANIZATIONPOST, VZLETBASE.POST "
                            "WHERE (ORGANIZATIONTYPEID = %1) AND (ORGANIZATIONPOST.POSTID = POST.POSTID) "
                            "AND (ORGANIZATIONPOST.POSTID IN(6, 7, 8, 22, 23, 24, 25)) ")
                    .arg(organizationTypeCode);


            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"postCode\":%1,"
                                          "\"postName\":\"%2\"},")
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
                    "WHERE "
                    "STATUSID IN (2, 3, 4) "
                    "ORDER BY STATUSNAME ASC ";


            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"statusCode\":%1,"
                                          "\"statusName\":\"%2\"},")
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





    //Вывод на экран списка статусов в системе
    if (params.at(0) == "getSchoolStatus") {
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
            int currentUserStatus = checkUserStatus(&db, userCode); //Статус текущего пользователя

            //Если координатор от школы
            if (currentUserStatus == 16) {
                query = "SELECT STATUSID, STATUSNAME FROM VZLETBASE.STATUS WHERE STATUSID IN (2, 3) ORDER BY STATUSNAME ASC "; //Выводится статус только - ученик и учитель
            }
            else {
                printf("[error]");
                a.quit();
                return (200);
            }

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"statusCode\":%1,"
                                          "\"statusName\":\"%2\"},")
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
        db = QSqlDatabase::addDatabase("QODBC", "db");
        db.setDatabaseName(QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1").arg(dbName));
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


    //Вывод на экран списка списка подразделений первого уровня по коду организации
    if (params.at(0) == "getSubdivisionByOrganizationCode") {
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

            query = QString("SELECT SUBDIVISIONLEVEL1ID, SUBDIVISIONLEVEL1NAME FROM VZLETBASE.SUBDIVISIONLEVEL1 WHERE (ORGANIZATIONID = %1)  OR (ORGANIZATIONID IS NULL) ")
                    .arg(params.at(1));

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"subdivisionLevel1Code\":%1,"
                                          "\"subdivisionLevel1Name\":\"%2\"},")
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




    //Вывод на экран списка списка подразделений первого уровня по коду организации
    if (params.at(0) == "getSubdivisionLevelBySubdivisionCode") {
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

            switch (params.at(1).toInt()) {
            case 1:
                query = QString("SELECT subdivisionLevel2ID, subdivisionLevel2Name FROM VZLETBASE.SUBDIVISIONLEVEL2 "
                                "WHERE (subdivisionLevel1ID = %1) OR (subdivisionLevel1ID IS NULL) ").arg(params.at(2));
                break;
            }

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    switch (params.at(1).toInt()) {
                    case 1:
                        result.append(QString("\"subdivisionLevel2Code\":%1,"
                                              "\"subdivisionLevel2Name\":\"%2\"},")
                                      .arg(q.value(0).toString())
                                      .arg(q.value(1).toString()));
                        break;
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
    }



    //Ввод в ситему нового пользователя
    if (params.at(0) == "setUser") {
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

            int currentUserID = QString("%1").arg(personID).toInt(); //Польватель системы (его код)

            //Проверяем - действительно ли ответственного от подразделения хочет зарегистрировать ответственный от организации
            if ((checkUserStatus(&db, currentUserID) == 6) || (checkUserStatus(&db, currentUserID) == 16)) {
                QByteArray b;
                b.append(params.at(1));

                QString userFIO = TEXT(QByteArray::fromBase64(b), v);

                b.clear();
                b.append(params.at(2));

                QString userHonoraryTitle = QByteArray::fromBase64(b);
                QString userPhone = TEXT(params.at(3), v);
                QString userMobilePhone = TEXT(params.at(4), v);

                b.clear();
                b.append(params.at(5));

                QString userEMail = QByteArray::fromBase64(b);
                QString degree = params.at(6);
                QString academicDirection = params.at(7);
                QString organization = params.at(8);
                QString status = params.at(9);

                QString subdivisionLevel1 = params.at(10);

                b.clear();
                b.append(params.at(11));

                QString post = TEXT(QByteArray::fromBase64(b), v);

                QString userPassword;
                userPassword = randomGenerate(7);

                QString childGrade = params.at(12);

                int partyRegistry = 0; //Характеристика участника (для ученика)
                if (status.toInt() == 2) {
                    partyRegistry = 1; //Активист реестра
                }

                int userID = 0; //Код пользователя, которого только что зарегистрировали

                //Запрос на ввод нового пользователя
                query = QString("INSERT INTO VZLETBASE.DOV (userHonoraryTitle, userPhone, userMobilePhone, userEMail, degreeID, academicDirectionID, userFIO, dovPassword, userRegDateTime) "
                                "VALUES "
                                "('%1', '%2', '%3', '%4', %5, %6, '%7', '%8', NOW()) ")
                        .arg(userHonoraryTitle)
                        .arg(userPhone)
                        .arg(userMobilePhone)
                        .arg(userEMail)
                        .arg(degree)
                        .arg(academicDirection)
                        .arg(userFIO)
                        .arg(userPassword);

                if (q.exec(query)) {
                    QString query1 = QString("SELECT LAST(dovID) FROM VZLETBASE.DOV WHERE "
                                             "(userFIO = '%1') AND (userPhone = '%2') AND (userMobilePhone = '%3') AND (userEMail = '%4') AND (degreeID = %5) AND (academicDirectionID = %6) ")
                            .arg(userFIO)
                            .arg(userPhone)
                            .arg(userMobilePhone)
                            .arg(userEMail)
                            .arg(degree)
                            .arg(academicDirection);


                    if (q.exec(query1)) {
                        while (q.next()) {
                            userID = q.value(0).toInt();
                        }

                        //Запрос на ввод с привязкой пользователя к организации

                        QString query2;

                        //Определение предварительного статуса (для неподтвержденных пользователей)
                        int preStatus = 0;
                        if (status.toInt() == 2) {
                            preStatus = 19;
                        }
                        else {
                            if (status.toInt() == 3) {
                                preStatus = 20;
                            }
                            else {
                                if (status.toInt() == 4) {
                                    preStatus = 21;
                                }
                            }
                        }

                        if (partyRegistry == 1) {

                            query2 = QString("INSERT INTO VZLETBASE.USERSTATUSORGANIZATIONSUBDIVISION (userID, statusID, organizationID, subdivisionLevel1ID, postName, childGradeID, partyRegistryID) VALUES "
                                             "(%1, %2, %3, %4, '%5', %6, 2) ")
                                    .arg(userID)
                                    .arg(preStatus)
                                    .arg(organization)
                                    .arg(subdivisionLevel1)
                                    .arg(post)
                                    .arg(childGrade);
                        }
                        else {
                            query2 = QString("INSERT INTO VZLETBASE.USERSTATUSORGANIZATIONSUBDIVISION (userID, statusID, organizationID, subdivisionLevel1ID, postName, childGradeID, partyRegistryID) VALUES "
                                             "(%1, %2, %3, %4, '%5', %6, NULL) ")
                                    .arg(userID)
                                    .arg(preStatus)
                                    .arg(organization)
                                    .arg(subdivisionLevel1)
                                    .arg(post)
                                    .arg(childGrade);
                        }


                        if (q.exec(query2)) {
                            //Пользователь был успешно зарегистрирован

                            QString statusName;

                            //Узнаем название статуса по коду статуса
                            query = QString("SELECT statusName FROM VZLETBASE.Status WHERE statusID = %1 ").arg(preStatus);
                            if (q.exec(query)) {
                                while (q.next()) {
                                    statusName = q.value(0).toString();
                                }

                                //Отправка ему уведомления на почту
                                QByteArray b;
                                b.clear();
                                b.append(userPassword);

                                QProcess p;
                                QStringList parametres;
                                parametres.clear();
                                parametres.append("registration");
                                parametres.append(userEMail);
                                parametres.append(QString::number(userID));
                                parametres.append(b.toBase64());

                                b.clear();
                                b.append(userFIO);

                                parametres.append(b.toBase64()); //ФИО пользователя

                                b.clear();
                                b.append(statusName);
                                parametres.append(b.toBase64()); //Статус пользователя

                                QString organizationName = getUserOrganization(&db, organization.toInt());
                                QString subdivisionLevel1Name = getUserSubdivision(&db, subdivisionLevel1.toInt());

                                b.clear();
                                b.append(organizationName); //Название организации пользователя
                                parametres.append(b.toBase64());

                                b.clear();
                                b.append(subdivisionLevel1Name); //Название подразделения пользователя
                                parametres.append(b.toBase64());

                                p.start("C:/vzlet/modules/emailNotifierVZLET.exe", parametres);
                                p.waitForReadyRead(10 * 1000);
                                int result = p.readAll().toInt();

                                switch (result) {
                                case 1:
                                    printf(QString("Поздравляем! Пользователь был успешно зарегистрированы в системе!\nЕго персональные коды доступа были отправлены на почтовый ящик %1.\nДля полноценной работы с системой ему потребуется подтвердить свое согласие на обработку персональных данных! ").arg(userEMail).toLocal8Bit());
                                    break;
                                case -1:
                                    printf(QString("Ошибка отправления кодов доступа на почтовый ящик %1. Обратитесь к администратору системы. Ваш персональный код - %2")
                                           .arg(userEMail)
                                           .arg(userID).toLocal8Bit());
                                    break;
                                case 2:
                                    //Недостаточно параметров передано
                                    printf(QString("Ошибка отправления кодов доступа на почтовый ящик %1. Обратитесь к администратору системы. Ваш персональный код - %2")
                                           .arg(userEMail)
                                           .arg(userID).toLocal8Bit());
                                    break;
                                }


                                a.quit();
                                return (200);

                            }
                            else {
                                printf(QString("Ошибка отправления кодов доступа на почтовый ящик %1. Обратитесь к администратору системы. Ваш персональный код - %2")
                                       .arg(userEMail)
                                       .arg(userID).toLocal8Bit());
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
                printf("error");
                a.quit();
                return (401);
            }
            a.quit();
            return (200);
        }
    }




    //Ввод в ситему нового пользователя - ответственного от подразделения
    if (params.at(0) == "setUserResponsibleSubdivision") {
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

            int currentUserID = QString("%1").arg(personID).toInt(); //Польватель системы (его код)


            //Проверяем - действительно ли ответственного от подразделения хочет зарегистрировать ответственный от организации
            if (checkUserStatus(&db, currentUserID) == 5) {
                QByteArray b;
                b.append(params.at(1));

                QString userFIO = TEXT(QByteArray::fromBase64(b), v);

                b.clear();
                b.append(params.at(2));

                QString userHonoraryTitle = TEXT(QByteArray::fromBase64(b), v);
                QString userPhone = params.at(3);
                QString userMobilePhone = params.at(4);

                b.clear();
                b.append(params.at(5));

                QString userEMail = TEXT(QByteArray::fromBase64(b), v);
                QString degree = params.at(6);
                QString academicDirection = params.at(7);

                int organization = params.at(8).toInt();

                int subdivisionLevel1 = params.at(9).toInt();

                b.clear();
                b.append(params.at(10));
                QString post = TEXT(QByteArray::fromBase64(b), v);

                b.clear();
                QString userPassword = randomGenerate(7); //Генерация пароля

                int userID = 0; //Код пользователя, которого только что зарегистрировали

                QString organizationName = getUserOrganization(&db, organization); //Название организации пользователя
                QString subdivisionLevel1Name = getUserSubdivision(&db, subdivisionLevel1); //Название подразделения пользователя


                //Запрос на ввод нового пользователя
                query = QString("INSERT INTO VZLETBASE.DOV (userHonoraryTitle, userPhone, userMobilePhone, userEMail, degreeID, academicDirectionID, userFIO, dovPassword, userRegDateTime) "
                                "VALUES "
                                "('%1', '%2', '%3', '%4', %5, %6, '%7', '%8', NOW()) ")
                        .arg(userHonoraryTitle)
                        .arg(userPhone)
                        .arg(userMobilePhone)
                        .arg(userEMail)
                        .arg(degree)
                        .arg(academicDirection)
                        .arg(userFIO)
                        .arg(userPassword);

                if (q.exec(query)) {
                    QString query1 = QString("SELECT LAST(dovID) FROM VZLETBASE.DOV WHERE "
                                             "(userFIO = '%1') AND (userPhone = '%2') AND (userMobilePhone = '%3') AND (userEMail = '%4') AND (degreeID = %5) AND (academicDirectionID = %6) ")
                            .arg(userFIO)
                            .arg(userPhone)
                            .arg(userMobilePhone)
                            .arg(userEMail)
                            .arg(degree)
                            .arg(academicDirection);

                    if (q.exec(query1)) {
                        while (q.next()) {
                            userID = q.value(0).toInt();
                        }

                        //Запрос на ввод с привязкой пользователя к организации
                        QString query2 = QString("INSERT INTO VZLETBASE.USERSTATUSORGANIZATIONSUBDIVISION (userID, statusID, organizationID, subdivisionLevel1ID, postName, childGradeID) VALUES "
                                                 "(%1, 22, %2, %3, '%4', 1) ")
                                .arg(userID)
                                .arg(organization)
                                .arg(subdivisionLevel1)
                                .arg(post);

                        if (q.exec(query2)) {

                            //Пользователь был успешно зарегистрирован

                            //Отправка ему уведомления на почту
                            QByteArray b;
                            b.clear();
                            b.append(userPassword);

                            QProcess p;
                            QStringList parametres;
                            parametres.clear();
                            parametres.append("registration");
                            parametres.append(userEMail);
                            parametres.append(QString::number(userID));
                            parametres.append(b.toBase64());

                            b.clear();
                            b.append(userFIO); //ФИО пользователя
                            parametres.append(b.toBase64());

                            b.clear();
                            b.append(getUserStatus(&db, userID)); //Статус арегистрированного пользователя
                            parametres.append(b.toBase64());

                            b.clear();
                            b.append(organizationName); //Название организации пользователя
                            parametres.append(b.toBase64());

                            if (subdivisionLevel1Name == "") {
                                b.clear();
                                b.append("no"); //У пользователя нет подразделения
                                parametres.append(b.toBase64());
                            }
                            else {
                                b.clear();
                                b.append(subdivisionLevel1Name); //Название подразделения пользователя
                                parametres.append(b.toBase64());
                            }

                            p.start("C:/vzlet/modules/emailNotifierVZLET.exe", parametres);
                            p.waitForReadyRead(10 * 1000);
                            int result = p.readAll().toInt();

                            switch (result) {
                            case 1:
                                printf(QString("Поздравляем!\nКоординатор от подразделения (кафедры/школы) - %1 был успешно зарегистрирован в системе!\nЕго персональные коды доступа были отправлены на почтовый ящик %2.\nДля полноценной работы с системой ему потребуется подтвердить свое согласие на обработку персональных данных! ").arg(userFIO).arg(userEMail).toLocal8Bit());
                                break;
                            case -1:
                                printf(QString("Ошибка отправления кодов доступа на почтовый ящик %1. Обратитесь к администратору системы. Персональный код зарегистрированного ответственного от подразделения - %2")
                                       .arg(userEMail)
                                       .arg(userID).toLocal8Bit());
                                break;
                            case 2:
                                //Недостаточно параметров передано
                                printf(QString("Ошибка отправления кодов доступа на почтовый ящик %1. Обратитесь к администратору системы. Персональный код зарегистрированного ответственного от подразделения - %2")
                                       .arg(userEMail)
                                       .arg(userID).toLocal8Bit());
                                break;
                            }

                            a.quit();
                            return (200);
                        }
                        else {
                            db.close();
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
                a.quit();
                return (401);
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




    //Ввод в ситему нового пользователя - ответственного от организации (вводить может только администратор)
    if (params.at(0) == "setUserResponsibleOrganization") {

        int userCode = QString("%1").arg(personID).toInt();

        //Данные может вводить только администратор
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
                QString query;

                QByteArray b;
                b.append(params.at(1));

                QString userFIO = TEXT(QByteArray::fromBase64(b), v);

                b.clear();
                b.append(params.at(2));

                QString userHonoraryTitle = TEXT(QByteArray::fromBase64(b), v);
                QString userPhone = TEXT(params.at(3), v);
                QString userMobilePhone = TEXT(params.at(4), v);

                b.clear();
                b.append(params.at(5));

                QString userEMail = TEXT(QByteArray::fromBase64(b), v);
                int degree = params.at(6).toInt();
                int academicDirection = params.at(7).toInt();
                int organization = params.at(8).toInt();

                b.clear();
                b.append(params.at(9));
                QString post = TEXT(QByteArray::fromBase64(b), v);


                int organizationType = params.at(10).toInt();


                b.clear();
                QString userPassword = randomGenerate(7); //Генерация пароля

                int userID = 0; //Код пользователя, которого только что зарегистрировали

                //Запрос на ввод нового пользователя
                query = QString("INSERT INTO VZLETBASE.DOV (userHonoraryTitle, userPhone, userMobilePhone, userEMail, degreeID, academicDirectionID, userFIO, dovPassword, userRegDateTime) "
                                "VALUES "
                                "('%1', '%2', '%3', '%4', %5, %6, '%7', '%8', NOW()) ")
                        .arg(userHonoraryTitle)
                        .arg(userPhone)
                        .arg(userMobilePhone)
                        .arg(userEMail)
                        .arg(degree)
                        .arg(academicDirection)
                        .arg(userFIO)
                        .arg(userPassword);

                if (q.exec(query)) {
                    QString query1 = QString("SELECT LAST(dovID) FROM VZLETBASE.DOV WHERE "
                                             "(userFIO = '%1') AND (userPhone = '%2') AND (userMobilePhone = '%3') AND (userEMail = '%4') AND (degreeID = %5) AND (academicDirectionID = %6) ")
                            .arg(userFIO)
                            .arg(userPhone)
                            .arg(userMobilePhone)
                            .arg(userEMail)
                            .arg(degree)
                            .arg(academicDirection);

                    if (q.exec(query1)) {
                        while (q.next()) {
                            userID = q.value(0).toInt();
                        }

                        //Запрос на ввод с привязкой пользователя к организации

                        //Если не выбрано, что регистрируется человек от организации-партнера
                        QString query2;

                        if (organizationType != 14) {
                            query2 = QString("INSERT INTO VZLETBASE.USERSTATUSORGANIZATIONSUBDIVISION (userID, statusID, organizationID, subdivisionLevel1ID, postName, childGradeID) VALUES "
                                             "(%1, 5, %2, 1, '%3', 1) ")
                                    .arg(userID)
                                    .arg(organization)
                                    .arg(post);
                        }
                        else {
                            query2 = QString("INSERT INTO VZLETBASE.USERSTATUSORGANIZATIONSUBDIVISION (userID, statusID, organizationID, subdivisionLevel1ID, postName, childGradeID) VALUES "
                                             "(%1, 17, %2, 1, '%3', 1) ")
                                    .arg(userID)
                                    .arg(organization)
                                    .arg(post);
                        }


                        if (q.exec(query2)) {

                            //Пользователь был успешно зарегистрирован

                            //Отправка ему уведомления на почту
                            QByteArray b;
                            b.clear();
                            b.append(userPassword);

                            QProcess p;
                            QStringList parametres;
                            parametres.clear();
                            parametres.append("registration");
                            parametres.append(userEMail);
                            parametres.append(QString::number(userID));
                            parametres.append(b.toBase64()); //Пароль пользователя

                            b.clear();
                            b.append(userFIO);

                            parametres.append(b.toBase64()); // ФИО пользователя

                            b.clear();
                            b.append(QString("%1").arg(getUserStatus(&db, userID)));

                            parametres.append(b.toBase64()); //Статус пользователя

                            QString organizationName = getUserOrganization(&db, organization);

                            b.clear();
                            b.append(organizationName); //Название организации пользователя
                            parametres.append(b.toBase64());

                            b.clear();
                            b.append("no"); //У пользователя нет подразделения
                            parametres.append(b.toBase64());


                            p.start("C:/vzlet/modules/emailNotifierVZLET.exe", parametres);
                            p.waitForReadyRead(10 * 1000);
                            int result = p.readAll().toInt();

                            switch (result) {
                            case 1:
                                printf(QString("Поздравляем!\nОтветственный от организации - %1 был успешно зарегистрирован в системе!\nЕго персональные коды доступа были отправлены на почтовый ящик %2").arg(userFIO).arg(userEMail).toLocal8Bit());
                                break;
                            case -1:
                                printf(QString("Ошибка отправления кодов доступа на почтовый ящик %1. Обратитесь к администратору системы. Персональный код зарегистрированного ответственного от организации - %2")
                                       .arg(userEMail)
                                       .arg(userID).toLocal8Bit());
                                break;
                            case 2:
                                //Недостаточно параметров передано
                                printf(QString("Ошибка отправления кодов доступа на почтовый ящик %1. Обратитесь к администратору системы. Персональный код зарегистрированного ответственного от организации - %2")
                                       .arg(userEMail)
                                       .arg(userID).toLocal8Bit());
                                break;
                            }

                            db.close();

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






    //Ввод в ситему новой организации (вводить может только администратор)
    if (params.at(0) == "setOrganization") {

        //Данные может вводить только администратор
        if (QString("%1").arg(personID).toInt() == 3) {
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

                QByteArray b;
                b.append(params.at(1));

                QString organizationName = TEXT(QByteArray::fromBase64(b), v); //Название организации

                b.clear();

                QString organizationType = params.at(2);

                query = QString("INSERT INTO vzletbase.Organization (organizationName, organizationTypeID, organizationNumberPPS) "
                                "VALUES "
                                "('%1', %2, 100) ")
                        .arg(organizationName)
                        .arg(organizationType);

                if (q.exec(query)) {
                    printf("ok ");
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



    //Ввод в ситему нового подразделения (вводить может только ответственный от организации)
    if (params.at(0) == "setSubdivision") {
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

            int userStatusCode = 0; //Статус пользователя в системе

            //Сначала проверим, действительно ли ответственный от организации хочет ввести подразделение
            query = QString("SELECT statusID from vzletbase.userStatusOrganizationSubdivision WHERE userID = %1 ").arg(personID);
            if (q.exec(query)) {
                while (q.next()) {
                    userStatusCode = q.value(0).toInt();
                }

                if (userStatusCode == 5) {
                    QByteArray b;
                    b.append(params.at(1));

                    QString subdivisionName = TEXT(QByteArray::fromBase64(b), v);

                    b.clear();

                    int organizationCode = params.at(2).toInt();

                    query = QString("INSERT INTO vzletbase.SubdivisionLevel1 (subdivisionLevel1Name, organizationID, subdivisionLevel1NumberPPS) "
                                    "VALUES "
                                    "('%1', %2, 100) ")
                            .arg(subdivisionName)
                            .arg(organizationCode);

                    if (q.exec(query)) {
                        printf("ok ");
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






    //Восстановление кодов доступа (забыли пароль?)
    if (params.at(0) == "recoveryPassword") {
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

            QByteArray b;
            b.append(params.at(1));

            QString userEMail = TEXT(QByteArray::fromBase64(b), v); //Адрес электронной почты пользователя

            query = QString("SELECT dovID, userFIO, dovPassword, userEMail, statusName, organizationName, subdivisionLevel1Name, userStatusOrganizationSubdivision.subdivisionLevel1ID "
                            "FROM vzletbase.dov, vzletbase.userStatusOrganizationSubdivision, vzletbase.status, vzletbase.organization, vzletbase.subdivisionLevel1 "
                            "WHERE (userEMail = '%1') AND "
                            "(userStatusOrganizationSubdivision.userID = dov.dovID) AND (userStatusOrganizationSubdivision.statusID = status.statusID) AND "
                            "(userStatusOrganizationSubdivision.organizationID = organization.organizationID) AND "
                            "(userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID) ")
                    .arg(userEMail.trimmed());

            b.clear();

            int countSuccess = 0; //количество успешных отправок сообщений пользователю

            QStringList parametres;

            if (q.exec(query)) {
                while (q.next()) {

                    parametres.clear();
                    parametres.append("recoveryAccessCodes");

                    b.clear();
                    b.append(userEMail.trimmed()); //Адрес электронной почты пользователя
                    parametres.append(b.toBase64());

                    b.clear();
                    b.append(QString("%1").arg(q.value(1).toString())); // ФИО пользователя
                    parametres.append(b.toBase64());

                    b.clear();
                    b.append(QString("%1").arg(q.value(4).toString())); // Статус пользователя
                    parametres.append(b.toBase64());

                    b.clear();
                    b.append(QString("%1@dov").arg(q.value(0).toString())); // Логин пользователя
                    parametres.append(b.toBase64());

                    b.clear();
                    b.append(QString("%1").arg(q.value(2).toString())); // Пароль пользователя
                    parametres.append(b.toBase64());

                    b.clear();
                    b.append(QString("%1").arg(q.value(5).toString())); // Организация пользователя
                    parametres.append(b.toBase64());

                    b.clear();
                    if (q.value(7).toInt() != 1) {
                        b.append(QString("%1").arg(q.value(6).toString())); // Подразделение пользователя
                        parametres.append(b.toBase64());
                    }
                    else {
                        b.append("no"); // У пользователя нет подразделения
                        parametres.append(b.toBase64());
                    }

                    //Отправка уведомлений
                    QProcess p;
                    p.start("C:/vzlet/modules/emailNotifierVZLET.exe", parametres);
                    p.waitForReadyRead(70 * 1000);
                    int result = p.readAll().toInt();

                    switch (result) {
                    case 1:
                        countSuccess++;
                        break;
                    }
                }

                db.close();

                if (countSuccess > 0) {
                    printf(QString("Коды доступа были отправлены на адрес электронной почты, указанный вами! ").toLocal8Bit());
                    a.quit();
                    return (200);
                }
                else {
                    printf(QString("Ошибка отправки кодов доступа! \n"
                                   "Повторите попытку, проверив правильность ввода адреса электронной почты или обратитесь по адресу admin@vzletsamara.ru ")
                           .toLocal8Bit());
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






    //Подтверждение согласия на обработку персональных данных
    if (params.at(0) == "setConfirm") {
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

            int currentUserCode = QString("%1").arg(personID).toInt(); //Пользователь системы (его код)
            int preStatusCode = checkUserStatus(&db, currentUserCode);

            //Если вошел неподтвержденный пользователь
            if ((preStatusCode == 19) || (preStatusCode == 20) || (preStatusCode == 21) || (preStatusCode == 22) || (preStatusCode == 23)) {

                int statusCode = 0; //Будущий реальный статус пользователя

                //Обратное преобразование статуса из неподтвержденного в подтвержденный
                switch (preStatusCode) {
                case 19:
                    statusCode = 2; //Ученик
                    break;

                case 20:
                    statusCode = 3; //Учитель
                    break;

                case 21:
                    statusCode = 4; //Консультант
                    break;

                case 22:
                    statusCode = 6; //Координатор от кафедры/школы
                    break;

                case 23:
                    statusCode = 17; //Координатор от организации-партнера
                    break;
                }


                QString userPassword;
                userPassword = randomGenerate(7);

                //Запрос на изменение пароля пользователя (на полноценный)
                query = QString("UPDATE vzletbase.dov "
                                "SET dovPassword = '%1' "
                                "WHERE dovID = %2 ")
                        .arg(userPassword)
                        .arg(currentUserCode);

                if (q.exec(query)) {
                    db.commit();

                    //Обновление статуса пользователя (превращение в подтвержденного), а также регистрация даты и времени, когда пользователь дал согласие
                    query = QString("UPDATE vzletbase.userStatusOrganizationSubdivision "
                                    "SET statusID = %1, userConfirmDateTime = NOW() "
                                    "WHERE userID = %2 ")
                            .arg(statusCode)
                            .arg(currentUserCode);

                    if (q.exec(query)) {

                        //Получение всех необходимых для отправки данных о пользователе
                        QStringList userData = getUserData(&db, currentUserCode);


                        //Отправка ему уведомления на почту
                        QByteArray b;
                        b.clear();
                        b.append(userPassword);

                        QProcess p;
                        QStringList parametres;
                        parametres.clear();
                        parametres.append("confirmation");
                        parametres.append(userData.at(4));
                        parametres.append(QString::number(currentUserCode)); //Логин пользователя
                        parametres.append(b.toBase64());

                        b.clear();
                        b.append(userData.at(0));

                        parametres.append(b.toBase64()); //ФИО пользователя

                        b.clear();
                        b.append(userData.at(3));
                        parametres.append(b.toBase64()); //Статус пользователя

                        b.clear();
                        b.append(userData.at(1)); //Название организации пользователя
                        parametres.append(b.toBase64());

                        b.clear();
                        b.append(userData.at(2)); //Название подразделения пользователя
                        parametres.append(b.toBase64());

                        p.start("C:/vzlet/modules/emailNotifierVZLET.exe", parametres);
                        p.waitForReadyRead(10 * 1000);
                        int result = p.readAll().toInt();

                        switch (result) {
                        case 1:
                            printf(QString("Вы подтвердили свое согласие на обработку персональных данных. \n"
                                           "На ваш адрес электронной почты были отправлены коды доступа, включающие полноценный пароль для работы с системой. ").toLocal8Bit());
                            break;
                        case -1:
                            printf(QString("Ошибка отправления кодов доступа на почтовый ящик %1. Обратитесь к администратору системы. Ваш персональный код - %2")
                                   .arg(userData.at(4))
                                   .arg(currentUserCode).toLocal8Bit());
                            break;
                        case 2:
                            //Недостаточно параметров передано
                            printf(QString("Ошибка отправления кодов доступа на почтовый ящик %1. Обратитесь к администратору системы. Ваш персональный код - %2")
                                   .arg(userData.at(4))
                                   .arg(currentUserCode).toLocal8Bit());
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



    a.quit();
    return 200;
}
