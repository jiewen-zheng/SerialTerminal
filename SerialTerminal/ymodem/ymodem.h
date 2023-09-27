//
// Created by moon on 2023/9/16.
//

#ifndef YMODEM_YMODEM_H
#define YMODEM_YMODEM_H

/* Header includes -----------------------------------------------------------*/
#include <stdint.h>
#include <QDebug>

/* Macro definitions ---------------------------------------------------------*/
#define YMODEM_PACKET_HEAD      (3)
#define YMODEM_PACKET_TRAIL     (2)
#define YMODEM_PACKET_OVERHEAD  (YMODEM_PACKET_HEAD + YMODEM_PACKET_TRAIL)
#define YMODEM_PACKET_SIZE      (128)
#define YMODEM_PACKET_1K_SIZE   (1024)

#define YMODEM_CODE_CAN_COUNT   (5)

#define YMODEM_LOG(format, ...) qDebug(format, ##__VA_ARGS__)

class YModem {
public:
    enum Code : uint8_t {
        CodeNone = 0x00,
        CodeSoh  = 0x01,
        CodeStx  = 0x02,
        CodeEot  = 0x04,
        CodeAck  = 0x06,
        CodeNak  = 0x15,
        CodeCan  = 0x18,
        CodeC    = 0x43,
    };

    enum Stage {
        StageNone,          //待机
        StageEstablishing,  //建立握手
        StageEstablished,   //建立完成
        StageTransmitting,  //传输中
        StageFinishing,     //结束通信
        StageFinished       //完成
    };

    enum Status {
        StatusEstablish,
        StatusTransmit,
        StatusFinish,
        StatusAbort,
        StatusTimeout,
        StatusError
    };

    enum OperationType {
        FileOpenReadOnly,
        FileOpenWriteOnly,
        FileOpenReadWrite,
        FileClose,
        FileGetName,
        FileSetName,
        FileGetSize,
        FileSetSize,
        FileWrite,
        FileRead
    };

    struct Counter {
        uint32_t timeInterval;  //重试间隔时间
        uint32_t timeMax;       //超时时间
        uint32_t errorMax;      //最大传输错误次数

        //当前计数
        uint32_t timeCount;
        uint32_t errorCount;
        uint32_t dataCount;
    };


public:
    YModem(uint32_t timeInterval = 200, uint32_t timeMax = 5, uint32_t errorMax = 10);
    ~YModem();

    void transmit();
    void receive();
    void abort(bool notify = true);

    int getProgress();
    Status getStatus();

protected:
    virtual uint32_t write(uint8_t *data, uint32_t len) = 0;
    virtual uint32_t read(uint8_t *data, uint32_t len) = 0;
    virtual uint32_t fileOperation(OperationType type, uint8_t *data, uint32_t len) = 0;

private:
    Code receivePacket();
    Code checkPacket(Code _code);
    Code receiveLeftPacket();

    void clearCounter();
    void errorCount();
    void timeOutCount();


    bool transmitFileOperation(Status _status);
    void transmitStageNone();
    void transmitStageEstablishing();
    void transmitStageEstablished();
    void transmitStageTransmitting();
    void transmitStageFinishing();
    void transmitStageFinished();

    bool receiveFileOperation(Status _status, uint32_t size = 0);
    void receiveStageNone();
    void receiveStageEstablishing();
    void receiveStageEstablished();
    void receiveStageTransmitting();
    void receiveStageFinishing();
    void receiveStageFinished();
    void storePacket(Code _code);

    uint16_t crc16(uint8_t *data, uint32_t len);
    uint16_t calculateCRC16(uint8_t *data, uint32_t len);

private:
    Code    code;
    Stage   stage;
    Counter counter;
    Status  status;
    int     progress;

    std::string fileName;
    size_t  fileSize;
    size_t  fileCount;

    uint8_t  txBuffer[YMODEM_PACKET_1K_SIZE + YMODEM_PACKET_OVERHEAD];
    uint8_t  rxBuffer[YMODEM_PACKET_1K_SIZE + YMODEM_PACKET_OVERHEAD];
    uint32_t txLength = 0;
    uint32_t rxLength = 0;
};


#endif //YMODEM_YMODEM_H
