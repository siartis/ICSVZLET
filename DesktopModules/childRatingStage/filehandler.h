#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <QObject>
#include <QFile>

class fileHandler : public QObject
{
    Q_OBJECT
public:
    explicit fileHandler(QObject *parent = 0);
    ~fileHandler();

    QFile m_file;

    void changeLog(const QString &fileName, const QString &textMessage);
    void stringError(const QString &fileName, const QString &errorMessage);

};

#endif // FILEHANDLER_H
