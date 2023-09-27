#include "console.h"


#include <QScrollBar>

#include <QString>
#include <QRegularExpression>


Console::Console(QWidget *parent) :
    QPlainTextEdit(parent)
{
    document()->setMaximumBlockCount(500);
    QPalette p = palette();
    p.setColor(QPalette::Base, Qt::black);
    p.setColor(QPalette::Text, Qt::white);
    setPalette(p);

    QFont font;
    font.setPixelSize(16);
    setFont(font);

    addStartChar();

    historyInputDataList.resize(5);
}

void Console::putEditData(const QByteArray &data)
{
    insertPlainText(data);

    if(data.last(1) == "\n"){
        insertPlainText(">");
    }

    //重新设置滚动条
    QScrollBar *bar = verticalScrollBar();
    bar->setValue(bar->maximum());
}

void Console::setLocaEchoEnable(bool en)
{
    localEchoEnabel = en;
}

void Console::setFontSize(int size)
{
    QFont font;
    font.setPixelSize(size);
    setFont(font);
}

void Console::initEdit()
{
    // 初始化内容
    setPlainText(">");

    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::EndOfBlock);
    setTextCursor(cursor);
}

void Console::showMessage(QString msg, int flag)
{
    QColor color = Qt::white;
    if(MessageFlag(flag) == MessageFlag::Error){
        color = QColor(200, 100, 100);
    }else if(MessageFlag(flag) == MessageFlag::Warning){
        color = QColor(100, 200, 100);
    }

    msg.insert(0, "\n");

    QTextCharFormat textFormat;
    textFormat.setForeground(color);

    QTextCursor cursor = textCursor();
    cursor.setCharFormat(textFormat);

    cursor.insertText(msg, textFormat);
//    cursor.movePosition(QTextCursor::End);
//    setTextCursor(cursor);


    textFormat.setForeground(Qt::white);
    cursor.insertText(" ", textFormat);

    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
}

void Console::keyPressEvent(QKeyEvent *e)
{
    //    if(e->modifiers() == Qt::ControlModifier){
    //        //组合键
    //        if(e->key() == Qt::Key_C){
    //        }
    //    }

    switch(e->key()){
    case Qt::Key_Left:
    case Qt::Key_Right:
        if(checkCursorPos())
            return;
        break;

    case Qt::Key_Up:
        selectHistoryData(&(++historyInputIndex));
        return;

    case Qt::Key_Down:
        selectHistoryData(&(--historyInputIndex));
        return;

    case Qt::Key_Backspace:
        if(!inputData.isEmpty())
            inputData.chop(1);
        if(checkCursorPos())
            return;
        break;

    case Qt::Key_Enter:
    case Qt::Key_Return:
        if(!inputData.isEmpty()){
            emit putData(inputData);
            // 将本次输入添加到列表
            historyInputDataList.prepend(inputData.data());
            historyInputDataList.removeLast();
            inputData.clear();
            historyInputIndex = 0;
        }
        qDebug() <<"input data list " <<  historyInputDataList;
        addStartChar();
        return;

    default:
        inputData.push_back(e->text().toUtf8());
        break;
    }

    if(localEchoEnabel){
        QPlainTextEdit::keyPressEvent(e);
    }
}

void Console::mousePressEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    setFocus();
}

bool Console::checkCursorPos()
{
    QTextCursor cursor = textCursor();
    int cursorPos = cursor.positionInBlock();
    qDebug() << cursorPos;

    return cursorPos <= QString(">").size();

}


void Console::addStartChar()
{
    QTextCursor cursor = textCursor();

    cursor.movePosition(QTextCursor::End);

    if(cursor.positionInBlock() > 0)
        cursor.insertText("\n");

    setTextCursor(cursor);

    insertPlainText(">");
}



void Console::selectHistoryData(int *idx)
{
    int index = *idx;
    QTextCursor cursor = textCursor();

    // 获取当前行的文本
    cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, cursor.positionInBlock() - QString(">").size());
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    // 获取当前行的文本内容
    //    QString currentLineText = cursor.selectedText();
    // 删除位置1到末尾的字符
    //    currentLineText.remove(designatorSize(currentMode), currentLineText.length() - designatorSize(currentMode));

    cursor.removeSelectedText();

    if(index < 1 || index > 5){
        return;
    }
    QString historyData = historyInputDataList[index - 1];
    if(historyData.isEmpty() && index >= 2){
        historyData = historyInputDataList[index - 2];
        *idx -= 1;
    }

    cursor.insertText(historyData);

    inputData = historyData.toUtf8();
}
