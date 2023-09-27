#include "transferdialog.h"
#include "qevent.h"
#include "ui_transferdialog.h"

#include <QDebug>
#include <QMimeData>
#include <QMessageBox>

#include <QDir>

TransferDialog::TransferDialog(QWidget *parent, QSerialPort *serial) :
    QDialog(parent),
    m_serial(serial),
    m_transferThread(new TransferThread()),
    ui(new Ui::TransferDialog)
{
    ui->setupUi(this);

    ui->StartPushButton->setEnabled(true);
    ui->CancelPushButton->setEnabled(false);
    setAcceptDrops(true);

    initActionsConnections();

    // 窗口关闭信号
    connect(this, &QDialog::finished, this, &TransferDialog::cancel);
    // 传输结束信号
    connect(m_transferThread, &QThread::finished, this, &TransferDialog::over);
    // 显示传输结果
    connect(m_transferThread, &TransferThread::transferOver,
            this, &TransferDialog::handleTransferOver);
    // 进度条更新信号
    connect(m_transferThread, &TransferThread::displayProgress,
            this, &TransferDialog::showProgress);

    fillProtocol();
}

TransferDialog::~TransferDialog()
{
    qDebug() << "TransferDialog delete";

    if(m_transferThread->isRunning()){
        m_transferThread->cancellation();
        m_transferThread->quit();
        m_transferThread->wait();
    }
    delete m_transferThread;
    delete ui;
}


bool TransferDialog::isTransfer()
{
    return transferState;
}

void TransferDialog::transfer(TransferDirection dir, const QString fileName)
{
    // 设置方向
    if(dir == Transmit){
        m_transferThread->setDirection(TransferThread::Transmit);
        //检查发送文件
        QFileInfo info(fileName);
        if(!info.isFile()){
            handleTransferOver(QString("找不到路径“%1”,因为该路径不存在。")
                                   .arg(fileName));
            return;
        }
        m_transferThread->setFileName(fileName);
    }else{
        m_transferThread->setDirection(TransferThread::Receive);
        //检查接收路径
        if(fileName.isEmpty() || !QDir(fileName).exists()){
            handleTransferOver(QString("找不到路径“%1”,因为该路径不存在。")
                                   .arg(fileName));
            return;
        }
        m_transferThread->setFileName(fileName);
    }

    if(m_serial->isOpen()){
        mainSerialOpen = true;
        m_serial->close();
    }

    ui->StartPushButton->setEnabled(false);
    ui->CancelPushButton->setEnabled(true);
    transferState = true;

    m_transferThread->setProtocol(TransferThread::Protocol(ui->protocolBox->currentIndex()));
    m_transferThread->setSerialInfo(serialSettings);
    m_transferThread->transaction();
}

void TransferDialog::setSerialParameters(const SettingsDialog::Settings &setting)
{
    serialSettings = setting;
}

void TransferDialog::start()
{
    if(m_transferThread->isRunning())
        return;

    QString path;
    if(ui->sendCheckBox->isChecked()){
        direction = Transmit;
        path = fileInfo.fileUrl.isEmpty() ? ui->pathLineEdit->text() :
                   fileInfo.fileUrl;
    }else{
        direction = Receive;
        path = ui->pathLineEdit->text();
    }

    transfer(direction, path);
}


void TransferDialog::cancel()
{
    qDebug() << "cancel file transfer";

    if(m_transferThread->isRunning()){
        m_transferThread->cancellation();
        m_transferThread->quit();
        m_transferThread->wait();
    }

    ui->CancelPushButton->setEnabled(false);
    ui->StartPushButton->setEnabled(true);
    transferState = false;

    // 重新打开主窗口的串口
    if(mainSerialOpen){
        mainSerialOpen = false;
        m_serial->open(QIODevice::ReadWrite);
    }

    showProgress(0);
}

void TransferDialog::over()
{
    qDebug() << "over file transfer";

    ui->CancelPushButton->setEnabled(false);
    ui->StartPushButton->setEnabled(true);
    transferState = false;

    // 重新打开主窗口的串口
    if(mainSerialOpen){
        mainSerialOpen = false;
        m_serial->open(QIODevice::ReadWrite);
    }
}


void TransferDialog::handleTransferOver(const QString result, bool successed)
{
    if(result.isEmpty())
        return;

    if(successed)
        emit putMessage(result, 1);
    else
        emit putMessage(result, 2);

    QMessageBox::about(this, "Message", result);

    cancel();
}

void TransferDialog::showProgress(int pro)
{
    ui->progressBar->setValue(pro);
}

void TransferDialog::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasUrls()){
        qDebug() << event->mimeData()->urls();
        event->acceptProposedAction();
    }else{
        event->ignore();
    }
}

void TransferDialog::dropEvent(QDropEvent *event)
{
    const QMimeData* mimeData = event->mimeData();

    if(mimeData->hasUrls()){
        QList<QUrl> urls = mimeData->urls();
        //获取第一个匹配的文件路径
        QString fileName = urls[0].toLocalFile();
        if(fileName.isEmpty()){
            return;
        }

        if(updateMessage(fileName))
            showFileMessage(&fileInfo);

    }else{
        event->ignore();
    }
}

void TransferDialog::initActionsConnections()
{
    connect(ui->StartPushButton, &QPushButton::clicked, this, &TransferDialog::start);
    connect(ui->CancelPushButton, &QPushButton::clicked, this, &TransferDialog::cancel);
}

void TransferDialog::fillProtocol()
{
    // QStringLiteral 在编译时生成为静态
    ui->protocolBox->addItem(QStringLiteral("YModem"));
    ui->protocolBox->setCurrentIndex(0);
}

void TransferDialog::showFileMessage(FileInfomation* info)
{
    if(info == nullptr){
        ui->nameLabel->setText(QString("名称："));
        ui->pathLineEdit->clear();
        ui->sizeLabel->setText(QString("大小："));
        ui->transferCountLabel->setText(QString("数据包："));
        return;
    }

    ui->nameLabel->setText(QString("名称：%1").arg(info->name));
    ui->pathLineEdit->setText(info->path);

    if(info->size / 1024)
        ui->sizeLabel->setText(QString("大小：%1 KB").arg(info->size / 1024));
    else
        ui->sizeLabel->setText(QString("大小：%1 Byte").arg(info->size));

    ui->transferCountLabel->setText(QString("数据包：%1").arg(info->packetSize));
}

bool TransferDialog::updateMessage(QString fileName)
{
    QFile file(fileName);

    //判断文件是否存在
    if(!file.exists()){
        qDebug() << "File does not exist.";
        return false;
    }

    QFileInfo info(file);

    fileInfo.fileUrl = fileName;
    fileInfo.name = info.fileName();
    fileInfo.path = info.path();
    fileInfo.size = info.size();

    if(fileInfo.size == 0){
        fileInfo.packetSize = 0;
    }else{
        size_t leftSize = fileInfo.size % 1024;
        fileInfo.packetSize = fileInfo.size / 1024;
        if(leftSize > 0)
            fileInfo.packetSize += 1;
    }

    return true;
}



