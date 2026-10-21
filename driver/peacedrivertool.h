#ifndef PEACEDRIVERTOOL_H
#define PEACEDRIVERTOOL_H

#include "atlstr.h"
#include "PeaceDriverAPI.h"

#include <cstdint>
#include <vector>

struct EPTHNODE {
    uint64_t hook_pfn;
    uint64_t shadow_pfn;
    __int64 shadow_address;
};

class PeaceDriverTool
{
public:
    PeaceDriverTool();

    /*
    * 功能        安装一个HOOK，无需跨页
    * 描述        用于一些比较简单的HOOK，比如原机器指令是3字节，HOOK代码也是3字节
    *
    * 参数1       HOOK地址
    * 参数2       替换的字节
    * 参数3       替换字节长度
    *
    * return      是否成功
    */
    static bool hookAsmcode(__int64 hook_address, PBYTE asmdata, ULONG64 asmdata_size);

    /*
    * 功能        卸载全部 EPT HOOK，释放影子页
    * 描述        驱动卸载前调用，按 hook_list 在每个 CPU 上 remove_ept_hook，再释放影子页
    */
    static void removeAllHook();

    /*
    * 功能        安装一个HOOK，需要跨页执行。
    * 描述        用于复杂场景的HOOK，比如原机器指令是3字节，但是HOOK代码长度大于3字节
    *
    * 参数1       HOOK地址
    * 参数2       替换的字节
    *
    * return      是否成功
    */


    static bool weaponDurability(bool on);
private:
    // 已安装的 EPT HOOK。同一物理页只保留一份影子页。
    static std::vector<EPTHNODE> hook_list;
};

#endif // PEACEDRIVERTOOL_H
