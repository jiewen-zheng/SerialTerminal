//
// Created by moon on 2023/9/16.
//

#include "ymodem.h"
#include <string.h>
#include <stdlib.h>

// CRC16 表
static const uint16_t crc16Table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};


YModem::YModem(uint32_t timeInterval, uint32_t timeMax, uint32_t errorMax) {
    counter = {
        .timeInterval = timeInterval,
        .timeMax      = timeMax,
        .errorMax     = errorMax,
        .timeCount    = 0,
        .errorCount   = 0,
        .dataCount    = 0
    };

    stage = StageNone;
    status = StatusEstablish;
    progress = 0;
}

YModem::~YModem() {

}

void YModem::transmit() {
    switch (stage) {
    case StageNone:
        transmitStageNone();
        break;

    case StageEstablishing:
        transmitStageEstablishing();
        break;

    case StageEstablished:
        transmitStageEstablished();
        break;

    case StageTransmitting:
        transmitStageTransmitting();
        break;

    case StageFinishing:
        transmitStageFinishing();
        break;

    case StageFinished:
        transmitStageFinished();
        break;
    }
}

void YModem::receive() {
    switch (stage) {
    case StageNone:
        receiveStageNone();
        break;

    case StageEstablishing:
        receiveStageEstablishing();
        break;

    case StageEstablished:
        receiveStageEstablished();
        break;

    case StageTransmitting:
        receiveStageTransmitting();
        break;

    case StageFinishing:
        receiveStageFinishing();
        break;

    default:
        receiveStageFinished();
    }
}


void YModem::abort(bool notify) {
    counter.timeCount  = 0;
    counter.errorCount = 0;
    counter.dataCount  = 0;
    code  = CodeNone;
    stage = StageNone;

    if (!notify)
        return;

    for (txLength = 0; txLength < YMODEM_CODE_CAN_COUNT; txLength++) {
        txBuffer[txLength] = CodeCan;
    }

    write(txBuffer, txLength);
}


int YModem::getProgress() {
    return progress;
}


YModem::Status YModem::getStatus()
{
    return status;
}


YModem::Code YModem::receivePacket() {
    if (code == CodeNone) {
        if (read(&rxBuffer[0], 1) <= 0)
            return CodeNone;

        return checkPacket((Code) rxBuffer[0]);
    }

    return receiveLeftPacket();
}

YModem::Code YModem::checkPacket(Code _code) {
    uint32_t size, len;

    if (_code == CodeSoh) {
        size = YMODEM_PACKET_SIZE + YMODEM_PACKET_OVERHEAD - 1;
        len  = read(&rxBuffer[1], size);
    } else if (_code == CodeStx) {
        size = YMODEM_PACKET_1K_SIZE + YMODEM_PACKET_OVERHEAD - 1;
        len  = read(&rxBuffer[1], size);
    } else {
        code = CodeNone;
        return _code;
    }

    if (len < size) {
        rxLength = len + 1;
        code     = _code;
        return CodeNone;
    }

    return _code;
}

YModem::Code YModem::receiveLeftPacket() {
    uint32_t size, len;

    if (code == CodeSoh) {
        size = YMODEM_PACKET_SIZE + YMODEM_PACKET_OVERHEAD - rxLength;
        len  = read(&rxBuffer[rxLength], size);
    } else if (code == CodeStx) {
        size = YMODEM_PACKET_1K_SIZE + YMODEM_PACKET_OVERHEAD - rxLength;
        len  = read(&rxBuffer[rxLength], size);
    } else {
        code = CodeNone;
        return CodeNone;
    }

    if (len < size) {
        rxLength += len;
        return CodeNone;
    }

    auto _code = code;
    code = CodeNone;
    return _code;
}

void YModem::clearCounter() {
    counter.timeCount  = 0;
    counter.errorCount = 0;
    code = CodeNone;
}

