#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QSqlDatabase>
#include "filehandler.h"

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QAction>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_action_triggered();

    void timerTimeout();
    void changeRating();
    void startTimer();
    void runFinished(); //Прогон завершен

    void on_startButton_clicked();
    void on_stopButton_clicked();


    //Для того, чтобы свернуть приложение в системный трей
    void changeEvent(QEvent*);
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void setTrayIconActions();
    void showTrayIcon();

signals:
    void runFinish();

private:
    Ui::MainWindow *ui;

    QTimer *m_timer;
    QSqlDatabase m_db;
    QString m_dbName;
    QString m_fileName;
    fileHandler m_fl;
    int m_i;
    bool m_ok;
    int m_run; //Количество прогонов

    //Для того, чтобы свернуть в трей
    QMenu *trayIconMenu;
    QAction *minimizeAction;
    QAction *restoreAction;
    QAction *quitAction;
    QSystemTrayIcon *trayIcon;

};

#endif // MAINWINDOW_H
