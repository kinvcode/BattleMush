#include "mem.h"
#include "global/globalplayer.h"

bool Mem::readMemory(ULONG64 addr, PBYTE pDataBuffer, ULONG size)
{
    if(Gplayer.p_gamepid == 0 || Gplayer.p_assemblybase == 0) return false;
    return MSDriverPRO::readBytes(Gplayer.p_gamepid, addr, pDataBuffer, size);
    return false;
}

byte Mem::readByte(ULONG64 address)
{
    byte result = 0;
    if(Gplayer.p_gamepid == 0 || Gplayer.p_assemblybase == 0) return result;

    result = MSDriverPRO::readByte(Gplayer.p_gamepid, address);
    return result;
}


WORD Mem::readWord(ULONG64 address)
{
    WORD result = 0;
    if(Gplayer.p_gamepid == 0 || Gplayer.p_assemblybase == 0) return result;

    if (readMemory(address, (byte*)&result, 2))
    {
        return result;
    }

    return result;
}

int Mem::readInt(ULONG64 address)
{
    int result = 0;
    if(Gplayer.p_gamepid == 0 || Gplayer.p_assemblybase == 0) return result;

    result = MSDriverPRO::readInt(Gplayer.p_gamepid, address);
    return result;
}

__int64 Mem::readLong(ULONG64 address)
{
    __int64 result = 0;
    if(Gplayer.p_gamepid == 0 || Gplayer.p_assemblybase == 0) return result;

    result = MSDriverPRO::readInt64(Gplayer.p_gamepid, address);
    return result;
}

float Mem::readFloat(ULONG64 address)
{
    float result = 0;
    if(Gplayer.p_gamepid == 0 || Gplayer.p_assemblybase == 0) return result;

    result = MSDriverPRO::readFloat(Gplayer.p_gamepid, address);
    return result;
}

std::vector<byte> Mem::readByteArray(ULONG64 address, int length)
{
    std::vector<byte> result;
    if(Gplayer.p_gamepid == 0 || Gplayer.p_assemblybase == 0) return result;

    byte* tempResult;
    tempResult = new byte[length];
    memset(tempResult, 0, length);

    readMemory(address, tempResult, length);

    result.resize(length);
    for (int i = 0; i < length; i++)
    {
        result[i] = tempResult[i];
    }

    delete[] tempResult;
    return result;
}

QString Mem::readString(ULONG64 address, int length)
{
    QString result = "";
    if(Gplayer.p_gamepid == 0 || Gplayer.p_assemblybase == 0) return result;

    int count = 1;
    WORD w_value = 0;
    __int64 start_address = address;
    while (true)
    {
        w_value = readWord(address);

        if (w_value == 0)  // 遇到字符串结束符，终止读取
        {
            break;
        }

        if (count >= length)  // 限制最大长度
        {
            break;
        }

        address += 2;
        count ++;
    }

    WCHAR* tempResult;
    tempResult = new WCHAR[count];
    readMemory(start_address,(byte*)tempResult, count*2);
    QString text = QString::fromWCharArray(tempResult);
    delete[] tempResult;
    return text;
}

