#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QByteArray>
#include <QStringList>
#include <QSqlDatabase>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QTime>
#include <QSqlRecord>
#include <QMessageBox>

#include <QtConcurrent>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_timer = new QTimer(this);
    m_fileName = "C:/vzlet/childRatingStage/childRatingStageLOG.txt";

    m_i = 0;
    m_ok = true;
    m_run = 0;

    connect(this, SIGNAL(runFinish()), SLOT(runFinished()));

    this -> setTrayIconActions();
    this -> showTrayIcon();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_triggered()
{
    qApp->quit();
}

void MainWindow::timerTimeout()
{
    QtConcurrent::run(this, &MainWindow::changeRating);
}

void MainWindow::changeRating()
{
    db = QSqlDatabase::addDatabase("QMYSQL", "db");
    db.setDatabaseName("vzletbase");
    db.setHostName("");
    db.setUserName("root");
    db.setPassword("");
    db.setPort(3306);
    db.open();
    if (m_db.isOpen()) {
        QString query;
        QSqlQuery q(m_db);

        int stageExecuteID = 0;
        QStringList currentStageExecuteIDs;
        QStringList themeIDs;
        QStringList childIDs;
        QString timeSheduleDateExecute;
        int teacherID = 0; //Код учителя
        int stageExecuteIDByTeacher = 0; //Стадия выполнения проекта по оценке учителя

        m_fl.changeLog(m_fileName, QString("Начало прогона - %1, %2 ").arg(QDate::currentDate().toString("dd.MM.yyyy")).arg(QTime::currentTime().toString("hh:mm:ss")));

        //Получаем этап из расписания, который должен быть сегодня
        query = QString("SELECT stageExecuteID, timeSheduleDateExecute FROM "
                        "vzletbase.timeShedule "
                        "WHERE "
                        "(Month(timeSheduleDateExecute) = %1 AND YEAR(timeSheduleDateExecute) = %2 AND DAY(timeSheduleDateExecute) = %3) ")
                .arg(QDate::currentDate().month())
                .arg(QDate::currentDate().year())
                .arg(QDate::currentDate().day());

        if (q.exec(query)) {
            while (q.next()) {
                stageExecuteID = q.value(0).toInt();
                timeSheduleDateExecute = QDate::fromString(q.value(1).toString(), "dd.MM.yyyy").toString("dd.MM.yyyy");
            }

            //Та самая дата
            if (stageExecuteID != 0) {

                childIDs.clear();
                themeIDs.clear();
                currentStageExecuteIDs.clear();

                //Получаем все выполняемые темы
                query = "SELECT theme.themeID, stageExecuteID, userTheme.userID "
                        "FROM vzletbase.theme, vzletbase.userTheme, vzletbase.userStatusOrganizationSubdivision  "
                        "WHERE statusThemeID = 2 AND (themeStaffing = TRUE) "
                        "AND (userTheme.userID IN (SELECT userID FROM vzletbase.userStatusOrganizationSubdivision WHERE statusID = 2)) AND (userTheme.themeID = theme.themeID) AND (userTheme.userID = userStatusOrganizationSubdivision.userID) ";

                if (q.exec(query)) {
                    while (q.next()) {
                        themeIDs.append(q.value(0).toString());
                        currentStageExecuteIDs.append(q.value(1).toString());
                        childIDs.append(q.value(2).toString());
                    }

                    QString g;

                    //Идем по кодам тем
                    for(int i = 0; i < themeIDs.length(); i++) {

                        if (themeIDs.at(i).toInt() == 1816) {
                            g = QString("%1   %2   %3")
                                    .arg(themeIDs.at(i).toInt())
                                    .arg(stageExecuteID)
                                    .arg(currentStageExecuteIDs.at(i).toInt());
                        }

                        //Если текущий этап темы совпадает с этапом по расписанию
                        if (currentStageExecuteIDs.at(i).toInt() == stageExecuteID) {

                            //Добавляем единицу к рейтингу школьника
                            query = QString("UPDATE vzletbase.userStatusOrganizationSubdivision "
                                            "SET childRatingStage = childRatingStage + 1 "
                                            "WHERE userID = %1 ")
                                    .arg(childIDs.at(i));

                            QDebug() << query;

                            if (q.exec(query)) {

                                //За что добавляем рейтинг, прописываем
                                query = QString("UPDATE vzletbase.userStatusOrganizationSubdivision "
                                                "SET childPortfolioStage = childPortfolioStage + '+1 за соблюдение графика (%1, %2); ' "
                                                "WHERE userID = %3 ")
                                        .arg(QDate::currentDate().toString("dd.MM.yyyy"))
                                        .arg(QTime::currentTime().toString("hh:mm:ss"))
                                        .arg(childIDs.at(i));

                                if (q.exec(query)) {
                                    m_fl.changeLog(m_fileName, QString("+1 балл ученику с кодом (%1), код темы (%2), текущая стадия темы (%3), %4, %5 ")
                                                   .arg(childIDs.at(i))
                                                   .arg(themeIDs.at(i))
                                                   .arg(currentStageExecuteIDs.at(i))
                                                   .arg(QDate::currentDate().toString("dd.MM.yyyy"))
                                                   .arg(QTime::currentTime().toString("hh:mm:ss")));
                                }
                                else {
                                    m_fl.stringError(m_fileName, QString("Невозможно добавить отчет за получение единицы к рейтингу ученика с кодом (%2), тема (%3), текущая стадия (%4), ошибка - %1, %5, %6 ")
                                                     .arg(q.lastError().text())
                                                     .arg(childIDs.at(i))
                                                     .arg(themeIDs.at(i))
                                                     .arg(currentStageExecuteIDs.at(i))
                                                     .arg(QDate::currentDate().toString("dd.MM.yyyy"))
                                                     .arg(QTime::currentTime().toString("hh:mm:ss")));
                                }
                            }
                            else {
                                m_fl.stringError(m_fileName, QString("Невозможно добавить единицу к рейтингу ученика с кодом (%2), тема (%3), текущая стадия (%4), ошибка - %1, %5, %6 ")
                                                 .arg(q.lastError().text())
                                                 .arg(childIDs.at(i))
                                                 .arg(themeIDs.at(i))
                                                 .arg(currentStageExecuteIDs.at(i))
                                                 .arg(QDate::currentDate().toString("dd.MM.yyyy"))
                                                 .arg(QTime::currentTime().toString("hh:mm:ss")));
                            }
                        }
                        else {
                            //Может быть учитель проставил этап выполнения работы, а консультант нет?!


                            //Получаем код учителя конкретной темы
                            query = QString("SELECT userID FROM vzletbase.userTheme WHERE (themeID = %1) AND (userID IN "
                                            "( "
                                            "SELECT userID FROM vzletbase.userStatusOrganizationSubdivision WHERE statusID = 3 "
                                            ")) ")
                                    .arg(themeIDs.at(i));

                            teacherID = 0;

                            if (q.exec(query)) {
                                while (q.next()) {
                                    teacherID = q.value(0).toInt();
                                }

                                if (teacherID != 0) {

                                    //Получаем оценку взаимодействия (по оценке учителя)
                                    query = QString("SELECT LAST(stageExecuteID), LAST(interactionDateTime) "
                                                    "FROM vzletbase.interaction "
                                                    "WHERE userIDWho = %1 ")
                                            .arg(teacherID);

                                    if (q.exec(query)) {
                                        while (q.next()) {
                                            stageExecuteIDByTeacher = q.value(0).toInt();
                                        }

                                        //Если оценка взаимодйствия по оценке учитителя (текущая стадия выполнения проекта) является той - которая должна быть по расписанию
                                        if (stageExecuteIDByTeacher >= stageExecuteID) {

                                            if (stageExecuteID == 2) {
                                                //Добавляем полбалла к рейтингу школьника
                                                query = QString("UPDATE vzletbase.userStatusOrganizationSubdivision "
                                                                "SET childRatingStage = childRatingStage + 0.5, "
                                                                "partyRegistryID = 3 "
                                                                "WHERE userID = %1 ")
                                                        .arg(childIDs.at(i));
                                            }
                                            else {
                                                //Добавляем полбалла к рейтингу школьника
                                                query = QString("UPDATE vzletbase.userStatusOrganizationSubdivision "
                                                                "SET childRatingStage = childRatingStage + 0.5 "
                                                                "WHERE userID = %1 ")
                                                        .arg(childIDs.at(i));
                                            }


                                            if (q.exec(query)) {

                                                //За что добавляем рейтинг, прописываем
                                                query = QString("UPDATE vzletbase.userStatusOrganizationSubdivision "
                                                                "SET childPortfolioStage = childPortfolioStage + '+0.5 (по оценке учителя %4) за соблюдение графика (%1, %2); ' "
                                                                "WHERE userID = %3 ")
                                                        .arg(QDate::currentDate().toString("dd.MM.yyyy"))
                                                        .arg(QTime::currentTime().toString("hh:mm:ss"))
                                                        .arg(childIDs.at(i))
                                                        .arg(teacherID);

                                                if (q.exec(query)) {
                                                    m_fl.changeLog(m_fileName, QString("+0.5 балла ученику с кодом (%1) по оценке учителя (код - %6), код темы (%2), текущая стадия темы (%3), %4, %5 ")
                                                                   .arg(childIDs.at(i))
                                                                   .arg(themeIDs.at(i))
                                                                   .arg(currentStageExecuteIDs.at(i))
                                                                   .arg(QDate::currentDate().toString("dd.MM.yyyy"))
                                                                   .arg(QTime::currentTime().toString("hh:mm:ss"))
                                                                   .arg(teacherID));
                                                }
                                                else {
                                                    m_fl.stringError(m_fileName, QString("Невозможно добавить отчет за получение единицы к рейтингу ученика с кодом (%2), тема (%3), текущая стадия (%4), ошибка - %1, %5, %6 ")
                                                                     .arg(q.lastError().text())
                                                                     .arg(childIDs.at(i))
                                                                     .arg(themeIDs.at(i))
                                                                     .arg(currentStageExecuteIDs.at(i))
                                                                     .arg(QDate::currentDate().toString("dd.MM.yyyy"))
                                                                     .arg(QTime::currentTime().toString("hh:mm:ss")));
                                                }
                                            }
                                            else {
                                                m_fl.stringError(m_fileName, QString("Невозможно добавить 0.5 балла к рейтингу ученика по оценке учителя (код - %7) с кодом (%2), тема (%3), текущая стадия (%4), ошибка - %1, %5, %6 ")
                                                                 .arg(q.lastError().text())
                                                                 .arg(childIDs.at(i))
                                                                 .arg(themeIDs.at(i))
                                                                 .arg(currentStageExecuteIDs.at(i))
                                                                 .arg(QDate::currentDate().toString("dd.MM.yyyy"))
                                                                 .arg(QTime::currentTime().toString("hh:mm:ss"))
                                                                 .arg(teacherID));
                                            }
                                        }
                                    }
                                    else {
                                        //В этой теме нет учителя! Балл проставлен не будет!
                                    }
                                }
                            }
                        }
                    }
                }
                else {
                    m_fl.stringError(m_fileName, QString("Невозможно получить список тем с учениками, ошибка - %1, %2, %3 ")
                                     .arg(q.lastError().text())
                                     .arg(QDate::currentDate().toString("dd.MM.yyyy"))
                                     .arg(QTime::currentTime().toString("hh:mm:ss")));
                }
            }
            else {
                m_fl.changeLog(m_fileName, QString("Дата (%1) не входит в расписание - %2 ")
                               .arg(QDate::currentDate().toString("dd.MM.yyyy"))
                               .arg(QTime::currentTime().toString("hh:mm:ss")));
            }
        }
        else {
            m_fl.stringError(m_fileName, QString("Невозможно получить расписание, ошибка - %1, %2, %3 ")
                             .arg(q.lastError().text())
                             .arg(QDate::currentDate().toString("dd.MM.yyyy"))
                             .arg(QTime::currentTime().toString("hh:mm:ss")));
        }

        //Очистка переменных (освобождение памяти)
        stageExecuteID = 0;
        currentStageExecuteIDs.clear();
        themeIDs.clear();
        childIDs.clear();
        timeSheduleDateExecute.clear();
        teacherID = 0; //Код учителя
        stageExecuteIDByTeacher = 0; //Стадия выполнения проекта по оценке учителя
    }
    else {
        m_fl.stringError(m_fileName, QString("Ошибка открытия базы данных - %1, %2, %3 ")
                         .arg(m_db.lastError().text())
                         .arg(QDate::currentDate().toString("dd.MM.yyyy"))
                         .arg(QTime::currentTime().toString("hh:mm:ss")));
    }

    m_db.close();
    m_i++;
    m_fl.changeLog(m_fileName, QString("Конец прогона - %1, %2 \r\n \r\n").arg(QDate::currentDate().toString("dd.MM.yyyy")).arg(QTime::currentTime().toString("hh:mm:ss")));
    emit runFinish();
}

