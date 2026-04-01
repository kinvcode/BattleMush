#ifndef GLOBALPLAYER_H
#define GLOBALPLAYER_H
#include <QObject>

class GlobalPlayer : public QObject
{
    Q_OBJECT
private:
    GlobalPlayer() = default;
public:
    static GlobalPlayer* instance() {
        static GlobalPlayer instance;
        return &instance;
    }

    // 属性
    int HP = 999999;
    int MAX_HP = 999999;

    int MP = 999999;
    int MAX_MP = 999999;

private:

};

// 内联全局引用
inline GlobalPlayer& Gplayer = *GlobalPlayer::instance();

#endif // GLOBALPLAYER_H
