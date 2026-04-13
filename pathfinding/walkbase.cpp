#include "walkbase.h"
#include "global/gameoffset.h"
#include "global/globalplayer.h"
#include "driver/mem.h"
#include "driver/mskeycode.h"
#include "utils/helper.h"
#include <QElapsedTimer>
#include "global/struct.h"
#include "utils/helper.h"
#include "utils/gamestruct.h"

using namespace Mem;

WalkBase::WalkBase(int target_x)
{
    m_transform = 0;
    m_deviation = 50;
    m_walk_timeout = 3000;
    m_target_x = target_x;
    m_target_y = target_x;
}

bool WalkBase::run()
{
    m_transform = readLong(Gplayer.p_localplayer + BASE::MODEntity::TransformComponent);

    int user_x = getUserX();

    if(user_x == m_target_x)
    {
        return true;
    }

    // 清空一下按键
    clearXkey();

    // 跑动按键判断
    int direction_x;
    if (m_target_x > user_x) {
        // 向右
        direction_x = MSKeyboard_RightArrow;
    } else {
        // 向左
        direction_x = MSKeyboard_LeftArrow;
    }

    // 是否到达
    bool arrived = false;

    QElapsedTimer begin_timer;
    begin_timer.restart();

    QElapsedTimer run_status_time;
    run_status_time.start();

    // 开始移动
    MSDriverPRO::keyboardDown(direction_x);

    while(true){

        if (begin_timer.elapsed() > m_walk_timeout) {
            DebugOutput::out("寻路超时~");
            clearXkey();
            return false;
        }

        // 到达
        if (arrived) {
            clearXkey();
            return true;
        }

        user_x = getUserX();

        if (abs(m_target_x - user_x) <= m_deviation) {
            arrived = true;
        }
    }

    clearXkey();
    return false;
}

bool WalkBase::runY()
{
    if(m_target_y == 0){
        return false;
    }

    m_transform = readLong(Gplayer.p_localplayer + BASE::MODEntity::TransformComponent);

    int user_y = getUserY();

    if(user_y == m_target_y)
    {
        return true;
    }

    // 清空一下按键
    clearYkey();

    // 跑动按键判断
    int direction_y;
    if (m_target_y > user_y) {
        // 向下
        direction_y = MSKeyboard_DownArrow;
    } else {
        // 向上
        direction_y = MSKeyboard_UpArrow;
    }

    // 是否到达
    bool arrived = false;

    QElapsedTimer begin_timer;
    begin_timer.restart();

    QElapsedTimer run_status_time;
    run_status_time.start();

    // 开始移动
    MSDriverPRO::keyboardDown(direction_y);

    while(true){

        if (begin_timer.elapsed() > m_walk_timeout) {
            DebugOutput::out("寻路超时~");
            user_y = getUserY();
            qDebug() << m_target_y << user_y;
            clearYkey();
            return false;
        }

        // 如果攀爬指针为空，直接退出
        quint64 climb_p = readLong(readLong(Gplayer.p_localplayer
            + BASE::MODEntity::RigidbodyComponent)
            + BASE::RigidbodyComponent::attachEntity);

        if(!climb_p) arrived = true;

        // 到达
        if (arrived) {
            clearYkey();
            return true;
        }

        user_y = getUserY();

        if (abs(m_target_y - user_y) <= m_deviation) {
            arrived = true;
        }
    }

    clearYkey();
    return false;
}

