#include "settingsdialog.h"
#include "ui_settingsdialog.h"


#include <QSerialPortInfo>
#include <QDebug>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    //刷新端口信息槽函数
    connect(ui->serialPortBox, &QComboBox::currentIndexChanged,
            this, &SettingsDialog::showPortInfo);
    connect(ui->ApplayButton, &QPushButton::clicked,
            this, &SettingsDialog::apply);

    fillPortsParameters();
    fillPortsInfo();
    fillFontSize();

    updateSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

SettingsDialog::Settings SettingsDialog::getSettings() const
{
    return currentSettings;
}

void SettingsDialog::showPortInfo(int idx)
{
    if(idx == -1)
        return;

    const QString blankString = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

    const QStringList list = ui->serialPortBox->itemData(idx).toStringList();
    ui->descriptionLabel->setText(QString("描述：%1").arg(list.value(1)));
    ui->manufacturerLabel->setText(QString("制造商：%1").arg(list.value(2)));
    ui->serialNumberLabel->setText(QString("序列号：%1").arg(list.value(3, blankString)));
    ui->locationLabel->setText(QString("端口：%1").arg(list.value(4)));
    ui->vidLabel->setText(QString("供应商ID：%1").arg(list.value(5)));
    ui->pidLabel->setText(QString("产品ID：%1").arg(list.value(6)));
}

void SettingsDialog::apply()
{
    updateSettings();
    emit setFontSize(currentSettings.fontSize);
    hide();
}

void SettingsDialog::showWindow()
{
    show();
    fillPortsInfo();
}

void SettingsDialog::fillPortsParameters()
{
    ui->baudRateBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    ui->baudRateBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    ui->baudRateBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    ui->baudRateBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    ui->baudRateBox->addItem(QStringLiteral("256000"), 256000);
    ui->baudRateBox->setCurrentIndex(3);

    ui->dataBitsBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
    ui->dataBitsBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
    ui->dataBitsBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
    ui->dataBitsBox->addItem(QStringLiteral("8"), QSerialPort::Data8);
    ui->dataBitsBox->setCurrentIndex(3);

    ui->parityBox->addItem(QStringLiteral("None"), QSerialPort::NoParity);
    ui->parityBox->addItem(QStringLiteral("Even"), QSerialPort::EvenParity);
    ui->parityBox->addItem(QStringLiteral("Odd"), QSerialPort::OddParity);

    ui->stopBitsBox->addItem(QStringLiteral("1"), QSerialPort::OneStop);
    ui->stopBitsBox->addItem(QStringLiteral("2"), QSerialPort::TwoStop);

    ui->flowControlBox->addItem(QStringLiteral("None"), QSerialPort::NoFlowControl);
    ui->flowControlBox->addItem(QStringLiteral("RTS/CTS"), QSerialPort::HardwareControl);
    ui->flowControlBox->addItem(QStringLiteral("XON/XOFF"), QSerialPort::SoftwareControl);

}

void SettingsDialog::fillPortsInfo()
{
    if(QSerialPortInfo::availablePorts().size() == currentSerialPorts)
        return;

    currentSerialPorts = QSerialPortInfo::availablePorts().size();
    ui->serialPortBox->clear();

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QStringList list;
        const QString portName = info.portName();
        const QString description = info.description();
        const QString manufacturer = info.manufacturer();
        const QString serialNumber = info.serialNumber();
        const auto vendorId = info.vendorIdentifier();
        const auto productId = info.productIdentifier();
        const QString showPort = portName + ": " + description;

        list << portName
             << description
             << manufacturer
             << serialNumber
             << info.systemLocation()
             << QString::number(vendorId, 16)
             << QString::number(productId, 16)
             << showPort;

        ui->serialPortBox->addItem(list.last(), list);
    }
}

void SettingsDialog::fillFontSize()
{
    ui->fontSizeBox->addItem("8", 8);
    ui->fontSizeBox->addItem("10", 10);
    ui->fontSizeBox->addItem("12", 12);
    ui->fontSizeBox->addItem("14", 14);
    ui->fontSizeBox->addItem("16", 16);
    ui->fontSizeBox->addItem("18", 18);
    ui->fontSizeBox->addItem("20", 20);
    ui->fontSizeBox->addItem("24", 24);
    ui->fontSizeBox->addItem("28", 28);
    ui->fontSizeBox->addItem("32", 32);
    ui->fontSizeBox->addItem("36", 36);
    ui->fontSizeBox->addItem("48", 48);
    ui->fontSizeBox->setCurrentIndex(4);
}

void SettingsDialog::updateSettings()
{
    // 端口号
    currentSettings.portName = ui->serialPortBox->itemData(ui->serialPortBox->currentIndex()).toStringList().first();
//    qDebug() << "port: " << currentSettings.portName;

    // 波特率
    currentSettings.baudRate = ui->baudRateBox->currentData().toUInt();
    currentSettings.strBaudRate = QString::number(currentSettings.baudRate);
//    qDebug() << "baudRate " << currentSettings.baudRate;

    // 数据位
    currentSettings.dataBits = ui->dataBitsBox->currentData().value<QSerialPort::DataBits>();
    currentSettings.strDataBits = ui->dataBitsBox->currentText();

    // 校验位
    currentSettings.parity = ui->parityBox->currentData().value<QSerialPort::Parity>();
    currentSettings.strParity = ui->parityBox->currentText();

    // 停止位
    currentSettings.stopBits = ui->stopBitsBox->currentData().value<QSerialPort::StopBits>();
    currentSettings.strStopBits = ui->stopBitsBox->currentText();

    // 硬件流控制
    currentSettings.flowControl = ui->flowControlBox->currentData().value<QSerialPort::FlowControl>();
    currentSettings.strFlowControl = ui->flowControlBox->currentText();

    // 回显设置
    currentSettings.localEchoEnable = ui->localEchoCheckBox->isChecked();
    currentSettings.lineFeed = ui->sendEnterCheckBox->isChecked();

    // 字体大小
    currentSettings.fontSize = ui->fontSizeBox->currentData().value<int>();
}
