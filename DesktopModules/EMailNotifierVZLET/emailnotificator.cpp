#include "emailnotificator.h"

EMailNotificator::EMailNotificator(QObject *parent) : QObject(parent) { }

void EMailNotificator::sendEMailNotification(const QString &to,
                                             const QString &subject,
                                             const QString &body,
                                             QStringList files,
                                             const QString &userNameFrom)
{
    m_smtp = new Smtp(QString("%1@vzletsamara.ru").arg(userNameFrom), m_userPassword, m_server, m_serverPort);
    m_smtp->sendMail(QString("%1@vzletsamara.ru").arg(userNameFrom), to, subject, body, files);
}

