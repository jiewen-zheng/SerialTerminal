#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSerialPort>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

signals:
    void setFontSize(int size);

public:
    //串口设置参数缓存
    struct Settings{
        QString portName;
        qint32 baudRate;
        QString strBaudRate;
        QSerialPort::DataBits dataBits;
        QString strDataBits;
        QSerialPort::Parity parity;
        QString strParity;
        QSerialPort::StopBits stopBits;
        QString strStopBits;
        QSerialPort::FlowControl flowControl;
        QString strFlowControl;
        bool localEchoEnable;
        bool lineFeed;

        int fontSize;
    };

public slots:
    void showWindow();

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    Settings getSettings() const;   //获取当前串口设置参数

private slots:
    void showPortInfo(int idx); //显示串口信息
    void apply();

private:
    void fillPortsParameters(); //填充端口可选参数
    void fillPortsInfo();       //填充端口信息
    void fillFontSize();
    void updateSettings();      //更新串口设置

private:
    Ui::SettingsDialog *ui;
    Settings currentSettings;
    size_t currentSerialPorts = 0;
};

#endif // SETTINGSDIALOG_H
