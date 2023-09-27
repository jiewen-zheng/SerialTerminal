#ifndef TRANSFERTHREAD_H
#define TRANSFERTHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include <QSerialPort>
#include <QFile>

#include "settingsdialog.h"


class TransferThread : public QThread
{
    Q_OBJECT

signals:
    void displayProgress(int progress);
    void transferOver(const QString &s, bool successed);

public:
    enum Protocol{
        ProtocolYMomde = 0,
    };

    enum Direction{
        Transmit,
        Receive
    };

public:
    TransferThread(QObject *parent = nullptr);
    ~TransferThread();

    void transaction();
    void cancellation();

    void setDirection(Direction _dir);
    void setFileName(const QString &name);
    void setProtocol(TransferThread::Protocol pro);
    void setSerialInfo(const SettingsDialog::Settings &serialSet);

protected:
    void run() override;

private:
    QString fileName;
    Protocol protocol;
    Direction dir = Transmit;

    QSerialPort *m_serial;


    bool m_quit = false;
};

#endif // TRANSFERTHREAD_H
