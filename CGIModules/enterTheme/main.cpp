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
QString v = " 0123456789qwertyuiopasdfghjklzxcvbnm.,йцукенгшщзхъфывапролджэячсмитьбю@№;%?()-+_";

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


//Проверка - относится ли конкретная тема к конкретному консультанту
bool checkConsultantTheme(QSqlDatabase *db, const int userID, const int themeID) {

    int isConsultantTheme = 0;
    QSqlQuery q(*db);
    if (q.exec(QString("SELECT userID "
                       "FROM vzletbase.userTheme "
                       "WHERE "
                       "(userID = %1) AND (themeID = %2) ")
               .arg(userID)
               .arg(themeID))) {
        while (q.next()) {
            isConsultantTheme = q.value(0).toInt();
        }

        if (isConsultantTheme == userID) {
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



//Проверка - относится ли конкретная тема к конкретному учителю/ученику
bool checkUserTheme(QSqlDatabase *db, const int userID, const int themeID) {

    int isUserTheme = 0;
    QSqlQuery q(*db);

    if (q.exec(QString("SELECT userID "
                       "FROM vzletbase.userTheme "
                       "WHERE "
                       "(userID = %1) AND (themeID = %2) ")
               .arg(userID)
               .arg(themeID))) {
        while (q.next()) {
            isUserTheme = q.value(0).toInt();
        }

        if (isUserTheme == userID) {
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


//Проверка - есть ли такой консультант/ученик
bool checkUserExist(QSqlDatabase *db, const int userID, const int statusID) {

    int isUserExist = 0;
    QSqlQuery q(*db);

    if (q.exec(QString("SELECT userID "
                       "FROM vzletbase.userStatusOrganizationSubdivision  "
                       "WHERE "
                       "(userID = %1) AND (statusID = %2) ")
               .arg(userID)
               .arg(statusID))) {
        while (q.next()) {
            isUserExist = q.value(0).toInt();
        }

        if (isUserExist == userID) {
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


//Проверка - разрешил ли учитель по конкретной теме отправку работы на конкурс
bool checkAllowProjectToSendConkurs(QSqlDatabase *db, const int themeID)
{
    QSqlQuery q(*db);

    if (q.exec(QString("SELECT sendContentAllow "
                       "FROM vzletbase.theme "
                       "WHERE themeID = %1 ").arg(themeID))) {
        while (q.next()) {
            return q.value(0).toBool();
        }
    }
    else {
        return false;
    }
}

//Есть ли по теме загруженная в систему работа?
int checkUploadFileByTheme(QSqlDatabase *db, const int themeID) {
    int themeCode = 0;
    QSqlQuery q(*db);
    if (q.exec(QString("SELECT themeID FROM vzletbase.projectFile WHERE themeID = %1").arg(themeID))) {
        while (q.next()) {
            themeCode = q.value(0).toInt();
        }

        if (themeCode == 0) {
            return true;
        }
        else {
            return false;
        }
    }
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



    //Вывод на экран списка тем для редактирования, привязанных именно к вошедшему пользователю - учителю или консультанту
    if (params.at(0) == "getThemesForEdit") {
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

            //Если вошел учитель или консультант
            if (currentUserStatus == 3 || currentUserStatus == 4) {

                QSqlQuery q(db);
                QString query = "";
                QString result = "[";

                query = QString("SELECT "
                                "themeName, themeComment, themeDateTime, "
                                "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_1) AS SCIENCEDIRECTIONNAME1, "
                                "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_2) AS SCIENCEDIRECTIONNAME, "
                                "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_3) AS SCIENCEDIRECTIONNAME3, "
                                "statusThemeName, sectionName, priorityDirectionName, scienceIndustrialSphereName "
                                " "
                                "FROM vzletbase.theme, vzletbase.statusTheme, vzletbase.section, vzletbase.priorityDirection, vzletbase.scienceIndustrialSphere "
                                "WHERE "
                                "(themeID IN (SELECT themeID FROM userTheme WHERE userID = %1)) AND "
                                "(theme.statusThemeID = statusTheme.statusThemeID) AND "
                                "(theme.sectionID = section.sectionID) AND "
                                "(theme.priorityDirectionID = priorityDirection.priorityDirectionID) AND "
                                "(theme.scienceIndustrialSphereID = scienceIndustrialSphere.scienceIndustrialSphereID) ")
                        .arg(userCode);

                if (q.exec(query)) {
                    while (q.next()) {
                        result.append("{");
                        result.append(QString("\"themeName\":%1,"
                                              "\"themeComment\":\"%2\""
                                              "\"themeScienceDirection1\":\"%3\","
                                              "\"themeScienceDirection2\":\"%4\","
                                              "\"themeScienceDirection3\":\"%5\","
                                              "\"themeStatusTheme\":\"%6\","
                                              "\"themeSection\":\"%7\""
                                              "\"themePriorityDirection\":\"%8\","
                                              "\"themeScienceIndustrialSphere\":\"%9\"},")
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
            a.quit();
            return (401);
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


    //Вывод на экран списка приоритетных направлений развития науки и техники
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




    //Вывод на экран списка статусов тем
    if (params.at(0) == "getStatusThemeByTheme") {
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
            int currentStatusThemeCode = 0;

            query = QString("SELECT statusThemeID from Theme where themeID = %1 ")
                    .arg(params.at(1));

            if (q.exec(query)) {

                while (q.next()) {
                    currentStatusThemeCode = q.value(0).toInt();
                }

                query = "SELECT statusThemeID, statusThemeName FROM vzletbase.statusTheme WHERE statusThemeID <> 1 ";

                if (q.exec(query)) {
                    while (q.next()) {
                        result.append("{");
                        result.append(QString("\"statusThemeCode\":%1,").arg(q.value(0).toString()));
                        if (q.value(0).toInt() == currentStatusThemeCode) {
                            result.append("\"current\":1,");
                        }
                        else {
                            result.append("\"current\":0,");
                        }

                        result.append(QString("\"statusThemeName\":\"%2\"},")
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





    //Вывод на экран списка уровней взаимодействия (со школьником, учителем, консультантом)
    if (params.at(0) == "getLevelInteraction") {
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

            query = "SELECT levelInteractionID, levelInteractionName FROM vzletbase.levelInteraction ";

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"levelInteractionCode\":%1,"
                                          "\"levelInteractionName\":\"%2\"},")
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




    //Вывод на экран текущего уровня взаимодействия, указанный консультантом
    if (params.at(0) == "getLevelInteractionByUser") {
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

            //Если вошел консультант/учитель/ученик
            if (currentUserStatus < 2 && currentUserStatus > 5) {
                a.quit();
                return (401);
            }
            else {
                QSqlQuery q(db);
                QString query = "";
                QString result = "{";

                //Получаем текущй уровень взаимодействия с пользователем
                bool ok = true;
                int userWho = userCode; //Кто оценил
                int themeCode = params.at(1).toInt(&ok);

                if (ok) {
                    //Проверяем, числится ли пользователь за запрашиваемой темой
                    if (checkUserTheme(&db, userCode, themeCode)) {


                        //Получаем статус пользователя
                        int userStatus = checkUserStatus(&db, userCode);


                        //По коду темы и статусу пользователя определим ее участников - учителя и консультанта

                        //Если консультант
                        if (userStatus == 4) {
                            query = QString("SELECT DISTINCT userTheme.userID, statusID FROM vzletbase.userTheme, vzletbase.userStatusOrganizationSubdivision WHERE (themeID = %1) AND "
                                            "( "
                                            "userTheme.userID IN (SELECT userStatusOrganizationSubdivision.userID FROM vzletbase.userStatusOrganizationSubdivision WHERE (statusID = 2) OR (statusID = 3)) "
                                            ") AND (userStatusOrganizationSubdivision.userID = userTheme.userID) ")
                                    .arg(themeCode);
                        }

                        //Если учитель
                        if (userStatus == 3) {
                            query = QString("SELECT DISTINCT userTheme.userID, statusID FROM vzletbase.userTheme, vzletbase.userStatusOrganizationSubdivision WHERE (themeID = %1) AND "
                                            "( "
                                            "userTheme.userID IN (SELECT userStatusOrganizationSubdivision.userID FROM vzletbase.userStatusOrganizationSubdivision WHERE (statusID = 2) OR (statusID = 4)) "
                                            ") AND (userStatusOrganizationSubdivision.userID = userTheme.userID) ")
                                    .arg(themeCode);
                        }

                        //Если ученик
                        if (userStatus == 2) {
                            query = QString("SELECT DISTINCT userTheme.userID, statusID FROM vzletbase.userTheme, vzletbase.userStatusOrganizationSubdivision WHERE (themeID = %1) AND "
                                            "( "
                                            "userTheme.userID IN (SELECT userStatusOrganizationSubdivision.userID FROM vzletbase.userStatusOrganizationSubdivision WHERE (statusID = 3) OR (statusID = 4)) "
                                            ") AND (userStatusOrganizationSubdivision.userID = userTheme.userID) ")
                                    .arg(themeCode);
                        }



                        //Те, кого оценили
                        int teacherCode, childCode, consultantCode = 0;

                        if (q.exec(query)) {
                            while (q.next()) {

                                //Если ученик
                                if (userStatus == 2) {
                                    if (q.value(1).toInt() == 3) {
                                        teacherCode = q.value(0).toInt();
                                    }

                                    if (q.value(1).toInt() == 4) {
                                        consultantCode = q.value(0).toInt();
                                    }
                                }

                                //Если учитель
                                if (userStatus == 3) {
                                    if (q.value(1).toInt() == 2) {
                                        childCode = q.value(0).toInt();
                                    }

                                    if (q.value(1).toInt() == 4) {
                                        consultantCode = q.value(0).toInt();
                                    }
                                }

                                //Если консультант
                                if (userStatus == 4) {
                                    if (q.value(1).toInt() == 2) {
                                        childCode = q.value(0).toInt();
                                    }

                                    if (q.value(1).toInt() == 3) {
                                        teacherCode = q.value(0).toInt();
                                    }
                                }
                            }




                            //Если ученик
                            if (userStatus == 2) {
                                query = QString("SELECT LAST(levelInteractionID) FROM vzletbase.interaction WHERE (userIDWho = %1) AND (userIDWhom = %2) ")
                                        .arg(userWho)
                                        .arg(teacherCode);

                                if (q.exec(query)) {
                                    while (q.next()) {
                                        result.append(QString("\"levelInteractionTeacherCode\":%1,")
                                                      .arg(q.value(0).toString()));
                                    }

                                    query = QString("SELECT LAST(levelInteractionID) FROM vzletbase.interaction WHERE (userIDWho = %1) AND (userIDWhom = %2) ")
                                            .arg(userWho)
                                            .arg(consultantCode);

                                    if (q.exec(query)) {
                                        while (q.next()) {
                                            result.append(QString("\"levelInteractionConsultantCode\":%2")
                                                          .arg(q.value(0).toString()));
                                        }

                                        result.append("}");

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


                            //Если учитель
                            if (userStatus == 3) {
                                query = QString("SELECT LAST(levelInteractionID) FROM vzletbase.interaction WHERE (userIDWho = %1) AND (userIDWhom = %2) ")
                                        .arg(userWho)
                                        .arg(childCode);

                                if (q.exec(query)) {
                                    while (q.next()) {
                                        result.append(QString("\"levelInteractionChildCode\":%1,")
                                                      .arg(q.value(0).toString()));
                                    }

                                    query = QString("SELECT LAST(levelInteractionID) FROM vzletbase.interaction WHERE (userIDWho = %1) AND (userIDWhom = %2) ")
                                            .arg(userWho)
                                            .arg(consultantCode);

                                    if (q.exec(query)) {
                                        while (q.next()) {
                                            result.append(QString("\"levelInteractionConsultantCode\":%2")
                                                          .arg(q.value(0).toString()));
                                        }

                                        result.append("}");

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

                            //Если консультант
                            if (userStatus == 4) {
                                query = QString("SELECT LAST(levelInteractionID) FROM vzletbase.interaction WHERE (userIDWho = %1) AND (userIDWhom = %2) ")
                                        .arg(userWho)
                                        .arg(teacherCode);

                                if (q.exec(query)) {
                                    while (q.next()) {
                                        result.append(QString("\"levelInteractionTeacherCode\":%1,")
                                                      .arg(q.value(0).toString()));
                                    }

                                    query = QString("SELECT LAST(levelInteractionID) FROM vzletbase.interaction WHERE (userIDWho = %1) AND (userIDWhom = %2) ")
                                            .arg(userWho)
                                            .arg(childCode);

                                    if (q.exec(query)) {
                                        while (q.next()) {
                                            result.append(QString("\"levelInteractionChildCode\":%2")
                                                          .arg(q.value(0).toString()));
                                        }

                                        result.append("}");

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
                        else {
                            printf("[error]");
                            a.quit();
                            return (200);
                        }

                    }
                    else {
                        printf(QString("Невозможно получить данные не по той теме, к которой Вы прикреплены!!! ").toLocal8Bit());
                        a.quit();
                        return (200);
                    }
                }
                else {
                    printf(QString("Проверьте код темы!!! ").toLocal8Bit());
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






    //Вывод на экран списка стадий выполнения тем (для консультанта)
    if (params.at(0) == "getStageExecuteByThemeConsultant") {
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

            //Если консультант
            if (currentUserStatus == 4) {

                QSqlQuery q(db);
                QString query = "";
                QString result = "[";
                int currentStageExecuteID = 0;
                bool ok = true;
                int themeCode = params.at(1).toInt(&ok);


                if (ok) {

                    //Получаем последнюю запись с состоянием текущего этапа выполнения от консультанта
                    query = QString("SELECT stageExecuteID "
                                    "FROM vzletbase.interaction "
                                    "WHERE userIDWho = %1 AND themeID = %2 ")
                            .arg(userCode)
                            .arg(themeCode);

                    if (q.exec(query)) {
                        while (q.next()) {
                            currentStageExecuteID = q.value(0).toInt();
                        }

                        query = "SELECT stageExecuteID, stageExecuteName FROM vzletbase.stageExecute "
                                "WHERE (stageExecuteID BETWEEN 1 AND 16) OR  stageExecuteID = 22 "
                                "ORDER BY stageExecuteName ASC ";

                        if (q.exec(query)) {
                            while (q.next()) {
                                result.append("{");
                                result.append(QString("\"stageExecuteCode\":%1,")
                                              .arg(q.value(0).toString()));

                                if (q.value(0).toInt() == currentStageExecuteID) {
                                    result.append("\"current\":1,");
                                }
                                else {
                                    result.append("\"current\":0,");
                                }

                                result.append(QString("\"stageExecuteName\":\"%1\"},")
                                              .arg(q.value(1).toString()));
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
                else {
                    printf(QString("Проверьте правильность кода темы! ").toLocal8Bit());
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



    //Вывод на экран списка стадий выполнения тем (для учителя)
    if (params.at(0) == "getStageExecuteByThemeTeacher") {
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
            int currentStageExecuteID = 0;

            int teacherCode = QString("%1").arg(personID).toInt();
            //int teacherCode = 45;

            //Получаем код текущей стадии проекта
            query = QString("SELECT LAST(STAGEEXECUTEID) FROM VZLETBASE.INTERACTION WHERE (USERIDWHO = %1) AND (THEMEID = %2) ")
                    .arg(teacherCode)
                    .arg(params.at(1));

            if (q.exec(query)) {
                while (q.next()) {
                    currentStageExecuteID = q.value(0).toInt();
                }

                query = "SELECT stageExecuteID, stageExecuteName FROM vzletbase.stageExecute "
                        "WHERE (stageExecuteID BETWEEN 1 AND 16) OR  stageExecuteID = 22 "
                        "ORDER BY stageExecuteName ASC ";

                if (q.exec(query)) {
                    while (q.next()) {
                        result.append("{");
                        result.append(QString("\"stageExecuteCode\":%1,")
                                      .arg(q.value(0).toString()));

                        if (q.value(0).toInt() == currentStageExecuteID) {
                            result.append("\"current\":1,");
                        }
                        else {
                            result.append("\"current\":0,");
                        }

                        result.append(QString("\"stageExecuteName\":\"%1\"},")
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


    //Вывод на экран списка стадий выполнения тем (для ученика)
    if (params.at(0) == "getStageExecuteByThemeChild") {
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
            int currentStageExecuteID = 0;

            int childCode = QString("%1").arg(personID).toInt();

            //Получаем код текущей стадии проекта
            query = QString("SELECT LAST(STAGEEXECUTEID) FROM VZLETBASE.INTERACTION WHERE (USERIDWHO = %1) AND (THEMEID = %2) ")
                    .arg(childCode)
                    .arg(params.at(1));

            if (q.exec(query)) {
                while (q.next()) {
                    currentStageExecuteID = q.value(0).toInt();
                }

                query = "SELECT stageExecuteID, stageExecuteName FROM vzletbase.stageExecute "
                        "WHERE (stageExecuteID BETWEEN 1 AND 16) OR  stageExecuteID = 22 "
                        "ORDER BY stageExecuteName ASC ";


                if (q.exec(query)) {
                    while (q.next()) {
                        result.append("{");
                        result.append(QString("\"stageExecuteCode\":%1,")
                                      .arg(q.value(0).toString()));

                        if (q.value(0).toInt() == currentStageExecuteID) {
                            result.append("\"current\":1,");
                        }
                        else {
                            result.append("\"current\":0,");
                        }

                        result.append(QString("\"stageExecuteName\":\"%1\"},")
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



    //Вывод на экран планового графика выполнения индивидуального проекта в 2015/2016 годах
    if (params.at(0) == "getPlannedShedule") {
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

            //Получаем план-график
            query = "SELECT timeSheduleID, stageExecuteName, timeSheduleDateExecute "
                    "FROM vzletbase.timeShedule, vzletbase.stageExecute "
                    "WHERE "
                    "(timeShedule.stageExecuteID = stageExecute.stageExecuteID) ";

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"timeSheduleCode\":%1,"
                                          "\"timeSheduleStageName\":\"%2\","
                                          "\"timeSheduleDateExecute\":\"%3\"},")
                                  .arg(q.value(0).toString())
                                  .arg(q.value(1).toString())
                                  .arg(QDate::fromString(q.value(2).toString().left(10),"yyyy-MM-dd").toString("dd.MM.yyyy")));

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








    //Вывод на экран списка желательных университетов
    if (params.at(0) == "getUnivDerisable") {
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

            query = "SELECT organizationID, organizationName FROM vzletbase.organization WHERE (organizationTypeID = 3) OR (organizationTypeID IS NULL) ";

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"univDerisableCode\":%1,"
                                          "\"univDerisableName\":\"%2\"},")
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







    //Ввод новой темы
    if (params.at(0) == "enterTheme") {
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

            //Если вошел консультант
            if (currentUserStatus != 4) {
                a.quit();
                return (401);
            }
            else {
                QSqlQuery q(db);
                QString query = "";

                QByteArray b;
                b.append(params.at(1));

                QString themeName = TEXT(QByteArray::fromBase64(b), v);
                b.clear();
                b.append(params.at(2));

                QString comment = TEXT(QByteArray::fromBase64(b), v);

                int priorityDirection = params.at(3).toInt();
                int section = params.at(4).toInt();
                int scienceIndustrialSphere = params.at(5).toInt();


                int scienceDirection1 = params.at(6).toInt();
                int scienceDirection2 = params.at(7).toInt();
                int scienceDirection3 = params.at(8).toInt();

                query = QString("INSERT INTO vzletbase.Theme "
                                "(themeName, themeComment, themeDateTime, scienceDirectionID_1, "
                                "scienceDirectionID_2, scienceDirectionID_3, priorityDirectionID, sectionID, scienceIndustrialSphereID) "
                                "VALUES "
                                "('%1', '%2', NOW(), %3, %4, %5, %6, %7, %8) ")
                        .arg(themeName)
                        .arg(comment)
                        .arg(scienceDirection1)
                        .arg(scienceDirection2)
                        .arg(scienceDirection3)
                        .arg(priorityDirection)
                        .arg(section)
                        .arg(scienceIndustrialSphere);


                if (q.exec(query)) {
                    //Получаем код только что введенной темы
                    query = QString("SELECT THEMEID FROM VZLETBASE,THEME WHERE "
                                    "(themeName = '%1') AND "
                                    "(themeComment = '%2') AND "
                                    "(scienceDirectionID_1 = %3) AND "
                                    "(scienceDirectionID_2 = %4) AND "
                                    "(scienceDirectionID_3 = %5) AND "
                                    "(priorityDirectionID = %6) AND "
                                    "(sectionID = %7) AND "
                                    "(scienceIndustrialSphereID = %8) ")
                            .arg(themeName)
                            .arg(comment)
                            .arg(scienceDirection1)
                            .arg(scienceDirection2)
                            .arg(scienceDirection3)
                            .arg(priorityDirection)
                            .arg(section)
                            .arg(scienceIndustrialSphere);

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
                            printf("Ok");
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










    //Ввод новой темы (Ввод запросов на консультирование) - учителем
    if (params.at(0) == "enterRequestByTeacher") {
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

            //Если вошел учитель
            if (currentUserStatus != 3) {
                a.quit();
                return (401);
            }
            else {
                QSqlQuery q(db);
                QString query = "";

                QByteArray b;
                b.append(params.at(1));

                QString themeName = TEXT(QByteArray::fromBase64(b), v);

                b.clear();
                b.append(params.at(2));

                QString comment = TEXT(QByteArray::fromBase64(b), v);

                int priorityDirection = params.at(3).toInt();
                int section = params.at(4).toInt();
                int scienceIndustrialSphere = params.at(5).toInt();

                int scienceDirection1 = params.at(6).toInt();
                int scienceDirection2 = params.at(7).toInt();
                int scienceDirection3 = params.at(8).toInt();

                int childCode = params.at(9).toInt(); //Код ученика

                b.clear();
                b.append(params.at(10));

                QString infoByTeacher = TEXT(QByteArray::fromBase64(b), v); //Кратко об учителе

                b.clear();
                b.append(params.at(11));

                QString infoByChild = TEXT(QByteArray::fromBase64(b), v); //Кратко об ученике


                int univDerisable = params.at(12).toInt(); //Желательный университет

                //Код ученика (имеющийся в базе)
                int childCodeExist = 0;

                //Сначала проверим, есть ли такой код ученика
                query = QString("SELECT userID "
                                "FROM vzletbase.userStatusOrganizationSubdivision "
                                "WHERE "
                                "(userID = %1) AND (statusID = 2) ")
                        .arg(childCode);

                if (q.exec(query)) {
                    while (q.next()) {
                        childCodeExist = q.value(0).toInt();
                    }

                    if (childCodeExist != childCode) {
                        printf("Вы ввели не код ученика, или такого ученика не существует! ");
                        a.quit();
                        return (200);
                    }
                    else {
                        query = QString("INSERT INTO vzletbase.Theme "
                                        "(themeName, themeComment, themeDateTime, scienceDirectionID_1, "
                                        "scienceDirectionID_2, scienceDirectionID_3, priorityDirectionID, sectionID, scienceIndustrialSphereID, "
                                        "themeAboutTutor, themeAboutChild, statusThemeID, organizationID, themeDateTimeUpdateStatus) "
                                        "VALUES "
                                        "('%1', '%2', NOW(), %3, %4, %5, %6, %7, %8, '%9', '%10', 9, %11, NOW()) ")
                                .arg(themeName)
                                .arg(comment)
                                .arg(scienceDirection1)
                                .arg(scienceDirection2)
                                .arg(scienceDirection3)
                                .arg(priorityDirection)
                                .arg(section)
                                .arg(scienceIndustrialSphere)
                                .arg(infoByTeacher)
                                .arg(infoByChild)
                                .arg(univDerisable);


                        if (q.exec(query)) {
                            //Получаем код только что введенной темы
                            query = QString("SELECT THEMEID FROM THEME WHERE "
                                            "(themeName = '%1') AND "
                                            "(themeComment = '%2') AND "
                                            "(scienceDirectionID_1 = %3) AND "
                                            "(scienceDirectionID_2 = %4) AND "
                                            "(scienceDirectionID_3 = %5) AND "
                                            "(priorityDirectionID = %6) AND "
                                            "(sectionID = %7) AND "
                                            "(scienceIndustrialSphereID = %8) "
                                            "AND (themeAboutTutor = '%9') AND "
                                            "(themeAboutChild = '%10') AND "
                                            "(organizationID = %11) ")
                                    .arg(themeName)
                                    .arg(comment)
                                    .arg(scienceDirection1)
                                    .arg(scienceDirection2)
                                    .arg(scienceDirection3)
                                    .arg(priorityDirection)
                                    .arg(section)
                                    .arg(scienceIndustrialSphere)
                                    .arg(infoByTeacher)
                                    .arg(infoByChild)
                                    .arg(univDerisable);

                            if (q.exec(query)) {
                                int themeCode = 0;
                                while (q.next()) {
                                    themeCode = q.value(0).toInt();
                                }

                                //Прикрепление учителя
                                query = QString("INSERT INTO VZLETBASE.USERTHEME (userID, themeID, userThemeDateTime, statusParticipationID) VALUES "
                                                "(%1, %2, NOW(), NULL) ")
                                        .arg(userCode)
                                        .arg(themeCode);

                                if (q.exec(query)) {
                                    //Прикрепление ученика
                                    query = QString("INSERT INTO VZLETBASE.USERTHEME (userID, themeID, userThemeDateTime, statusParticipationID) VALUES "
                                                    "(%1, %2, NOW(), NULL) ")
                                            .arg(childCode)
                                            .arg(themeCode);

                                    if (q.exec(query)) {

                                        //Получаем ФИО учителя и его адрес электронной почты
                                        QString teacherFIO, teacherEMail;

                                        query = QString("SELECT userFIO, userEMail FROM vzletbase.dov WHERE dovID = %1 ").arg(userCode);
                                        if (q.exec(query)) {
                                            while (q.next()) {
                                                teacherFIO = q.value(0).toString();
                                                teacherEMail = q.value(1).toString();
                                            }


                                            //Получаем ФИО ученика и его адрес электронной почты
                                            QString childFIO, childEMail;

                                            query = QString("SELECT userFIO, userEMail FROM vzletbase.dov WHERE dovID = %1 ").arg(childCode);
                                            if (q.exec(query)) {
                                                while (q.next()) {
                                                    childFIO = q.value(0).toString();
                                                    childEMail = q.value(1).toString();
                                                }
                                            }
                                            else {
                                                printf("[error]");
                                                a.quit();
                                                return (200);
                                            }


                                            QProcess p;
                                            QStringList parametres;
                                            parametres.clear();

                                            parametres.append("requestAdvice");

                                            b.clear();
                                            b.append(teacherFIO); // ФИО учителя
                                            parametres.append(b.toBase64());

                                            b.clear();
                                            b.append(childFIO); // ФИО ученика
                                            parametres.append(b.toBase64());

                                            b.clear();
                                            b.append(teacherEMail); // EMail учителя
                                            parametres.append(b.toBase64());

                                            b.clear();
                                            b.append(childEMail); // EMail учителя
                                            parametres.append(b.toBase64());

                                            b.clear();
                                            b.append(themeName); // Название запроса
                                            parametres.append(b.toBase64());


                                            //Отправка уведомлений
                                            p.start("C:/vzlet/modules/emailNotifierVZLET.exe", parametres);
                                            p.waitForReadyRead(20 * 1000);
                                            int result = p.readAll().toInt();

                                            switch (result) {
                                            case 1:
                                                printf(QString("Ваш запрос успешно принят! ").toLocal8Bit());
                                                a.quit();
                                                return (200);
                                                break;
                                            case -1:
                                                printf("error ");
                                                a.quit();
                                                return (200);
                                                break;

                                            default:
                                                printf("error sent e-mail");
                                                a.quit();
                                                return (200);
                                            }
                                        }
                                        else {
                                            printf("[error]");
                                            a.quit();
                                            return (200);
                                        }


                                        printf("Ok");
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




    //Получить статус в системе (для всех участников Программы)
    if (params.at(0) == "getStatusForAllUsers") {
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

            query = "SELECT STATUSID, STATUSNAME FROM VZLETBASE.STATUS WHERE STATUSID <> 1 "; //Выводится статус только - ученик, учитель и консультант

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













    //Получить статус в системе
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

            query = "SELECT STATUSID, STATUSNAME FROM VZLETBASE.STATUS WHERE STATUSID IN (2, 3, 4, 19, 20, 21) "; //Выводится статус только - ученик, учитель и консультант (в том числе подтвержденные)

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




    //Удалить темы из предлагаемых
    if (params.at(0) == "removeFromProposed") {
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

            query = QString("UPDATE vzletbase.Theme "
                            "SET statusThemeID = 5, themeDateTimeUpdateStatus = NOW() "
                            "WHERE themeID = %1 ")
                    .arg(params.at(1));

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
            printf("[error]");
            a.quit();
            return (200);
        }
    }




    //Удалить темы из предлагаемых
    if (params.at(0) == "getConsultantNote") {
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

            query = QString("SELECT CONSULTANTNOTE FROM VZLETBASE.THEME WHERE THEMEID = %1 ")
                    .arg(params.at(1));

            if (q.exec(query)) {
                while (q.next()) {
                    result.append(QString("{\"consultantNote\":\"%1\"}").arg(q.value(0).toString()));
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




    //Консультант оценил взаимодействие с учеником и учителем
    if (params.at(0) == "setInteractionByConsultant") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {

            int consultantCode = QString("%1").arg(personID).toInt();
            int currentUserStatus = checkUserStatus(&db, consultantCode); //Статус текущего пользователя

            //Вошел консультант
            if (currentUserStatus != 4) {
                a.quit();
                return (401);
            }
            else {
                QSqlQuery q(db);
                QString query = "";

                int tutorCode = 0;
                int childCode = 0;

                //Получаем параметры
                int themeCode = params.at(1).toInt();
                int statusThemeCode = params.at(2).toInt();
                int levelInteractionChildCode = params.at(3).toInt();
                int levelInteractionTutorCode = params.at(4).toInt();
                int currentStageExecute = params.at(5).toInt();

                QByteArray b;
                b.clear();
                b.append(params.at(6));

                QString consultantNote = TEXT(QByteArray::fromBase64(b), v);

                //Получаем из списка всех участников темы код учителя
                query = QString("SELECT USERID FROM VZLETBASE.USERSTATUSORGANIZATIONSUBDIVISION "
                                "WHERE (USERID IN (SELECT USERID FROM VZLETBASE.USERTHEME WHERE THEMEID = %1)) AND "
                                "STATUSID = 3 ")
                        .arg(params.at(1));
                if (q.exec(query)) {
                    while (q.next()) {
                        tutorCode = q.value(0).toInt();
                    }

                    //Получаем из списка всех участников темы код ученика
                    query = QString("SELECT USERID FROM USERSTATUSORGANIZATIONSUBDIVISION "
                                    "WHERE (USERID IN (SELECT USERID FROM VZLETBASE.USERTHEME WHERE THEMEID = %1)) AND "
                                    "STATUSID = 2 ")
                            .arg(params.at(1));
                    if (q.exec(query)) {
                        while (q.next()) {
                            childCode = q.value(0).toInt();
                        }
                    }


                    //Запись в БД оценки консультанта (оценка для учителя)
                    query = QString("INSERT INTO VZLETBASE.INTERACTION (themeID, userIDWho, userIDWhom, interactionDateTime, levelInteractionID, stageExecuteID, statusThemeID) VALUES "
                                    "(%1, %2, %3, NOW(), %4, %5, %6) ")
                            .arg(themeCode)
                            .arg(consultantCode)
                            .arg(tutorCode)
                            .arg(levelInteractionTutorCode)
                            .arg(currentStageExecute)
                            .arg(statusThemeCode);

                    if (q.exec(query)) {
                        //Запись в БД оценки консультанта (оценка для ученика)
                        query = QString("INSERT INTO VZLETBASE.INTERACTION (themeID, userIDWho, userIDWhom, interactionDateTime, levelInteractionID, stageExecuteID, statusThemeID) VALUES "
                                        "(%1, %2, %3, NOW(), %4, %5, %6) ")
                                .arg(themeCode)
                                .arg(consultantCode)
                                .arg(childCode)
                                .arg(levelInteractionChildCode)
                                .arg(currentStageExecute)
                                .arg(statusThemeCode);

                        if (q.exec(query)) {

                            query = QString("UPDATE vzletbase.Theme "
                                            "SET consultantNote = '%1', stageExecuteByConsultantID = %2 "
                                            "WHERE themeID = %3 ")
                                    .arg(consultantNote)
                                    .arg(currentStageExecute)
                                    .arg(themeCode);

                            if (q.exec(query)) {
                                printf(QString("Данные были успешно введены в систему! ").toLocal8Bit());
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
        }

    }






    //Учитель оценил взаимодействие с консультантом и учеником
    if (params.at(0) == "setInteractionByTeacher") {
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

            //Если вошел учитель
            if (currentUserStatus != 3) {
                a.quit();
                return (401);
            }
            else {
                QSqlQuery q(db);
                QString query = "";

                int consultantCode = 0;
                int childCode = 0;

                //Получаем параметры
                int themeCode = params.at(1).toInt();
                int levelInteractionChildCode = params.at(2).toInt();
                int levelInteractionTutorCode = params.at(3).toInt();
                int currentStageExecute = params.at(4).toInt();

                //Получаем из списка всех участников темы код консультанта
                query = QString("SELECT USERID FROM VZLETBASE.USERSTATUSORGANIZATIONSUBDIVISION "
                                "WHERE (USERID IN (SELECT USERID FROM VZLETBASE.USERTHEME WHERE THEMEID = %1)) AND "
                                "STATUSID = 4 ")
                        .arg(params.at(1));
                if (q.exec(query)) {
                    while (q.next()) {
                        consultantCode = q.value(0).toInt();
                    }

                    //Получаем из списка всех участников темы код ученика
                    query = QString("SELECT USERID FROM VZLETBASE.USERSTATUSORGANIZATIONSUBDIVISION "
                                    "WHERE (USERID IN (SELECT USERID FROM VZLETBASE.USERTHEME WHERE THEMEID = %1)) AND "
                                    "STATUSID = 2 ")
                            .arg(params.at(1));
                    if (q.exec(query)) {
                        while (q.next()) {
                            childCode = q.value(0).toInt();
                        }
                    }


                    //Запись в БД оценки учителя (оценка для консультанта)
                    query = QString("INSERT INTO VZLETBASE.INTERACTION (themeID, userIDWho, userIDWhom, interactionDateTime, levelInteractionID, stageExecuteID, statusThemeID) VALUES "
                                    "(%1, %2, %3, NOW(), %4, %5, NULL) ")
                            .arg(themeCode)
                            .arg(userCode)
                            .arg(consultantCode)
                            .arg(levelInteractionTutorCode)
                            .arg(currentStageExecute);

                    if (q.exec(query)) {
                        //Запись в БД оценки учителя (оценка для ученика)
                        query = QString("INSERT INTO VZLETBASE.INTERACTION (themeID, userIDWho, userIDWhom, interactionDateTime, levelInteractionID, stageExecuteID, statusThemeID) VALUES "
                                        "(%1, %2, %3, NOW(), %4, %5, NULL) ")
                                .arg(themeCode)
                                .arg(userCode)
                                .arg(childCode)
                                .arg(levelInteractionChildCode)
                                .arg(currentStageExecute);

                        if (q.exec(query)) {

                            query = QString("UPDATE vzletbase.Theme "
                                            "SET stageExecuteID = %1 "
                                            "WHERE themeID = %2 ")
                                    .arg(currentStageExecute)
                                    .arg(themeCode);
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






    //Ученик оценил взаимодействие с учителем и консультантом
    if (params.at(0) == "setInteractionByChild") {
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

            //Если вошел ученик
            if (currentUserStatus != 2) {
                a.quit();
                return (401);
            }
            else {
                QSqlQuery q(db);
                QString query = "";

                int consultantCode = 0;
                int tutorCode = 0;

                //Получаем параметры
                int themeCode = params.at(1).toInt();
                int levelInteractionTutorCode = params.at(3).toInt();
                int levelInteractionConsultantCode = params.at(2).toInt();
                int currentStageExecute = params.at(4).toInt();

                //Получаем из списка всех участников темы код консультанта
                query = QString("SELECT USERID FROM VZLETBASE.USERSTATUSORGANIZATIONSUBDIVISION "
                                "WHERE (USERID IN (SELECT USERID FROM VZLETBASE.USERTHEME WHERE THEMEID = %1)) AND "
                                "STATUSID = 4 ")
                        .arg(params.at(1));
                if (q.exec(query)) {
                    while (q.next()) {
                        consultantCode = q.value(0).toInt();
                    }

                    //Получаем из списка всех участников темы код учителя
                    query = QString("SELECT USERID FROM VZLETBASE.USERSTATUSORGANIZATIONSUBDIVISION "
                                    "WHERE (USERID IN (SELECT USERID FROM VZLETBASE.USERTHEME WHERE THEMEID = %1)) AND "
                                    "STATUSID = 3 ")
                            .arg(params.at(1));
                    if (q.exec(query)) {
                        while (q.next()) {
                            tutorCode = q.value(0).toInt();
                        }
                    }


                    //Запись в БД оценки ученика (оценка для консультанта)
                    query = QString("INSERT INTO VZLETBASE.INTERACTION (themeID, userIDWho, userIDWhom, interactionDateTime, levelInteractionID, stageExecuteID, statusThemeID) VALUES "
                                    "(%1, %2, %3, NOW(), %4, %5, NULL) ")
                            .arg(themeCode)
                            .arg(userCode)
                            .arg(consultantCode)
                            .arg(levelInteractionConsultantCode)
                            .arg(currentStageExecute);

                    if (q.exec(query)) {
                        //Запись в БД оценки ученика (оценка для учителя)
                        query = QString("INSERT INTO VZLETBASE.INTERACTION (themeID, userIDWho, userIDWhom, interactionDateTime, levelInteractionID, stageExecuteID, statusThemeID) VALUES "
                                        "(%1, %2, %3, NOW(), %4, %5, NULL) ")
                                .arg(themeCode)
                                .arg(userCode)
                                .arg(tutorCode)
                                .arg(levelInteractionTutorCode)
                                .arg(currentStageExecute);

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






    //Получить список консультантов в системе (или получить подробную информацию о конкретном пользователе)
    if (params.at(0) == "getStatusList") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {
            int currentUserID = QString("%1").arg(personID).toInt(); //Пользователь системы (его код)
            //Если вошел не пользователь системы
            if (QString("%1").arg(personID).toInt() != 3) {
                if ((checkUserStatus(&db, currentUserID) != 16) || (checkUserStatus(&db, currentUserID) != 17)) {
                    if ((checkUserStatus(&db, currentUserID) < 2) && (checkUserStatus(&db, currentUserID) > 6)) {
                        a.quit();
                        return (401);
                    }
                }
            }


            //Проверка статуса - подтвержденный или неподтвержденный
            int oldStatus = 0;
            if (params.at(1).toInt() == 2) {
                oldStatus = 19;
            }
            else {
                if (params.at(1).toInt() == 3) {
                    oldStatus = 20;
                }
                else {
                    if (params.at(1).toInt() == 4) {
                        oldStatus = 21;
                    }
                }
            }


            QSqlQuery q(db);
            QString query = "";
            QString result = "[";


            if (params.length() == 2) {
                //Получить список всех пользователей с полной информацией о них

                if (params.at(1).toInt() != 0) {

                    query = QString("SELECT Dov.dovID, Dov.userFIO, Organization.organizationName, SubdivisionLevel1.subdivisionLevel1Name, userStatusOrganizationSubdivision.postName, Status.statusName, StageExecute.stageExecuteName, Theme.stageExecuteID, Theme.themeName, StatusTheme.statusThemeName, Theme.statusThemeID, Theme.themeDateTimeUpdateStatus "
                                    "FROM (StatusTheme INNER JOIN (StageExecute INNER JOIN Theme ON StageExecute.stageExecuteID = Theme.stageExecuteID) ON StatusTheme.statusThemeID = Theme.statusThemeID) INNER JOIN (Status INNER JOIN ((Organization INNER JOIN SubdivisionLevel1 ON Organization.organizationID = SubdivisionLevel1.organizationID) INNER JOIN ((Dov INNER JOIN userStatusOrganizationSubdivision ON Dov.dovID = userStatusOrganizationSubdivision.userID) INNER JOIN UserTheme ON Dov.dovID = UserTheme.userID) ON (SubdivisionLevel1.subdivisionLevel1ID = userStatusOrganizationSubdivision.subdivisionLevel1ID) AND (Organization.organizationID = userStatusOrganizationSubdivision.organizationID)) ON Status.statusID = userStatusOrganizationSubdivision.statusID) ON Theme.themeID = UserTheme.themeID "
                                    "WHERE (userStatusOrganizationSubdivision.statusID=%1) ")
                            .arg(params.at(1))
                            .arg(oldStatus);


                }
                else {
                    query = "SELECT Dov.dovID, Dov.userFIO, Organization.organizationName, SubdivisionLevel1.subdivisionLevel1Name, "
                            "userStatusOrganizationSubdivision.postName, Status.statusName, StageExecute.stageExecuteName, Theme.stageExecuteID, "
                            "Theme.themeName, StatusTheme.statusThemeName, Theme.themeDateTimeUpdateStatus "
                            "FROM (StatusTheme INNER JOIN (StageExecute INNER JOIN Theme ON StageExecute.stageExecuteID = Theme.stageExecuteID) ON StatusTheme.statusThemeID = Theme.statusThemeID) INNER JOIN (Status INNER JOIN ((Organization INNER JOIN SubdivisionLevel1 ON Organization.organizationID = SubdivisionLevel1.organizationID) INNER JOIN ((Dov INNER JOIN userStatusOrganizationSubdivision ON Dov.dovID = userStatusOrganizationSubdivision.userID) INNER JOIN UserTheme ON Dov.dovID = UserTheme.userID) ON (SubdivisionLevel1.subdivisionLevel1ID = userStatusOrganizationSubdivision.subdivisionLevel1ID) AND (Organization.organizationID = userStatusOrganizationSubdivision.organizationID)) ON Status.statusID = userStatusOrganizationSubdivision.statusID) ON Theme.themeID = UserTheme.themeID "
                            "WHERE (((userStatusOrganizationSubdivision.statusID)=2 Or (userStatusOrganizationSubdivision.statusID)=3 Or (userStatusOrganizationSubdivision.statusID)=4) OR userStatusOrganizationSubdivision.statusID=19 OR userStatusOrganizationSubdivision.statusID=20 OR userStatusOrganizationSubdivision.statusID=21) ";
                }
            }
            else if (params.length() == 3) {
                //Получить полную информацию о конкретном пользователе
                query = QString("SELECT Dov.dovID, Dov.userFIO, Organization.organizationName, SubdivisionLevel1.subdivisionLevel1Name, userStatusOrganizationSubdivision.postName, Status.statusName, StageExecute.stageExecuteName, Theme.stageExecuteID, Theme.themeName, Theme.themeDateTimeUpdateStatus "
                                "FROM (StageExecute INNER JOIN Theme ON StageExecute.stageExecuteID = Theme.stageExecuteID) INNER JOIN (Status INNER JOIN ((Organization INNER JOIN SubdivisionLevel1 ON Organization.organizationID = SubdivisionLevel1.organizationID) INNER JOIN ((Dov INNER JOIN userStatusOrganizationSubdivision ON Dov.dovID = userStatusOrganizationSubdivision.userID) INNER JOIN UserTheme ON Dov.dovID = UserTheme.userID) ON (SubdivisionLevel1.subdivisionLevel1ID = userStatusOrganizationSubdivision.subdivisionLevel1ID) AND (Organization.organizationID = userStatusOrganizationSubdivision.organizationID)) ON Status.statusID = userStatusOrganizationSubdivision.statusID) ON Theme.themeID = UserTheme.themeID "
                                "WHERE ((userStatusOrganizationSubdivision.statusID=%1)) AND ((userStatusOrganizationSubdivision.userID)=%2)) ")
                        .arg(params.at(1))
                        .arg(params.at(2))
                        .arg(oldStatus);
            }

            if (q.exec(query)) {
                while (q.next()) {

                    if (params.at(1).toInt() != 0) {
                        result.append("{");
                        result.append(QString("\"statusListCode\":%1,"
                                              "\"statusListFIO\":\"%2\","
                                              "\"statusListOrganizationName\":\"%3\","
                                              "\"statusListSubdivisionLevel1Name\":\"%4\","
                                              "\"statusListPostName\":\"%5\","
                                              "\"statusListStatusName\":\"%6\","
                                              "\"statusListCurrentStageExecute\":\"%7\","
                                              "\"statusListCurrentStageCode\":%8,"
                                              "\"statusListThemeName\":\"%9\","
                                              "\"statusListStatusThemeName\":\"%10\","
                                              "\"statusListStatusThemeCode\":%11,"
                                              "\"statusListThemeDateTimeUpdateStatus\":\"%12\"},")
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
                                      .arg(QDate::fromString(q.value(11).toString().mid(0, 10), "yyyy-MM-dd").toString("dd.MM.yyyy")));
                    }
                    else {
                        result.append("{");
                        result.append(QString("\"statusListCode\":%1,"
                                              "\"statusListFIO\":\"%2\","
                                              "\"statusListOrganizationName\":\"%3\","
                                              "\"statusListSubdivisionLevel1Name\":\"%4\","
                                              "\"statusListPostName\":\"%5\","
                                              "\"statusListStatusName\":\"%6\","
                                              "\"statusListCurrentStageExecute\":\"%7\","
                                              "\"statusListCurrentStageCode\":%8,"
                                              "\"statusListThemeName\":\"%9\","
                                              "\"statusListStatusThemeName\":\"%10\","
                                              "\"statusListThemeDateTimeUpdateStatus\":\"%11\"},")
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



    //Получить список тем по коду пользователя
    if (params.at(0) == "getThemeByUser") {
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

            //Если нужно получить только список тем, к которым привязан конкретный участник системы
            if ((params.length() == 3) && (params.at(2) == "part")) {
                query = QString("SELECT THEMEID, THEMENAME, STATUSTHEMENAME, THEME.STATUSTHEMEID FROM VZLETBASE.THEME, VZLETBASE.STATUSTHEME "
                                "WHERE (THEMEID IN (SELECT THEMEID FROM VZLETBASE.USERTHEME WHERE USERID = %1)) AND "
                                "(THEME.STATUSTHEMEID = STATUSTHEME.STATUSTHEMEID) ")
                        .arg(params.at(1));
            }
            else if ((params.length() == 3) && (params.at(2) == "otv")) {
                //Вывести список тем, привязанных к участникам соответствующей организации
                query = QString("SELECT THEMEID, THEMENAME "
                                "FROM VZLETBASE.THEME, VZLETBASE.STATUSTHEME "
                                "WHERE (THEMEID IN (SELECT THEMEID FROM VZLETBASE.USERTHEME WHERE USERID IN (SELECT USERID FROM VZLETBASE.USERSTATUSORGANIZATIONSUBDIVISION WHERE ORGANIZATIONID = (SELECT ORGANIZATIONID FROM USERSTATUSORGANIZATIONSUBDIVISION WHERE USERID = %1)))) AND "
                                "(THEME.STATUSTHEMEID = STATUSTHEME.STATUSTHEMEID) ")
                        .arg(params.at(1));
            }

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"themeCode\":%1,"
                                          "\"themeName\":\"%2\","
                                          "\"statusThemeName\":\"%3\","
                                          "\"statusThemeCode\":\"%4\"},")
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




    //Получить список (с подробной информацией) по участникам, кто привязан к теме
    if (params.at(0) == "getUserInfoByConnectedTheme") {
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

            query = QString("SELECT USERID, USERFIO, ORGANIZATIONNAME, SUBDIVISIONLEVEL1NAME, POSTNAME, STATUSNAME, USEREMAIL, USERPHONE, USERMOBILEPHONE "
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
                                          "\"statusListStatusName\":\"%6\","
                                          "\"statusListUserEMail\":\"%7\","
                                          "\"statusListUserPhone\":\"%8\","
                                          "\"statusListUserMobilePhone\":\"%9\"},")
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






    //Получить список (с подробной информацией) по участникам, кто привязан к теме
    if (params.at(0) == "getUserInfoByConnectedOfferedTheme") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {

            int themeCode = params.at(1).toInt(); //Код темы
            int userCode = QString("%1").arg(personID).toInt();

            //Если пользователь не относится к этой теме
            if (!checkUserTheme(&db, userCode, themeCode)) {
                a.quit();
                return (401);
            }

            QSqlQuery q(db);
            QString query = "";
            QString result = "[";

            query = QString("SELECT USERID, USERFIO, ORGANIZATIONNAME, SUBDIVISIONLEVEL1NAME, POSTNAME, STATUSNAME, USEREMAIL, USERPHONE, USERMOBILEPHONE  "
                            "FROM VZLETBASE.USERSTATUSORGANIZATIONSUBDIVISION, VZLETBASE.DOV, VZLETBASE.ORGANIZATION, VZLETBASE.SUBDIVISIONLEVEL1, VZLETBASE.STATUS "
                            "WHERE "
                            "(USERSTATUSORGANIZATIONSUBDIVISION.USERID IN (SELECT USERID FROM VZLETBASE.USERTHEME WHERE THEMEID = %1)) AND "
                            "(USERSTATUSORGANIZATIONSUBDIVISION.USERID = DOV.DOVID) AND "
                            "(USERSTATUSORGANIZATIONSUBDIVISION.ORGANIZATIONID = ORGANIZATION.ORGANIZATIONID) AND (USERSTATUSORGANIZATIONSUBDIVISION.SUBDIVISIONLEVEL1ID = SUBDIVISIONLEVEL1.SUBDIVISIONLEVEL1ID) AND "
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
                                          "\"statusListStatusName\":\"%6\","
                                          "\"statusListUserEMail\":\"%7\","
                                          "\"statusListUserPhone\":\"%8\","
                                          "\"statusListUserMobilePhone\":\"%9\"},")
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





    //Получить информацию о теме по ее коду
    if (params.at(0) == "getThemeInfo") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {
            int userCode = QString("%1").arg(personID).toInt();
            int themeCode = params.at(1).toInt(); //Код темы

            //Если пользователь не относится к этой теме
            if (!checkUserTheme(&db, userCode, themeCode)) {
                a.quit();
                return (401);
            }

            QSqlQuery q(db);
            QString query = "";
            QString result;

            int statusThemeID = 0; //Статус выбранной темы

            query = QString("SELECT THEMEID, THEMENAME, THEMECOMMENT, THEMEDATETIME, "
                            "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_1) AS SCIENCEDIRECTIONNAME1, "
                            "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_2) AS SCIENCEDIRECTIONNAME2, "
                            "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_3) AS SCIENCEDIRECTIONNAME3, "
                            "STATUSTHEMENAME, THEMEABOUTTUTOR, THEMEABOUTCHILD, STAGEEXECUTENAME, Theme.statusThemeID, consultantNote, "
                            "PRIORITYDIRECTIONNAME, SECTIONNAME, SCIENCEINDUSTRIALSPHERENAME "
                            "FROM VZLETBASE.THEME, VZLETBASE.STATUSTHEME, VZLETBASE.STAGEEXECUTE, VZLETBASE.PRIORITYDIRECTION, VZLETBASE.SECTION, VZLETBASE.SCIENCEINDUSTRIALSPHERE "
                            "WHERE (THEMEID = %1) AND (THEME.STATUSTHEMEID = STATUSTHEME.STATUSTHEMEID) AND "
                            "(THEME.STAGEEXECUTEID = STAGEEXECUTE.STAGEEXECUTEID) AND "
                            "(THEME.PRIORITYDIRECTIONID = PRIORITYDIRECTION.PRIORITYDIRECTIONID) AND "
                            "(THEME.SECTIONID = SECTION.SECTIONID) AND "
                            "(THEME.SCIENCEINDUSTRIALSPHEREID = SCIENCEINDUSTRIALSPHERE.SCIENCEINDUSTRIALSPHEREID) ")
                    .arg(themeCode);

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"themeCode\":%1,"
                                          "\"themeName\":\"%2\","
                                          "\"themeComment\":\"%3\","
                                          "\"themeDateTime\":\"%4\","
                                          "\"themeScienceDirectionName1\":\"%5\","
                                          "\"themeScienceDirectionName2\":\"%6\","
                                          "\"themeScienceDirectionName3\":\"%7\","
                                          "\"themeStatusThemeName\":\"%8\","
                                          "\"themeAboutTutor\":\"%9\","
                                          "\"themeAboutChild\":\"%10\","
                                          "\"themeStageExecuteNameByConsulntant\":\"%11\","
                                          "\"StatusThemeID\":%12,"
                                          "\"consultantNote\":\"%13\","
                                          "\"priorityDirectionName\":\"%14\","
                                          "\"sectionName\":\"%15\","
                                          "\"scienceIndustrialSphereName\":\"%16\"}")
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
                                  .arg(q.value(13).toString())
                                  .arg(q.value(14).toString())
                                  .arg(q.value(15).toString()));

                    statusThemeID = q.value(11).toInt();
                }

                if (statusThemeID == 1) {
                    //Тема предлагаемая, проверка не требуется
                    db.close();
                    printf(result.toLocal8Bit());
                    a.quit();
                    return (200);
                }
                else {
                    //Требуется проверка на то, является ли вошедший пользователь участником этой темы
                    query = QString("SELECT userID "
                                    "FROM vzletbase.userTheme "
                                    "WHERE "
                                    "themeID = %1 ")
                            .arg(themeCode);
                    QStringList userCode;
                    userCode.clear();
                    if (q.exec(query)) {
                        while (q.next()) {
                            userCode.append(q.value(0).toString());
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






    //Получить информацию о теме по ее коду (для режима "Выбор тематики и формирование коллектива"
    if (params.at(0) == "getThemeInfoForChooseTheme") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {
            int userCode = QString("%1").arg(personID).toInt();
            int themeCode = params.at(1).toInt();
            int currentUserStatus = checkUserStatus(&db, userCode); //Статус текущего пользователя

            //Если консультант или ученик
            if (currentUserStatus > 4 && currentUserStatus < 2) {
                a.quit();
                return (401);
            }

            QSqlQuery q(db);
            QString query = "";
            QString result;

            int statusThemeID = 0; //Статус выбранной темы

            query = QString("SELECT THEMEID, THEMENAME, THEMECOMMENT, THEMEDATETIME, "
                            "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_1) AS SCIENCEDIRECTIONNAME1, "
                            "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_2) AS SCIENCEDIRECTIONNAME2, "
                            "(SELECT SCIENCEDIRECTIONNAME FROM VZLETBASE.SCIENCEDIRECTION WHERE SCIENCEDIRECTIONID = SCIENCEDIRECTIONID_3) AS SCIENCEDIRECTIONNAME3, "
                            "STATUSTHEMENAME, THEMEABOUTTUTOR, THEMEABOUTCHILD, STAGEEXECUTENAME, Theme.statusThemeID, consultantNote, "
                            "PRIORITYDIRECTIONNAME, SECTIONNAME, SCIENCEINDUSTRIALSPHERENAME "
                            "FROM VZLETBASE.THEME, VZLETBASE.STATUSTHEME, VZLETBASE.STAGEEXECUTE, VZLETBASE.PRIORITYDIRECTION, VZLETBASE.SECTION, VZLETBASE.SCIENCEINDUSTRIALSPHERE "
                            "WHERE (THEMEID = %1) AND (THEME.STATUSTHEMEID = STATUSTHEME.STATUSTHEMEID) AND "
                            "(THEME.STAGEEXECUTEID = STAGEEXECUTE.STAGEEXECUTEID) AND "
                            "(THEME.PRIORITYDIRECTIONID = PRIORITYDIRECTION.PRIORITYDIRECTIONID) AND "
                            "(THEME.SECTIONID = SECTION.SECTIONID) AND "
                            "(THEME.SCIENCEINDUSTRIALSPHEREID = SCIENCEINDUSTRIALSPHERE.SCIENCEINDUSTRIALSPHEREID) ")
                    .arg(themeCode);

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"themeCode\":%1,"
                                          "\"themeName\":\"%2\","
                                          "\"themeComment\":\"%3\","
                                          "\"themeDateTime\":\"%4\","
                                          "\"themeScienceDirectionName1\":\"%5\","
                                          "\"themeScienceDirectionName2\":\"%6\","
                                          "\"themeScienceDirectionName3\":\"%7\","
                                          "\"themeStatusThemeName\":\"%8\","
                                          "\"themeAboutTutor\":\"%9\","
                                          "\"themeAboutChild\":\"%10\","
                                          "\"themeStageExecuteNameByConsulntant\":\"%11\","
                                          "\"StatusThemeID\":%12,"
                                          "\"consultantNote\":\"%13\","
                                          "\"priorityDirectionName\":\"%14\","
                                          "\"sectionName\":\"%15\","
                                          "\"scienceIndustrialSphereName\":\"%16\"}")
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
                                  .arg(q.value(13).toString())
                                  .arg(q.value(14).toString())
                                  .arg(q.value(15).toString()));

                    statusThemeID = q.value(11).toInt();
                }

                if (statusThemeID == 1) {
                    //Тема предлагаемая, проверка не требуется
                    db.close();
                    printf(result.toLocal8Bit());
                    a.quit();
                    return (200);
                }
                else {
                    //Требуется проверка на то, является ли вошедший пользователь участником этой темы
                    query = QString("SELECT userID "
                                    "FROM vzletbase.userTheme "
                                    "WHERE "
                                    "themeID = %1 ")
                            .arg(themeCode);
                    QStringList userCode;
                    userCode.clear();
                    if (q.exec(query)) {
                        while (q.next()) {
                            userCode.append(q.value(0).toString());
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









    //Подключение учителя и ученика к выбранной теме
    if (params.at(0) == "applyTutorChildTheme") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {

            int tutorCode = QString("%1").arg(personID).toInt();
            int currentUserStatus = checkUserStatus(&db, tutorCode);

            //Если вошел не учитель
            if (currentUserStatus != 3) {
                a.quit();
                return (401);
            }

            QSqlQuery q(db);
            QString query = "";

            bool okThemeCode, okChildCode = true;

            int themeCode = params.at(1).toInt(&okThemeCode);
            int childCode = params.at(3).toInt(&okChildCode);

            if (!okThemeCode || !okChildCode || childCode == 0) {
                printf("{\"errorCode\":3}");
                a.quit();
                return (200);
            }

            //Количество участников в выбранной теме
            int countParticipants = 0;

            //Код ученика (имеющийся)
            int childCodeExist = 0;

            //По коду темы смотрим, есть ли еще кто-нибудь из участников, присоединенных к ней
            query = QString("SELECT COUNT(userID) "
                            "FROM vzletbase.userTheme "
                            "WHERE "
                            "themeID = %1 ")
                    .arg(themeCode);
            if (q.exec(query)) {
                while (q.next()) {
                    countParticipants = q.value(0).toInt();
                }

                if (countParticipants == 3) {
                    printf("{\"errorCode\":2}");
                    a.quit();
                    return (200);
                }
                else {

                    //Проверяем, есть ли такой код ученика
                    query = QString("SELECT userID "
                                    "FROM vzletbase.userStatusOrganizationSubdivision "
                                    "WHERE "
                                    "(userID = %1) AND (statusID = 2) ")
                            .arg(childCode);

                    if (q.exec(query)) {
                        while (q.next()) {
                            childCodeExist = q.value(0).toInt();
                        }

                        if (childCode != childCodeExist) {
                            printf("{\"errorCode\":3}");
                            a.quit();
                            return (200);
                        }
                        else {
                            QByteArray b;
                            b.clear();
                            b.append(params.at(4));

                            QString tutorAbout = TEXT(QByteArray::fromBase64(b), v); //Краткая информация об учителе

                            b.clear();
                            b.append(params.at(5));

                            QString childAbout = TEXT(QByteArray::fromBase64(b), v); //Краткая информация об ученике

                            query = QString("INSERT INTO vzletbase.userTheme "
                                            "(userID, themeID, userThemeDateTime, statusParticipationID) "
                                            "VALUES "
                                            "(%1, %2, NOW(), NULL) ")
                                    .arg(tutorCode)
                                    .arg(themeCode);

                            if (q.exec(query)) {

                                query = QString("INSERT INTO vzletbase.userTheme "
                                                "(userID, themeID, userThemeDateTime, statusParticipationID) "
                                                "VALUES "
                                                "(%1, %2, NOW(), NULL) ")
                                        .arg(childCode)
                                        .arg(themeCode);

                                if (q.exec(query)) {
                                    query = QString("UPDATE vzletbase.Theme "
                                                    "SET themeStaffing = TRUE, "
                                                    "themeAboutTutor = '%1', "
                                                    "themeAboutChild = '%2', "
                                                    "statusThemeID = 6, "
                                                    "themeDateTimeUpdateStatus = NOW() "
                                                    "WHERE themeID = %3 ")
                                            .arg(tutorAbout)
                                            .arg(childAbout)
                                            .arg(themeCode);



                                    if (q.exec(query)) {

                                        //Создание дубликата темы (и придание ей статуса - Предлагаемая)

                                        int consultantCode = 0;
                                        //Получаем для начала код консультанта
                                        query = QString("SELECT userID "
                                                        "FROM vzletbase.userTheme "
                                                        "WHERE (themeID = %1) AND userID IN "
                                                        "( "
                                                        "SELECT userID "
                                                        "FROM vzletbase.userStatusOrganizationSubdivision "
                                                        "WHERE "
                                                        "statusID = 4 OR statusID = 21 "
                                                        ") ")
                                                .arg(themeCode);

                                        if (q.exec(query)) {
                                            while (q.next()) {
                                                consultantCode = q.value(0).toInt();
                                            }

                                            //Получаем информацию о выбранной теме
                                            QString themeName, themeComment;//, themeDateTime;
                                            int scienceDirectionCode1, scienceDirectionCode2, scienceDirectionCode3 = 0;
                                            int sectionCode, priorityDirectionCode, scienceIndustrialSphereCode = 0;

                                            //Получаем информацию по выбранной теме
                                            query = QString("SELECT themeName, themeComment, scienceDirectionID_1, scienceDirectionID_2, scienceDirectionID_3, "
                                                            "sectionID, priorityDirectionID, scienceIndustrialSphereID "
                                                            "FROM vzletbase.Theme "
                                                            "WHERE themeID = %1 ")
                                                    .arg(themeCode);

                                            if (q.exec(query)) {
                                                while (q.next()) {
                                                    themeName = q.value(0).toString();
                                                    themeComment = q.value(1).toString();
                                                    scienceDirectionCode1 = q.value(2).toInt();
                                                    scienceDirectionCode2 = q.value(3).toInt();
                                                    scienceDirectionCode3 = q.value(4).toInt();
                                                    sectionCode = q.value(5).toInt();
                                                    priorityDirectionCode = q.value(6).toInt();
                                                    scienceIndustrialSphereCode = q.value(7).toInt();
                                                }

                                                //Получаем ФИО учителя и ученика для отправки им уведомления
                                                QString teacherFIO, teacherEMail;

                                                query = QString("SELECT userFIO, userEMail FROM vzletbase.dov WHERE dovID = %1 ").arg(tutorCode);
                                                if (q.exec(query)) {
                                                    while (q.next()) {
                                                        teacherFIO = q.value(0).toString();
                                                        teacherEMail = q.value(1).toString();
                                                    }


                                                    //Получаем ФИО ученика и его адрес электронной почты
                                                    QString childFIO, childEMail;

                                                    query = QString("SELECT userFIO, userEMail FROM vzletbase.dov WHERE dovID = %1 ").arg(childCode);
                                                    if (q.exec(query)) {
                                                        while (q.next()) {
                                                            childFIO = q.value(0).toString();
                                                            childEMail = q.value(1).toString();
                                                        }

                                                        //Получаем ФИО и EMail консультанта
                                                        QString consultantFIO, consultantEMail;

                                                        query = QString("SELECT userFIO, userEMail FROM vzletbase.dov WHERE dovID = %1 ").arg(consultantCode);
                                                        if (q.exec(query)) {
                                                            while (q.next()) {
                                                                consultantFIO = q.value(0).toString();
                                                                consultantEMail = q.value(1).toString();
                                                            }


                                                            QProcess p;
                                                            QStringList parametres;
                                                            parametres.clear();

                                                            //Письмо для учителя и ученика
                                                            parametres.append("teamBuildingForTeacherChild");

                                                            b.clear();
                                                            b.append(teacherFIO); // ФИО учителя
                                                            parametres.append(b.toBase64());

                                                            b.clear();
                                                            b.append(childFIO); // ФИО ученика
                                                            parametres.append(b.toBase64());

                                                            b.clear();

                                                            b.append(teacherEMail); // EMail учителя
                                                            parametres.append(b.toBase64());

                                                            b.clear();

                                                            b.append(childEMail); // EMail учителя
                                                            parametres.append(b.toBase64());

                                                            b.clear();
                                                            b.append(themeName); // Название темы, предложенной консультантом
                                                            parametres.append(b.toBase64());

                                                            b.clear();
                                                            b.append(consultantFIO); // ФИО консультанта
                                                            parametres.append(b.toBase64());


                                                            //Отправка уведомлений
                                                            p.start("C:/vzlet/modules/emailNotifierVZLET.exe", parametres);
                                                            p.waitForReadyRead(20 * 1000);
                                                            int result = p.readAll().toInt();


                                                            //Письмо консультанту
                                                            parametres.clear();
                                                            parametres.append("teamBuildingForConsultant");

                                                            b.clear();
                                                            b.append(consultantFIO); // ФИО консультанта
                                                            parametres.append(b.toBase64());

                                                            b.clear();

                                                            b.append(consultantEMail); // EMail консультанта
                                                            parametres.append(b.toBase64());

                                                            b.clear();
                                                            b.append(themeName); // Название темы проекта
                                                            parametres.append(b.toBase64());


                                                            //Получаем информацию об учителе
                                                            QString teacherInfo;

                                                            query = QString("SELECT organizationName, subdivisionLevel1Name, postName "
                                                                            "FROM vzletbase.userStatusOrganizationSubdivision, vzletbase.organization, vzletbase.subdivisionLevel1 "
                                                                            "WHERE "
                                                                            "(userID = %1) AND "
                                                                            "(userStatusOrganizationSubdivision.organizationID = organization.organizationID) AND "
                                                                            "(userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID) ")
                                                                    .arg(tutorCode);
                                                            if (q.exec(query)) {
                                                                while (q.next()) {
                                                                    teacherInfo.append(QString("%1 (%2, %3, %4); ")
                                                                                       .arg(teacherFIO)
                                                                                       .arg(q.value(0).toString())
                                                                                       .arg(q.value(1).toString())
                                                                                       .arg(q.value(2).toString()));
                                                                }


                                                                //Получаем информацию об ученике
                                                                QString childInfo;

                                                                query = QString("SELECT organizationName, subdivisionLevel1Name, childGradeName  "
                                                                                "FROM vzletbase.userStatusOrganizationSubdivision, vzletbase.organization, vzletbase.subdivisionLevel1, vzletbase.childGrade "
                                                                                "WHERE "
                                                                                "(userID = %1) AND "
                                                                                "(userStatusOrganizationSubdivision.organizationID = organization.organizationID) AND "
                                                                                "(userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID) AND "
                                                                                "(userStatusOrganizationSubdivision.childGradeID = childGrade.childGradeID) ")
                                                                        .arg(childCode);
                                                                if (q.exec(query)) {
                                                                    while (q.next()) {
                                                                        childInfo.append(QString("%1 (%2, %3, %4); ")
                                                                                         .arg(childFIO)
                                                                                         .arg(q.value(0).toString())
                                                                                         .arg(q.value(1).toString())
                                                                                         .arg(q.value(2).toString()));
                                                                    }


                                                                    b.clear();
                                                                    b.append(teacherInfo); // Информация об учителе
                                                                    parametres.append(b.toBase64());

                                                                    b.clear();
                                                                    b.append(childInfo); // Информация об ученике
                                                                    parametres.append(b.toBase64());


                                                                    //Информация об учителе и ученике, указанная при подаче заявки
                                                                    b.clear();
                                                                    b.append(tutorAbout); // Об учителе
                                                                    parametres.append(b.toBase64());

                                                                    b.clear();
                                                                    b.append(childAbout); // Об ученике
                                                                    parametres.append(b.toBase64());

                                                                    //Отправка уведомлений
                                                                    QProcess p1;
                                                                    p1.start("C:/vzletbase/modules/emailNotifierVZLET.exe", parametres);
                                                                    p1.waitForReadyRead(20 * 1000);
                                                                    result = 0;
                                                                    result = p1.readAll().toInt();

                                                                    switch (result) {
                                                                    case 1:                                                                                        return (200);
                                                                        printf("{\"errorCode\":0}");
                                                                        a.quit();
                                                                        return (200);
                                                                        break;
                                                                    case -1:
                                                                        printf("error ");
                                                                        a.quit();
                                                                        return (200);
                                                                        break;
                                                                    default:
                                                                        printf("error sent e-mail");
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










    //Сделать тему, так сказать, активной - точнее выполняемой
    if (params.at(0) == "setActiveTheme") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {

            int consultantCode = QString("%1").arg(personID).toInt();
            int currentUserStatus = checkUserStatus(&db, consultantCode); //Статус текущего пользователя

            //Если вошел консультант
            if (currentUserStatus != 4) {
                a.quit();
                return (200);
            }
            else {

                QSqlQuery q(db);
                QString query = "";

                QString themeName, themeComment;
                int scienceDirectionCode1, scienceDirectionCode2, scienceDirectionCode3, statusThemeCode = 0;

                int themeCode = params.at(1).toInt();

                //Получаем информацию по выбранной теме
                query = QString("SELECT themeName, themeComment, scienceDirectionID_1, scienceDirectionID_2, scienceDirectionID_3, statusThemeID "
                                "FROM vzletbase.Theme "
                                "WHERE themeID = %1 ")
                        .arg(themeCode);

                if (q.exec(query)) {
                    while (q.next()) {
                        themeName = q.value(0).toString();
                        themeComment = q.value(1).toString();
                        scienceDirectionCode1 = q.value(2).toInt();
                        scienceDirectionCode2 = q.value(3).toInt();
                        scienceDirectionCode3 = q.value(4).toInt();
                        statusThemeCode = q.value(5).toInt();
                    }

                    query = QString("INSERT INTO vzletbase.Theme (themeName, themeComment, themeDateTime, scienceDirectionID_1, scienceDirectionID_2, scienceDirectionID_3, statusThemeID, stageExecuteID)"
                                    " VALUES ('%1', '%2', NOW(), %3, %4, %5, 1, NULL) ")
                            .arg(themeName)
                            .arg(themeComment)
                            .arg(scienceDirectionCode1)
                            .arg(scienceDirectionCode2)
                            .arg(scienceDirectionCode3);

                    if (q.exec(query)) {
                        query = QString("SELECT themeID FROM vzletbase.Theme "
                                        "WHERE "
                                        "(themeName = '%1') AND (themeComment = '%2') AND"
                                        "(scienceDirectionID_1 = %3) AND "
                                        "(scienceDirectionID_2 = %4) AND "
                                        "(scienceDirectionID_3 = %5) AND "
                                        "(themeID <> %6) ")
                                .arg(themeName)
                                .arg(themeComment)
                                .arg(scienceDirectionCode1)
                                .arg(scienceDirectionCode2)
                                .arg(scienceDirectionCode3)
                                .arg(themeCode);

                        if (q.exec(query)) {
                            int newTheme = 0;
                            while (q.next()) {
                                newTheme = q.value(0).toInt();
                            }

                            //Связываем новую тему с консультантом
                            query = QString("INSERT INTO vzletbase.UserTheme (userID, themeID, userThemeDateTime, statusParticipationID) VALUES "
                                            "(%1, %2, NOW(), NULL) ")
                                    .arg(consultantCode)
                                    .arg(newTheme);

                            if (q.exec(query)) {
                                //Завершающий этап - изменяем статус старой темы с "Предлагаемая" на "Выполняемая"
                                query = QString("UPDATE vzletbase.Theme "
                                                "SET statusThemeID = 2 "
                                                "WHERE themeID = %1")
                                        .arg(themeCode);

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
                        printf(q.lastError().text().toLocal8Bit());
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






    //Получить список тем по коду пользователя
    if (params.at(0) == "reOfferThemeByConsultant") {
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

            int consultantCode = QString("%1").arg(personID).toInt();
            int currentUserStatus = checkUserStatus(&db, consultantCode); //Статус текущего пользователя

            if (currentUserStatus != 4) {
                a.quit();
                return (401);
            }
            else {
                QString themeName, themeComment;
                int scienceDirectionCode1, scienceDirectionCode2, scienceDirectionCode3 = 0;
                int section, priorityDirection, scienceIndustrialSphere = 0;

                //Получаем информацию по выбранной теме
                query = QString("SELECT themeName, themeComment, scienceDirectionID_1, scienceDirectionID_2, scienceDirectionID_3, "
                                "sectionID, priorityDirectionID, scienceIndustrialSphereID "
                                "FROM vzletbase.Theme "
                                "WHERE themeID = %1 ")
                        .arg(themeCode);

                if (q.exec(query)) {
                    while (q.next()) {
                        themeName = q.value(0).toString();
                        themeComment = q.value(1).toString();
                        scienceDirectionCode1 = q.value(2).toInt();
                        scienceDirectionCode2 = q.value(3).toInt();
                        scienceDirectionCode3 = q.value(4).toInt();

                        section = q.value(5).toInt();
                        priorityDirection = q.value(6).toInt();
                        scienceIndustrialSphere = q.value(7).toInt();
                    }

                    query = QString("INSERT INTO vzletbase.Theme (themeName, themeComment, themeDateTime, "
                                    "scienceDirectionID_1, scienceDirectionID_2, scienceDirectionID_3, "
                                    "statusThemeID, stageExecuteID, sectionID, priorityDirectionID, scienceIndustrialSphereID)"
                                    " VALUES ('%1', '%2', NOW(), %3, %4, %5, 1, 1, %6, %7, %8) ")
                            .arg(themeName)
                            .arg(themeComment)
                            .arg(scienceDirectionCode1)
                            .arg(scienceDirectionCode2)
                            .arg(scienceDirectionCode3)
                            .arg(section)
                            .arg(priorityDirection)
                            .arg(scienceIndustrialSphere);

                    if (q.exec(query)) {
                        //Получаем код новой созданной темы
                        query = QString("SELECT themeID FROM vzletbase.Theme "
                                        "WHERE "
                                        "(themeName = '%1') AND (themeComment = '%2') AND"
                                        "(scienceDirectionID_1 = %3) AND "
                                        "(scienceDirectionID_2 = %4) AND "
                                        "(scienceDirectionID_3 = %5) AND "
                                        "(sectionID = %7) AND (priorityDirectionID = %8) AND (scienceIndustrialSphereID = %9) AND "
                                        "(themeID <> %6) "
                                        "ORDER BY themeID ASC ")
                                .arg(themeName)
                                .arg(themeComment)
                                .arg(scienceDirectionCode1)
                                .arg(scienceDirectionCode2)
                                .arg(scienceDirectionCode3)
                                .arg(themeCode)
                                .arg(section)
                                .arg(priorityDirection)
                                .arg(scienceIndustrialSphere);

                        int reOfferThemeCode = 0;

                        if (q.exec(query)) {
                            while (q.next()) {
                                reOfferThemeCode = q.value(0).toInt();
                            }

                            //Подключаем консультанта к новой теме
                            query = QString("INSERT INTO vzletbase.UserTheme (userID, themeID, userThemeDateTime, statusParticipationID) VALUES "
                                            "(%1, %2, NOW(), NULL) ")
                                    .arg(consultantCode)
                                    .arg(reOfferThemeCode);

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




    //Проверка укомплектованности "тройки" - есть ли уже консультант, учитель и ученик
    if (params.at(0) == "checkCompleteness") {
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

            //Признак укомплектованности
            bool staffing = true;
            //Получаем количество участников по коду темы
            query = QString("SELECT themeStaffing From vzletbase.theme WHERE themeID = %1 ")
                    .arg(themeCode);
            if (q.exec(query)) {
                while (q.next()) {
                    staffing = q.value(0).toBool();
                }

                if (staffing) {
                    printf("{\"staffing\":1}");
                    a.quit();
                    return (200);
                }
                else {
                    printf("{\"staffing\":0}");
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


    //Прикрепление консультанта к выбранной теме
    if (params.at(0) == "applyConsultantForTheme") {
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

            //Если вошел консультант
            if (currentUserStatus != 4) {
                a.quit();
                return (401);
            }
            else {
                QSqlQuery q(db);
                QString query = "";
                QByteArray b;

                int themeCode = params.at(1).toInt();

                b.clear();
                b.append(params.at(2));

                QString themeName = TEXT(QByteArray::fromBase64(b), v);

                int nameTheme = -1;

                query = QString("INSERT INTO vzletbase.userTheme "
                                "(userID, themeID, userThemeDateTime, statusParticipationID) "
                                "VALUES "
                                "(%1, %2, NOW(), NULL) ")
                        .arg(userCode)
                        .arg(themeCode);

                if (q.exec(query)) {

                    //Запись в БД успешно добавилась

                    //Получаем старое название темы
                    QString themeOldName;

                    query = QString("SELECT themeName "
                                    "FROM theme "
                                    "WHERE themeID = %1" )
                            .arg(themeCode);

                    if (q.exec(query)) {
                        while (q.next()) {
                            //Получаем старое название темы
                            themeOldName = q.value(0).toString();
                        }


                        //Если консультант решил оставить прежнее название темы (ничего не ввел, нажал ОК)
                        bool ok = true;
                        nameTheme = params.at(2).toInt(&ok);
                        if (ok) {
                            query = QString("UPDATE vzletbase.theme "
                                            "SET "
                                            "statusThemeID = 2, themeStaffing = TRUE, stageExecuteID = 2, themeDateTimeUpdateStatus = NOW() "
                                            "WHERE themeID = %1 ")
                                    .arg(themeCode);
                        }
                        else {
                            query = QString("UPDATE vzletbase.theme "
                                            "SET "
                                            "themeName = '%1', statusThemeID = 2, themeStaffing = TRUE, stageExecuteID = 2, themeDateTimeUpdateStatus = NOW() "
                                            "WHERE themeID = %2 ")
                                    .arg(themeName)
                                    .arg(themeCode);
                        }


                        if (q.exec(query)) {
                            //Все прошло успешно

                            int teacherCode = 0;
                            QString teacherFIO, teacherEMail;

                            //Получение E-Mail учителя для отправки ему уведомления
                            query = QString("SELECT dovID, userFIO, userEMail "
                                            "FROM vzletbase.dov "
                                            "WHERE dovID IN "
                                            "(SELECT userID "
                                            "FROM vzletbase.userStatusOrganizationSubdivision "
                                            "WHERE (statusID = 3) AND userID IN "
                                            "("
                                            "SELECT userID "
                                            "FROM vzletbase.userTheme "
                                            "WHERE themeID = %1 "
                                            ")) ")
                                    .arg(themeCode);

                            if (q.exec(query)) {
                                while (q.next()) {
                                    teacherCode = q.value(0).toInt();
                                    teacherFIO = q.value(1).toString();
                                    teacherEMail = q.value(2).toString().trimmed();
                                }


                                //Получаем все сведения об участниках данной темы
                                query = QString("SELECT Dov.userFIO, userStatusOrganizationSubdivision.statusID, Degree.degreeName, AcademicDirection.academicDirectionName, "
                                                "Organization.organizationName, SubdivisionLevel1.subdivisionLevel1Name, userStatusOrganizationSubdivision.postName, Dov.userEMail "
                                                "FROM (Organization INNER JOIN SubdivisionLevel1 ON Organization.organizationID = SubdivisionLevel1.organizationID) INNER JOIN ((AcademicDirection INNER JOIN (Degree INNER JOIN Dov ON Degree.degreeID = Dov.degreeID) ON AcademicDirection.academicDirectionID = Dov.academicDirectionID) INNER JOIN userStatusOrganizationSubdivision ON Dov.dovID = userStatusOrganizationSubdivision.userID) ON (SubdivisionLevel1.subdivisionLevel1ID = userStatusOrganizationSubdivision.subdivisionLevel1ID) AND (Organization.organizationID = userStatusOrganizationSubdivision.organizationID) "
                                                "WHERE (((Dov.dovID) In (SELECT userID FROM userTheme WHERE themeID = %1))) ")
                                        .arg(themeCode);

                                QString consultantFIO, consultantDegree, consultantAcademicDirection, consultantPost, consultantSubdivision, consultantOrganization;
                                QString childFIO, childOrganization, childSubdivision, childPost;
                                QString teacherFIO, teacherOrganization, teacherSubdivision, teacherPost;
                                QString consultantEMail, teacherEMail, childEMail;

                                int userStatus = 0;

                                if (q.exec(query)) {
                                    while (q.next()) {
                                        userStatus = q.value(1).toInt();

                                        //Если консультант
                                        if (userStatus == 4) {
                                            consultantFIO = q.value(0).toString();
                                            consultantDegree = q.value(2).toString();
                                            consultantAcademicDirection = q.value(3).toString();
                                            consultantOrganization = q.value(4).toString();
                                            consultantSubdivision = q.value(5).toString();
                                            consultantPost = q.value(6).toString();
                                            consultantEMail = q.value(7).toString();
                                        }

                                        //Если учитель
                                        if (userStatus == 3) {
                                            teacherFIO = q.value(0).toString();
                                            teacherOrganization = q.value(4).toString();
                                            teacherSubdivision = q.value(5).toString();
                                            teacherPost = q.value(6).toString();
                                            teacherEMail = q.value(7).toString();
                                        }

                                        //Если ученик
                                        if (userStatus == 2) {
                                            childFIO = q.value(0).toString();
                                            childOrganization = q.value(4).toString();
                                            childSubdivision = q.value(5).toString();
                                            childPost = q.value(6).toString();
                                            childEMail = q.value(7).toString();
                                        }
                                    }


                                    //Непосредственная отправка уведомления учителю

                                    QByteArray b;
                                    b.clear();
                                    b.append(teacherFIO);

                                    QProcess p;
                                    QStringList parametres;
                                    parametres.clear();

                                    parametres.append("proposedPerformed");

                                    //Информация о консультанте
                                    b.clear();
                                    b.append(consultantFIO); // ФИО консультанта
                                    parametres.append(b.toBase64());

                                    b.clear();
                                    b.append(consultantDegree);
                                    parametres.append(b.toBase64());

                                    b.clear();
                                    b.append(consultantAcademicDirection);
                                    parametres.append(b.toBase64());

                                    b.clear();
                                    b.append(consultantPost);
                                    parametres.append(b.toBase64());

                                    b.clear();
                                    b.append(consultantSubdivision);
                                    parametres.append(b.toBase64());

                                    b.clear();
                                    b.append(consultantOrganization);
                                    parametres.append(b.toBase64());

                                    //Информация об ученике
                                    b.clear();
                                    b.append(childFIO);
                                    parametres.append(b.toBase64());

                                    b.clear();
                                    b.append(childOrganization);
                                    parametres.append(b.toBase64());

                                    b.clear();
                                    b.append(childSubdivision);
                                    parametres.append(b.toBase64());

                                    b.clear();
                                    b.append(childPost);
                                    parametres.append(b.toBase64());

                                    //Информация об учителе
                                    b.clear();
                                    b.append(teacherFIO);
                                    parametres.append(b.toBase64());

                                    b.clear();
                                    b.append(teacherOrganization);
                                    parametres.append(b.toBase64());

                                    b.clear();
                                    b.append(teacherSubdivision);
                                    parametres.append(b.toBase64());

                                    b.clear();
                                    b.append(teacherPost);
                                    parametres.append(b.toBase64());

                                    //Информация о теме
                                    b.clear();
                                    b.append(themeName);
                                    parametres.append(b.toBase64());

                                    //Адреса электронной почты участников
                                    b.clear();
                                    b.append(consultantEMail.trimmed());
                                    parametres.append(b.toBase64());

                                    b.clear();
                                    b.append(teacherEMail.trimmed());
                                    parametres.append(b.toBase64());

                                    b.clear();
                                    b.append(childEMail.trimmed());
                                    parametres.append(b.toBase64());

                                    p.start("C:/vzlet/modules/emailNotifierVZLET.exe", parametres);
                                    p.waitForReadyRead(10 * 1000);
                                    int result = p.readAll().toInt();

                                    switch (result) {
                                    case 1:
                                        //printf(QString("Вы были успешно закреплены за выбранной темой! ").toLocal8Bit());
                                        printf("ok");
                                        break;
                                    case -1:
                                        printf("error ");
                                        break;
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
        }
        else {
            printf("[error]");
            a.quit();
            return (200);
        }

    }






    //Вывод на экран текущей даты (для расчетов)
    if (params.at(0) == "getCurrentDate") {
        printf(QString("{\"currentDate\":\"%1\"}").arg(QDate::currentDate().toString("yyyy-MM-dd")).toLocal8Bit());
        a.quit();
        return (200);
    }

    //Вывод на экран текущей даты
    if (params.at(0) == "getServerCurrentDate") {
        printf(QString("{\"currentDate\":\"%1\"}").arg(QDate::currentDate().toString("dd.MM.yyyy")).toLocal8Bit());
        a.quit();
        return (200);
    }




    //Ввод новой темы (Ввод запросов на консультирование) - учителем
    if (params.at(0) == "renameTheme") {
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

            //Если вошел консультант
            if (currentUserStatus != 4) {
                a.quit();
                return (401);
            }
            else {
                QSqlQuery q(db);
                QString query;

                int themeCode = params.at(1).toInt();

                QByteArray b;
                b.clear();
                b.append(params.at(2));

                QString themeName = TEXT(QByteArray::fromBase64(b), v);

                b.clear();
                b.append(params.at(3));

                QString themeComment = TEXT(QByteArray::fromBase64(b), v);

                b.clear();

                query = QString("UPDATE vzletbase.theme "
                                "SET themeName = '%1', themeComment = '%2' "
                                "WHERE themeID = %3 ")
                        .arg(themeName)
                        .arg(themeComment)
                        .arg(themeCode);

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
    }





    //Вывод на экран списка университетов
    if (params.at(0) == "getUniversity") {
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
                    "WHERE (organizationTypeID = 3) AND (universityObserver = FALSE) ";

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"universityCode\":%1,"
                                          "\"universityName\":\"%2\"},")
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


    //Вывод на экран списка университетов
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

            int universityCode = params.at(1).toInt();

            query = QString("SELECT dovID, userFIO FROM vzletbase.dov WHERE "
                            "dovID IN "
                            "( "
                            "SELECT userID FROM vzletbase.userStatusOrganizationSubdivision "
                            "WHERE (statusID = 4) AND (organizationID = %1) "
                            ")  ")
                    .arg(universityCode);

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



    //Ввод новой темы (Ввод запросов на консультирование) - учителем - с известным консультантом (формирование коллектива)
    if (params.at(0) == "enterRequestByTeacher1") {
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

            //Если вошел учитель
            if (currentUserStatus != 3) {
                a.quit();
                return (401);
            }
            else {
                QByteArray b;
                b.append(params.at(1));

                QString themeName = TEXT(QByteArray::fromBase64(b), v);

                b.clear();
                b.append(params.at(2));

                QString comment = TEXT(QByteArray::fromBase64(b), v);

                int priorityDirection = params.at(3).toInt();
                int section = params.at(4).toInt();
                int scienceIndustrialSphere = params.at(5).toInt();

                int scienceDirection1 = params.at(6).toInt();
                int scienceDirection2 = params.at(7).toInt();
                int scienceDirection3 = params.at(8).toInt();


                bool ok1, ok2 = true; //Проверка ввода числа
                int childCode = params.at(9).toInt(&ok1); //Код ученика
                int consultantCode = params.at(12).toInt(&ok2); //Код консультанта


                if (ok1 && ok2) {
                    b.clear();
                    b.append(params.at(10));

                    QString infoByTeacher = TEXT(QByteArray::fromBase64(b), v); //Кратко об учителе

                    b.clear();
                    b.append(params.at(11));

                    QString infoByChild = TEXT(QByteArray::fromBase64(b), v); //Кратко об ученике

                    //Проверим, есть ли такой ученик и консультант
                    if (checkUserExist(&db, childCode, 2) && checkUserExist(&db, consultantCode, 4)) {

                        query = QString("INSERT INTO vzletbase.Theme "
                                        "(themeName, themeComment, themeDateTime, scienceDirectionID_1, "
                                        "scienceDirectionID_2, scienceDirectionID_3, priorityDirectionID, sectionID, scienceIndustrialSphereID, "
                                        "themeAboutTutor, themeAboutChild, statusThemeID, themeStaffing, stageExecuteID, themeDateTimeUpdateStatus) "
                                        "VALUES "
                                        "('%1', '%2', NOW(), %3, %4, %5, %6, %7, %8, '%9', '%10', 2, TRUE, 2, NOW()) ")
                                .arg(themeName)
                                .arg(comment)
                                .arg(scienceDirection1)
                                .arg(scienceDirection2)
                                .arg(scienceDirection3)
                                .arg(priorityDirection)
                                .arg(section)
                                .arg(scienceIndustrialSphere)
                                .arg(infoByTeacher)
                                .arg(infoByChild);


                        if (q.exec(query)) {
                            //Получаем код только что введенной темы
                            query = QString("SELECT THEMEID FROM VZLETBASE.THEME WHERE "
                                            "(themeName = '%1') AND "
                                            "(themeComment = '%2') AND "
                                            "(scienceDirectionID_1 = %3) AND "
                                            "(scienceDirectionID_2 = %4) AND "
                                            "(scienceDirectionID_3 = %5) AND "
                                            "(priorityDirectionID = %6) AND "
                                            "(sectionID = %7) AND "
                                            "(scienceIndustrialSphereID = %8) "
                                            "AND (stageExecuteID = 2) "
                                            "AND (themeAboutTutor = '%9') AND (themeAboutChild = '%10') ")
                                    .arg(themeName)
                                    .arg(comment)
                                    .arg(scienceDirection1)
                                    .arg(scienceDirection2)
                                    .arg(scienceDirection3)
                                    .arg(priorityDirection)
                                    .arg(section)
                                    .arg(scienceIndustrialSphere)
                                    .arg(infoByTeacher)
                                    .arg(infoByChild);

                            if (q.exec(query)) {
                                int themeCode = 0;
                                while (q.next()) {
                                    themeCode = q.value(0).toInt();
                                }

                                //Прикрепление учителя
                                query = QString("INSERT INTO VZLETBASE.USERTHEME (userID, themeID, userThemeDateTime, statusParticipationID) VALUES "
                                                "(%1, %2, NOW(), NULL) ")
                                        .arg(userCode)
                                        .arg(themeCode);

                                if (q.exec(query)) {
                                    //Прикрепление ученика
                                    query = QString("INSERT INTO VZLETBASE.USERTHEME (userID, themeID, userThemeDateTime, statusParticipationID) VALUES "
                                                    "(%1, %2, NOW(), NULL) ")
                                            .arg(childCode)
                                            .arg(themeCode);

                                    if (q.exec(query)) {

                                        //Прикрепление консультанта
                                        query = QString("INSERT INTO VZLETBASE.USERTHEME (userID, themeID, userThemeDateTime, statusParticipationID) VALUES "
                                                        "(%1, %2, NOW(), NULL) ")
                                                .arg(consultantCode)
                                                .arg(themeCode);
                                        if (q.exec(query)) {

                                            //Получаем данные консультанта
                                            query = QString("SELECT userID, userFIO, organizationName, subdivisionLevel1Name, userEMail "
                                                            "FROM vzletbase.userStatusOrganizationSubdivision, vzletbase.dov, vzletbase.organization, vzletbase.subdivisionLevel1 "
                                                            "WHERE "
                                                            "(userID = 140) AND "
                                                            "(userStatusOrganizationSubdivision.statusID = 4) AND "
                                                            "(userStatusOrganizationSubdivision.userID = dov.dovID) AND "
                                                            "(userStatusOrganizationSubdivision.organizationID = organization.organizationID) AND "
                                                            "(userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID) ")
                                                    .arg(consultantCode);

                                            QString consultantFIO, consultantOrganizationName, consultantSubdivisionName, consultantEMail;

                                            if (q.exec(query)) {
                                                while (q.next()) {
                                                    consultantFIO = q.value(1).toString();
                                                    consultantOrganizationName = q.value(2).toString();
                                                    consultantSubdivisionName = q.value(3).toString();
                                                    consultantEMail = q.value(4).toString();
                                                }


                                                //Получаем данные учителя
                                                query = QString("SELECT userID, userFIO, organizationName, subdivisionLevel1Name, postName, userEMail "
                                                                "FROM vzletbase.userStatusOrganizationSubdivision, vzletbase.dov, vzletbase.organization, vzletbase.subdivisionLevel1 "
                                                                "WHERE "
                                                                "(userID = %1) AND "
                                                                "(userStatusOrganizationSubdivision.statusID = 3) AND "
                                                                "(userStatusOrganizationSubdivision.userID = dov.dovID) AND "
                                                                "(userStatusOrganizationSubdivision.organizationID = organization.organizationID) AND "
                                                                "(userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID) ")
                                                        .arg(userCode);

                                                QString teacherFIO, teacherOrganizationName, teacherSubdivisionName, teacherPost, teacherEMail;

                                                if (q.exec(query)) {
                                                    while (q.next()) {
                                                        teacherFIO = q.value(1).toString();
                                                        teacherOrganizationName = q.value(2).toString();
                                                        teacherSubdivisionName = q.value(3).toString();
                                                        teacherPost = q.value(4).toString();
                                                        teacherEMail = q.value(5).toString();
                                                    }

                                                    //Получаем данные ученика
                                                    query = QString("SELECT userID, userFIO, organizationName, subdivisionLevel1Name, childGradeName, userEMail "
                                                                    "FROM vzletbase.userStatusOrganizationSubdivision, vzletbase.dov, vzletbase.organization, vzletbase.subdivisionLevel1, vzletbase.childGrade "
                                                                    "WHERE "
                                                                    "(userID = %1) AND "
                                                                    "(userStatusOrganizationSubdivision.statusID = 2) AND "
                                                                    "(userStatusOrganizationSubdivision.userID = dov.dovID) AND "
                                                                    "(userStatusOrganizationSubdivision.organizationID = organization.organizationID) AND "
                                                                    "(userStatusOrganizationSubdivision.subdivisionLevel1ID = subdivisionLevel1.subdivisionLevel1ID) AND "
                                                                    "(userStatusOrganizationSubdivision.childGradeID = childGrade.childGradeID) ")
                                                            .arg(childCode);

                                                    QString childFIO, childOrganizationName, childSubdivisionName, childGrade, childEMail;

                                                    if (q.exec(query)) {
                                                        while (q.next()) {
                                                            childFIO = q.value(1).toString();
                                                            childOrganizationName = q.value(2).toString();
                                                            childSubdivisionName = q.value(3).toString();
                                                            childGrade = q.value(4).toString();
                                                            childEMail = q.value(5).toString();
                                                        }

                                                        db.close();

                                                        QByteArray b;

                                                        QProcess p;
                                                        QStringList parametres;
                                                        parametres.clear();

                                                        parametres.append("setCollective");

                                                        //Информация о консультанте
                                                        b.clear();
                                                        b.append(consultantFIO); // ФИО консультанта
                                                        parametres.append(b.toBase64());

                                                        b.clear();
                                                        b.append(consultantOrganizationName);
                                                        parametres.append(b.toBase64());

                                                        b.clear();
                                                        b.append(consultantSubdivisionName);
                                                        parametres.append(b.toBase64());


                                                        //Информация об ученике
                                                        b.clear();
                                                        b.append(childFIO);
                                                        parametres.append(b.toBase64());

                                                        b.clear();
                                                        b.append(childOrganizationName);
                                                        parametres.append(b.toBase64());

                                                        b.clear();
                                                        b.append(childSubdivisionName);
                                                        parametres.append(b.toBase64());

                                                        b.clear();
                                                        b.append(childGrade);
                                                        parametres.append(b.toBase64());

                                                        b.clear();
                                                        b.append(infoByChild);
                                                        parametres.append(b.toBase64());

                                                        //Информация об учителе
                                                        b.clear();
                                                        b.append(teacherFIO);
                                                        parametres.append(b.toBase64());

                                                        b.clear();
                                                        b.append(teacherOrganizationName);
                                                        parametres.append(b.toBase64());

                                                        b.clear();
                                                        b.append(teacherSubdivisionName);
                                                        parametres.append(b.toBase64());

                                                        b.clear();
                                                        b.append(teacherPost);
                                                        parametres.append(b.toBase64());

                                                        b.clear();
                                                        b.append(infoByTeacher);
                                                        parametres.append(b.toBase64());

                                                        //Информация о теме
                                                        b.clear();
                                                        b.append(themeName);
                                                        parametres.append(b.toBase64());

                                                        //Напоминание консультанту о прежней совместной работе
                                                        b.clear();
                                                        b.append(comment);
                                                        parametres.append(b.toBase64());

                                                        //Адреса электронной почты участников
                                                        b.clear();
                                                        b.append(consultantEMail.trimmed());
                                                        parametres.append(b.toBase64());

                                                        b.clear();
                                                        b.append(teacherEMail.trimmed());
                                                        parametres.append(b.toBase64());

                                                        b.clear();
                                                        b.append(childEMail.trimmed());
                                                        parametres.append(b.toBase64());


                                                        //Отправка уведомлений
                                                        p.start("C:/vzlet/modules/emailNotifierVZLET.exe", parametres);
                                                        p.waitForReadyRead(20 * 1000);
                                                        int result = p.readAll().toInt();

                                                        switch (result) {
                                                        case 1:
                                                            printf("ok");
                                                            a.quit();
                                                            return (200);
                                                            break;
                                                        case -1:
                                                            printf("error ");
                                                            a.quit();
                                                            return (200);
                                                            break;
                                                        default:
                                                            printf("error sent e-mail");
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
                            printf("[error]");
                            a.quit();
                            return (200);
                        }
                    }
                    else {
                        if (!checkUserExist(&db, childCode, 2)) {
                            printf(QString("Проверьте правильность выбора ученика! ").toLocal8Bit());
                            a.quit();
                            return (200);
                        }

                        if (!checkUserExist(&db, consultantCode, 4)) {
                            printf(QString("Проверьте правильность выбора консультанта! ").toLocal8Bit());
                            a.quit();
                            return (200);
                        }
                    }
                }
                else {
                    if (!ok1) {
                        printf(QString("Проверьте правильность выбора ученика! ").toLocal8Bit());
                        a.quit();
                        return (200);
                    }

                    if (!ok2) {
                        printf(QString("Проверьте правильность выбора консультанта! ").toLocal8Bit());
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



    //Вывод на экран списка типов организаций
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

            query = "SELECT organizationTypeID, organizationTypeName FROM vzletbase.organizationType "
                    "WHERE organizationTypeID IN (2, 3, 14) ";

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



    //Вывод на экран списка организаций по типу организации
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

            int organizationTypeCode = params.at(1).toInt();

            query = QString("SELECT organizationID, organizationName FROM vzletbase.organization "
                            "WHERE organizationTypeID = %1 ")
                    .arg(organizationTypeCode);

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





    //Получить список пользователей в системе в зависимости от выбранного типа организации, организации и статуса пользователя (ученик, учитель ии консультант)
    if (params.at(0) == "getStatusList1") {
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

            int statusCode = params.at(1).toInt();
            int organizationCode = params.at(2).toInt();

            query = QString("SELECT Dov.dovID, Dov.userFIO, Organization.organizationName, SubdivisionLevel1.subdivisionLevel1Name, "
                            "userStatusOrganizationSubdivision.postName, Status.statusName, StageExecute.stageExecuteName, "
                            "Theme.stageExecuteID, Theme.themeName, StatusTheme.statusThemeName, theme.statusThemeID "
                            "FROM (StatusTheme INNER JOIN (StageExecute INNER JOIN Theme ON StageExecute.stageExecuteID = Theme.stageExecuteID) ON StatusTheme.statusThemeID = Theme.statusThemeID) INNER JOIN (Status INNER JOIN ((Organization INNER JOIN SubdivisionLevel1 ON Organization.organizationID = SubdivisionLevel1.organizationID) INNER JOIN ((Dov INNER JOIN userStatusOrganizationSubdivision ON Dov.dovID = userStatusOrganizationSubdivision.userID) INNER JOIN UserTheme ON Dov.dovID = UserTheme.userID) ON (SubdivisionLevel1.subdivisionLevel1ID = userStatusOrganizationSubdivision.subdivisionLevel1ID) AND (Organization.organizationID = userStatusOrganizationSubdivision.organizationID)) ON Status.statusID = userStatusOrganizationSubdivision.statusID) ON Theme.themeID = UserTheme.themeID "
                            "WHERE (((userStatusOrganizationSubdivision.statusID)=%1) AND ((userStatusOrganizationSubdivision.organizationID)=%2)) ")
                    .arg(statusCode)
                    .arg(organizationCode);

            if (q.exec(query)) {
                while (q.next()) {
                    result.append("{");
                    result.append(QString("\"statusListCode\":%1,"
                                          "\"statusListFIO\":\"%2\","
                                          "\"statusListOrganizationName\":\"%3\","
                                          "\"statusListSubdivisionLevel1Name\":\"%4\","
                                          "\"statusListPostName\":\"%5\","
                                          "\"statusListStatusName\":\"%6\","
                                          "\"statusListCurrentStageExecute\":\"%7\","
                                          "\"statusListCurrentStageCode\":%8,"
                                          "\"statusListThemeName\":\"%9\","
                                          "\"statusListStatusThemeName\":\"%10\","
                                          "\"statusListStatusThemeCode\":%11},")
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




    //Консультант отклонил тему
    if (params.at(0) == "themeReject") {
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

            //Если консультант
            if (currentUserStatus == 4) {

                int themeCode = params.at(1).toInt(); //Код темы

                //Является ли консультант консультантом по этой теме?
                if (checkConsultantTheme(&db, userCode, themeCode)) {

                    QByteArray b;
                    b.clear();
                    b.append(params.at(2));

                    //Причина отклонения (вводит консультант)
                    QString causeReject = TEXT(QByteArray::fromBase64(b), v);

                    causeReject = causeReject.replace("\n", "");
                    causeReject = causeReject.replace("\r", "");
                    causeReject = causeReject.replace("\r\n", "");

                    //Отправка письма участникам коллектива об отказе
                    QProcess p;
                    QStringList parametres;
                    parametres.clear();

                    parametres.append("themeReject");

                    QString consultantFIO;
                    QStringList emails;
                    QString themeName;

                    //Получаем текущий статус темы
                    int currentStatusTheme = 0;
                    query = QString("SELECT statusThemeID FROM vzletbase.theme WHERE themeID = %1 ").arg(themeCode);

                    if (q.exec(query)) {
                        while (q.next()) {
                            currentStatusTheme = q.value(0).toInt();
                        }

                        //Если тема предлагаемая или запрос не отработал
                        if (currentStatusTheme == 1 || currentStatusTheme == 0) {
                            printf(QString("Вы не можете повторно отклонить заявку по данной теме! ").toLocal8Bit());
                            a.quit();
                            return (200);
                        }
                        else {
                            //Получаем ФИО консультанта
                            query = QString("SELECT userFIO FROM vzletbase.dov WHERE dovID = %1 ").arg(userCode);
                            if (q.exec(query)) {
                                while (q.next()) {
                                    consultantFIO = q.value(0).toString();
                                }


                                //Получаем тему проекта
                                query = QString("SELECT themeName FROM vzletbase.theme WHERE themeID = %1 ").arg(themeCode);
                                if (q.exec(query)) {
                                    while (q.next()) {
                                        themeName = q.value(0).toString();
                                    }


                                    //Получаем адреса электронной почты ученика и учителя
                                    query = QString("SELECT userEMail "
                                                    "FROM vzletbase.dov "
                                                    "WHERE dovID IN "
                                                    "( "
                                                    "SELECT userID FROM vzletbase.userTheme WHERE (themeID = %1) AND (userID <> %2) "
                                                    ") ")
                                            .arg(themeCode)
                                            .arg(userCode);

                                    if (q.exec(query)) {

                                        emails.clear();
                                        while (q.next()) {
                                            emails.append(q.value(0).toString());
                                        }


                                        b.clear();
                                        b.append(emails.at(0)); // EMail 1
                                        parametres.append(b.toBase64());

                                        b.clear();
                                        b.append(emails.at(1)); // EMail 2
                                        parametres.append(b.toBase64());

                                        b.clear();
                                        b.append(consultantFIO); // ФИО консультанта
                                        parametres.append(b.toBase64());

                                        b.clear();
                                        b.append(themeName); // Название темы
                                        parametres.append(b.toBase64());

                                        b.clear();
                                        b.append(causeReject); //Причина отказа
                                        parametres.append(b.toBase64());


                                        //Производим расформирование коллектива по этой теме

                                        //При условии, что статус темы не "Предлагаемая консультантом"

                                        //Получаем список участников темы (только учителя и ученика)
                                        query = QString("SELECT userThemeID FROM vzletbase.userTheme WHERE "
                                                        "userID IN ( "
                                                        "SELECT dovID "
                                                        "FROM vzletbase.dov, vzletbase.userStatusOrganizationSubdivision  "
                                                        "WHERE "
                                                        "(dovID IN (SELECT userID FROM vzletbase.userTheme WHERE themeID = %1)) AND "
                                                        "(userStatusOrganizationSubdivision.userID = dov.dovID) AND (userStatusOrganizationSubdivision.statusID <> 4) "
                                                        ") "
                                                        "AND themeID = %1 ")
                                                .arg(themeCode);

                                        //Номера записей, которые нужно удалить
                                        QStringList records;

                                        if(q.exec(query)) {
                                            records.clear();
                                            while (q.next()) {
                                                records.append(q.value(0).toString());
                                            }


                                            //Теперь удаляем этих пользователей из участников данной темы
                                            for(int i = 0; i <= 1; i++) {
                                                query = QString("DELETE FROM vzletbase.userTheme WHERE userThemeID = %1 ").arg(records.at(i));
                                                if (!q.exec(query)) {
                                                    printf(QString("Ошибка расформирования коллектива по данной теме! \nОбратитесь к администратору систему по адресу admin@vzletsamara.ru").toLocal8Bit());
                                                    a.quit();
                                                    return (200);
                                                }
                                            }


                                            //Изменение данных темы - статуса на "Предлагаемая консультантом" и стадии выполнения - "проект в стадии формирования"
                                            //А также изменяем дату изменения статуса темы
                                            query = QString("UPDATE vzletbase.theme "
                                                            "SET statusThemeID = 1, stageExecuteID = 1, themeStaffing = FALSE, themeDateTimeUpdateStatus = NOW() "
                                                            "WHERE themeID = %1 ").arg(themeCode);

                                            if (q.exec(query)) {
                                                //Отправка уведомлений
                                                p.start("C:/vzlet/modules/emailNotifierVZLET.exe", parametres);
                                                p.waitForReadyRead(20 * 1000);
                                                int result = p.readAll().toInt();

                                                switch (result) {
                                                case 1:
                                                    printf(QString("Поданная заявка успешно отклонена! ").toLocal8Bit());
                                                    a.quit();
                                                    return (200);
                                                    break;
                                                case -1:
                                                    printf("error ");
                                                    a.quit();
                                                    return (200);
                                                    break;

                                                default:
                                                    printf("error sent e-mail");
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




    //Перевод консультантом темы в "Выполняемую"
    if (params.at(0) == "setBringToPerformedTheme") {
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

            //Если консультант
            if (currentUserStatus == 4)
            {
                bool ok = true;
                int themeCode = params.at(1).toInt(&ok);

                if (ok)
                {
                    //Проверка, что консультант имеет право редактировать данные этой темы (что эта тема принадлежит ему)
                    if (checkConsultantTheme(&db, userCode, themeCode)) {

                        QSqlQuery q(db);
                        QString query = "";

                        //Получаем текущий статус темы
                        int currentStatusTheme = 0;

                        query = QString("SELECT statusThemeID FROM vzletbase.theme WHERE themeID = %1 ").arg(themeCode);
                        if (q.exec(query)) {
                            while (q.next()) {
                                currentStatusTheme = q.value(0).toInt();
                            }


                            //Если текущий статус темы не "Предлагаемая консультантом"
                            if (currentStatusTheme != 6) {
                                printf(QString("Вы не можете сменить статус этой темы на \"Выполняемая\"").toLocal8Bit());
                                a.quit();
                                return (200);
                            }
                            else {
                                //Изменение статуса темы на "Выполняемая" и этапа выполнения на "сформирован коллектив", а также даты изменения статуса темы
                                query = QString("UPDATE vzletbase.theme "
                                                "SET statusThemeID = 2, stageExecuteID = 2, themeDateTimeUpdateStatus = NOW() "
                                                "WHERE themeID = %1 ").arg(themeCode);
                                if (q.exec(query)) {
                                    QString themeName;
                                    //Получаем название темы
                                    query = QString("SELECT themeName "
                                                    "FROM vzletbase.theme "
                                                    "WHERE "
                                                    "themeID = %1 ")
                                            .arg(themeCode);

                                    if (q.exec(query)) {
                                        while (q.next()) {
                                            themeName = q.value(0).toString();
                                        }


                                        //Получаем все сведения об участниках данной темы
                                        query = QString("SELECT Dov.userFIO, userStatusOrganizationSubdivision.statusID, Degree.degreeName, AcademicDirection.academicDirectionName, "
                                                        "Organization.organizationName, SubdivisionLevel1.subdivisionLevel1Name, userStatusOrganizationSubdivision.postName, Dov.userEMail "
                                                        "FROM (Organization INNER JOIN SubdivisionLevel1 ON Organization.organizationID = SubdivisionLevel1.organizationID) INNER JOIN ((AcademicDirection INNER JOIN (Degree INNER JOIN Dov ON Degree.degreeID = Dov.degreeID) ON AcademicDirection.academicDirectionID = Dov.academicDirectionID) INNER JOIN userStatusOrganizationSubdivision ON Dov.dovID = userStatusOrganizationSubdivision.userID) ON (SubdivisionLevel1.subdivisionLevel1ID = userStatusOrganizationSubdivision.subdivisionLevel1ID) AND (Organization.organizationID = userStatusOrganizationSubdivision.organizationID) "
                                                        "WHERE (((Dov.dovID) In (SELECT userID FROM userTheme WHERE themeID = %1))) ")
                                                .arg(themeCode);

                                        QString consultantFIO, consultantDegree, consultantAcademicDirection, consultantPost, consultantSubdivision, consultantOrganization;
                                        QString childFIO, childOrganization, childSubdivision, childPost;
                                        QString teacherFIO, teacherOrganization, teacherSubdivision, teacherPost;
                                        QString consultantEMail, teacherEMail, childEMail;

                                        int userStatus = 0;

                                        if (q.exec(query)) {
                                            while (q.next()) {
                                                userStatus = q.value(1).toInt();

                                                //Если консультант
                                                if (userStatus == 4) {
                                                    consultantFIO = q.value(0).toString();
                                                    consultantDegree = q.value(2).toString();
                                                    consultantAcademicDirection = q.value(3).toString();
                                                    consultantOrganization = q.value(4).toString();
                                                    consultantSubdivision = q.value(5).toString();
                                                    consultantPost = q.value(6).toString();
                                                    consultantEMail = q.value(7).toString();
                                                }

                                                //Если учитель
                                                if (userStatus == 3) {
                                                    teacherFIO = q.value(0).toString();
                                                    teacherOrganization = q.value(4).toString();
                                                    teacherSubdivision = q.value(5).toString();
                                                    teacherPost = q.value(6).toString();
                                                    teacherEMail = q.value(7).toString();
                                                }

                                                //Если ученик
                                                if (userStatus == 2) {
                                                    childFIO = q.value(0).toString();
                                                    childOrganization = q.value(4).toString();
                                                    childSubdivision = q.value(5).toString();
                                                    childPost = q.value(6).toString();
                                                    childEMail = q.value(7).toString();
                                                }
                                            }


                                            //Отправка письма участникам

                                            QByteArray b;

                                            QProcess p;
                                            QStringList parametres;
                                            parametres.clear();

                                            parametres.append("proposedPerformed");

                                            //Информация о консультанте
                                            b.clear();
                                            b.append(consultantFIO); // ФИО консультанта
                                            parametres.append(b.toBase64());

                                            b.clear();
                                            b.append(consultantDegree);
                                            parametres.append(b.toBase64());

                                            b.clear();
                                            b.append(consultantAcademicDirection);
                                            parametres.append(b.toBase64());

                                            b.clear();
                                            b.append(consultantPost);
                                            parametres.append(b.toBase64());

                                            b.clear();
                                            b.append(consultantSubdivision);
                                            parametres.append(b.toBase64());

                                            b.clear();
                                            b.append(consultantOrganization);
                                            parametres.append(b.toBase64());

                                            //Информация об ученике
                                            b.clear();
                                            b.append(childFIO);
                                            parametres.append(b.toBase64());

                                            b.clear();
                                            b.append(childOrganization);
                                            parametres.append(b.toBase64());

                                            b.clear();
                                            b.append(childSubdivision);
                                            parametres.append(b.toBase64());

                                            b.clear();
                                            b.append(childPost);
                                            parametres.append(b.toBase64());

                                            //Информация об учителе
                                            b.clear();
                                            b.append(teacherFIO);
                                            parametres.append(b.toBase64());

                                            b.clear();
                                            b.append(teacherOrganization);
                                            parametres.append(b.toBase64());

                                            b.clear();
                                            b.append(teacherSubdivision);
                                            parametres.append(b.toBase64());

                                            b.clear();
                                            b.append(teacherPost);
                                            parametres.append(b.toBase64());

                                            //Информация о теме
                                            b.clear();
                                            b.append(themeName);
                                            parametres.append(b.toBase64());

                                            //Адреса электронной почты участников
                                            b.clear();
                                            b.append(consultantEMail.trimmed());
                                            parametres.append(b.toBase64());

                                            b.clear();
                                            b.append(teacherEMail.trimmed());
                                            parametres.append(b.toBase64());

                                            b.clear();
                                            b.append(childEMail.trimmed());
                                            parametres.append(b.toBase64());


                                            //Отправка уведомлений
                                            p.start("C:/vzlet/modules/emailNotifierVZLET.exe", parametres);
                                            p.waitForReadyRead(20 * 1000);
                                            int result = p.readAll().toInt();

                                            switch (result) {
                                            case 1:
                                                printf("ok");
                                                a.quit();
                                                return (200);
                                                break;
                                            case -1:
                                                printf("error ");
                                                a.quit();
                                                return (200);
                                                break;

                                            default:
                                                printf("error sent e-mail");
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
                                    printf(QString("Ошибка при изменении статуса темы! \nОбратитесь к администратору системы по адресу admin@vzletsamara.ru ").toLocal8Bit());
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
                        printf(QString("Вы не имеете права изменять статус этой темы!").toLocal8Bit());
                        a.quit();
                        return (200);
                    }
                }
                else {
                    printf(QString("Вы не имеете права изменять статус этой темы!").toLocal8Bit());
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







    //Проверка - разрещил ли учитель отправку работы на конкурс
    if (params.at(0) == "checkAllowSendToConkurs") {
        db = QSqlDatabase::addDatabase("QMYSQL", "db");
        db.setDatabaseName("vzletbase");
        db.setHostName("");
        db.setUserName("root");
        db.setPassword("");
        db.setPort(3306);
        db.open();
        if (db.isOpen()) {
            QSqlQuery q(db);

            int userCode = QString("%1").arg(personID).toInt();
            int currentUserStatus = checkUserStatus(&db, userCode);

            //Если ученик или учитель
            if (currentUserStatus == 2 || currentUserStatus == 3) {

                bool ok = true;
                int themeCode = params.at(1).toInt(&ok);

                if (ok) {
                    //Относится ли данная тема к данному пользователю - ученику
                    if (checkUserTheme(&db, userCode, themeCode)) {
                        //Проверка - разрешил ли учитель
                        if (checkAllowProjectToSendConkurs(&db, themeCode)) {
                            printf("{\"resultCode\":1}");
                            a.quit();
                            return (200);
                        }
                        else {
                            printf("{\"resultCode\":0}");
                            a.quit();
                            return (200);
                        }
                    }
                    else {
                        printf(QString("Вы не можете получить такую информацию по теме другого пользователя! ").toLocal8Bit());
                        a.quit();
                        return (200);
                    }
                }
                else {
                    printf(QString("Проверьте правильность кода темы! ").toLocal8Bit());
                    a.quit();
                    return (200);
                }
            }
            else {
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



    //Установить признак того, что разрешается отправить работу на конкурс
    if (params.at(0) == "setAllowToSendConkurs") {
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

            //Если учитель
            if (currentUserStatus == 2 || currentUserStatus == 3) {

                bool ok = true;
                int themeCode = params.at(1).toInt(&ok);

                if (ok) {
                    //Относится ли данная тема к данному пользователю - учителю
                    if (checkUserTheme(&db, userCode, themeCode)) {
                        //Установка признака разрешения
                        query = QString("UPDATE vzletbase.theme "
                                        "SET sendContentAllow = TRUE "
                                        "WHERE themeID = %1 ").arg(themeCode);
                        if (q.exec(query)){
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
                    printf(QString("Проверьте правильность кода темы! ").toLocal8Bit());
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


    //Снять признак того, что разрешается отправить работу на конкурс
    if (params.at(0) == "unSetAllowToSendConkurs") {
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

            //Если учитель
            if (currentUserStatus == 3) {

                bool ok = true;
                int themeCode = params.at(1).toInt(&ok);

                if (ok) {

                    //Проверка на то, что ученик загрузил файл
                    if (checkUploadFileByTheme(&db, themeCode)) {
                        //Относится ли данная тема к данному пользователю - учителю
                        if (checkUserTheme(&db, userCode, themeCode)) {
                            //Установка признака неразрешения
                            query = QString("UPDATE vzletbase.theme "
                                            "SET sendContentAllow = FALSE "
                                            "WHERE themeID = %1 ").arg(themeCode);
                            if (q.exec(query)){
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
                        printf(QString("Невозможно снять разрешение на отправку проекта на конкурс, т.к. файл проекта уже загружен в систему! ").toLocal8Bit());
                        a.quit();
                        return (200);
                    }
                }
                else {
                    printf(QString("Проверьте правильность кода темы! ").toLocal8Bit());
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




    //Снять признак того, что разрешается отправить работу на конкурс
    if (params.at(0) == "checkFileByTheme") {
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

            //Если учитель
            if (currentUserStatus == 3) {

                bool ok = true;
                int themeCode = params.at(1).toInt(&ok);

                if (ok) {

                    //Относится ли данная тема к данному пользователю - учителю
                    if (checkUserTheme(&db, userCode, themeCode)) {

                        //Проверка на то, что ученик загрузил файл
                        if (checkUploadFileByTheme(&db, themeCode)) {
                            printf("{\"checkCode\":0}");
                            a.quit();
                            return (200);
                        }
                        else {
                            printf("{\"checkCode\":1}");
                            a.quit();
                            return (200);
                        }
                    }
                    else {
                        printf(QString("Вы не можете узнать данный параметр по работе другого пользователя! ").toLocal8Bit());
                        a.quit();
                        return (200);
                    }
                }
                else {
                    printf(QString("Проверьте правильность кода темы! ").toLocal8Bit());
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









    //Ввод рейтинга по теме (на очном этапе Конкурса)
    if (params.at(0) == "setThemeIntramuralRating") {
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

            //Только для Администратора
            if (userCode == 3) {
                bool ok1, ok2 = true;

                int themeCode = params.at(1).toInt(&ok1);
                int themeRating = params.at(2).toInt(&ok2);

                if (ok1 & ok2){
                    query = QString("UPDATE vzletbase.theme "
                                    "SET themeRatingAdmission = %1 "
                                    "WHERE themeID = %2 ")
                            .arg(themeRating)
                            .arg(themeCode);

                    if (q.exec(query)) {
                        db.close();

                        printf("{\"resultCode\":1}");
                        a.quit();
                        return (200);
                    }
                    else {
                        printf("{\"resultCode\":3}");
                        a.quit();
                        return (200);
                    }
                }
                else {
                    printf("{\"resultCode\":2}");
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



    //Получить статусы тем в системе
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

            query = "SELECT statusThemeID, statusThemeName FROM vzletbase.statusTheme WHERE statusThemeID <=9 ";

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


    a.quit();
    return 200;
}
