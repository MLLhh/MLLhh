#include "utils.h"
#include <iostream>
#include <bitset>
#include <QDateTime>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

Utils::Utils()
{

}
void Utils::printBinary(QByteArray &data)
{
    QMutexLocker locker(&mutex);
    for (int i=0; i<data.size(); ++i) {
        char byte = static_cast<char>(data.at(i));
        std::bitset<8> bits(byte);
        std::cout << bits << ' ';
    }
    std::cout << std::endl;
}
void Utils::printBinary(const char *data, int size)
{
    QMutexLocker locker(&mutex);
    for (int i = 0; i < size; i++) {
        u_char byte = data[i];
        for (int bit = 7; bit >= 0; bit--) {
            std::cout << ((byte >> bit) & 1);
        }
        std::cout << " ";
    }
    std::cout << std::endl;
}
void Utils::printHex(const char *data, int size)
{
    for (int i = 0; i < size; i++) {
        char byte = data[i];
        std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
    }
    std::cout << std::resetiosflags(std::ios::basefield | std::ios::uppercase) << std::endl;
}
void Utils::printHex(const uchar *data, int size)
{
    for (int i = 0; i < size; i++) {
        uchar byte = data[i];
        std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
    }
    std::cout << std::resetiosflags(std::ios::basefield | std::ios::uppercase) << std::endl;
}
void Utils::printIP(const char *data, int size)
{
    for (int i = 0; i < size; i++) {
        char byte = data[i];
        std::cout << std::dec << static_cast<int>(byte);
        if (i < size-1) {
            std::cout << ".";
        }
    }
    std::cout << std::resetiosflags(std::ios::basefield | std::ios::uppercase) << std::endl;
}
void Utils::printOut(const char *data, int size)
{
    QMutexLocker locker(&mutex);
    std::cout << "[SIZE=" << size << "] >>> ";
    printHex(data, size);
}
void Utils::printIn(const char *data, int size)
{
    QMutexLocker locker(&mutex);
    std::cout << "[SIZE=" << size << "] <<< ";
    printHex(data, size);
}
void Utils::printASCII(const char *data, int size)
{
    QMutexLocker locker(&mutex);
    for (int i = 0; i < size; i++) {
        char byte = data[i];
        if (byte >= 32 && byte <= 126) {
            std::cout << byte;
        } else {
            std::cout << '*';
        }
    }
}
long Utils::find_dt_us(struct timespec *time1, struct timespec *time2)
{
    long ds = time2->tv_sec - time1->tv_sec;
    long dns = time2->tv_nsec - time1->tv_nsec;
    long dt_us = ds * 1000000 + dns / 1000;
    return dt_us;
}
QString Utils::getTimeString()
{
    QTime time = QTime::currentTime();
    QString str = time.toString("[hh:mm:ss.zzz]");
    return str;
}
std::string Utils::getStdTimeString()
{
    QTime time = QTime::currentTime();
    QString str = time.toString("[hh:mm:ss.zzz]");
    return str.toStdString();
}
QString Utils::getDateTimeString()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    QString str = dateTime.toString("yyyy-MM-dd hh:mm:ss");
    return str;
}
struct timespec Utils::getCurrentTime()
{
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return time;
}
QString Utils::ucharToBinaryString(const char *data, int length)
{
    QString binaryString;
    for (int i = 0; i < length; i++) {
        u_char byte = data[i];
        for (int j = 7; j >= 0; j--) {
            binaryString.append(((byte >> j) & 1) ? '1' : '0');
        }
        if (i % 5 == 4) {
            binaryString.append("\n");
        }
        else {
            binaryString.append(" ");
        }
    }
    return binaryString;
}
std::string Utils::timevalToString(struct timeval tv)
{
    std::time_t seconds = tv.tv_sec;
    std::tm* timeinfo = std::localtime(&seconds);

    std::ostringstream oss;
    oss << std::put_time(timeinfo, "%Y-%m-%d %H:%M:%S") << ".";
    oss << std::setw(6) << std::setfill('0') << tv.tv_usec;

    return oss.str();
}
QString Utils::timevalToString(long tv_sec, long tv_usec)
{
    QDateTime dateTime;
    dateTime.setTime_t(tv_sec);
    dateTime = dateTime.addMSecs(tv_usec / 1000);
    QString str = dateTime.toString("yyyy-MM-dd hh:mm:ss.zzz");
    long usec = tv_usec % 1000;
    QString padded = QString("%1").arg(usec, 3, 10, QChar('0'));
    str.append(padded);
    return str;
}
const char* Utils::copyData(const char* data, size_t size)
{
    const char* copy = new char[size];
    memcpy(const_cast<char*>(copy), data, size);
    return copy;
}

std::string Utils::charPtrToIPAddress(char* ipAddress)
{
    std::ostringstream oss;
    oss << static_cast<int>(ipAddress[0]) << '.'
        << static_cast<int>(ipAddress[1]) << '.'
        << static_cast<int>(ipAddress[2]) << '.'
        << static_cast<int>(ipAddress[3]);
    return oss.str();
}

void Utils::ipAddressToCharPtr(const std::string &ipString, char *ipAddress)
{
    std::istringstream iss(ipString);
    std::string segment;
    int segmentValue;
    int index = 0;

    while (std::getline(iss, segment, '.'))
    {
        std::istringstream segmentStream(segment);
        segmentStream >> segmentValue;
        ipAddress[index++] = static_cast<char>(segmentValue);
    }
}
void Utils::printDatagram(QNetworkDatagram datagram)
{
    std::cout << "datagram: " << std::endl;
    std::cout << "senderAddress: " << datagram.senderAddress().toString().toStdString() << std::endl;
    std::cout << "senderPort: " << datagram.senderPort() << std::endl;
    std::cout << "destinationAddress: " << datagram.destinationAddress().toString().toStdString() << std::endl;
    std::cout << "destinationPort: " << datagram.destinationPort() << std::endl;
    std::cout << "data: " << datagram.data().toHex(' ').toStdString() << std::endl;
}
