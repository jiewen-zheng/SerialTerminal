#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    //! [0]
    serial(new QSerialPort),
    status(new QLabel),
    statusIcon(new QLabel),
    console(new Console(this)),
    settings(new SettingsDialog(this))
{
    ui->setupUi(this);

    transfer = new TransferDialog(this, serial);

    // 文本编辑框居中处理
//    console->setEnabled(false);
    setCentralWidget(console);

    ui->actionConnect->setEnabled(true);
    ui->actionDisConnect->setEnabled(false);

    //初始化加载资源
    initResource();

    //初始化信号槽
    initActionsConnections();

    connect(serial, &QSerialPort::errorOccurred,  this, &MainWindow::serialPortError);
    connect(serial, &QSerialPort::readyRead, this, &MainWindow::readData);
    connect(console, &Console::putData, this, &MainWindow::writeData);
    connect(settings, &SettingsDialog::setFontSize, this, &MainWindow::setConsoleFontSize);
    connect(transfer, &TransferDialog::putMessage, console, &Console::showMessage);
}

MainWindow::~MainWindow()
{
    delete serial;
    delete status;
    delete statusIcon;
    delete greenLight;
    delete redLight;

    delete ui;
}

void MainWindow::initActionsConnections()
{
    connect(ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(ui->actionDisConnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionSettings, &QAction::triggered, settings, &SettingsDialog::showWindow);
    connect(ui->actionTransfer, &QAction::triggered, this, &MainWindow::showTransferDialog);
    connect(ui->actionClear, &QAction::triggered, console, &Console::initEdit);
}

void MainWindow::initResource()
{
    // 状态栏加载图标
    redLight = new QPixmap(":/images/redLight.png");
    *redLight = redLight->scaled(20, 20);

    greenLight = new QPixmap(":/images/greenLight.png");
    *greenLight = greenLight->scaled(20, 20);

    // 将图标标签添加到状态栏
    ui->statusBar->addWidget(statusIcon);
    statusIcon->setPixmap(*redLight);
    //添加状态栏信息
    ui->statusBar->addWidget(status);
    status->setText("Disconnect");
}

void MainWindow::showStatusMessage(const QString &message)
{
    status->setText(message);
}

void MainWindow::showStatusIcon(bool connected)
{
    if(connected){
        statusIcon->setPixmap(*greenLight);
    }else{
        statusIcon->setPixmap(*redLight);
    }
}

void MainWindow::openSerialPort()
{
    const SettingsDialog::Settings p = settings->getSettings();
    serial->setPortName(p.portName);
    serial->setBaudRate(p.baudRate);
    serial->setDataBits(p.dataBits);
    serial->setParity(p.parity);
    serial->setStopBits(p.stopBits);
    serial->setFlowControl(p.flowControl);

    if(!serial->open(QIODevice::ReadWrite)){
        QMessageBox::critical(this, "Error", serial->errorString());
        showStatusIcon(false);
        showStatusMessage("Open fail");
        return;
    }

    ui->actionConnect->setEnabled(false);
    ui->actionSettings->setEnabled(false);
    ui->actionDisConnect->setEnabled(true);

//    console->setEnabled(true);  //打开终端
    console->setLocaEchoEnable(p.localEchoEnable);
    showStatusIcon(true);
    showStatusMessage(QString("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.portName, p.strBaudRate, p.strDataBits,
                               p.strParity, p.strStopBits, p.strFlowControl));

}

void MainWindow::closeSerialPort()
{
    if(serial->isOpen()){
        serial->close();
    }

    ui->actionDisConnect->setEnabled(false);
    ui->actionConnect->setEnabled(true);
    ui->actionSettings->setEnabled(true);

//    console->setEnabled(false);
    showStatusIcon(false);
    showStatusMessage("Disconnected");
}

void MainWindow::serialPortError(QSerialPort::SerialPortError error)
{
    if(error == QSerialPort::ResourceError){
        closeSerialPort();
    }
}

void MainWindow::writeData(QByteArray &data)
{
    SettingsDialog::Settings set = settings->getSettings();

    if(set.lineFeed)
        data.push_back("\r\n");

    qint64 bytesWritten  = serial->write(data);

    if(bytesWritten == -1){
        console->showMessage(serial->errorString());
    }
}

void MainWindow::readData()
{
    //未开始文件传输时接收显示
    if(!transfer->isTransfer()){
        const QByteArray data = serial->readAll();
        console->putEditData(data);
    }
}

void MainWindow::showTransferDialog()
{
    transfer->setSerialParameters(settings->getSettings());
    transfer->show();
}

void MainWindow::setConsoleFontSize(int size)
{
    console->setFontSize(size);
}


