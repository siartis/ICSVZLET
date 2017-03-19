#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <QObject>
#include <QFile>

class FileHandler : public QObject
{
    Q_OBJECT
public:
    explicit FileHandler(QObject *parent = 0);

    //Чтение из шаблона html-файла
    QString readHtmlFileTemplate(const QString &path);

private:
    QFile m_file;
};

#endif // FILEHANDLER_H
