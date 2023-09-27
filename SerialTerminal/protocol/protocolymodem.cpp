#include "protocolymodem.h"

#include <QFileInfo>
#include <QDir>

ProtocolYModem::ProtocolYModem(QSerialPort *serial):
    m_serial(serial),
    file(new QFile)
{

}

ProtocolYModem::~ProtocolYModem()
{
    delete file;
}

void ProtocolYModem::setFileName(const QString &name)
{
    file->setFileName(name);
}

void ProtocolYModem::setPath(const QString &_path)
{
    path = _path;
}

uint32_t ProtocolYModem::write(uint8_t *data, uint32_t len)
{
    uint32_t writeLen = m_serial->write((char *)data, len);
    m_serial->waitForReadyRead(1000);

    return writeLen;
}

uint32_t ProtocolYModem::read(uint8_t *data, uint32_t len)
{
    uint32_t readLen = m_serial->read((char *)data, len);
    m_serial->waitForReadyRead(10);
    return readLen;
}

uint32_t ProtocolYModem::fileOperation(OperationType type, uint8_t *data, uint32_t len)
{
    switch(type){
    case FileOpenReadOnly:
        if(!file->open(QIODevice::ReadOnly)){
            return 0;
        }
        return 1;

    case FileOpenWriteOnly:
        if(!file->open(QIODevice::WriteOnly)){
            return 0;
        }
        return 1;

    case FileOpenReadWrite:
        if(!file->open(QIODevice::ReadWrite)){
            return 0;
        }
        return 1;

    case FileClose:
        file->close();
        return 1;

    case FileGetName:{
        QFileInfo info(*file);
        memcpy((char *)data, info.fileName().toUtf8().data(), info.fileName().size());
        return info.fileName().size();
    }

    case FileSetName:{
        QString fileName = QDir(path).filePath(QString((char *)data));
        file->setFileName(fileName);
        return 1;
    }

    case FileGetSize:{
        QFileInfo info(*file);
        memcpy((char *)data, QByteArray::number(info.size()).data(), QByteArray::number(info.size()).size());
        return info.size();
    }

    case FileWrite:
        return file->write((char *)data, len);

    case FileRead:
        return file->read((char *)data, len);

    default:
        break;
    }

    return 0;
}

