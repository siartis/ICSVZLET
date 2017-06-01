#ifndef COOKIE_H
#define COOKIE_H

#include <QtCore>

typedef struct _COOKIE{
    QString Name;
    time_t LastTime;
    long personID;
    QString personName;
    long roleID;
    QString roleName;
    QString Autor;
    QString xxx;
    QString toString(const char *sep) const {


//        QString gg = QString("%1\r\n%2\r\n%3\r\n%4\r\n%5\r\n%6")
//                .arg(Name).arg(LastTime).arg(personID).arg(personName)
//                .arg(roleID).arg(roleName)
//                .replace("\r\n",sep);



        return QString("%1\r\n%2\r\n%3\r\n%4\r\n%5\r\n%6")
                .arg(Name).arg(LastTime).arg(personID).arg(personName)
                .arg(roleID).arg(roleName)
                .replace("\r\n",sep);

    }
/*    static _COOKIE fromString(const QString& s, const char *sep) {
        QStringList x=s.split(sep);
        //if(x.size()!=11)return _COOKIE::Empty();
        _COOKIE q;
        q.Name=x.at(0);
        q.LastTime=x.at(1).toInt();
        q.personID=x.at(2).toInt();
        q.personName=x.at(3);
        //q.Autor=x.at(10);
        return q;
    } */
} COOKIE;

#endif
