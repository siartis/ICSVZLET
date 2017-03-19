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


//Проверка, рецензент (первый или второй) уже вводил рецензию по определенной теме или нет??
bool checkHasReview(QSqlDatabase *db, const int userID, const int themeID, const int userStatusCode) {
    int userCode = 0;
    bool hasReviewerReview = false; //Рецензент еще не отрецензировал данную работу
    QSqlQuery q(*db);
    QString query;

    if (q.exec(QString("SELECT userID FROM vzletbase.review WHERE (userID = %1) AND (themeID = %2) ")
               .arg(userID)
               .arg(themeID))) {
        while (q.next()) {
            userCode = q.value(0).toInt();
        }

        //Если рецензент
        if (userStatusCode == 9 || userStatusCode == 18) {

            if (userStatusCode == 9) {
                query = QString("SELECT themeReview FROM vzletbase.theme WHERE (reviewerID = %1) AND (themeID = %2) ")
                        .arg(userID)
                        .arg(themeID);
            }

            if (userStatusCode == 18) {
                query = QString("SELECT themeReview2 FROM vzletbase.theme WHERE (reviewer2ID = %1) AND (themeID = %2) ")
                        .arg(userID)
                        .arg(themeID);
            }

            if (q.exec(query)) {
                while (q.next()) {
                    hasReviewerReview = q.value(0).toBool();
                }

                if ((userCode == userID) || hasReviewerReview) {
                    //Рецензия по данной теме уже была введена
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
        else {
            if (userCode == userID) {
                //Рецензия по этой теме уже была введена
                return false;
            }
            else {
                return true;
            }
        }
    }
    else {
        return false;
    }
}


//Проверка, имеет ли право рецензент рецензировать именно данную тему
bool checkHasReviewerReviewCurrentTheme(QSqlDatabase *db, const int userID, const int themeID, const int currentUserStatus) {
    int reviewerCode = 0;
    QSqlQuery q(*db);
    QString query;

    if (currentUserStatus == 9) {
        query = QString("SELECT reviewerID FROM vzletbase.theme WHERE (reviewerID = %1) AND (themeID = %2) ")
                .arg(userID)
                .arg(themeID);
    }

    if (currentUserStatus == 18) {
        query = QString("SELECT reviewer2ID FROM vzletbase.theme WHERE (reviewer2ID = %1) AND (themeID = %2) ")
                .arg(userID)
                .arg(themeID);
    }

    if (q.exec(query)) {
        while (q.next()) {
            reviewerCode = q.value(0).toInt();
        }

        if (reviewerCode == userID) {
            //Рецензент имеет право рецензировать эту тему
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


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    setmode(fileno(stdout),O_BINARY);
    setmode(fileno(stdin ),O_BINARY);

    QString ps = Input();
    QStringList params = ps.split("&");


    //Ввод рецензии
    if (params.at(0) == "setReview") {
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

            //Если консультант, ученик или учитель
            if (currentUserStatus == 4 || currentUserStatus == 3 || currentUserStatus == 2 || currentUserStatus == 9 || currentUserStatus == 18) {

                QSqlQuery q(db);
                QStringList criteria, briefComment;
                QByteArray b;

                //Начало с 3 апреля 2017 года
                QDate currentDate = QDate::currentDate();

                QDate evaluateDate(2017, 4, 3);


                if (currentDate.daysTo(evaluateDate) >= 0) {
                    bool ok = true;
                    double themeRating = 0; //Рейтинг за работу
                    double weight = 0; //В
                    int criteriaCode = 0;

                    for(int i = 2; i<= 17; i++) {
                        //Проверка кода критерия (ответа на критерий) на число
                        ok = true;
                        criteriaCode = params.at(i).toInt(&ok);
                        if (!ok) {
                            printf(QString("Проверьте правильность выбора ответов на критерии! ").toLocal8Bit());
                            a.quit();
                            return (200);
                        }
                        else {
                            //Проверка вхождения кода ответа в диапазон ответов - от 0 до 4
                            if (criteriaCode < 0  && criteriaCode > 4) {
                                printf(QString("Ошибка ввода данных. Баллы за ответы на критерии должны входить в диапазон [0; 4] ! ").toLocal8Bit());
                                a.quit();
                                return (200);
                            }
                            else {
                                criteria.append(QString::number(criteriaCode));
                            }
                        }
                    }

                    //Весовые коэффициенты 16 критериев
                    QStringList weightCriteria;
                    weightCriteria << "0.16" << "0.06" << "0.06" << "0.06" << "0.06" << "0.06" << "0.06"  << "0.06" << "0.06" << "0.06" << "0.06" << "0.06" << "0.06" << "0.06" << "0.06" << "0";

                    for(int i = 0; i < criteria.length(); i++) {
                        weight = 0;


                        switch (i) {

                        //Первый критерий
                        case 0:
                            switch (criteria.at(i).toInt()) {
                            case 1:
                                weight = 0.57;
                                break;
                            case 2:
                                weight = 1;
                                break;
                            }
                            break;

                            //Второй критерий
                        case 1:
                            switch (criteria.at(i).toInt()) {
                            case 1:
                                weight = 0.57;
                                break;
                            case 2:
                                weight = 1;
                                break;
                            }
                            break;

                            //Третий критерий
                        case 2:
                            switch (criteria.at(i).toInt()) {
                            case 1:
                                weight = 0.57;
                                break;
                            case 2:
                                weight = 1;
                                break;
                            }
                            break;

                            //Четвертый критерий
                        case 3:
                            switch (criteria.at(i).toInt()) {
                            case 1:
                                weight = 0.57;
                                break;
                            case 2:
                                weight = 1;
                                break;
                            }
                            break;

                            //Пятый критерий
                        case 4:
                            switch (criteria.at(i).toInt()) {
                            case 1:
                                weight = 0.57;
                                break;
                            case 2:
                                weight = 1;
                                break;
                            }
                            break;

                            //Шестой критерий
                        case 5:
                            switch (criteria.at(i).toInt()) {
                            case 1:
                                weight = 0.57;
                                break;
                            case 2:
                                weight = 1;
                                break;
                            }
                            break;

                            //Седьмой критерий
                        case 6:
                            switch (criteria.at(i).toInt()) {
                            case 1:
                                weight = 0.57;
                                break;
                            case 2:
                                weight = 1;
                                break;
                            }
                            break;

                            //Восьмой критерий
                        case 7:
                            switch (criteria.at(i).toInt()) {
                            case 1:
                                weight = 0.57;
                                break;
                            case 2:
                                weight = 1;
                                break;
                            }
                            break;

                            //Девятый критерий
                        case 8:
                            switch (criteria.at(i).toInt()) {
                            case 1:
                                weight = 0.57;
                                break;
                            case 2:
                                weight = 1;
                                break;
                            }
                            break;

                            //Десятый критерий
                        case 9:
                            switch (criteria.at(i).toInt()) {
                            case 1:
                                weight = 0.56;
                                break;
                            case 2:
                                weight = 0.72;
                                break;
                            case 3:
                                weight = 0.87;
                                break;
                            case 4:
                                weight = 1;
                                break;
                            }
                            break;


                            //Одиннадцатый критерий
                        case 10:
                            switch (criteria.at(i).toInt()) {
                            case 1:
                                weight = 0.57;
                                break;
                            case 2:
                                weight = 1;
                                break;
                            }
                            break;

                            //Двенадцатый критерий
                        case 11:
                            switch (criteria.at(i).toInt()) {
                            case 1:
                                weight = 0.57;
                                break;
                            case 2:
                                weight = 1;
                                break;
                            }
                            break;

                            //Тринадцатый критерий
                        case 12:
                            switch (criteria.at(i).toInt()) {
                            case 1:
                                weight = 0.55;
                                break;
                            case 2:
                                weight = 0.78;
                                break;
                            case 3:
                                weight = 1;
                                break;
                            }
                            break;

                            //Четырнадцатый критерий
                        case 13:
                            switch (criteria.at(i).toInt()) {
                            case 1:
                                weight = 0.55;
                                break;
                            case 2:
                                weight = 0.75;
                                break;
                            case 3:
                                weight = 1;
                                break;
                            }
                            break;

                            //Пятнадцатый критерий
                        case 14:
                            switch (criteria.at(i).toInt()) {
                            case 1:
                                weight = 0.57;
                                break;
                            case 2:
                                weight = 1;
                                break;
                            }
                            break;

                            //Шестнадцатый критерий
                        case 15:
                            switch (criteria.at(i).toInt()) {
                            case 1:
                                weight = 0.57;
                                break;
                            case 2:
                                weight = 1;
                                break;
                            }
                            break;

                        }

                        themeRating += (weight * weightCriteria.at(i).toDouble());
                    }

                    themeRating *= 100;

                    printf(QString("Рейтинг творческой работы   %1 ").arg(themeRating).toLocal8Bit());
                    a.quit();
                    return (200);


                }
                else {

                    //НАСТОЯЩИЙ ВВОД РЕЦЕНЗИЙ


                    QSqlQuery q(db);
                    QString query = "";

                    bool ok = true;
                    int themeCode = params.at(1).toInt(&ok);


                    //Если это рецензент, то может ли он рецензировать именно эту тему?
                    if (currentUserStatus == 9 || currentUserStatus == 18) {
                        if (!checkHasReviewerReviewCurrentTheme(&db, userCode, themeCode, currentUserStatus)) {
                            printf(QString("Вы не имеете права рецензировать данный проект! ").toLocal8Bit());
                            a.quit();
                            return (200);
                        }
                    }



                    //Проверка - пользователь уже рецензировал данную работу или нет?!
                    if(checkHasReview(&db, userCode, themeCode, currentUserStatus)) {

                        int criteriaCode = 0;

                        //Если код темы - число
                        if (ok) {
                            for(int i = 2; i<= 17; i++) {

                                //Проверка кода критерия (ответа на критерий) на число
                                ok = true;
                                criteriaCode = params.at(i).toInt(&ok);
                                if (!ok) {
                                    printf(QString("Проверьте правильность выбора ответов на критерии! ").toLocal8Bit());
                                    a.quit();
                                    return (200);
                                }
                                else {
                                    //Проверка вхождения кода ответа в диапазон ответов - от 0 до 4
                                    if (criteriaCode < 0  && criteriaCode > 4) {
                                        printf(QString("Ошибка ввода данных. Баллы за ответы на критерии должны входить в диапазон [0; 4] ! ").toLocal8Bit());
                                        a.quit();
                                        return (200);
                                    }
                                    else {
                                        criteria.append(QString::number(criteriaCode));
                                    }
                                }
                            }


                            for(int i = 18; i<= 33; i++) {
                                b.clear();
                                b.append(params.at(i));
                                briefComment.append(TEXT(QByteArray::fromBase64(b), v));
                                b.clear();
                            }

                            double themeRating = 0;

                            query = "INSERT INTO vzletbase.review "
                                    "(userID, themeID, criteria1, criteria2, criteria3, criteria4, criteria5, criteria6, criteria7, criteria8, criteria9, criteria10, "
                                    "criteria11, criteria12, criteria13, criteria14, criteria15, criteria16, "
                                    "briefComment1, briefComment2, briefComment3, briefComment4, briefComment5, briefComment6, briefComment7, briefComment8, briefComment9, "
                                    "briefComment10, briefComment11, briefComment12, briefComment13, briefComment14, briefComment15, briefComment16, reviewThemeRating, "
                                    "reviewDateTime) "
                                    "VALUES ";
                            query.append(QString("(%1, %2,  ").arg(userCode).arg(themeCode));


                            //Если оценивает ученик/учитель/консультант
                            if (currentUserStatus == 2 || currentUserStatus == 3 || currentUserStatus == 4) {

                                QStringList weightCriteria;

                                //Весовые коэффициенты для 2015/2016 учебного года
                                //weightCriteria << "0.124" << "0.046" << "0.019" << "0.019" << "0.046" << "0.046" << "0.124"  << "0.124" << "0.046" << "0.046" << "0.046" << "0.124" << "0.124" << "0.046" << "0.019" << "0";

                                //Весовые коэффициенты для 2016/2017 учебного года
                                weightCriteria << "0.12" << "0.046" << "0.019" << "0.019" << "0.046" << "0.046" <<
                                                  "0.12"  << "0.12" << "0.046" << "0.046" << "0.046" << "0.12" <<
                                                  "0.12" << "0.046" << "0.02" << "0";


                                for(int i = 0; i < criteria.length(); i++) {

                                    switch (i) {

                                    //Первый критерий
                                    case 0:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.164 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (0.405 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если третий ответ
                                        case 3:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Второй критерий
                                    case 1:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.33 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Третий критерий
                                    case 2:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.33 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Четвертый критерий
                                    case 3:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.33 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Пятый критерий
                                    case 4:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.33 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;


                                        //Шестой критерий
                                    case 5:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.164 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (0.405 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 3:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Седьмой критерий
                                    case 6:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.33 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Восьмой критерий
                                    case 7:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.33 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Девятый критерий
                                    case 8:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.164 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (0.405 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если третий ответ
                                        case 3:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Десятый критерий
                                    case 9:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.089 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (0.176 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если третий ответ
                                        case 3:
                                            themeRating += (0.429 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если четвертый ответ
                                        case 4:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Одиннадцатый критерий
                                    case 10:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.33 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Двенадцатый критерий
                                    case 11:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.33 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Тринадцатый критерий
                                    case 12:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.164 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (0.405 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если третий ответ
                                        case 3:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Четырнадцатый критерий
                                    case 13:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.164 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (0.405 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если третий ответ
                                        case 3:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Пятнадцатый критерий
                                    case 14:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.164 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (0.405 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если третий ответ
                                        case 3:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                    }

                                    query.append(QString("%1, ").arg(criteria.at(i)));
                                }
                                themeRating *= 100;
                            }

                            //Если оценивает первый или второй рецензент
                            if (currentUserStatus == 9 || currentUserStatus == 18) {
                                QStringList weightCriteria;

                                //Критерии для оценки в 2015/2016 учебном году
                                //weightCriteria << "0.120" << "0.046" << "0.019" << "0.019" << "0.046" << "0.046" << "0.120"  << "0.120" << "0.046" << "0.046" << "0.046" << "0.120" << "0.120" << "0.046" << "0.02" << "0.02";

                                //Критерии для оценки в 2016/2017 учебном году
                                weightCriteria << "0.12" << "0.046" << "0.019" << "0.019" << "0.046" << "0.046" <<
                                                  "0.12"  << "0.12" << "0.046" << "0.046" << "0.046" << "0.12" <<
                                                  "0.12" << "0.046" << "0.02" << "0.02";

                                for(int i = 0; i < criteria.length(); i++) {

                                    switch (i) {

                                    //Первый критерий
                                    case 0:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.164 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (0.405 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если третий ответ
                                        case 3:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Второй критерий
                                    case 1:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.33 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Третий критерий
                                    case 2:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.33 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Четвертый критерий
                                    case 3:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.33 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Пятый критерий
                                    case 4:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.33 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;


                                        //Шестой критерий
                                    case 5:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.164 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (0.405 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 3:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Седьмой критерий
                                    case 6:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.33 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Восьмой критерий
                                    case 7:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.33 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Девятый критерий
                                    case 8:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.164 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (0.405 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если третий ответ
                                        case 3:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Десятый критерий
                                    case 9:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.089 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (0.176 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если третий ответ
                                        case 3:
                                            themeRating += (0.429 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если четвертый ответ
                                        case 4:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Одиннадцатый критерий
                                    case 10:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.33 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Двенадцатый критерий
                                    case 11:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.33 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Тринадцатый критерий
                                    case 12:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.164 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (0.405 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если третий ответ
                                        case 3:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Четырнадцатый критерий
                                    case 13:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.164 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (0.405 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если третий ответ
                                        case 3:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Пятнадцатый критерий
                                    case 14:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.164 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (0.405 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если третий ответ
                                        case 3:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                        //Шестнадцатый критерий
                                    case 15:
                                        switch (criteria.at(i).toInt()) {
                                        //Если первый ответ
                                        case 1:
                                            themeRating += (0.164 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если второй ответ
                                        case 2:
                                            themeRating += (0.405 * weightCriteria.at(i).toDouble());
                                            break;

                                            //Если третий ответ
                                        case 3:
                                            themeRating += (1 * weightCriteria.at(i).toDouble());
                                            break;
                                        }
                                        break;

                                    }

                                    query.append(QString("%1, ").arg(criteria.at(i)));
                                }
                                themeRating *= 100;
                            }


                            for(int i = 0; i < briefComment.length(); i++) {
                                query.append(QString("'%1', ").arg(briefComment.at(i)));
                            }

                            query.append(QString("%1, ").arg(themeRating));
                            query.append("NOW()) ");

                            if (q.exec(query)) {

                                //Отметка от том, что рецензент отрецензировал эту работу (для рецензента)
                                if (currentUserStatus == 9 || currentUserStatus == 18) {

                                    if (currentUserStatus == 9) {
                                        query = QString("UPDATE vzletbase.theme "
                                                        "SET themeReview = TRUE, themeReviewDateTime = NOW(), themeCreativeRatingRemoteReviewer1 = %2 "
                                                        "WHERE themeID = %1 ")
                                                .arg(themeCode)
                                                .arg(themeRating);
                                    }
                                    if (currentUserStatus == 18) {
                                        query = QString("UPDATE vzletbase.theme "
                                                        "SET themeReview2 = TRUE, themeReviewDateTime2 = NOW(), themeCreativeRatingRemoteReviewer2 = %2 "
                                                        "WHERE themeID = %1 ")
                                                .arg(themeCode)
                                                .arg(themeRating);
                                    }


                                    if (q.exec(query)) {
                                        printf(QString("Рецензия успешно введена в систему!").toLocal8Bit());
                                        a.quit();
                                        return (200);
                                    }
                                    else {
                                        printf("[error]");
                                        a.quit();
                                        return (200);
                                    }
                                }

                                //Если ученик/учитель или консультант
                                if (currentUserStatus == 2 || currentUserStatus == 3 || currentUserStatus == 4) {
                                    switch (currentUserStatus) {
                                    case 2:
                                        query = QString("UPDATE vzletbase.theme "
                                                        "SET themeCreativeRatingRemoteChild = %1 "
                                                        "WHERE themeID = %2")
                                                .arg(themeRating)
                                                .arg(themeCode);
                                        break;
                                    case 3:
                                        query = QString("UPDATE vzletbase.theme "
                                                        "SET themeCreativeRatingRemoteTeacher = %1 "
                                                        "WHERE themeID = %2")
                                                .arg(themeRating)
                                                .arg(themeCode);
                                        break;
                                    case 4:
                                        query = QString("UPDATE vzletbase.theme "
                                                        "SET themeCreativeRatingRemoteConsultant = %1 "
                                                        "WHERE themeID = %2")
                                                .arg(themeRating)
                                                .arg(themeCode);
                                        break;
                                    }


                                    if (q.exec(query)) {
                                        printf(QString("Рецензия успешно введена в систему!").toLocal8Bit());
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
                            printf(QString("Проверьте код темы! ").toLocal8Bit());
                            a.quit();
                            return (200);
                        }
                    }
                    else {
                        printf(QString("Вы не можете повторно ввести рецензию на данную работу! ").toLocal8Bit());
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
        else {
            printf("[error]");
            a.quit();
            return (200);
        }
    }







    //Вывод на экран названия темы по ее коду
    if (params.at(0) == "getThemeName") {
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

            //Здесь будет проверка прав доступа

            //Если консультант, учитель, ученик или рецензент (первый или второй) или Администратор

            if (currentUserStatus == 4 || currentUserStatus == 3 || currentUserStatus == 2 || currentUserStatus == 9 || currentUserStatus == 18 || userCode == 3) {
                QSqlQuery q(db);
                QString query = "";
                QString result = "{";

                bool ok = true;
                int themeCode = params.at(1).toInt(&ok);

                //Если код темы - число
                if (ok) {
                    query = QString("SELECT themeName FROM vzletbase.theme WHERE themeID = %1 ")
                            .arg(themeCode);
                    if (q.exec(query)) {
                        while (q.next()) {
                            result.append(QString("\"themeName\":\"%1\"}").arg(q.value(0).toString()));
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
                    printf(QString("Проверьте код темы! ").toLocal8Bit());
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






    //Вывод на экран списка тем для рецензирования
    if (params.at(0) == "getThemeReview") {
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

            //Если рецензент
            if (currentUserStatus == 9 || currentUserStatus == 18) {
                QSqlQuery q(db);
                QString query = "";
                QString result = "[";


                if (currentUserStatus == 9) {
                    query = QString("SELECT themeID, themeName, themeReview FROM vzletbase.theme WHERE reviewerID = %1 ")
                            .arg(userCode);
                }

                if (currentUserStatus == 18) {
                    query = QString("SELECT themeID, themeName, themeReview2 FROM vzletbase.theme WHERE reviewer2ID = %1 ")
                            .arg(userCode);
                }


                if (q.exec(query)) {
                    while (q.next()) {
                        result.append("{");
                        result.append(QString("\"themeCode\":%1,"
                                              "\"themeName\":\"%2\","
                                              "\"themeReview\":%3")
                                      .arg(q.value(0).toString())
                                      .arg(q.value(1).toString())
                                      .arg(q.value(2).toInt()));
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





    //Проверка - имеет ли право рецензент рецензировать данную работу
    if (params.at(0) == "checkHasReview") {
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

            //Если рецензент
            if (currentUserStatus == 9 || currentUserStatus == 18) {
                QSqlQuery q(db);

                bool ok = true;
                int themeCode = params.at(1).toInt(&ok);

                if (ok) {
                    if (checkHasReview(&db, userCode, themeCode, currentUserStatus)) {
                        printf("{\"reviewCode\":0}");
                        a.quit();
                        return (200);
                    }
                    else {
                        printf("{\"reviewCode\":1}");
                        a.quit();
                        return (200);
                    }
                }
                else {
                    printf(QString("Проверьте код темы").toLocal8Bit());
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








    //Получение рецензии пользователя (ученика, учителя и консультанта)
    if (params.at(0) == "getUserReview") {
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

            //Если рецензент
            if (currentUserStatus == 9 || currentUserStatus == 18 || userCode == 3) {
                QSqlQuery q(db);
                QString query;
                QString result = "{";

                bool ok1, ok2 = true;
                int themeCode = params.at(1).toInt(&ok1);
                int userStatusCode = params.at(2).toInt(&ok2);

                if (ok1 && ok2) {

                    if (userStatusCode >=2 && userStatusCode <= 4) {

                        int userID = 0; //Код пользователя, прикрепленного к теме (ученика, учителя или консультанта) - по запросу рецензента

                        //Получаем код пользователя, которого запросили рецензию (учителя, ученика или консультанта)
                        query = QString("SELECT userID "
                                        "FROM vzletbase.userTheme "
                                        "WHERE "
                                        "(themeID = %1) AND (userID IN (SELECT userID FROM userStatusOrganizationSubdivision WHERE statusID = %2)) ")
                                .arg(themeCode)
                                .arg(userStatusCode);

                        if (q.exec(query)) {
                            while (q.next()) {
                                userID = q.value(0).toInt();
                            }

                            if (userID > 0) {
                                //Получаем рецензию пользователя
                                query = QString("SELECT criteria1, criteria2, criteria3, criteria4, criteria5, criteria6, criteria7, criteria8, criteria9, criteria10, criteria11, criteria12, criteria13, criteria14, criteria15, criteria16, "
                                                "briefComment1, briefComment2, briefComment3, briefComment4, briefComment5, briefComment6, briefComment7, briefComment8, briefComment9, briefComment10, "
                                                "briefComment11, briefComment12, briefComment13, briefComment14, briefComment15, briefComment16 "
                                                "FROM vzletbase.review "
                                                "WHERE (userID = %1) AND (themeID = %2) ")
                                        .arg(userID)
                                        .arg(themeCode);

                                if (q.exec(query)) {
                                    while (q.next()) {

                                        //Номера ответов
                                        for(int i = 0; i < 16; i++) {
                                            result.append(QString("\"criteria%1\":%2,")
                                                          .arg(i)
                                                          .arg(q.value(i).toString()));
                                        }

                                        int j = 0;
                                        //Краткие комментарии
                                        for(int i = 16; i < 32; i++) {
                                            result.append(QString("\"briefComment%1\":\"%2\",")
                                                          .arg(j)
                                                          .arg(q.value(i).toString()));
                                            j++;
                                        }
                                    }

                                    result.chop(1);
                                    result.append("}");

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
                                printf(QString("К сожалению, в этом проекте нет пользователя с выбранным статусом! ").toLocal8Bit());
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
                        printf(QString("Просмотр рецензии доступен только по ученикам, учителям и консультантам! ").toLocal8Bit());
                        a.quit();
                        return (200);
                    }
                }
                else {

                    if (!ok1) {
                        printf(QString("Проверьте код темы! \n").toLocal8Bit());
                    }

                    if (!ok2) {
                        printf(QString("Проверьте код статуса пользователя \n").toLocal8Bit());
                    }

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




    //Тестовый ввод рецензии
    if (params.at(0) == "setTestReview") {
        int userCode = QString("%1").arg(personID).toInt();
        int currentUserStatus = checkUserStatus(&db, userCode);

        //Если пользователь системы
        if (currentUserStatus == 4 ||
                currentUserStatus == 3 ||
                currentUserStatus == 2 ||
                currentUserStatus == 9 ||
                currentUserStatus == 18 ||
                userCode == 3 ||
                currentUserStatus == 6 ||
                currentUserStatus == 9 ||
                currentUserStatus == 14 ||
                currentUserStatus == 16 ||
                currentUserStatus == 17) {

            QStringList criteria;
            int criteriaCode = 0;
            bool ok = true;

            for(int i = 2; i<= 17; i++) {

                //Проверка кода критерия (ответа на критерий) на число
                ok = true;
                criteriaCode = params.at(i).toInt(&ok);
                if (!ok) {
                    printf(QString("Проверьте правильность выбора ответов на критерии! ").toLocal8Bit());
                    a.quit();
                    return (200);
                }
                else {
                    //Проверка вхождения кода ответа в диапазон ответов - от 0 до 4
                    if (criteriaCode < 0  && criteriaCode > 4) {
                        printf(QString("Ошибка ввода данных. Баллы за ответы на критерии должны входить в диапазон [0; 4] ! ").toLocal8Bit());
                        a.quit();
                        return (200);
                    }
                    else {
                        criteria.append(QString::number(criteriaCode));
                    }
                }
            }

            QStringList weightCriteria;
            double themeRating = 0;

            //Весовые коэффициенты для 2016/2017 учебного года
            weightCriteria << "0.12" << "0.046" << "0.019" << "0.019" << "0.046" << "0.046" <<
                              "0.12"  << "0.12" << "0.046" << "0.046" << "0.046" << "0.12" <<
                              "0.12" << "0.046" << "0.02" << "0";


            for(int i = 0; i < criteria.length(); i++) {

                switch (i) {

                //Первый критерий
                case 0:
                    switch (criteria.at(i).toInt()) {
                    //Если первый ответ
                    case 1:
                        themeRating += (0.164 * weightCriteria.at(i).toDouble());
                        break;

                        //Если второй ответ
                    case 2:
                        themeRating += (0.405 * weightCriteria.at(i).toDouble());
                        break;

                        //Если третий ответ
                    case 3:
                        themeRating += (1 * weightCriteria.at(i).toDouble());
                        break;
                    }
                    break;

                    //Второй критерий
                case 1:
                    switch (criteria.at(i).toInt()) {
                    //Если первый ответ
                    case 1:
                        themeRating += (0.33 * weightCriteria.at(i).toDouble());
                        break;

                        //Если второй ответ
                    case 2:
                        themeRating += (1 * weightCriteria.at(i).toDouble());
                        break;
                    }
                    break;

                    //Третий критерий
                case 2:
                    switch (criteria.at(i).toInt()) {
                    //Если первый ответ
                    case 1:
                        themeRating += (0.33 * weightCriteria.at(i).toDouble());
                        break;

                        //Если второй ответ
                    case 2:
                        themeRating += (1 * weightCriteria.at(i).toDouble());
                        break;
                    }
                    break;

                    //Четвертый критерий
                case 3:
                    switch (criteria.at(i).toInt()) {
                    //Если первый ответ
                    case 1:
                        themeRating += (0.33 * weightCriteria.at(i).toDouble());
                        break;

                        //Если второй ответ
                    case 2:
                        themeRating += (1 * weightCriteria.at(i).toDouble());
                        break;
                    }
                    break;

                    //Пятый критерий
                case 4:
                    switch (criteria.at(i).toInt()) {
                    //Если первый ответ
                    case 1:
                        themeRating += (0.33 * weightCriteria.at(i).toDouble());
                        break;

                        //Если второй ответ
                    case 2:
                        themeRating += (1 * weightCriteria.at(i).toDouble());
                        break;
                    }
                    break;


                    //Шестой критерий
                case 5:
                    switch (criteria.at(i).toInt()) {
                    //Если первый ответ
                    case 1:
                        themeRating += (0.164 * weightCriteria.at(i).toDouble());
                        break;

                        //Если второй ответ
                    case 2:
                        themeRating += (0.405 * weightCriteria.at(i).toDouble());
                        break;

                        //Если второй ответ
                    case 3:
                        themeRating += (1 * weightCriteria.at(i).toDouble());
                        break;
                    }
                    break;

                    //Седьмой критерий
                case 6:
                    switch (criteria.at(i).toInt()) {
                    //Если первый ответ
                    case 1:
                        themeRating += (0.33 * weightCriteria.at(i).toDouble());
                        break;

                        //Если второй ответ
                    case 2:
                        themeRating += (1 * weightCriteria.at(i).toDouble());
                        break;
                    }
                    break;

                    //Восьмой критерий
                case 7:
                    switch (criteria.at(i).toInt()) {
                    //Если первый ответ
                    case 1:
                        themeRating += (0.33 * weightCriteria.at(i).toDouble());
                        break;

                        //Если второй ответ
                    case 2:
                        themeRating += (1 * weightCriteria.at(i).toDouble());
                        break;
                    }
                    break;

                    //Девятый критерий
                case 8:
                    switch (criteria.at(i).toInt()) {
                    //Если первый ответ
                    case 1:
                        themeRating += (0.164 * weightCriteria.at(i).toDouble());
                        break;

                        //Если второй ответ
                    case 2:
                        themeRating += (0.405 * weightCriteria.at(i).toDouble());
                        break;

                        //Если третий ответ
                    case 3:
                        themeRating += (1 * weightCriteria.at(i).toDouble());
                        break;
                    }
                    break;

                    //Десятый критерий
                case 9:
                    switch (criteria.at(i).toInt()) {
                    //Если первый ответ
                    case 1:
                        themeRating += (0.089 * weightCriteria.at(i).toDouble());
                        break;

                        //Если второй ответ
                    case 2:
                        themeRating += (0.176 * weightCriteria.at(i).toDouble());
                        break;

                        //Если третий ответ
                    case 3:
                        themeRating += (0.429 * weightCriteria.at(i).toDouble());
                        break;

                        //Если четвертый ответ
                    case 4:
                        themeRating += (1 * weightCriteria.at(i).toDouble());
                        break;
                    }
                    break;

                    //Одиннадцатый критерий
                case 10:
                    switch (criteria.at(i).toInt()) {
                    //Если первый ответ
                    case 1:
                        themeRating += (0.33 * weightCriteria.at(i).toDouble());
                        break;

                        //Если второй ответ
                    case 2:
                        themeRating += (1 * weightCriteria.at(i).toDouble());
                        break;
                    }
                    break;

                    //Двенадцатый критерий
                case 11:
                    switch (criteria.at(i).toInt()) {
                    //Если первый ответ
                    case 1:
                        themeRating += (0.33 * weightCriteria.at(i).toDouble());
                        break;

                        //Если второй ответ
                    case 2:
                        themeRating += (1 * weightCriteria.at(i).toDouble());
                        break;
                    }
                    break;

                    //Тринадцатый критерий
                case 12:
                    switch (criteria.at(i).toInt()) {
                    //Если первый ответ
                    case 1:
                        themeRating += (0.164 * weightCriteria.at(i).toDouble());
                        break;

                        //Если второй ответ
                    case 2:
                        themeRating += (0.405 * weightCriteria.at(i).toDouble());
                        break;

                        //Если третий ответ
                    case 3:
                        themeRating += (1 * weightCriteria.at(i).toDouble());
                        break;
                    }
                    break;

                    //Четырнадцатый критерий
                case 13:
                    switch (criteria.at(i).toInt()) {
                    //Если первый ответ
                    case 1:
                        themeRating += (0.164 * weightCriteria.at(i).toDouble());
                        break;

                        //Если второй ответ
                    case 2:
                        themeRating += (0.405 * weightCriteria.at(i).toDouble());
                        break;

                        //Если третий ответ
                    case 3:
                        themeRating += (1 * weightCriteria.at(i).toDouble());
                        break;
                    }
                    break;

                    //Пятнадцатый критерий
                case 14:
                    switch (criteria.at(i).toInt()) {
                    //Если первый ответ
                    case 1:
                        themeRating += (0.164 * weightCriteria.at(i).toDouble());
                        break;

                        //Если второй ответ
                    case 2:
                        themeRating += (0.405 * weightCriteria.at(i).toDouble());
                        break;

                        //Если третий ответ
                    case 3:
                        themeRating += (1 * weightCriteria.at(i).toDouble());
                        break;
                    }
                    break;

                }
            }
            themeRating *= 100;

            printf(QString("%1 ")
                   .arg(themeRating).toLocal8Bit());
            a.quit();
            return (200);
        }
        else {
            a.quit();
            return (401);
        }

    }


    return 200;
}
