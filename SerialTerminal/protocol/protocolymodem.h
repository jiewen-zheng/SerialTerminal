#ifndef PROTOCOLYMODEM_H
#define PROTOCOLYMODEM_H


#include <QSerialPort>
#include <QFile>

#include <qdebug.h>

#include "../ymodem/ymodem.h"

class ProtocolYModem : public YModem
{
public:
    ProtocolYModem(QSerialPort *serial);
    ~ProtocolYModem();

    void setFileName(const QString &name);
    void setPath(const QString &_path);

protected:
     uint32_t write(uint8_t *data, uint32_t len) override;
     uint32_t read(uint8_t *data, uint32_t len) override;
     uint32_t fileOperation(OperationType type, uint8_t *data, uint32_t len) override;

 private:
     QSerialPort *m_serial = nullptr;

     QFile *file;
     QString path;
};

#endif // PROTOCOLYMODEM_H
