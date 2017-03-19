#include "filehandler.h"

FileHandler::FileHandler(QObject *parent) : QObject(parent)
{

}

//Чтение из шаблона html-файла
QString FileHandler::readHtmlFileTemplate(const QString &path)
{
    m_file.setFileName(path);
    m_file.open(QIODevice::ReadOnly);
    if (m_file.isOpen()) {
        QString result = QString::fromLocal8Bit(m_file.readAll());
        m_file.close();

        return result;
    }
    else {
        return "error oper file";
    }
}