bool YModem::transmitFileOperation(YModem::Status _status) {
    status = _status;
    uint16_t crc;

    switch (status) {
    case StatusEstablish:
        //打开文件
        if (fileOperation(FileOpenReadOnly, nullptr, 0)) {
            auto nameLength = fileOperation(FileGetName, &txBuffer[YMODEM_PACKET_HEAD], 0);
            fileSize  = fileOperation(FileGetSize, &txBuffer[YMODEM_PACKET_HEAD + nameLength + 1], 0);
            fileCount = 0;

            crc = crc16(&txBuffer[YMODEM_PACKET_HEAD], YMODEM_PACKET_SIZE);
            txBuffer[0]                                               = CodeSoh;
            txBuffer[1]                                               = 0x00;
            txBuffer[2]                                               = 0xFF;
            txBuffer[YMODEM_PACKET_SIZE + YMODEM_PACKET_OVERHEAD - 2] = (uint8_t) (crc >> 8);
            txBuffer[YMODEM_PACKET_SIZE + YMODEM_PACKET_OVERHEAD - 1] = (uint8_t) (crc & 0xFF);
            txLength = YMODEM_PACKET_SIZE + YMODEM_PACKET_OVERHEAD;

            return true;
        }
        status = StatusError;
        return false;

    case StatusTransmit:
        if (fileCount >= fileSize) {
            return false;
        }

        if (fileSize - fileCount > YMODEM_PACKET_SIZE) {
            fileCount += fileOperation(FileRead, &txBuffer[YMODEM_PACKET_HEAD],
                                       YMODEM_PACKET_1K_SIZE);
            txLength = YMODEM_PACKET_1K_SIZE;
        } else {
            fileCount += fileOperation(FileRead, &txBuffer[YMODEM_PACKET_HEAD],
                                       YMODEM_PACKET_SIZE);
            txLength = YMODEM_PACKET_SIZE;
        }

        counter.dataCount += 1;
        progress = (int) (fileCount * 100 / fileSize);
        crc      = crc16(&txBuffer[YMODEM_PACKET_HEAD], txLength);

        txBuffer[0]                                     = txLength > YMODEM_PACKET_SIZE ? CodeStx : CodeSoh;
        txBuffer[1]                                     = counter.dataCount;
        txBuffer[2]                                     = 0xFF - counter.dataCount;
        txBuffer[txLength + YMODEM_PACKET_OVERHEAD - 2] = (uint8_t) (crc >> 8);
        txBuffer[txLength + YMODEM_PACKET_OVERHEAD - 1] = (uint8_t) (crc & 0xFF);
        txLength = txLength + YMODEM_PACKET_OVERHEAD;
        return true;

    default:
        fileOperation(FileClose, nullptr, 0);
        break;
    }

    return false;
}

void YModem::transmitStageNone() {
    fileSize = 0;
    fileCount = 0;

    counter.timeCount  = 0;
    counter.errorCount = 0;
    counter.dataCount  = 0;
    code  = CodeNone;
    stage = StageEstablishing;

    status = StatusEstablish;

    txLength = 0;

    YMODEM_LOG("YModem ready");
}

void YModem::transmitStageEstablishing() {
    switch (receivePacket()) {
    case CodeC:
        memset(&txBuffer[YMODEM_PACKET_HEAD], 0, YMODEM_PACKET_SIZE);
        abort(false);   //清除计数

        YMODEM_LOG("establishing");
        //获取文件名和大小填充txBuffer,并赋值txLength = YMODEM_PACKET_SIZE;
        if (transmitFileOperation(StatusEstablish)) {
            stage = StageEstablished;
            write(txBuffer, txLength);
        } else { //文件打开失败取消本次传输
            abort();
        }
        break;

    case CodeCan:
        abort();
        transmitFileOperation(StatusAbort);
        break;

    default:
        counter.timeCount++;
        if ((counter.timeCount / counter.timeInterval) > counter.timeMax) {
            abort();
            transmitFileOperation(StatusTimeout);
        }
        break;
    }
}

