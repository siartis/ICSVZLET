#ifndef OPTION_H
#define OPTION_H

namespace OPTION {
    extern QString PATH;
    extern int PORT;
    extern int PORTADMIN;
    extern QString UID;
    extern QString IP;
    namespace DB {
        extern QString DRIVER;
        extern QString CONNECT;
    }
    namespace HTTP {
        extern QHash <QString, QString> MIME;
        extern QHash <int, QByteArray> CODE;
        extern QHash <int, QByteArray> spCODE;
    }
    extern int maxThread;
    extern int zipMode;
    extern int cookieLive;
    extern int maxCOOKIE;
    extern int pauseCOOKIE;
}

#endif // OPTION_H
