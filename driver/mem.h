#ifndef MEM_H
#define MEM_H

#include <vector>
#include <string>
#include <QString>
#include "atlstr.h"
#include "process/process.h"
#include "driver/msdriverpro.h"

std::vector<byte> operator+(std::vector<byte> a, std::vector<byte> b);

namespace Mem{

    bool readMemory(ULONG64 addr, PBYTE pDataBuffer, ULONG size);

    byte readByte(ULONG64 address);

    WORD readWord(ULONG64 address);

    int readInt(ULONG64 address);

    __int64 readLong(ULONG64 address);

    // 可变参数版本 - 支持任意数量偏移
    template<typename... Offsets>
    quint64 readPointer(quint64 base, Offsets... offsets) {
        quint64 current = base;
        // C++17 折叠表达式
        ((current = readLong(current + offsets)), ...);
        return current;
    }

    float readFloat(ULONG64 address);

    std::vector<byte> readByteArray(ULONG64 address, int length);

    QString readString(ULONG64 address, int length);
};

#endif // MEM_H
