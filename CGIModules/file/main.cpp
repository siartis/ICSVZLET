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

#include "D:/newOdarmol/server/modules/sources/Classes/removetagssql.h";

char *DBDRIVER=getenv("DBDRIVER");
char *DBCONNECT=getenv("DBCONNECT");
char *personID=getenv("personID");
char *personRole=getenv("personRole");
char *personName=getenv("personName");

QByteArray Input(void) {
    QFile f;
    if(!f.open(fileno(stdin),QIODevice::ReadOnly)) {
        qApp->quit();
        exit(404);
    }
    return f.readAll();
}


int checkUserStatus(QSqlDatabase *db, const int userID) {
    int userStatusCode = 0;
    QSqlQuery q(*db);
    if (q.exec(QString("SELECT statusID "
                       "FROM userStatusOrganizationSubdivision "
                       "WHERE "
                       "userID = %1").arg(userID))) {
        while (q.next()) {
            userStatusCode = q.value(0).toInt();
        }

    }
    return userStatusCode;
}

//Проверка - относится ли выбранный файл к данному пользователю
bool checkUserFile(QSqlDatabase *db, const int userID, const int fileID) {
    int userCode = 0;
    QSqlQuery q(*db);
    if (q.exec(QString("SELECT userID FROM projectFile "
                       "WHERE (userID = %1) AND (projectFileID = %2) ")
               .arg(userID)
               .arg(fileID))) {
        while (q.next()) {
            userCode = q.value(0).toInt();
        }

        //Если существует запись о файле с таким пользователем
        if (userCode == userID) {
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



//Проверка количества загруженных файлов пользователем (предполагается не больше двух - презентация и текст работы)
bool checkCountFilesByUser(QSqlDatabase *db, const int userID) {
    int countFiles = 0;
    QSqlQuery q(*db);
    if (q.exec(QString("SELECT COUNT(projectFileID) "
                       "FROM vzletbase.projectFile "
                       "WHERE userID = %1 ")
               .arg(userID))) {
        while (q.next()) {
            countFiles = q.value(0).toInt();
        }

        //Если больше одного
        //Если один
        if (countFiles == 1) {
            return false;
        }
        else {
            return true;
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


//Для (пока) всех типов строк
QString v = " 0123456789qwertyuiopasdfghjklzxcvbnm.,йцукенгшщзхъфывапролджэячсмитьбю@№;%?()-+_";

//Выборка из текста только определенных разрешенных символов
QString TEXT(const QString& s, const QString& x) {
    QString n=x;//QString::fromLocal8Bit(x);
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

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    setmode(fileno(stdout),O_BINARY);
    setmode(fileno(stdin ),O_BINARY);

    QString ps = Input();
    QStringList params = ps.split("&");


    //Загрузка файла в БД
    if (params.at(0) == "uploadFile") {
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

            //Если ученик
            if (currentUserStatus == 2) {

                //Проверяем, есть ли у пользователя еще загруженные файлы
                if (checkCountFilesByUser(&db, userCode)) {

                    bool ok = true;
                    int themeID = params.at(1).toInt(&ok);

                    if (ok) {

                        //Проверка - по своей ли теме ученик загружает файл
                        if (checkUserTheme(&db, userCode, themeID)) {
                            QByteArray b;
                            b.clear();
                            b.append(params.at(2));
                            QString fileName = TEXT(QByteArray::fromBase64(b), v);
                            b.clear();

                            fileName = fileName.replace("Cfakepath", "", Qt::CaseInsensitive);

                            //Количество символов в названии файла (учитывая расширение)
                            if (fileName.length() > 77) {
                                printf(QString("Файл не был загружен, т.к. имеет слишком длинное название.\n"
                                               "Максимально доспустимое количество символов в названии файла - 70."
                                               "Уменьшите количество символов в названии файла и повторите попытку.")
                                       .toLocal8Bit());
                                a.quit();
                                return (200);
                            }
                            else {
                                QString textDocument;
                                {
                                    QString x1=params.at(3);
                                    QStringList p= x1.remove(' ').split(',');

                                    QFile f;
                                    for(int i=0;i<p.length();i++) {
                                        f.setFileName(QString("c://vzlet//PARTs/%1").arg(p.at(i)));
                                        f.open(QIODevice::ReadOnly);
                                        textDocument.append(QString::fromLocal8Bit(f.readAll()));
                                        f.close();
                                        f.remove(QString("c:/odarmol/parts/%1").arg(p.at(i)));
                                    }

                                    //Запись отдельного файла проектм (помимо в базу) на жесткий диск (на всякий случай)
                                    QFile f1;
                                    f1.setFileName(QString("C://vzlet//vzletProjects/%1_%2_%3")
                                                   .arg(themeID)
                                                   .arg(userCode)
                                                   .arg(fileName));
                                    f1.open(QIODevice::WriteOnly);

                                    QString textDocument1 = textDocument; //Копия файла
                                    QByteArray b;
                                    b.clear();
                                    b.append(textDocument1.mid(textDocument1.indexOf(",", 0)+1, textDocument1.length()).toLocal8Bit());

                                    f1.write(QByteArray::fromBase64(b));
                                    f1.close();

                                    b.clear();
                                }

                                query = QString("INSERT INTO vzletbase.projectFile "
                                                "(themeID, userID, projectFileName, projectFileDateTime, projectFileData) "
                                                "VALUES (%1, %2, '%3', NOW(), ?) ")
                                        .arg(themeID)
                                        .arg(userCode)
                                        .arg(fileName);
                                bool z_ok = true;
                                q.prepare(query);
                                q.bindValue(0,textDocument.toLocal8Bit());
                                z_ok = q.exec();

                                if (z_ok) {


                                    //Получаем коды рецензентов по секции, в которых данная работа
                                    int reviewerCode1, reviewerCode2 = 0;

                                    query = QString("SELECT reviewerID1, reviewerID2 "
                                                    "FROM vzletbase.sectionReviewer "
                                                    "WHERE "
                                                    "sectionID IN (SELECT sectionID FROM theme WHERE themeID = %1) ")
                                            .arg(themeID);
                                    z_ok = q.exec(query);

                                    if (z_ok) {
                                        while (q.next()) {
                                            reviewerCode1 = q.value(0).toInt();
                                            reviewerCode2 = q.value(1).toInt();
                                        }

                                        query = QString("UPDATE vzletbase.theme "
                                                        "SET reviewerID = %1, reviewer2ID = %2, reviewerAddDateTimeReviewer1 = NOW(), reviewerAddDateTimeReviewer2 = NOW(), "
                                                        "stageExecuteID = 16, "
                                                        "statusThemeID = 8 "
                                                        "WHERE themeID = %3 ")
                                                .arg(reviewerCode1)
                                                .arg(reviewerCode2)
                                                .arg(themeID);
                                        z_ok = q.exec(query);

                                        if (z_ok) {
                                            printf(QString("Файл успешно отправлен! ")
                                                   .toLocal8Bit());
                                            a.quit();
                                            return (200);
                                        }
                                        else {
                                            printf(QString("Ошибка загрузки файла! ").toLocal8Bit());
                                            a.quit();
                                            return (200);
                                        }
                                    }
                                    else {
                                        printf(QString("Ошибка загрузки файла! ").toLocal8Bit());
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
                            printf(QString("Вы не можете загрузить файл по проекту другого пользователя! ").toLocal8Bit());
                            a.quit();
                            return (200);
                        }
                    }
                    else {
                        printf(QString("Проверьте правильность кода темы").toLocal8Bit());
                        a.quit();
                        return (200);
                    }
                }
                else {
                    printf(QString("Вы не можете загрузить больше одного файлов (файл текста работы)! \n").toLocal8Bit());
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


    //Выгрузка списка файлов из БД
    if (params.at(0) == "getFiles") {
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

            //Если ученик
            if (currentUserStatus == 2) {
                query = QString("SELECT projectFileID, projectFileName "
                                "FROM vzletbase.ProjectFile "
                                "WHERE userID = %1 ")
                        .arg(userCode);

                if (q.exec(query)) {
                    while (q.next()) {
                        result.append(QString("{\"projectFileCode\":%1,"
                                              "\"projectFileName\":\"%2\"},")
                                      .arg(q.value(0).toString())
                                      .arg(q.value(1).toString().mid(0, 50)));
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
                return (200);
            }
        }
        else {
            printf("[error]");
            a.quit();
            return (200);
        }
    }


    //Выгрузка содержимого определенного файла из БД
    if (params.at(0) == "getFile") {
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

            int userCode = QString("%1").arg(personID).toInt();
            int currentUserStatus = checkUserStatus(&db, userCode);

            //Если ученик
            if (currentUserStatus == 2) {

                bool ok = true;
                int fileCode = params.at(1).toInt(&ok);

                if (ok) {

                    //Проверка - принадлежит ли выбранный файл данному пользователю
                    if (checkUserFile(&db, userCode, fileCode)) {

                        //Получаем содержимое файла
                        query = QString("SELECT projectFileID, projectFileName, projectFileData "
                                        "FROM vzletbase.ProjectFile "
                                        "WHERE (userID = %1) AND (projectFileID = %2) ")
                                .arg(userCode)
                                .arg(fileCode);

                        if (q.exec(query)) {
                            while (q.next()) {
                                result.append(QString("{\"projectFileCode\":%1,"
                                                      "\"projectFileName\":\"%2\","
                                                      "\"projectFileData\":\"%3\"},")
                                              .arg(q.value(0).toString())
                                              .arg(q.value(1).toString())
                                              .arg(q.value(2).toString()));
                            }

                            result.chop(1);
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
                        printf(QString("Вы не можете получить доступ к файлу другого пользователя! ").toLocal8Bit());
                        a.quit();
                        return (200);
                    }
                }
                else {
                    printf(QString("Проверьте правильность выбора файла! ").toLocal8Bit());
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


    //Для рецензента - выгрузка файла по коду темы
    if (params.at(0) == "getFileByTheme") {
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

            int userCode = QString("%1").arg(personID).toInt();
            int currentUserStatus = checkUserStatus(&db, userCode);

            //Если рецензент (первый или второй) или учитель
            if (currentUserStatus == 9 || currentUserStatus == 18 || currentUserStatus == 3) {

                bool ok = true;
                int themeCode = params.at(1).toInt(&ok);

                if (ok) {
                    int projectFileCode = 0;

                    //Получаем код файла из темы
                    query = QString("SELECT projectFileID FROM vzletbase.projectFile WHERE themeID = %1")
                            .arg(themeCode);
                    if (q.exec(query)) {
                        while (q.next()) {
                            projectFileCode = q.value(0).toInt();
                        }

                        if (projectFileCode == 0) {
                            printf("{\"errorCode\":1}");
                            a.quit();
                            return (200);
                        }
                        else {
                            //Получаем содержимое файла (текста работы)
                            query = QString("SELECT projectFileID, projectFileName, projectFileData "
                                            "FROM vzletbase.ProjectFile "
                                            "WHERE (projectFileID = %1) AND ((Right(projectFileName, 3) = 'doc') OR (Right(projectFileName, 4) = 'docx') OR (Right(projectFileName, 3) = 'pdf') OR (Right(projectFileName, 3) = 'rtf')) ")
                                    .arg(projectFileCode);

                            if (q.exec(query)) {
                                while (q.next()) {
                                    result.append(QString("{\"projectFileCode\":%1,"
                                                          "\"projectFileName\":\"%2\","
                                                          "\"projectFileData\":\"%3\","
                                                          "\"errorCode\":0},")
                                                  .arg(q.value(0).toString())
                                                  .arg(q.value(1).toString())
                                                  .arg(q.value(2).toString()));

                                }

                                result.chop(1);
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


    //Проверка - загрузил ли школьник файл с работой
    if (params.at(0) == "checkProjectFile") {
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

            //Если учитель или ученик
            if (currentUserStatus == 3 || currentUserStatus == 2) {

                bool ok = true;
                int themeCode = params.at(1).toInt(&ok);

                if (ok) {
                    //Если учитель или ученик относится к этой теме
                    if (checkUserTheme(&db, userCode, themeCode)) {

                        int projectFileCode = 0;

                        //Получаем код файла из темы
                        query = QString("SELECT projectFileID FROM vzletbase.projectFile WHERE themeID = %1")
                                .arg(themeCode);
                        if (q.exec(query)) {
                            while (q.next()) {
                                projectFileCode = q.value(0).toInt();
                            }

                            if (projectFileCode == 0) {
                                printf("{\"resultCode\":0}");
                                a.quit();
                                return (200);
                            }
                            else {
                                printf("{\"resultCode\":1}");
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
                        printf(QString("Вы не можете получить доступ к информации не по своей теме! ").toLocal8Bit());
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


    a.quit();
    return (200);
}