void WalkBase::clearMonster(Monster monster, cv::Point range)
{
    cv::Point user_pos = getUserPos();

    // 跑动按键判断
    int direction_x;
    bool is_left = false;
    qDebug() << "清理怪物,人物" << user_pos.x << "," << user_pos.y << "怪物"  << monster.pos.x << "," << monster.pos.y;
    if (monster.pos.x > user_pos.x) {
        // 向右
        qDebug() << "向右走~";
        direction_x = MSKeyboard_RightArrow;
    } else {
        // 向左
        qDebug() << "向左走~";
        is_left = true;
        direction_x = MSKeyboard_LeftArrow;
    }

    m_skill_range = range;
    m_transform = readLong(Gplayer.p_localplayer + BASE::MODEntity::TransformComponent);

    if(Math::posInRange(is_left, user_pos, monster.pos, range))
    {
        // 因为还没有走路，方向可能不对。打怪之前纠正方向
        MSDriverPRO::keyboardClick(direction_x);
        attackMonster(user_pos, monster.pos);
        return;
    }

    // 清空一下按键
    clearXkey();
    m_walk_timeout = abs(monster.pos.x - user_pos.x) / 14 * 100 + 3000;

    // 是否到达
    bool arrived = false;

    QElapsedTimer begin_timer;
    begin_timer.restart();

    int last_time = begin_timer.elapsed();

    // 开始移动
    MSDriverPRO::keyboardDown(direction_x);

    int map_start = 0;

    // 先执行一次范围检测
    cv::Point pos = monsterInRange(is_left, map_start);
    if(pos.x != -1){
        DebugOutput::out("范围内存在怪物，无需寻路即可攻击。攻击中~");
        user_pos = getUserPos();
        attackMonster(user_pos, pos);
        return;
    }

    while(true){

        int cur_time = begin_timer.elapsed();
        if (cur_time > m_walk_timeout) {
            DebugOutput::out("寻路超时~");
            clearXkey();
            return;
        }

        // 每0.1秒检测一次怪物
        if(cur_time - last_time > 100){
            last_time = cur_time;
            // qDebug() << "检测范围内是否存在怪物~";
            cv::Point pos = monsterInRange(is_left, map_start);
            if(pos.x != -1){
                DebugOutput::out("范围内存在怪物，进行攻击");
                user_pos = getUserPos();
                attackMonster(user_pos, pos); // 实际上必须依赖这里进行攻击怪物。攻击频率10帧
                qDebug() << "寻怪提前完毕~";
                return;
            }
        }

        // 到达
        if (arrived) {
            break;
        }

        user_pos = getUserPos();
        // 判断怪物原始位置为目标地点
        if(Math::posInRange(is_left, user_pos, monster.pos, range)){
            qDebug() << "寻怪目标坐标在人物攻击范围，判定到达~";
            arrived = true;
        }
    }

    clearXkey();

    if(arrived){
        qDebug() << "到达目标怪物地点~";
        cv::Point pos = monsterInRange(is_left, map_start);
        if(pos.x != -1){
            DebugOutput::out("范围内存在怪物，进行攻击");
            user_pos = getUserPos();
            attackMonster(user_pos, pos);
            qDebug() << "寻怪结束，最后一次攻击怪物~";
            return;
        }
    }

    qDebug() << "寻怪结束~";
}

