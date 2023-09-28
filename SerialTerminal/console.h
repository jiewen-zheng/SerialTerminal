#ifndef CONSOLE_H
#define CONSOLE_H

#include <QPlainTextEdit>


class Console : public QPlainTextEdit
{
    Q_OBJECT

signals:
    void putData(QByteArray &data);

public:
    enum MessageFlag : int{
        Infomation = 0,
        Warning,
        Error,
    };

    explicit Console(QWidget *parent = nullptr);

    void putEditData(const QByteArray &data);
    void setLocaEchoEnable(bool en);
    void setFontSize(int size);

public slots:
    void initEdit();
    void showMessage(QString msg, int flag = Error);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;

private:
    bool checkCursorPos();
    void addStartChar();
    void selectHistoryData(bool upSelect);

private:
    QByteArray inputData = nullptr;
    bool localEchoEnabel = true;

    QStringList historyInputDataList;
    int historyInputIndex = 0;
};

#endif // CONSOLE_H
