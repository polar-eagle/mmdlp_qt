#ifndef PRINTTHREAD_H
#define PRINTTHREAD_H

#include <QThread>
#include <QMutex>
#include <QVector>

class PrintThread : public QThread
{
    Q_OBJECT

public:
    explicit PrintThread(QObject *parent = nullptr);
    void setSliceName(const QString &name);
    bool isPrinting = false;

    QVector<QString> commandList;
    int commandIndex = -1;
    QMutex mutex;
    bool stopRequested;
    QMutex stopMutex;

protected:
    void run() override;

private:
    QString sliceName;

public slots:
    void sendCommand();

signals:
    void transitCommand(const QString &command);
    void transSliceName(const QString &sliceName);
    void finished();
};

#endif // PRINTTHREAD_H