bool WalkBase::moveToItem(cv::Point range)
{
    m_skill_range = range;
    m_transform = readLong(Gplayer.p_localplayer + BASE::MODEntity::TransformComponent);

    int user_x = getUserX();

    if(user_x == m_target_x)
    {
        qDebug() << "与物品X坐标相同";
        return true;
    }

    // 清空一下按键
    clearXkey();

    // 跑动按键判断
    int direction_x;
    bool is_left = false;
    if (m_target_x > user_x) {
        // 向右
        qDebug() << "向右走~";
        direction_x = MSKeyboard_RightArrow;
    } else {
        // 向左
        qDebug() << "向左走~";
        is_left = true;
        direction_x = MSKeyboard_LeftArrow;
    }

    // 是否到达
    bool arrived = false;

    QElapsedTimer begin_timer;
    begin_timer.restart();

    int last_time = begin_timer.elapsed();

    // 开始移动
    MSDriverPRO::keyboardDown(direction_x);

    int map_start = 0;

    // 先执行一次范围检测
    cv::Point pos = monsterInRange(is_left, map_start);
    if(pos.x != -1){
        DebugOutput::out("范围内存在怪物，无需寻路即可攻击。攻击中~");
        cv::Point user_pos = getUserPos();
        attackMonster(user_pos, pos);
        return false; // 因为走路中断了，所以直接返回寻物失败即可
    }

    while(true){
        int cur_time = begin_timer.elapsed();
        if (cur_time > m_walk_timeout) {
            DebugOutput::out("拾取物品寻路超时~");
            clearXkey();
            return false;
        }

        // 每0.1秒检测一次怪物
        if(cur_time - last_time > 100){
            last_time = cur_time;
            cv::Point pos = monsterInRange(is_left, map_start);
            if(pos.x != -1){
                DebugOutput::out("范围内存在怪物，进行攻击");
                cv::Point user_pos = getUserPos();
                attackMonster(user_pos, pos);
                return false; // 因为走路中断了，所以直接返回寻物失败即可
            }
        }

        // 到达
        if (arrived) {
            qDebug() << "到达目标物品~";
            clearXkey();
            return true;
        }

        user_x = getUserX();

        if (abs(m_target_x - user_x) <= m_deviation) {
            arrived = true;
        }
    }

    qDebug() << "拾取物品结束,失败~";
    return false;
}

bool WalkBase::moveToPlatform(cv::Point range, bool self_climb)
{
    m_transform = readLong(Gplayer.p_localplayer + BASE::MODEntity::TransformComponent);

    int user_pos;
    if(self_climb){
        qDebug() << "攀爬体寻路到连接点";
        user_pos = getUserY();
    }else{
        qDebug() << "地板寻路到连接点";
        user_pos = getUserX();
    }

    qDebug() << "移动平台连接点开始 人物X：" << user_pos;

    if(user_pos >= range.x && user_pos <= range.y)
    {
        qDebug() << "已在平台连接点范围，无需移动平台连接点~";
        return true;
    }

    // 清空一下按键
    if(self_climb){
        clearYkey();
    }else{
       clearXkey();
    }

    int m_target_pos = range.x + (range.y - range.x) / 2;

    if(self_climb){
        m_walk_timeout = abs(m_target_pos - user_pos) / 7 * 100 + 3000;
    }else{
        m_walk_timeout = abs(m_target_pos - user_pos) / 14 * 100 + 3000;
    }

    // 跑动按键判断
    int direction_arrow;
    bool is_left = false;
    if (m_target_pos > user_pos) {
        if(self_climb){
            qDebug() << "向下攀爬~";
            direction_arrow = MSKeyboard_DownArrow;
        }else{
            qDebug() << "向右走~";
            direction_arrow = MSKeyboard_RightArrow;
        }
    } else {
        if(self_climb){
            qDebug() << "向上攀爬~";
            direction_arrow = MSKeyboard_UpArrow;
        }else{
            is_left = true;
            qDebug() << "向左走~";
            direction_arrow = MSKeyboard_LeftArrow;
        }
    }

    QElapsedTimer begin_timer;
    begin_timer.restart();

    int last_time = begin_timer.elapsed();

    // 开始移动
    MSDriverPRO::keyboardDown(direction_arrow);

    int map_start = 0;

    // 先执行一次范围检测
    if(!self_climb){
        cv::Point pos = monsterInRange(is_left, map_start);
        if(pos.x != -1){
            qDebug() << "范围内存在怪物，结束移动到平台连接点。攻击目标:" << pos.x << "," << pos.y;
            if(self_climb){
                clearYkey();
            }else{
                clearXkey();
            }
            cv::Point user_pos = getUserPos();
            attackMonster(user_pos, pos);
            return false;
        }
    }

    while(true){

        int cur_time = begin_timer.elapsed();
        if (cur_time > m_walk_timeout) {
            DebugOutput::out("寻路超时~");
            if(self_climb){
                clearYkey();
            }else{
                clearXkey();
            }
            return false;
        }

        // 每0.1秒检测一次怪物
        if(!self_climb){
            if(cur_time - last_time > 100){
                last_time = cur_time;
                cv::Point pos = monsterInRange(is_left, map_start);
                if(pos.x != -1){
                    DebugOutput::out("范围内存在怪物，进行攻击");
                    if(self_climb){
                        clearYkey();
                    }else{
                        clearXkey();
                    }
                    cv::Point user_pos = getUserPos();
                    attackMonster(user_pos, pos);
                    qDebug() << "移动平台连接点提前完毕~";
                    return false;
                }
            }
        }

        if(self_climb){
            user_pos = getUserY();
        }else{
            user_pos = getUserX();
        }

        if(user_pos >= range.x && user_pos <= range.y) {
            if(self_climb){
                clearYkey();
            }else{
                clearXkey();
            }
            return true;
        }
    }

    if(self_climb){
        clearYkey();
    }else{
        clearXkey();
    }

    qDebug() << "移动到平台连接点结束~";
    return false;
}

