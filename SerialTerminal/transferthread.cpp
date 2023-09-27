#include "transferthread.h"

#include <QCoreApplication>


#include "protocol/protocolymodem.h"

TransferThread::TransferThread(QObject *parent) :
    QThread(parent),
    m_serial(new QSerialPort)
{
    m_serial->moveToThread(this);
}

TransferThread::~TransferThread()
{
    qDebug() << "TransferThread deleteLater";

    delete m_serial;
}

void TransferThread::transaction()
{
    if(!isRunning()){
        m_quit = false;
        start();
    }
}

void TransferThread::cancellation()
{
    if(isRunning()){
        m_quit = true;
    }
}

void TransferThread::setDirection(Direction _dir)
{
    dir = _dir;
}

void TransferThread::setFileName(const QString &name)
{
    fileName = name;
}

void TransferThread::setProtocol(Protocol pro)
{
    protocol = pro;
}

void TransferThread::setSerialInfo(const SettingsDialog::Settings &serialSet)
{
    m_serial->setPortName(serialSet.portName);
    m_serial->setPortName(serialSet.portName);
    m_serial->setBaudRate(serialSet.baudRate);
    m_serial->setDataBits(serialSet.dataBits);
    m_serial->setParity(serialSet.parity);
    m_serial->setStopBits(serialSet.stopBits);
    m_serial->setFlowControl(serialSet.flowControl);
}

void TransferThread::run()
{
    qDebug() << "TransferThread::run, thread = " << QThread::currentThread();
    qDebug() << "TransferThread::run, TransferThread thread = " << this->thread();
    qDebug() << "TransferThread::run, m_serial = " << m_serial->thread();

    if(!m_serial->open(QIODevice::ReadWrite)){
        qDebug() << __FILE__ << "Serial open failed";
        return;
    }

    ProtocolYModem ymodem(m_serial);

    if(dir == Transmit)
        ymodem.setFileName(fileName);
    else
        ymodem.setPath(fileName);

    while(!m_quit){

        if(dir == Transmit)
            ymodem.transmit();
        else
            ymodem.receive();

        emit displayProgress(ymodem.getProgress());

        if((ymodem.getStatus() != YModem::StatusEstablish) &&
            (ymodem.getStatus() != YModem::StatusTransmit)){
            break;
        }

        msleep(10);
    }

    m_serial->close();

    if(ymodem.getStatus() != YModem::StatusFinish)
        emit transferOver("文件传输失败", false);
    else
        emit transferOver("文件传输成功", true);

}
