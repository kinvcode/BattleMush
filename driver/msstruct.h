#ifndef MSSTRUCT_H
#define MSSTRUCT_H

#include <windows.h>
#pragma once

// 驱动操作
#define	TAG_READ		0x85648485
#define	TAG_WRITE		0x89756453
#define TAG_GETMODULE	0x56165511
#define TAG_PROTECT		0x98844446
#define TAG_PROBEREAD	0x68468846
#define TAG_GETPID		0x22668800
#define TAG_HIDEPID		0x22668801
#define TAG_RESTORE		0x22668802
#define TAG_ALLOC		0x22668803
#define TAG_FREE		0x22668804
#define TAG_MOUSE		0x22668805
#define TAG_KEYBOARD	0x22668806
#define TAG_REMOTETHREAD	0x22668807
#define TAG_QUERYMEM	0x22668808

// 鼠标事件
#define MOUSE_LEFT_BUTTON_DOWN   0x0001  // Left Button changed to down.
#define MOUSE_LEFT_BUTTON_UP     0x0002  // Left Button changed to up.
#define MOUSE_RIGHT_BUTTON_DOWN  0x0004  // Right Button changed to down.
#define MOUSE_RIGHT_BUTTON_UP    0x0008  // Right Button changed to up.
#define MOUSE_MIDDLE_BUTTON_DOWN 0x0010  // Middle Button changed to down.
#define MOUSE_MIDDLE_BUTTON_UP   0x0020  // Middle Button changed to up.
#define MOUSE_BUTTON_1_DOWN     MOUSE_LEFT_BUTTON_DOWN
#define MOUSE_BUTTON_1_UP       MOUSE_LEFT_BUTTON_UP
#define MOUSE_BUTTON_2_DOWN     MOUSE_RIGHT_BUTTON_DOWN
#define MOUSE_BUTTON_2_UP       MOUSE_RIGHT_BUTTON_UP
#define MOUSE_BUTTON_3_DOWN     MOUSE_MIDDLE_BUTTON_DOWN
#define MOUSE_BUTTON_3_UP       MOUSE_MIDDLE_BUTTON_UP
#define MOUSE_BUTTON_4_DOWN     0x0040
#define MOUSE_BUTTON_4_UP       0x0080
#define MOUSE_BUTTON_5_DOWN     0x0100
#define MOUSE_BUTTON_5_UP       0x0200
#define MOUSE_WHEEL             0x0400
#define MOUSE_HWHEEL		0x0800

#pragma pack(push,1)
typedef struct _BOOM_PROCESS_OPERA {
    ULONG		tag;		// 标记
    ULONG		flags;		// 标记
    ULONG64		pid;		// target process
    ULONG64		address;	// 读取地址
    ULONG64		buf;		// 缓冲区指针
    ULONG		size;		// 需要读取大小
}BOOM_PROCESS_OPERA, *PBOOM_PROCESS_OPERA;
#pragma pack(pop)

typedef struct _MOUSE_INPUT_DATA {
    USHORT UnitId;               // 表示设备的单元 ID
    USHORT Flags;                // 鼠标事件类型
    union {
        ULONG Buttons;			 // 参考下方
        struct {
            USHORT  ButtonFlags; // 按键动作
            USHORT  ButtonData;  // 当 ButtonFlags 设置为 MOUSE_WHEEL 时，ButtonData 表示滚轮滚动的增量值（正值表示向前滚动，负值表示向后滚动）
        };
    };
    ULONG RawButtons;           // 默认0
    LONG LastX;                 // X坐标
    LONG LastY;                 // Y坐标
    ULONG ExtraInformation;     // 默认0
} MOUSE_INPUT_DATA, * PMOUSE_INPUT_DATA;

typedef struct _KEYBOARD_INPUT_DATA {
    USHORT UnitId;              // 键盘设备单元ID（一般为0）
    USHORT MakeCode;            // 按键的扫描码（Scan Code）
    USHORT Flags;               // 键盘事件类型和属性（见下方说明）
    USHORT Reserved;            // 保留，通常为0
    ULONG ExtraInformation;     // 附加信息，一般为0
} KEYBOARD_INPUT_DATA, * PKEYBOARD_INPUT_DATA;

#endif // MSSTRUCT_H
