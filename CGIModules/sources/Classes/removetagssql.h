#ifndef REMOVETAGSSQL_H
#define REMOVETAGSSQL_H

#include <QString>
#include <QRegExp>

class removeTagsSQL
{
public:
    removeTagsSQL();
    ~removeTagsSQL();

    //Удаление всех тегов и SQL и скриптов из строки
    QString removeTSQL(QString text);
};

#endif // REMOVETAGSSQL_H
