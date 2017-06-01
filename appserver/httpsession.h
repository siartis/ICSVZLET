#ifndef HTTPSESSION_H
#define HTTPSESSION_H

#include <QtCore>
#include "cookie.h"

namespace HTTPSESSION {
    extern QHash <QString, COOKIE> cookie;
    extern QHash <QString, QHash<QString, QString> > envSession;
    extern QHash <QString, QString > envAll;
    extern QHash <QString, QSet <QString> > envParent;
    extern QHash <QString, int> lastCode;
    QString getCookie(COOKIE *cookie, const QString &ses);
    void updateCookie(const COOKIE& cookie);
    int env(QByteArray *anser, const QString& Path,
            const QString& param, const COOKIE& cookie);
}

#endif // HTTPSESSION_H
