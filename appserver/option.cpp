#include <QtCore>

namespace OPTION {
    namespace DB {
//        QString DRIVER="QODBC";
//        QString CONNECT="DRIVER={Microsoft Access Driver (*.mdb)};"
//                        "FIL={MS Access};DBQ=�://sait/core.mdb;";

    //��� Microsoft SQL Server
//    QString DRIVER="QODBC3";
//    QString CONNECT="DRIVER={SQL Server};Database=VzletBase; Server=Win8Pro-��\\SQLEXPRESS;";

    QString DRIVER="QMYSQL";
    QString CONNECT="DRIVER={SQL Server};Database=VzletBase; Server=Win8Pro-��\\SQLEXPRESS;";



    }
    namespace HTTP {
        QHash <QString, QString> MIME;
        QHash <int, QByteArray> CODE;
        QHash <int, QByteArray> spCODE;
    }
    //QString PATH="�://Apache/";
    QString PATH="D://odarmolTest/odarmol/";
    QString UID="exe";
    QString IP="127.0.0.1";
    int PORT=80;
    int PORTADMIN=81;
    int maxThread=2;
    int zipMode=9;
    int cookieLive=900;
    int maxCOOKIE=0;
    int pauseCOOKIE=10;
}
