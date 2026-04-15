#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include "global/struct.h"

class Controller : public QObject
{
    Q_OBJECT
private:
    Controller() = default;
public:
    static Controller* instance() {
        static Controller instance;
        return &instance;
    }

// ====== 成员属性 ======
public:
    static AttackInfo m_attack_info;

    static cv::Point m_attack_range_general;

    static cv::Point m_attack_range_skill1;


// ====== 便捷函数 ======
public:
    static void updateAttackInfo();
};

// 内联全局引用
inline Controller& Gcontroller = *Controller::instance();

#endif // CONTROLLER_H