void MainWindow::on_startButton_clicked()
{
    ui->startButton->setEnabled(false);
    ui->stopButton->setEnabled(true);

    startTimer();
}

void MainWindow::startTimer()
{
    m_timer->setInterval(60 * 1000 * 60 * 24); //Настроенный интервал
    connect(m_timer, SIGNAL(timeout()), SLOT(timerTimeout()));
    m_timer->start();
}

void MainWindow::on_stopButton_clicked()
{
    m_timer->stop();
    ui->stopButton->setEnabled(false);
    ui->startButton->setEnabled(true);
}

void MainWindow::runFinished()
{
    m_run++;
    ui->countLabel->setText(QString("Количество прогонов - %1, последний в %2, %3 ")
                            .arg(m_run)
                            .arg(QDate::currentDate().toString("dd.MM.yyyy"))
                            .arg(QTime::currentTime().toString("hh:mm:ss")));
}


//Сворачивание в трей
void MainWindow::showTrayIcon()
{
    // Создаём экземпляр класса и задаём его свойства...
    trayIcon = new QSystemTrayIcon(this);
    QIcon trayImage(":/ico.png");
    trayIcon -> setIcon(trayImage);
    trayIcon -> setContextMenu(trayIconMenu);

    // Подключаем обработчик клика по иконке...
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));

    // Выводим значок...
    trayIcon -> show();
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        break;
    default:
        break;
    }
}

void MainWindow::setTrayIconActions()
{
    // Setting actions...
    minimizeAction = new QAction("Свернуть", this);
    restoreAction = new QAction("Восстановить", this);
    quitAction = new QAction("Выход", this);

    // Connecting actions to slots...
    connect (minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));
    connect (restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));
    connect (quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    // Setting system tray's icon menu...
    trayIconMenu = new QMenu(this);
    trayIconMenu -> addAction (minimizeAction);
    trayIconMenu -> addAction (restoreAction);
    trayIconMenu -> addAction (quitAction);
}

void MainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    if (event -> type() == QEvent::WindowStateChange)
    {
        if (isMinimized())
        {
            this -> hide();
        }
    }
}
