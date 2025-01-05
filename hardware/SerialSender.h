#ifndef SERIALSENDER_H
#define SERIALSENDER_H

#include <QObject>
#include <QStringList>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

class SerialSender : public QObject
{
    Q_OBJECT

public:
    explicit SerialSender(QObject *parent = nullptr);
    ~SerialSender();

    bool openPort(const QString &portName, int baudRate);
    void closePort();
    bool isPortOpen() const;
    bool sendData(const QByteArray &data);

    QStringList availablePorts() const;

    void fanOn();
    void fanOff();
    void cleanOn();
    void cleanOff();

signals:
    void errorOccurred(const QString &message);

private:
    boost::asio::io_service ioService;
    boost::asio::serial_port* serialPort;
    bool portOpen;
};

#endif // SERIALSENDER_H
