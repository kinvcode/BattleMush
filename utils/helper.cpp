#include "helper.h"

Helper::Helper() {}

void DebugOutput::out(const QString& message){
    QString prefix = "[myprint] ";
    QString formattedMsg = prefix + message;
    QString output = formattedMsg + "\n";

    // 输出到 DebugView
    OutputDebugStringW(output.toStdWString().c_str());
}
