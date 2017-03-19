#include <QCoreApplication>

#include "emailnotificator.h"
#include "filehandler.h"

//Перевод строки из Base64 кодировки в текст
QByteArray getDataFromBase64(const QString text)
{
    QByteArray b;
    b.clear();
    b.append(text);

    return QByteArray::fromBase64(b);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QString dirPath = "C:/vzlet/modules/EMailNotifierVZLET/template";
    EMailNotificator emailNotificator;
    FileHandler fileHandler;

    //Консультант перевел тему из предлагаемой в выполняемую
    if (args[1] == "proposedPerformed")
    {
        QString textMessage = fileHandler.readHtmlFileTemplate(QString("%1/proposedPerformed/forAll.htm")
                                                               .arg(dirPath));

        QString consultantFIO, consultantDegree, consultantAcademicDirection, consultantPost, consultantSubdivision, consultantOrganization;
        QString childFIO, childOrganization, childSubdivision, childPost;
        QString teacherFIO, teacherOrganization, teacherSubdivision, teacherPost;
        QString themeName;

        QString consultantEMail, teacherEMail, childEMail;

        //Информация о консультанте
        consultantFIO = getDataFromBase64(QString("%1").arg(argv[2]));
        consultantDegree = getDataFromBase64(QString("%1").arg(argv[3]));
        consultantAcademicDirection = getDataFromBase64(QString("%1").arg(argv[4]));
        consultantPost = getDataFromBase64(QString("%1").arg(argv[5]));
        consultantSubdivision = getDataFromBase64(QString("%1").arg(argv[6]));
        consultantOrganization = getDataFromBase64(QString("%1").arg(argv[7]));


        //Информация об ученике
        childFIO = getDataFromBase64(QString("%1").arg(argv[8]));
        childOrganization = getDataFromBase64(QString("%1").arg(argv[9]));
        childSubdivision = getDataFromBase64(QString("%1").arg(argv[10]));
        childPost =getDataFromBase64(QString("%1").arg(argv[11]));

        //Информация об учителе
        teacherFIO = getDataFromBase64(QString("%1").arg(argv[12]));
        teacherOrganization = getDataFromBase64(QString("%1").arg(argv[13]));
        teacherSubdivision = getDataFromBase64(QString("%1").arg(argv[14]));
        teacherPost = getDataFromBase64(QString("%1").arg(argv[15]));

        //Информация о теме
        themeName = getDataFromBase64(QString("%1").arg(argv[16]));

        //Адреса электронной почты "тройки"
        consultantEMail = getDataFromBase64(QString("%1").arg(argv[17]));
        teacherEMail = getDataFromBase64(QString("%1").arg(argv[18]));
        childEMail =getDataFromBase64(QString("%1").arg(argv[19]));


        //Данные консультанта
        textMessage = textMessage.replace("$$$consultantFIO$$$", consultantFIO);
        textMessage = textMessage.replace("$$$consultantDegree$$$", consultantDegree);
        textMessage = textMessage.replace("$$$consultantAcademicDirection$$$", consultantAcademicDirection);
        textMessage = textMessage.replace("$$$consultantPost$$$", consultantPost);
        textMessage = textMessage.replace("$$$consultantOrganization$$$", consultantOrganization);
        textMessage = textMessage.replace("$$$consultantSubdivision$$$", consultantSubdivision);
        textMessage = textMessage.replace("$$$consultantEMail$$$", consultantEMail);

        //Данные ученика
        textMessage = textMessage.replace("$$$childFIO$$$", childFIO);
        textMessage = textMessage.replace("$$$childOrganization$$$", childOrganization);
        textMessage = textMessage.replace("$$$childSubdivision$$$", childSubdivision);
        textMessage = textMessage.replace("$$$childPost$$$", childPost);

        //Данные учителя
        textMessage = textMessage.replace("$$$teacherFIO$$$", teacherFIO);
        textMessage = textMessage.replace("$$$teacherOrganization$$$", teacherOrganization);
        textMessage = textMessage.replace("$$$teacherSubdivision$$$", teacherSubdivision);
        textMessage = textMessage.replace("$$$teacherPost$$$", teacherPost);

        //Данные о теме
        textMessage = textMessage.replace("$$$themeName$$$", themeName);

        textMessage = textMessage.replace("$$$teacherEMail$$$", teacherEMail);
        textMessage = textMessage.replace("$$$childEMail$$$", childEMail);

        textMessage = textMessage.Replace("\r", "");
        textMessage = textMessage.Replace("\n", "");
        textMessage = textMessage.Replace("\r\n", "");
        textMessage = textMessage.Replace("\t", "");

        //Формирование адресов электронной почты участников для отправки уведомления
        QStringList files;
        files.append(consultantEMail);
        files.append(teacherEMail);
        files.append(childEMail);

        emailNotificator.sendEMailNotification(email,
                                               "Уведомление от системы \"ВЗЛЕТ\"",
                                               textMessage,
                                               files,
                                               "project");
        printf("1");
        a.quit();
        return 0;
    }




    //Регистрация пользователя
    if (args[1] == "registration")
    {
        QString textMessage = fileHandler.readHtmlFileTemplate(QString("%1/regUser/registration.htm")
                                                               .arg(dirPath));

        QString email = QString("%1").arg(args[2]).toInt(); //Адрес, куда отправлять
        int userCode = QString("%1").arg(args[3]).toInt(); //Код пользователя (часть логина)
        QString password = getDataFromBase64(QString("%1").arg(argv[4])); //Пароль пользователя
        QString userFIO = getDataFromBase64(QString("%1").arg(argv[5])); //ФИО пользователя
        QString userStatus = getDataFromBase64(QString("%1").arg(argv[6])); //Статус пользователя
        QString userOrganization = getDataFromBase64(QString("%1").arg(argv[7])); //Название организации пользователя
        QString userSubdivision = getDataFromBase64(QString("%1").arg(argv[8])); //Название подразделения пользователя

        textMessage = textMessage.replace("$$$userLogin$$$", QString("%1@dov").arg(userCode));
        textMessage = textMessage.replace("$$$userPassword$$$", password);
        textMessage = textMessage.replace("$$$userFIO$$$", userFIO);
        textMessage = textMessage.replace("$$$userStatus$$$", userStatus);

        textMessage = textMessage.replace("$$$userOrganization$$$", userOrganization);

        if (userSubdivision == "no")
        {
            textMessage = textMessage.replace("$$$userSubdivision$$$", "нет");
        }
        else
        {
            textMessage = textMessage.replace("$$$userSubdivision$$$", userSubdivision);
        }

        textMessage = textMessage.replace("\r", "");
        textMessage = textMessage.replace("\n", "");
        textMessage = textMessage.replace("\r\n", "");
        textMessage = textMessage.replace("\t", "");

        QStringList files;
        files.append(email);

        emailNotificator.sendEMailNotification(email,
                                               "Коды доступа к системе \"ВЗЛЕТ\"",
                                               textMessage,
                                               files,
                                               "regUser");
    }



    //Консультант отклонил тему
    if (args[1] == "themeReject")
    {
        QString textMessage = fileHandler.readHtmlFileTemplate(QString("%1/themeReject/themeReject.htm")
                                                               .arg(dirPath));

        QString email1 = getDataFromBase64(QString("%1").arg(argv[2])); //EMail 1
        QString email2 = getDataFromBase64(QString("%1").arg(argv[3])); //EMail 2
        QString consultantFIO = getDataFromBase64(QString("%1").arg(argv[4])); //ФИО консультанта
        QString themeName = getDataFromBase64(QString("%1").arg(argv[5])); //Название темы
        QString causeReject = getDataFromBase64(QString("%1").arg(argv[6])); //Причина отказа (пишет консультант)

        textMessage = textMessage.replace("$$$causeReject$$$", causeReject);
        textMessage = textMessage.replace("$$$consultantFIO$$$", consultantFIO);
        textMessage = textMessage.replace("$$$themeName$$$", themeName);

        textMessage = textMessage.replace("\r", "");
        textMessage = textMessage.replace("\n", "");
        textMessage = textMessage.replace("\r\n", "");
        textMessage = textMessage.replace("\t", "");

        QStringList files;
        files.append(email1);
        files.append(email2);

        emailNotificator.sendEMailNotification(email,
                                               "Поданная заявка была отклонена консультантом - уведомление от системы \"ВЗЛЕТ\"",
                                               textMessage,
                                               files,
                                               "project");
        printf("1");
        a.quit();
        return 0;
    }





    //Поступил запрос на консультирование
    if (args[1] == "requestAdvice")
    {
        QString textMessage = fileHandler.readHtmlFileTemplate(QString("%1/requestAdvice/requestAdvice.htm")
                                                               .arg(dirPath));

        QString teacherFIO = getDataFromBase64(QString("%1").arg(argv[2])); //ФИО учителя
        QString childFIO = getDataFromBase64(QString("%1").arg(argv[3])); //ФИО ученика
        QString teacherEMail = getDataFromBase64(QString("%1").arg(argv[4])); //EMail учителя
        QString childEMail = getDataFromBase64(QString("%1").arg(argv[5])); //EMail ученика
        QString themeName = getDataFromBase64(QString("%1").arg(argv[6])); //Название темы (запроса)

        textMessage = textMessage.replace("$$$teacherFIO$$$", teacherFIO);
        textMessage = textMessage.replace("$$$childFIO$$$", childFIO);
        textMessage = textMessage.replace("$$$teacherFIO$$$", teacherFIO);
        textMessage = textMessage.replace("$$$themeName$$$", themeName);

        textMessage = textMessage.replace("\r", "");
        textMessage = textMessage.replace("\n", "");
        textMessage = textMessage.replace("\r\n", "");
        textMessage = textMessage.replace("\t", "");

        QStringList files;
        files.append(teacherEMail);
        files.append(childEMail);

        emailNotificator.sendEMailNotification(email,
                                               "Запрос на консультирование успешно принят - уведомление от системы \"ВЗЛЕТ\"",
                                               textMessage,
                                               files,
                                               "project");

        printf("1");
        a.quit();
        return 0;
    }


    //Поступил запрос на консультирование (для учителя и ученика)
    if (args[1] == "teamBuildingForTeacherChild")
    {
        QString textMessage = fileHandler.readHtmlFileTemplate(QString("%1/teamBuilding/forTeacherChild.htm")
                                                               .arg(dirPath));

        QString teacherFIO = getDataFromBase64(QString("%1").arg(argv[2])); //ФИО учителя
        QString childFIO = getDataFromBase64(QString("%1").arg(argv[3])); //ФИО ученика
        QString teacherEMail = getDataFromBase64(QString("%1").arg(argv[4])); //EMail учителя
        QString childEMail = getDataFromBase64(QString("%1").arg(argv[5])); //EMail ученика
        QString themeName = getDataFromBase64(QString("%1").arg(argv[6])); //Название темы, предложенной консультантом
        QString consultantFIO = getDataFromBase64(QString("%1").arg(argv[7])); //ФИО консультанта

        textMessage = textMessage.replace("$$$teacherFIO$$$", teacherFIO);
        textMessage = textMessage.replace("$$$childFIO$$$", childFIO);
        textMessage = textMessage.replace("$$$teacherFIO$$$", teacherFIO);
        textMessage = textMessage.replace("$$$themeName$$$", themeName);
        textMessage = textMessage.replace("$$$consultantFIO$$$", consultantFIO);


        textMessage = textMessage.replace("\r", "");
        textMessage = textMessage.replace("\n", "");
        textMessage = textMessage.replace("\r\n", "");
        textMessage = textMessage.replace("\t", "");

        QStringList files;
        files.append(teacherEMail);
        files.append(childEMail);

        emailNotificator.sendEMailNotification(email,
                                               "Запрос на консультирование успешно принят - уведомление от системы \"ВЗЛЕТ\"",
                                               textMessage,
                                               files,
                                               "project");

        printf("1");
        a.quit();
        return 0;
    }


    //Поступил запрос на консультирование - информация для консультанта
    if (args[1] == "teamBuildingForConsultant")
    {
        QString textMessage = fileHandler.readHtmlFileTemplate(QString("%1/teamBuilding/forConsultant.htm")
                                                               .arg(dirPath));

        QString consultantFIO = getDataFromBase64(QString("%1").arg(argv[2])); //ФИО консультанта
        QString consultantEMail = getDataFromBase64(QString("%1").arg(argv[3])); //EMail консультанта
        QString themeName = getDataFromBase64(QString("%1").arg(argv[4])); //Тема проекта, предложенная консультантом
        QString teacherInfo = getDataFromBase64(QString("%1").arg(argv[5])); //Информация об учителе
        QString childInfo = getDataFromBase64(QString("%1").arg(argv[6])); //Информация об ученике

        //Информация об учителе и ученике, указанная ими при заполнении заявки
        QString teacherAbout = getDataFromBase64(QString("%1").arg(argv[7])); //Об учителе
        QString childAbout = getDataFromBase64(QString("%1").arg(argv[8])); //Об ученике

        textMessage = textMessage.replace("$$$consultantFIO$$$", consultantFIO);
        textMessage = textMessage.replace("$$$themeName$$$", themeName);
        textMessage = textMessage.replace("$$$teacherInfo$$$", teacherInfo);
        textMessage = textMessage.replace("$$$childInfo$$$", childInfo);
        textMessage = textMessage.replace("$$$teacherInfo1$$$", teacherAbout);
        textMessage = textMessage.replace("$$$childInfo1$$$", childAbout);

        textMessage = textMessage.replace("\r", "");
        textMessage = textMessage.replace("\n", "");
        textMessage = textMessage.replace("\r\n", "");
        textMessage = textMessage.replace("\t", "");

        QStringList files;
        files.append(consultantEMail);

        emailNotificator.sendEMailNotification(email,
                                               "Заявка на осуществление консультирования (от учителя и ученика) по предложенной Вами теме проекта - уведомление от системы \"ВЗЛЕТ\"",
                                               textMessage,
                                               files,
                                               "project");
        printf("1");
        a.quit();
        return 0;
    }

    //Ввод учителем сложившегося коллектива
    if (args[1] == "setCollective")
    {
        QString textMessage = fileHandler.readHtmlFileTemplate(QString("%1/setCollective/forAll.htm")
                                                               .arg(dirPath));

        //Информация о консультанте
        QString consultantFIO = getDataFromBase64(QString("%1").arg(argv[2])); //ФИО консультанта
        QString consultantOrganization = getDataFromBase64(QString("%1").arg(argv[3])); //Организация консультанта
        QString consultantSubdivision = getDataFromBase64(QString("%1").arg(argv[4])); //Подразделение консультанта (кафедра)

        //Информация об ученике
        QString childFIO = getDataFromBase64(QString("%1").arg(argv[5])); //ФИО ученика
        QString childOrganization = getDataFromBase64(QString("%1").arg(argv[6])); //Организация ученика
        QString childSubdivision = getDataFromBase64(QString("%1").arg(argv[7])); //Подразделение ученика
        QString childGrade = getDataFromBase64(QString("%1").arg(argv[8])); //Класс ученика
        QString childAbout = getDataFromBase64(QString("%1").arg(argv[9])); //Информация об ученике

        //Информация об учителе
        QString teacherFIO = getDataFromBase64(QString("%1").arg(argv[10])); //ФИО учителя
        QString teacherOrganization =getDataFromBase64(QString("%1").arg(argv[11])); //Организация учителя
        QString teacherSubdivision = getDataFromBase64(QString("%1").arg(argv[12])); //Подразделение учителя
        QString teacherPost = getDataFromBase64(QString("%1").arg(argv[13])); //Должность учителя
        QString teacherAbout = getDataFromBase64(QString("%1").arg(argv[14])); //Информация об учителе

        //Информация о теме
        QString themeName = getDataFromBase64(QString("%1").arg(argv[15]));

        //Напоминание консультанту о прежней совместной работе
        QString comment = getDataFromBase64(QString("%1").arg(argv[16]));

        //Адреса электронной почты участников
        QString consultantEMail = getDataFromBase64(QString("%1").arg(argv[17])); //EMail консультанта
        QString teacherEMail = getDataFromBase64(QString("%1").arg(argv[18])); //EMail учителя
        QString childEMail = getDataFromBase64(QString("%1").arg(argv[19])); //EMail ученика

        textMessage = textMessage.replace("$$$themeName$$$", themeName);

        textMessage = textMessage.replace("$$$childFIO$$$", childFIO);
        textMessage = textMessage.replace("$$$childOrganization$$$", childOrganization);
        textMessage = textMessage.replace("$$$childSubdivision$$$", childSubdivision);
        textMessage = textMessage.replace("$$$childGrade$$$", childGrade);

        textMessage = textMessage.replace("$$$teacherFIO$$$", teacherFIO);
        textMessage = textMessage.replace("$$$teacherOrganization$$$", teacherOrganization);
        textMessage = textMessage.replace("$$$teacherSubdivision$$$", teacherSubdivision);
        textMessage = textMessage.replace("$$$teacherPost$$$", teacherPost);

        textMessage = textMessage.replace("$$$consultantFIO$$$", consultantFIO);
        textMessage = textMessage.replace("$$$consultantOrganization$$$", consultantOrganization);
        textMessage = textMessage.replace("$$$consultantSubdivision$$$", consultantSubdivision);

        textMessage = textMessage.replace("$$$comment$$$", comment);

        textMessage = textMessage.replace("$$$infoByChild$$$", childAbout);
        textMessage = textMessage.replace("$$$infoByTeacher$$$", teacherAbout);

        textMessage = textMessage.replace("$$$consultantEMail$$$", consultantEMail);

        textMessage = textMessage.replace("\r", "");
        textMessage = textMessage.replace("\n", "");
        textMessage = textMessage.replace("\r\n", "");
        textMessage = textMessage.replace("\t", "");

        QStringList files;
        files.append(consultantEMail);
        files.append(teacherEMail);
        files.append(childEMail);

        emailNotificator.sendEMailNotification(email,
                                               "Формирование коллектива - уведомление от системы \"ВЗЛЕТ\"",
                                               textMessage,
                                               files,
                                               "project");
        printf("1");
        a.quit();
        return 0;
    }

    //Восстановление кодов доступа
    if (args[1] == "recoveryAccessCodes")
    {
        QString textMessage = fileHandler.readHtmlFileTemplate(QString("%1/recoveryAccessCodes/recoveryPassword.htm")
                                                               .arg(dirPath));

        QString email = getDataFromBase64(QString("%1").arg(argv[2])); //Адрес электронной почты пользователя
        QString userFIO = getDataFromBase64(QString("%1").arg(argv[3])); //ФИО пользователя

        QString userStatus = getDataFromBase64(QString("%1").arg(argv[4])); //Статус пользователя
        QString login = getDataFromBase64(QString("%1").arg(argv[5])); //Логин пользователя
        QString password = getDataFromBase64(QString("%1").arg(argv[6])); //Пароль пользователя
        QString userOrganization = getDataFromBase64(QString("%1").arg(argv[7])); //Организация пользователя
        QString userSubdivision = getDataFromBase64(QString("%1").arg(argv[8])); //Подразделение пользователя

        textMessage = textMessage.replace("$$$userFIO$$$", userFIO);

        QString result;
        result = QString("<p><span><b>Статус в системе - %1</b></span></p>").arg(userStatus);
        result.append(QString("<p><span><b>Организация - %1</b></span></p>").arg(userOrganization));

        if (userSubdivision != "no")
        {
            result.append(QString("<p><span><b>Подразделение - %1</b></span></p>").arg(userSubdivision));
        }

        result.append("<br>");
        result.append(QString("<p style=\"margin-left: 30px; margin-top: 0; margin-bottom: 0\">" + "Логин - %1</span></p>").arg(login));
        result.append(QString("<p style=\"margin-left: 30px; margin-top: 0; margin-bottom: 0\">" + "Пароль - %1</span></p>").arg(password));
        result.append("<br>");

        textMessage = textMessage.replace("$$$message$$$", result);

        textMessage = textMessage.replace("\r", "");
        textMessage = textMessage.replace("\n", "");
        textMessage = textMessage.replace("\r\n", "");
        textMessage = textMessage.replace("\t", "");

        QStringList files;
        files.append(email);

        emailNotificator.sendEMailNotification(email,
                                               "Восстановление кодов доступа к системе \"ВЗЛЕТ\"",
                                               textMessage,
                                               files,
                                               "vzletRecovery");
        printf("1");
        a.quit();
        return 0;
    }

    //Принятие участия в мероприятии участником
    if (args[1] == "applyToEvent")
    {
        QString textMessage = fileHandler.readHtmlFileTemplate(QString("%1/applyToEvent/applyToEvent.htm")
                                                               .arg(dirPath));

        QString email = getDataFromBase64(QString("%1").arg(argv[2])); //Адрес электронной почты пользователя
        QString userFIO = getDataFromBase64(QString("%1").arg(argv[3])); //ФИО пользователя
        QString message = getDataFromBase64(QString("%1").arg(argv[4])); //Текст сообщения

        textMessage = textMessage.replace("$$$userFIO$$$", userFIO);
        textMessage = textMessage.replace("$$$message$$$", message);

        textMessage = textMessage.replace("\t", " ");

        QStringList files;
        files.append(email);

        emailNotificator.sendEMailNotification(email,
                                               "Информация для записавшегося на участие в мероприятии Программы \"ВЗЛЕТ\"",
                                               textMessage,
                                               files,
                                               "applyEvent");
        printf("1");
        a.quit();
        return 0;
    }

    //Подтверждение согласия на обработку персональных данных пользователя
    if (args[1] == "confirmation")
    {
        QString textMessage = fileHandler.readHtmlFileTemplate(QString("%1/regUser/confirmation.htm")
                                                               .arg(dirPath));

        QString email = getDataFromBase64(QString("%1").arg(argv[2])); //Адрес, куда отправлять
        int userCode = getDataFromBase64(QString("%1").arg(argv[3])).toInt(); //Код пользователя (часть логина)
        QString password = getDataFromBase64(QString("%1").arg(argv[4])); //Пароль пользователя
        QString userFIO = getDataFromBase64(QString("%1").arg(argv[5])); //ФИО пользователя
        QString userStatus = getDataFromBase64(QString("%1").arg(argv[6])); //Статус пользователя
        QString userOrganization = getDataFromBase64(QString("%1").arg(argv[7])); //Название организации пользователя
        QString userSubdivision = getDataFromBase64(QString("%1").arg(argv[8])); //Название подразделения пользователя

        textMessage = textMessage.replace("$$$userLogin$$$", QString("%1@dov").arg(userCode));
        textMessage = textMessage.replace("$$$userPassword$$$", password);
        textMessage = textMessage.replace("$$$userFIO$$$", userFIO);
        textMessage = textMessage.replace("$$$userStatus$$$", userStatus);

        textMessage = textMessage.replace("$$$userOrganization$$$", userOrganization);

        if (userSubdivision == "no")
        {
            textMessage = textMessage.replace("$$$userSubdivision$$$", "нет");
        }
        else
        {
            textMessage = textMessage.replace("$$$userSubdivision$$$", userSubdivision);
        }

        textMessage = textMessage.replace("\r", "");
        textMessage = textMessage.replace("\n", "");
        textMessage = textMessage.replace("\r\n", "");
        textMessage = textMessage.replace("\t", "");

        QStringList files;
        files.append(email);

        emailNotificator.sendEMailNotification(email,
                                               "Согласие на обработку персональных данный в системе \"ВЗЛЕТ\"",
                                               textMessage,
                                               files,
                                               "confirm");
        printf("1");
        a.quit();
        return 0;
    }


    a.quit();
    return 0;
}
