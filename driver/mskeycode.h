#pragma once
#ifndef MSKEYCODE_H
#define MSKEYCODE_H

// 字母键
#define MSKeyboard_a        0x1E   // A
#define MSKeyboard_b        0x30   // B
#define MSKeyboard_c        0x2E   // C
#define MSKeyboard_d        0x20   // D
#define MSKeyboard_e        0x12   // E
#define MSKeyboard_f        0x21   // F
#define MSKeyboard_g        0x22   // G
#define MSKeyboard_h        0x23   // H
#define MSKeyboard_i        0x17   // I
#define MSKeyboard_j        0x24   // J
#define MSKeyboard_k        0x25   // K
#define MSKeyboard_l        0x26   // L
#define MSKeyboard_m        0x32   // M
#define MSKeyboard_n        0x31   // N
#define MSKeyboard_o        0x18   // O
#define MSKeyboard_p        0x19   // P
#define MSKeyboard_q        0x10   // Q
#define MSKeyboard_r        0x13   // R
#define MSKeyboard_s        0x1F   // S
#define MSKeyboard_t        0x14   // T
#define MSKeyboard_u        0x16   // U
#define MSKeyboard_v        0x2F   // V
#define MSKeyboard_w        0x11   // W
#define MSKeyboard_x        0x2D   // X
#define MSKeyboard_y        0x15   // Y
#define MSKeyboard_z        0x2C   // Z

// 数字键（主键盘区）
#define MSKeyboard_1        0x02   // 1
#define MSKeyboard_2        0x03   // 2
#define MSKeyboard_3        0x04   // 3
#define MSKeyboard_4        0x05   // 4
#define MSKeyboard_5        0x06   // 5
#define MSKeyboard_6        0x07   // 6
#define MSKeyboard_7        0x08   // 7
#define MSKeyboard_8        0x09   // 8
#define MSKeyboard_9        0x0A   // 9
#define MSKeyboard_0        0x0B   // 0

// 功能键和控制键
#define MSKeyboard_ENTER    0x1C   // Enter
#define MSKeyboard_ESCAPE   0x01   // Esc
#define MSKeyboard_Backspace 0x0E  // Backspace
#define MSKeyboard_Tab      0x0F   // Tab
#define MSKeyboard_KongGe   0x39   // Space
#define MSKeyboard_JianHao  0x0C   // -
#define MSKeyboard_DengHao  0x0D   // =
#define MSKeyboard_ZuoZhongKuoHao  0x1A // [
#define MSKeyboard_YouZhongKuoHao  0x1B // ]
#define MSKeyboard_FanXieGang      0x2B // '\'
#define MSKeyboard_FenHao   0x27   // ;
#define MSKeyboard_DanYinHao 0x28  // '
#define MSKeyboard_BoLangXian 0x29 // `
#define MSKeyboard_Douhao   0x33   // ,
#define MSKeyboard_JuHao    0x34   // .
#define MSKeyboard_XieGang_WenHao 0x35 // /
#define MSKeyboard_CapsLock 0x3A   // CapsLock

// 功能键 F1–F12
#define MSKeyboard_F1       0x3B
#define MSKeyboard_F2       0x3C
#define MSKeyboard_F3       0x3D
#define MSKeyboard_F4       0x3E
#define MSKeyboard_F5       0x3F
#define MSKeyboard_F6       0x40
#define MSKeyboard_F7       0x41
#define MSKeyboard_F8       0x42
#define MSKeyboard_F9       0x43
#define MSKeyboard_F10      0x44
#define MSKeyboard_F11      0x57
#define MSKeyboard_F12      0x58

#define MSKeyboard_PrintScreen 0x37
#define MSKeyboard_ScrollLock  0x46
#define MSKeyboard_Pause       0xE1  // Pause 是多字节：E1 1D 45 E1 9D C5
#define MSKeyboard_Insert      0x52
#define MSKeyboard_Home        0x47
#define MSKeyboard_PageUp      0x49
#define MSKeyboard_Delete      0x53
#define MSKeyboard_End         0x4F
#define MSKeyboard_PageDown    0x51
#define MSKeyboard_RightArrow  0x4D
#define MSKeyboard_LeftArrow   0x4B
#define MSKeyboard_DownArrow   0x50
#define MSKeyboard_UpArrow     0x48

// 小键盘区（NumPad）
#define MSKeypad_NumLock      0x45
#define MSKeypad_ChuHao       0xE0,0x35  // 扩展码 E0 35
#define MSKeypad_ChengHao     0x37
#define MSKeypad_JianHao      0x4A
#define MSKeypad_JiaHao       0x4E
#define MSKeypad_ENTER        0xE0,0x1C  // 扩展码 E0 1C
#define MSKeypad_1_and_End    0x4F
#define MSKeypad_2_and_Down   0x50
#define MSKeypad_3_and_PageDn 0x51
#define MSKeypad_4_and_Left   0x4B
#define MSKeypad_5            0x4C
#define MSKeypad_6_and_Right  0x4D
#define MSKeypad_7_and_Home   0x47
#define MSKeypad_8_and_Up     0x48
#define MSKeypad_9_and_PageUp 0x49
#define MSKeypad_0_and_Insert 0x52
#define MSKeypad_Dian_and_Delete 0x53

// 修饰键
#define MSKeyboard_LeftControl   0x1D
#define MSKeyboard_LeftShift     0x2A
#define MSKeyboard_LeftAlt       0x38
#define MSKeyboard_LeftWindows   0xE0,0x5B
#define MSKeyboard_RightControl  0xE0,0x1D
#define MSKeyboard_RightShift    0x36
#define MSKeyboard_RightAlt      0xE0,0x38
#define MSKeyboard_RightWindows  0xE0,0x5C
#define MSKeyboard_Application   0xE0,0x5D

#endif // MSKEYCODE_H