void YModem::transmitStageEstablished() {
    switch (receivePacket()) {
    case CodeAck:
        memset(&txBuffer[YMODEM_PACKET_HEAD], 0, YMODEM_PACKET_1K_SIZE);
        clearCounter();
        stage = StageEstablished;
        if (!transmitFileOperation(StatusTransmit)) {
            //文件无数据读出发送“Eot”准备结束本次传输
            counter.dataCount = 2;
            txBuffer[0] = CodeEot;
            txLength = 1;
            write(txBuffer, txLength);
        }
        break;

    case CodeNak:
        errorCount();
        break;

    case CodeC:
        counter.errorCount++;

        if (counter.errorCount > counter.errorMax) {
            abort();
            transmitFileOperation(StatusError);
        } else {
            clearCounter();
            //未收到起始包的ack，datacount = 0了，继续发送起始包
            stage = (Stage) (stage + counter.dataCount);
            write(txBuffer, txLength);
        }
        break;

    case CodeCan:
        abort(false);
        transmitFileOperation(StatusAbort);
        break;

    default:
        timeOutCount();
        break;
    }
}

void YModem::transmitStageTransmitting() {
    switch (receivePacket()) {
    case CodeAck:
        memset(&txBuffer[YMODEM_PACKET_HEAD], 0, YMODEM_PACKET_1K_SIZE);
        clearCounter();
        stage = StageTransmitting;
        if (!transmitFileOperation(StatusTransmit)) {
            stage = StageFinishing;
            txBuffer[0] = CodeEot;
            txLength = 1;
            write(txBuffer, txLength);
        } else {
            write(txBuffer, txLength);
        }
        break;

    case CodeNak:
        errorCount();
        break;

    case CodeCan:
        abort(false);
        transmitFileOperation(StatusAbort);
        break;

    default:
        timeOutCount();
        break;
    }
}


void YModem::transmitStageFinishing() {
    uint16_t crc;

    switch (receivePacket()) {
    case CodeNak:
        clearCounter();
        stage = StageFinishing;
        txBuffer[0] = CodeEot;
        txLength = 1;
        write(txBuffer, txLength);
        break;

    case CodeAck:
        write(txBuffer, txLength);
        break;

    case CodeC:
        memset(&txBuffer[YMODEM_PACKET_HEAD], 0, YMODEM_PACKET_SIZE);
        clearCounter();
        stage = StageFinished;
        crc   = crc16(&(txBuffer[YMODEM_PACKET_HEAD]), YMODEM_PACKET_SIZE);
        txBuffer[0]                                               = CodeSoh;
        txBuffer[1]                                               = 0x00;
        txBuffer[2]                                               = 0xFF;
        txBuffer[YMODEM_PACKET_SIZE + YMODEM_PACKET_OVERHEAD - 2] = (uint8_t) (crc >> 8);
        txBuffer[YMODEM_PACKET_SIZE + YMODEM_PACKET_OVERHEAD - 1] = (uint8_t) (crc & 0xFF);
        txLength = YMODEM_PACKET_SIZE + YMODEM_PACKET_OVERHEAD;
        write(txBuffer, txLength);
        break;

    case CodeCan:
        abort(false);
        transmitFileOperation(StatusAbort);
        break;

    default:
        timeOutCount();
        break;
    }
}

void YModem::transmitStageFinished() {
    switch (receivePacket()) {
    case CodeAck:
        abort(false);
        transmitFileOperation(StatusFinish);
        break;

    case CodeNak:
    case CodeC:
        errorCount();
        break;

    case CodeCan:
        abort(false);
        transmitFileOperation(StatusAbort);
        break;

    default:
        timeOutCount();
        break;
    }
}


