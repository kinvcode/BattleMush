#include "peacedrivertool.h"
#include "global/globalplayer.h"
#include <QDebug>
#include <memory>

using namespace PeaceDriver;

template <typename Fn>
inline void for_each_cpu(Fn const fn) {
    SYSTEM_INFO info = {};
    GetSystemInfo(&info);

    for (uint32_t i = 0; i < info.dwNumberOfProcessors; ++i) {
        auto const prev_affinity = SetThreadAffinityMask(GetCurrentThread(), 1ull << i);
        fn(i);
        SetThreadAffinityMask(GetCurrentThread(), prev_affinity);
    }
}

PeaceDriverTool::PeaceDriverTool() {}

std::vector<EPTHNODE> PeaceDriverTool::hook_list;

bool PeaceDriverTool::hookAsmcode(__int64 hook_address, PBYTE asmdata, ULONG64 asmdata_size)
{
    if(Gplayer.p_gamepid == 0){
        qDebug() << "目标进程不存在,PID:0";
        return false;
    }

    ULONG64   shadowpage_address  = 0;
    ULONG64   asmcode_address     = 0;
    ULONG64   target_cr3          = 0;
    uint64_t  hook_pfn            = 0;
    uint64_t  shadow_pfn          = 0;
    bool      ept_installed       = false;

    const auto cleanupPartial = [&]() {

        if (ept_installed && hook_pfn != 0) {
            for_each_cpu([hook_pfn](uint32_t cpu_id) {
                Q_UNUSED(cpu_id);
                remove_ept_hook(hook_pfn);
            });
            ept_installed = false;
        }

        if (asmcode_address != 0) {
            freeMemory(Gplayer.p_gamepid, asmcode_address);
            asmcode_address = 0;
        }
        if (shadowpage_address != 0) {
            freeMemory(Gplayer.p_gamepid, shadowpage_address);
            shadowpage_address = 0;
        }
    };

    // 读取目标程序CR3
    target_cr3 = query_process_cr3(Gplayer.p_gamepid);
    if(!target_cr3){
        qDebug() << "cr3查询失败";
        return false;
    }

    qDebug() << "CR3:" << QString::number(target_cr3,16).toUpper();

    if ((target_cr3 & 0xFFF) != 0) {
        qDebug() << "CR3值无效（未页对齐）：" << QString::number(target_cr3,16).toUpper();
        return false;
    }

    // 计算HOOK页面的地址和hook位置偏移
    uintptr_t hook_page_start  = hook_address & ~0xFFF;
    void*     hook_page_ptr    = reinterpret_cast<void*>(hook_page_start);
    uintptr_t hook_offset      = hook_address - reinterpret_cast<uintptr_t>(hook_page_ptr);
    qDebug() << "HOOK页面起始地址：" << QString::number((uint64_t)hook_page_ptr,16).toUpper();
    qDebug() << "HOOK函数偏移："    << QString::number((uint64_t)hook_offset,16).toUpper();

    // 读取HOOK页面数据,一会需要复制给影子页
    // 获取物理页之前，必须主动访问一次虚拟地址
    auto hook_page_cache = std::make_unique<byte[]>(0x1000);
    if (!readMemory(Gplayer.p_gamepid, reinterpret_cast<uint64_t>(hook_page_ptr), 0x1000, hook_page_cache.get())) {
        qDebug() << "读取HOOK页面失败";
        cleanupPartial();
        return false;
    }

    // 获取HOOK页面的物理地址（使用页面对齐的地址）
    uint64_t hook_phys_addr = get_physical_address(target_cr3, hook_page_ptr);
    if (!hook_phys_addr) {
        qDebug() << "无法获取HOOK物理页，后续操作中断";
        return false;
    }

    qDebug() << "HOOK物理地址：" << QString::number(hook_phys_addr,16).toUpper();

    hook_pfn = hook_phys_addr >> 12;
    qDebug() << "HOOK物理页帧号(PFN)：" << QString::number(hook_pfn,16).toUpper();

    // 同一物理页已有影子页时，只把本次指令写进去，不再建页、不再重装 EPT
    for (const EPTHNODE& node : hook_list) {
        if (node.hook_pfn != hook_pfn) {
            continue;
        }

        shadowpage_address = static_cast<ULONG64>(node.shadow_address);
        qDebug() << "物理页已存在HOOK，复用影子页：" << QString::number(shadowpage_address,16).toUpper();

        // 执行走影子页，补丁写到影子页；原页保持干净给读/写
        const ULONG64 shadow_patch_va = shadowpage_address + hook_offset;
        if (!writeMemory(Gplayer.p_gamepid, shadow_patch_va, asmdata_size, asmdata)) {
            qDebug() << "写入影子页HOOK指令失败";
            return false;
        }

        qDebug() << "已在已有影子页写入HOOK";
        return true;
    }

    // 在目标进程中申请一段内存，作为影子页
    shadowpage_address = allocMemory(Gplayer.p_gamepid, 0x1000);
    if(shadowpage_address == 0){
        qDebug() << "申请影子内存失败";
        return false;
    }

    qDebug() << "申请影子空间：" << QString::number(shadowpage_address,16).toUpper();


    qDebug() << "准备复制页面数据";
    if (!writeMemory(Gplayer.p_gamepid, shadowpage_address, 0x1000, hook_page_cache.get())) {
        qDebug() << "写入影子页失败";
        cleanupPartial();
        return false;
    }

    // 获取影子页的物理帧
    uint64_t shadow_phys_addr = get_physical_address(target_cr3, reinterpret_cast<void*>(shadowpage_address));
    if (!shadow_phys_addr) {
        qDebug() << "无法获取影子页物理地址，后续操作中断";
        cleanupPartial();
        return false;
    }
    qDebug() << "影子页物理地址：" << QString::number(shadow_phys_addr,16).toUpper();
    shadow_pfn = shadow_phys_addr >> 12;
    qDebug() << "影子页物理页帧号(PFN)：" << QString::number(shadow_pfn,16).toUpper();

    // 补丁写到影子页（exec）。原页不改，读/写仍看到原始机器码。
    const ULONG64 shadow_patch_va = shadowpage_address + hook_offset;
    if (!writeMemory(Gplayer.p_gamepid, shadow_patch_va, asmdata_size, asmdata)) {
        qDebug() << "写入影子页HOOK指令失败";
        cleanupPartial();
        return false;
    }

    qDebug() << "开始安装EPT HOOK";

    // 查询CPU信息
    SYSTEM_INFO cpu_info = {};
    GetSystemInfo(&cpu_info);
    const uint32_t cpu_count = cpu_info.dwNumberOfProcessors;
    uint32_t success_count = 0;

    // 在每一个CPU上面安装EPT HOOK
    for_each_cpu([&](uint32_t cpu_id) {
        // orig = 游戏代码页 GPA：读/写走 orig_pfn，取指走 exec_pfn（影子页）
        // RIP 仍是原函数 VA，影子页里的 E8 相对寻址不会跑飞
        if (install_ept_hook(hook_pfn, shadow_pfn)) {
            qDebug() << "EPT安装成功，CPU：" + QString::number(cpu_id);
            ++success_count;
        } else {
            qDebug() << "EPT安装失败，CPU：" + QString::number(cpu_id);
        }
    });

    // 如果安装EPT失败，则回滚
    if (success_count != cpu_count) {
        for_each_cpu([hook_pfn](uint32_t cpu_id) {
            Q_UNUSED(cpu_id);
            remove_ept_hook(hook_pfn);
        });
        cleanupPartial();
        return false;
    }

    EPTHNODE node = {};
    node.hook_pfn = hook_pfn;
    node.shadow_pfn = shadow_pfn;
    node.shadow_address = static_cast<__int64>(shadowpage_address);
    hook_list.push_back(node);

    qDebug() << "EPT HOOK安装完毕";
    ept_installed = true;
    return true;
}

void PeaceDriverTool::removeAllHook()
{
    if (hook_list.empty()) {
        qDebug() << "没有需要清理的EPT HOOK";
        return;
    }

    qDebug() << "开始清理EPT HOOK，数量：" << hook_list.size();

    for (const EPTHNODE& node : hook_list) {
        // 原页从未打补丁，卸 EPT 后 GPA 重新指向 orig 即恢复
        if (node.hook_pfn != 0) {
            for_each_cpu([hook_pfn = node.hook_pfn](uint32_t cpu_id) {
                Q_UNUSED(cpu_id);
                remove_ept_hook(hook_pfn);
            });
        }

        if (Gplayer.p_gamepid != 0 && node.shadow_address != 0) {
            freeMemory(Gplayer.p_gamepid, static_cast<ULONG64>(node.shadow_address));
        }
    }

    hook_list.clear();
    qDebug() << "EPT HOOK已全部清理";
}

bool PeaceDriverTool::weaponDurability(bool)
{
    return false;
}
