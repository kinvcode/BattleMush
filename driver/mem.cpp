#include "mem.h"
#include "driver/PeaceDriverAPI.h"
#include "global/globalplayer.h"

std::vector<byte> operator+(std::vector<byte> a, std::vector<byte> b)
{
    for (int i = 0; i < b.size(); i++)
    {
        a.push_back(b[i]);
    }

    b.clear();

    return a;
}

namespace Mem{

bool readMemory(ULONG64 addr, PBYTE pDataBuffer, ULONG size)
{
    return PeaceDriver::readMemory(Gplayer.p_gamepid, addr, size, pDataBuffer);
}

bool writeMemory(ULONG64 addr, PBYTE pDataBuffer, ULONG size)
{
    return PeaceDriver::writeMemory(Gplayer.p_gamepid, addr, size, pDataBuffer);
}

byte readByte(long long address)
{
    if (Gplayer.p_gamepid == NULL || address == 0)
    {
        return -1;
    }

    byte value;

    if (readMemory(address, &value, 1))
    {
        return value;
    }

    return -1;
}

bool writeWord(long long address, WORD value)
{
    if (Gplayer.p_gamepid == NULL || address == 0)
    {
        return false;
    }

    if (writeMemory(address, (byte*)&value, 2))
    {
        return true;
    }

    return false;
}

int readInt(long long address)
{
    if (Gplayer.p_gamepid == NULL || address == 0)
    {
        return -1;
    }

    int value;

    if (readMemory(address, (byte*)&value, 4))
    {
        return value;
    }

    return -1;
}

bool writeInt(long long address, long long value)
{
    if (Gplayer.p_gamepid == NULL || address == 0)
    {
        return false;
    }

    if (writeMemory(address, (byte*)&value, 4))
    {
        return true;
    }

    return false;
}

long long readLong(long long address)
{
    if (Gplayer.p_gamepid == NULL || address == 0)
    {
        return -1;
    }

    __int64 value;

    if (readMemory(address, (byte*)&value, 8))
    {
        return value;
    }

    return -1;
}

bool writeLong(long long address, long long value)
{
    if (Gplayer.p_gamepid == NULL || address == 0)
    {
        return false;
    }

    bool res = writeMemory(address, (byte*)&value, 8);

    return res;
}

float readFloat(long long address)
{
    if (Gplayer.p_gamepid == NULL || address == 0)
    {
        return -1;
    }

    float value;

    if (readMemory(address, (byte*)&value, 4))
    {
        return value;
    }

    return -1;
}

bool writeFloat(long long address, float value)
{
    if (Gplayer.p_gamepid == NULL || address == 0)
    {
        return false;
    }

    bool res = writeMemory(address, (byte*)&value, 4);

    return res;
}

std::vector<byte> readByteArray(long long address, int length)
{
    std::vector<byte> result;

    if (Gplayer.p_gamepid == NULL|| address == 0)
    {
        return result;
    }

    byte* tempResult;
    tempResult = new byte[length];
    memset(tempResult, 0, length);

    readMemory(address,tempResult, length);

    result.resize(length);
    for (int i = 0; i < length; i++)
    {
        result[i] = tempResult[i];
    }

    delete[] tempResult;
    return result;
}

bool writeByteArray(long long address, std::vector<byte> Data)
{
    if (Gplayer.p_gamepid == NULL || address == 0)
    {
        return false;
    }

    int length;
    length = (int)Data.size();
    byte* val = new byte[length];
    for (int i = 0; i < length; i++)
    {
        val[i] = Data[i];
    }

    bool res = writeMemory(address,val, length);

    delete[] val;
    return res;
}

QString readString(long long address, int length)
{
    if (Gplayer.p_gamepid == NULL || address <= 0 || length <= 0)
    {
        return QString();
    }

    // 防止异常超大 length
    if (length > 4096) {
        length = 4096;
    }

    // 旧实现：达到 max length 时缓冲区无 L'\0'，QString::fromWCharArray(ptr)
    // 会走 qustrlen 越界读（dump: AutoPlay -> getDungeonAllObj -> readString）。
    // 这里保证终止符，并用带 size 的重载，避免扫描越界。
    WCHAR* tempResult = new WCHAR[static_cast<size_t>(length) + 1];
    int nchars = 0;
    for (; nchars < length; ++nchars)
    {
        WORD w_value = 0;
        if (!readMemory(address + static_cast<__int64>(nchars) * 2,
                        reinterpret_cast<byte*>(&w_value), sizeof(WORD)))
        {
            break;
        }
        if (w_value == 0)
        {
            break;
        }
        tempResult[nchars] = static_cast<WCHAR>(w_value);
    }
    tempResult[nchars] = L'\0';

    QString text = QString::fromWCharArray(tempResult, nchars);
    delete[] tempResult;
    return text;
}

bool writeString(long long address, QString text)
{
    if (Gplayer.p_gamepid == NULL || address == 0)
    {
        return false;
    }

    const ushort* p_str = text.utf16();

    int length = text.length() * 2 + 2;
    bool res = writeMemory(address,(byte*)p_str, length);
    return res;
}

std::vector<byte> wstringToBytes(std::wstring w_string)
{
    std::vector<byte> data;
    // 获取宽字符串指针
    const wchar_t* ptr1 = w_string.c_str();
    // 转为字符指针
    byte* pChar = (byte*)ptr1;
    // 获取字符长度（不包含\0）;
    size_t leng = w_string.size() * 2;
    // 字符放入到容器
    for (size_t i = 0; i < leng; i++) {
        data.push_back((byte) * (pChar + i));
    }

    data.push_back(0);
    data.push_back(0);
    return data;
}

std::wstring bytesToWstring(std::vector<byte> data)
{
    // 声明宽字符串
    std::wstring wstr;
    // 获取字符长度（不包含\0）;
    size_t length = data.size() - 2;
    // 重新设置字符串大小
    wstr.resize(length / 2, 0);
    // 获取字符串指针
    byte* pByte = (byte*)wstr.data();
    // 填充字符串
    for (size_t i = 0; i < length; i++)
    {
        *pByte = data[i];
        pByte++;
    }
    return wstr;
}

std::vector<byte> makeByteArray(std::vector<byte> data)
{
    return data;
}

std::vector<byte> makeEmptyByteArray(int length)
{
    std::vector<byte> data;
    for (int i = 0; i < length; i++)
    {
        data.push_back(NULL);
    }
    return data;
}

std::vector<byte> intToBytes(int length)
{
    __int64  adr = (__int64)&length;
    std::vector<byte>c;
    for (size_t i = 0; i < 4; i++) {
        c.push_back(*(byte*)adr++);
    }
    return c;
}

std::vector<byte> longToBytes(long long length)
{
    __int64  adr = (__int64)&length;
    std::vector<byte>c;
    for (size_t i = 0; i < 8; i++) {
        c.push_back(*(byte*)adr++);
    }
    return c;
}

std::vector<byte> reverseBytes(std::vector<byte> data)
{
    std::vector<byte> new_data;
    int length = (int)data.size();
    for (int i = 0; i < length; i++)
    {
        new_data.push_back(data.back());
        data.pop_back();
    }
    return new_data;
}

float intToFloat(int value)
{
    int* p_x = &value;
    return (float)*(float*)p_x;
}

__int64 allocMemory(ULONG size)
{
    return PeaceDriver::allocMemory(Gplayer.p_gamepid, size);
}

void freeMemory(__int64 address)
{
    PeaceDriver::freeMemory(Gplayer.p_gamepid, address);
}

bool writeByte(long long address, BYTE value)
{
    if (Gplayer.p_gamepid == NULL || address == 0)
    {
        return false;
    }

    if (writeMemory(address, (byte*)&value, 1))
    {
        return true;
    }

    return false;
}

WORD readWord(long long address)
{
    if (Gplayer.p_gamepid == NULL || address == 0)
    {
        return -1;
    }

    WORD value;

    if (readMemory(address, (byte*)&value, 2))
    {
        return value;
    }

    return -1;
}

QString readAnsiString(ULONG64 address, int maxLength)
{
    if (Gplayer.p_gamepid == 0 || address == 0 || maxLength <= 0)
        return {};

    int count = 0;
    __int64 start_address = static_cast<__int64>(address);
    while (count < maxLength) {
        byte c_value = readByte(start_address + count);
        if (c_value == 0)
            break;
        ++count;
    }

    if (count == 0)
        return {};

    char* tempBuffer = new char[count + 1];
    readMemory(start_address, reinterpret_cast<byte*>(tempBuffer), count);
    tempBuffer[count] = '\0';
    QString text = QString::fromUtf8(tempBuffer, count);
    delete[] tempBuffer;
    return text;
}

}
