#pragma once

// 确保源代码文件以UTF-8编码
#pragma execution_character_set("utf-8")

#ifdef _WIN32
#include <windows.h>
// 在Windows上设置控制台代码页
inline void setConsoleCodePage() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}
#else
inline void setConsoleCodePage() {
    // 非Windows平台不需要特殊处理
}
#endif