#ifndef SCRIPTCONFIG_H
#define SCRIPTCONFIG_H

#include <QObject>
#include "global/struct.h"

class ScriptConfig : public QObject
{
    Q_OBJECT
private:
    ScriptConfig() = default;
public:
    static ScriptConfig* instance() {
        static ScriptConfig instance;
        return &instance;
    }
};

// 内联全局引用
inline ScriptConfig& Gscriptconfig = *ScriptConfig::instance();

#endif // SCRIPTCONFIG_H
