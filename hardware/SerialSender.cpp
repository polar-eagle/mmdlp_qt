
#include "SerialSender.h"
#include <QDebug>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

SerialSender::SerialSender(QObject *parent)
    : QObject(parent), serialPort(nullptr), portOpen(false)
{
}

SerialSender::~SerialSender()
{
    closePort();
}
/**
 * @brief 打开串口
 * @param portName 端口名称
 * @param baudRate 波特率
 * @return bool 是否打开成功
 */
bool SerialSender::openPort(const QString &portName, int baudRate)
{
    if (portOpen) {
        closePort();
    }

    try {
        serialPort = new boost::asio::serial_port(ioService);
        serialPort->open(portName.toStdString());
        serialPort->set_option(boost::asio::serial_port_base::baud_rate(baudRate));
        serialPort->set_option(boost::asio::serial_port_base::character_size(8));
        serialPort->set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
        serialPort->set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
        serialPort->set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));

        portOpen = true;
        qDebug() << "Serial port opened successfully: " << portName;
    } catch (const boost::system::system_error &e) {
        emit errorOccurred("Failed to open port: " + portName);
        return false;
    }

    return true;
}
/**
 * @brief 关闭串口
 * @return void
 */
void SerialSender::closePort()
{
    if (portOpen && serialPort) {
        try {
            serialPort->close();
            delete serialPort;
            serialPort = nullptr;
            portOpen = false;
            qDebug() << "Port closed.";
        } catch (const boost::system::system_error &e) {
            emit errorOccurred("Failed to close port.");
        }
    }
}
/**
 * @brief 判断串口是否打开
 * @return bool 是否打开
 */
bool SerialSender::isPortOpen() const
{
    return portOpen;
}
/**
 * @brief 串口发送数据
 * @param data 数据
 * @return bool 是否发送成功
 */
bool SerialSender::sendData(const QByteArray &data)
{
    if (!portOpen) {
        emit errorOccurred("Serial port is not open.");
        return false;
    }

    try {
        boost::asio::write(*serialPort, boost::asio::buffer(data.constData(), data.size()));
        qDebug() << "Data sent:" << data;
        
        char response = '\0';
        while (response != 'D') {
            boost::asio::read(*serialPort, boost::asio::buffer(&response, 1));
        }
        qDebug() << "Received response:" << response;
        
    } catch (const boost::system::system_error &e) {
        emit errorOccurred("Failed to send data.");
        return false;
    }

    return true;
}
/**
 * @brief 获取可用串口列表
 * @return QStringList 串口列表
 */
QStringList SerialSender::availablePorts() const
{
    QStringList portList;
    boost::asio::io_service io_service;
    boost::asio::serial_port_base::baud_rate baud_option(9600);
    boost::asio::serial_port_base::stop_bits stop_bits_option(boost::asio::serial_port_base::stop_bits::one);

    for (int i = 1; i <= 256; i++) {
        QString portName = QString("COM%1").arg(i);
        try {
            boost::asio::serial_port serial(io_service);
            serial.open(portName.toStdString());
            portList.append(portName);
            serial.close();
        } catch (const boost::system::system_error &e) {
            // Ignore errors, meaning the port is not available
        }
    }

    return portList;
}
