#ifndef TRANSFERDIALOG_H
#define TRANSFERDIALOG_H

#include <QDialog>
#include <QSerialPort>
#include <QFile>
#include <QFileInfo>

#include "transferthread.h"
#include "settingsdialog.h"

namespace Ui {
class TransferDialog;
}

class TransferDialog : public QDialog
{
    Q_OBJECT

signals:
    void putMessage(QString msg, int flag = 2);

public:
    enum TransferDirection{
        Transmit,
        Receive
    };

    struct FileInfomation{
        QString fileUrl;
        QString name;
        QString path;
        quint32 size;
        quint32 packetSize;
    };

public:
    explicit TransferDialog(QWidget *parent = nullptr, QSerialPort *serial = nullptr);
    ~TransferDialog();

    bool isTransfer();
    void transfer(TransferDirection dir, const QString fileName);
    void setSerialParameters(const SettingsDialog::Settings &setting);

public slots:
    void start();
    void cancel();
    void over();

    void handleTransferOver(const QString result, bool successed = false);
    void showProgress(int pro);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    void initActionsConnections();

    void fillProtocol();    //填充可选协议
    void showFileMessage(FileInfomation* info = nullptr);
    bool updateMessage(QString fileName);


private:
    Ui::TransferDialog *ui;
    QSerialPort *m_serial;
    TransferThread* m_transferThread = nullptr;

    SettingsDialog::Settings serialSettings;  //串口设置，传递到thread的串口
    FileInfomation fileInfo;
    TransferDirection direction;
    bool transferState = false;//文件传输状态标志
    bool mainSerialOpen = false;//主窗口原始串口开关状态
};

#endif // TRANSFERDIALOG_H