int WalkBase::getUserX()
{
    if(!m_transform){
        return -1;
    }

    quint64 transform = readLong(Gplayer.p_localplayer + BASE::MODEntity::TransformComponent);
    float user_x = readFloat(transform + BASE::TransformComponent::PositionX);

    return RoomMap::toMapPosX(user_x,Gplayer.curmapinfo.map_bounding.x_offset);
}

int WalkBase::getUserY()
{
    if(!m_transform){
        return -1;
    }

    quint64 transform = readLong(Gplayer.p_localplayer + BASE::MODEntity::TransformComponent);
    float user_y = readFloat(transform + BASE::TransformComponent::PositionY);

    return RoomMap::toMapPosY(user_y,Gplayer.curmapinfo.map_bounding.y_offset,Gplayer.curmapinfo.map_bounding.height);
}

cv::Point WalkBase::getUserPos()
{
    cv::Point pos = {-1, -1};

    quint64 transform = readLong(Gplayer.p_localplayer + BASE::MODEntity::TransformComponent);
    float user_x = readFloat(transform + BASE::TransformComponent::PositionX);
    float user_y = readFloat(transform + BASE::TransformComponent::PositionY);
    pos.x = RoomMap::toMapPosX(user_x,Gplayer.curmapinfo.map_bounding.x_offset);
    pos.y = RoomMap::toMapPosY(user_y,Gplayer.curmapinfo.map_bounding.y_offset, Gplayer.curmapinfo.map_bounding.height);
    return pos;
}

void WalkBase::clearXkey()
{
    MSDriverPRO::keyboardUp(MSKeyboard_RightArrow);
    MSDriverPRO::keyboardUp(MSKeyboard_LeftArrow);
}

void WalkBase::clearYkey()
{
    MSDriverPRO::keyboardUp(MSKeyboard_UpArrow);
    MSDriverPRO::keyboardUp(MSKeyboard_DownArrow);
}

bool WalkBase::isAttaking()
{
    return readByte(readLong(Gplayer.p_localplayer + BASE::MODEntity::StateComponent) + BASE::StateComponent::attacking) == 0;
}

