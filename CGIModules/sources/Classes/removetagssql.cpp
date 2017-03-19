#include "removetagssql.h"

removeTagsSQL::removeTagsSQL() { }
removeTagsSQL::~removeTagsSQL() { }

//Удаление всех тегов и SQL и скриптов из строки
QString removeTagsSQL::removeTSQL(QString text)
{
    QString p = text;
    text = "";

    p.replace("\r", " ");
    p.replace("\n", " ");
    p.replace("\r\n", " ");
    p.replace("&nbsp", " ");

    p.replace("SELECT", "", Qt::CaseInsensitive);
    p.replace("CHAR", "", Qt::CaseInsensitive);
    p.replace("WHERE", "", Qt::CaseInsensitive);
    p.replace("FROM", "", Qt::CaseInsensitive);
    p.replace("DELETE", "", Qt::CaseInsensitive);
    p.replace("DROP", "", Qt::CaseInsensitive);
    p.replace("CREATE TABLE", "", Qt::CaseInsensitive);
    p.replace("ALTER TABLE", "", Qt::CaseInsensitive);
    p.replace("CREATE", "", Qt::CaseInsensitive);
    p.replace("ALTER", "", Qt::CaseInsensitive);
    p.replace("TABLE", "", Qt::CaseInsensitive);
    p.replace("GROUP BY", "", Qt::CaseInsensitive);
    p.replace("UPDATE", "", Qt::CaseInsensitive);
    p.replace("VALUE", "", Qt::CaseInsensitive);
    p.replace("INSERT", "", Qt::CaseInsensitive);
    p.replace("INTO", "", Qt::CaseInsensitive);
    p.replace("VALUES", "", Qt::CaseInsensitive);
    p.replace("COUNT", "", Qt::CaseInsensitive);
    p.replace("THEN", "", Qt::CaseInsensitive);
    p.replace("ELSE", "", Qt::CaseInsensitive);
    p.replace("FUNCTION", "", Qt::CaseInsensitive);
    p.replace("ALERT", "", Qt::CaseInsensitive);
    p.replace("SCRIPT", "", Qt::CaseInsensitive);

    p.replace("<div>", "", Qt::CaseInsensitive);
    p.replace("</div>", "", Qt::CaseInsensitive);
    p.replace("<body>", "", Qt::CaseInsensitive);
    p.replace("</body>", "", Qt::CaseInsensitive);
    p.replace("<head>", "", Qt::CaseInsensitive);
    p.replace("</head>", "", Qt::CaseInsensitive);
    p.replace("<title>", "", Qt::CaseInsensitive);
    p.replace("</title>", "", Qt::CaseInsensitive);
    p.replace("<table>", "", Qt::CaseInsensitive);
    p.replace("<td>", "", Qt::CaseInsensitive);
    p.replace("</td>", "", Qt::CaseInsensitive);
    p.replace("<tr>", "", Qt::CaseInsensitive);
    p.replace("</tr>", "", Qt::CaseInsensitive);
    p.replace("<font>", "", Qt::CaseInsensitive);
    p.replace("<span>", "", Qt::CaseInsensitive);
    p.replace("</span>", "", Qt::CaseInsensitive);
    p.replace("<p>", "", Qt::CaseInsensitive);
    p.replace("</p>", "", Qt::CaseInsensitive);
    p.replace("<br>", "", Qt::CaseInsensitive);

    p.replace(QRegExp("/<!-- .*?>/g"), "");
    p.replace(QRegExp("/<xml.*?>(.*?)<\/xml.*?>/gm"), "");
    p.replace(QRegExp("/<style.*?>(.*?)<\/style.*?>/gm"), "");

    p.replace(QRegExp("/\r|\n|/g"), "");
    p.replace(QRegExp("/<\/?[^>]+>/gi"), "");

    return p;
}

