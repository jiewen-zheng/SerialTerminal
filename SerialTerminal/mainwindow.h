#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort>
#include <QLabel>

#include "console.h"
#include "settingsdialog.h"
#include "transferdialog.h"

QT_BEGIN_NAMESPACE

namespace Ui { class MainWindow; }

QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openSerialPort();
    void closeSerialPort();
    void serialPortError(QSerialPort::SerialPortError error);

    void writeData(QByteArray &data);
    void readData();

    void showTransferDialog();
    void setConsoleFontSize(int size);

private:
    void initActionsConnections();
    void initResource();

private:
    void showStatusMessage(const QString &message); //设置状态栏消息
    void showStatusIcon(bool connected);

private:
    Ui::MainWindow *ui;
    QLabel *status = nullptr;     //状态栏文字
    QLabel *statusIcon = nullptr; //状态栏图标
    QPixmap *redLight = nullptr;
    QPixmap *greenLight = nullptr;

    Console *console = nullptr;   //控制台
    SettingsDialog *settings = nullptr; //设置页面
    TransferDialog *transfer = nullptr; //传输页面

    QSerialPort *serial = nullptr;  //串口对象
};
#endif // MAINWINDOW_H