bool YModem::receiveFileOperation(YModem::Status _status, uint32_t size) {
    status = _status;

    switch(_status){
    case StatusEstablish:{
        if(rxBuffer[YMODEM_PACKET_HEAD] == 0){
            // 无文件名
            status = StatusError;
            return false;
        }

        uint32_t nameLength = strlen((char *)&rxBuffer[YMODEM_PACKET_HEAD]);

        fileName.clear();
        fileName = (char *)&rxBuffer[YMODEM_PACKET_HEAD];
        fileSize = strtoul((char *)&rxBuffer[YMODEM_PACKET_HEAD + nameLength + 1], nullptr, 10);
        fileCount = 0;
        fileOperation(FileSetName, (uint8_t *)fileName.c_str(), fileName.size());
        fileOperation(FileSetSize, nullptr, fileSize);

        if(!fileOperation(FileOpenWriteOnly, &rxBuffer[YMODEM_PACKET_HEAD], nameLength)){
            // 文件创建失败
            status = StatusError;
            return false;
        }

        counter.dataCount = 0;
        return true;
    }

    case StatusTransmit:
        if(fileCount >= fileSize)
            return false;

        if(fileSize - fileCount > size){
            fileOperation(FileWrite, &rxBuffer[YMODEM_PACKET_HEAD], size);
            fileCount += size;
        }else{
            fileOperation(FileWrite, &rxBuffer[YMODEM_PACKET_HEAD], fileSize - fileCount);
            fileCount += fileSize - fileCount;
        }

        counter.dataCount += 1;
        progress = (int)(fileCount * 100 / fileSize);
        return true;

    default:
        fileOperation(FileClose, nullptr, 0);
        break;
    }

    return false;
}

void YModem::receiveStageNone() {
    fileSize = 0;
    fileCount = 0;

    counter.timeCount  = 0;
    counter.errorCount = 0;
    counter.dataCount  = 0;
    code  = CodeNone;

    stage = StageEstablishing;
    rxLength = 0;

    txBuffer[0] = CodeC;
    txLength = 1;
    write(txBuffer, txLength);
}

void YModem::receiveStageEstablishing() {
    uint16_t crc;

    switch (receivePacket()) {
    case CodeSoh:
        crc = ((uint16_t) rxBuffer[YMODEM_PACKET_SIZE + YMODEM_PACKET_OVERHEAD - 2] << 8) |
              ((uint16_t) rxBuffer[YMODEM_PACKET_SIZE + YMODEM_PACKET_OVERHEAD - 1]);

        if (rxBuffer[1] != 0x00 || rxBuffer[2] != 0xFF ||
            crc != crc16(&rxBuffer[YMODEM_PACKET_HEAD], YMODEM_PACKET_SIZE)) {
            txBuffer[0] = CodeC;
            txLength = 1;
            errorCount();
            break;
        }

        if (receiveFileOperation(StatusEstablish)) {
            abort(false); // 清除标志
            stage = StageEstablished;
            txBuffer[0] = CodeAck;
            txBuffer[1] = CodeC;
            txLength = 2;
            write(txBuffer, txLength);
        }else{
            abort();
        }
        break;

    case CodeCan:
        abort(false);
        transmitFileOperation(StatusAbort);
        break;

    default:
        timeOutCount();
        break;
    }
}

void YModem::receiveStageEstablished() {
    uint16_t crc;

    switch (receivePacket()) {
    case CodeSoh:
        storePacket(CodeSoh);
        break;

    case CodeStx:
        storePacket(CodeStx);
        break;

    case CodeEot:
        clearCounter();
        stage = StageFinishing;
        txBuffer[0] = CodeNak;
        txLength = 1;
        write(txBuffer, txLength);
        break;

    case CodeCan:
        abort(false);
        receiveFileOperation(StatusAbort);
        break;

    default:
        txBuffer[0] = CodeNak;
        txLength = 1;
        timeOutCount();
        break;
    }
}

void YModem::receiveStageTransmitting() {
    uint16_t crc;

    switch(receivePacket()){
    case CodeSoh:
        storePacket(CodeSoh);
        break;

    case CodeStx:
        storePacket(CodeStx);
        break;

    case CodeEot:
        clearCounter();
        stage = StageFinishing;
        txBuffer[0] = CodeNak;
        txLength = 1;
        write(txBuffer, txLength);
        break;

    case CodeCan:
        abort(false);
        receiveFileOperation(StatusAbort);
        break;

    default:
        txBuffer[0] = CodeNak;
        txLength = 1;
        timeOutCount();
        break;
    }
}

