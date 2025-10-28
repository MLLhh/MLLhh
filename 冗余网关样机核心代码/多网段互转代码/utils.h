#ifndef UTILS_H
#define UTILS_H

#include <QByteArray>
#include <QMutex>
#include <QNetworkDatagram>

class Utils
{
public:
    Utils();
    static void printBinary(QByteArray &data);
    static void printBinary(const char *data, int length);
    static void printHex(const char *data, int size);
    static void printHex(const uchar *data, int size);
    static void printIP(const char *data, int size);

    static void printOut(const char *data, int size);
    static void printIn(const char *data, int size);
    static void printASCII(const char *data, int length);
    static long find_dt_us(struct timespec *time1, struct timespec *time2);
    static QString getTimeString();
    static std::string getStdTimeString();
    static QString getDateTimeString();
    static struct timespec getCurrentTime();
    static QString ucharToBinaryString(const char *data, int length);
    static std::string timevalToString(struct timeval tv);
    static QString timevalToString(long tv_sec, long tv_usec);
    static const char* copyData(const char* data, size_t size);

    static std::string charPtrToIPAddress(char* ipAddress);
    static void ipAddressToCharPtr(const std::string &ipString, char *ipAddress);
    static void printDatagram(QNetworkDatagram datagram);
};
static QMutex mutex;

#endif // UTILS_H
