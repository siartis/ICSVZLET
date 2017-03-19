#ifndef EMAILNOTIFICATOR_H
#define EMAILNOTIFICATOR_H

#include <QObject>

#include "smtp.h"

class EMailNotificator : public QObject
{
    Q_OBJECT
public:
    explicit EMailNotificator(QObject *parent = 0);

    void sendEMailNotification(const QString &to,
                               const QString &subject,
                               const QString &body,
                               QStringList files,
                               const QString &userNameFrom);

private:
    Smtp *m_smtp;

    QString m_userPassword = "*****";
    QString m_server = "smtp.mail.ru";
    int m_serverPort = 465;
};

#endif // EMAILNOTIFICATOR_H