void YModem::receiveStageFinishing() {
    switch(receivePacket()){
    case CodeEot:
        clearCounter();
        stage = StageFinished;
        txBuffer[0] = CodeAck;
        txBuffer[1] = CodeC;
        txLength = 2;
        write(txBuffer, txLength);
        break;

    case CodeCan:
        abort(false);
        receiveFileOperation(StatusAbort);
        break;

    default:
        timeOutCount();
        break;
    }
}

void YModem::receiveStageFinished() {
    switch(receivePacket()){
    case CodeSoh:{
        uint16_t crc = ((uint16_t) rxBuffer[YMODEM_PACKET_SIZE + YMODEM_PACKET_OVERHEAD - 2] << 8) |
              ((uint16_t) rxBuffer[YMODEM_PACKET_SIZE + YMODEM_PACKET_OVERHEAD - 1]);

        if (rxBuffer[1] != 0x00 || rxBuffer[2] != 0xFF ||
            crc != crc16(&rxBuffer[YMODEM_PACKET_HEAD], YMODEM_PACKET_SIZE)) {
            txBuffer[0] = CodeNak;
            txLength = 1;
            errorCount();
            break;
        }
        txBuffer[0] = CodeAck;
        txLength = 1;
        write(txBuffer, txLength);

        abort(false);
        transmitFileOperation(StatusFinish);
        break;
    }

    case CodeEot:
        errorCount();
        break;

    case CodeCan:
        abort(false);
        receiveFileOperation(StatusAbort);
        break;

    default:
        timeOutCount();
        break;
    }
}

void YModem::storePacket(Code _code)
{
    uint32_t dataLength = _code == CodeSoh ? YMODEM_PACKET_SIZE : YMODEM_PACKET_1K_SIZE;

    uint16_t crc = ((uint16_t) rxBuffer[dataLength + YMODEM_PACKET_OVERHEAD - 2] << 8) |
                   ((uint16_t) rxBuffer[dataLength + YMODEM_PACKET_OVERHEAD - 1]);

    // 发送端未接收到上一包反馈
    if(rxBuffer[1] == counter.dataCount && (rxBuffer[2] == 0xFF - counter.dataCount)){
        errorCount();   //继续发送ack
        return ;
    }

    if(rxBuffer[1] != counter.dataCount + 1 || (rxBuffer[2] != 0xFE - counter.dataCount) ||
        crc != crc16(&rxBuffer[YMODEM_PACKET_HEAD], dataLength)){
        txBuffer[0] = CodeNak;
        txLength = 1;
        errorCount();
        return ;
    }

    clearCounter();
    if(receiveFileOperation(StatusTransmit, dataLength)){
        stage = StageTransmitting;
        txBuffer[0] = CodeAck;
        txLength = 1;
        write(txBuffer, txLength);
    }else{
        abort();
    }
}


uint16_t YModem::crc16(uint8_t *data, uint32_t len) {
    uint16_t    crc = 0xFFFF; // 初始值
    for (size_t i   = 0; i < len; ++i) {
        crc = (crc << 8) ^ crc16Table[((crc >> 8) ^ data[i]) & 0xFF];
    }
    return crc;
}

uint16_t YModem::calculateCRC16(uint8_t *data, uint32_t len) {
    uint16_t crc = 0;

    while (len--) {
        crc ^= (uint16_t) (*(data++)) << 8;

        for (int i = 0; i < 8; i++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc = crc << 1;
            }
        }
    }

    return crc;
}

void YModem::errorCount() {
    counter.errorCount++;

    if (counter.errorCount > counter.errorMax) {
        abort();
        transmitFileOperation(StatusError);
    } else {
        write(txBuffer, txLength);
    }
}

void YModem::timeOutCount() {
    counter.timeCount++;
    if ((counter.timeCount / counter.timeInterval) > counter.timeMax) {
        abort();
        transmitFileOperation(StatusTimeout);
    }else if (counter.timeCount % counter.timeInterval == 0) {
        write(txBuffer, txLength);
    }
}