cv::Point WalkBase::monsterInRange(bool is_left, int &start_index)
{
    quint64 count = readInt(readLong(Gplayer.p_curmapentity + BASE::MODEntity::children) + BASE::List::size);
    quint64 head = readLong(readLong(Gplayer.p_curmapentity
                                            + BASE::MODEntity::children)
                                   + BASE::List::InternalArray)
                          + BASE::InternalArray::Head;

    cv::Point monster_pos = {-1,-1};
    // qDebug() << "======范围检测======";
    cv::Point user_pos = getUserPos();
    // qDebug() << "人物：" << user_pos.x << "," << user_pos.y;
    for(int i = start_index; i < count; i++){
        // 节省遍历时间
        quint64 cur_entity_address = head + i * 8;
        quint64 entity = readLong(cur_entity_address);

        // 找到End标记位
        QString name = GameStruct::readEntityName(entity);
        if(name == "EndObj"){
            start_index = i + 1;
            continue;
        }
        // 也可以找玩家
        // 找怪物
        if(name != "Monster"){
            continue;
        }

        // 判断怪物是否存在
        bool exists = readByte(readLong(entity + BASE::MODEntity::RigidbodyComponent) + BASE::RigidbodyComponent::exists);
        if(!exists){
            continue;
        }

        QString status = readString( readLong( readLong(entity + BASE::MODEntity::StateComponent)
            + BASE::StateComponent::_CurrentStateName) + BASE::String::text, 10);
        if(status == "DEAD"){
            continue;
        }

        // 存放怪物信息:怪物指针，怪物所在地板，怪物所在平台
        // int foothold_id = readInt(readLong(readLong(entity + BASE::MODEntity::RigidbodyComponent)
            // + BASE::RigidbodyComponent::currentFoothold) + BASE::Foothold::ID);
        // int line_id = RoomMap::getFootPlatform(foothold_id, Gplayer.curmapinfo.platform_list);
        // if(line_id == -1){
        //     continue;
        // }

        quint64 pos_p = readLong(entity + BASE::MODEntity::RigidbodyComponent);
        float float_x = readFloat(pos_p + BASE::RigidbodyComponent::airPos);
        float float_y = readFloat(pos_p + BASE::RigidbodyComponent::airPos + 4);
        int monster_x = RoomMap::toMapPosX(float_x, Gplayer.curmapinfo.map_bounding.x_offset);
        int monster_y = RoomMap::toMapPosY(float_y, Gplayer.curmapinfo.map_bounding.y_offset,Gplayer.curmapinfo.map_bounding.height);
        // qDebug() << monster_x << "," << monster_y;
        monster_pos = {monster_x, monster_y};
        if(Math::posInRange(is_left, user_pos, monster_pos,m_skill_range)){
            monster_pos = {monster_x,monster_y};
            qDebug() << "人物当前位置:" << user_pos.x << "," << user_pos.y;
            qDebug() << "发现怪物在攻击范围：" << monster_x << "," << monster_y;
            return monster_pos;
        }else{
            monster_pos = {-1, -1};
        }
    }
    // qDebug() << "======范围检测======";

    return monster_pos;
}

void WalkBase::attackMonster(cv::Point user_pos, cv::Point monster_pos)
{
    clearXkey();
    qDebug() << "怪物在攻击范围内，调整方向后攻击";
    qDebug() << "人物:" << user_pos.x << "," << user_pos.y << " 怪物:" << monster_pos.x << "," << monster_pos.y;
    while(isAttaking()){
        ProcessHelper::programDelay(50);
        QApplication::processEvents();
    }

    // 攻击之前再次检查怪物存在状态和是否在范围内

    if(user_pos.x > monster_pos.x){
        qDebug() << "调整人物方向，向左";
        // MSDriverPRO::keyboardClick(MSKeyboard_LeftArrow);
    }else{
        qDebug() << "调整人物方向，向右";
        // MSDriverPRO::keyboardClick(MSKeyboard_RightArrow);
    }

    // 如果血量低于10%
    int cur_mp_rate = Gplayer.MP * 100 / Gplayer.MAX_MP;
    if(cur_mp_rate < 10){
        qDebug() << "使用普通攻击";
        MSDriverPRO::keyboardClick(Gplayer.m_bindkey.general_atk, 1);
        return;
    }

    // 使用技能
    qDebug() << "使用技能攻击" << QDateTime::currentMSecsSinceEpoch();
    MSDriverPRO::keyboardClick(Gplayer.m_bindkey.skill_atk1, 1);
}
