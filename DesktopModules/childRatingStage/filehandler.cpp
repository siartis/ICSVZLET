#include "filehandler.h"

fileHandler::fileHandler(QObject *parent) : QObject(parent) { }
fileHandler::~fileHandler() { }

void fileHandler::changeLog(const QString &fileName, const QString &textMessage)
{
    m_file.setFileName(fileName);
    m_file.open(QIODevice::Append);
    m_file.write(textMessage.toLocal8Bit());
    m_file.write("\r\n");
    m_file.close();
}

void fileHandler::stringError(const QString &fileName, const QString &errorMessage)
{
    m_file.setFileName(fileName);
    m_file.open(QIODevice::Append);
    m_file.write(errorMessage.toLocal8Bit());
    m_file.write("\r\n");
    m_file.close();
}
