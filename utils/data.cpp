#include "data.h"
#include "global/globalplayer.h"
#include "driver/msdriverpro.h"
#include "driver/mem.h"
#include "utils/helper.h"
#include "global/gameoffset.h"
#include "utils/gamestruct.h"

using namespace Mem;

quint64 Data::Game::gamecontext()
{
    quint64 pointer = 0;
    // 检测游戏是否存在
    Gplayer.p_gamepid = MSDriverPRO::getPid("msw.exe");

    if(Gplayer.p_gamepid == 0){
        DebugOutput::out("游戏进程不存在");
        Helper::console("游戏进程不存在");
        return pointer;
    }

    // DebugOutput::out("游戏进程:" + QString::number(Gplayer.p_gamepid));
    Gplayer.p_assemblybase = MSDriverPRO::getModuleAddr(Gplayer.p_gamepid, "GameAssembly.dll");
    // DebugOutput::out("模块基址:" + QString::number(Gplayer.p_assemblybase, 16).toUpper());

    Gplayer.p_gamecontext_entries = readPointer(Gplayer.p_assemblybase,
                                                BASE::ClientCoreThreadManager_base,
                                                BASE::ClientCoreThreadManager_offset_lv1,
                                                BASE::ClientCoreThreadManager_offset_lv2,
                                                BASE::ClientCoreThreadManager_GameDictionary,
                                                BASE::Dictionary::InternalArray_entries);
    Gplayer.p_gamecontext = readPointer(Gplayer.p_gamecontext_entries, BASE::InternalArray::Head + 0x10);
    // DebugOutput::out("GameContext:" + QString::number(Gplayer.p_gamecontext, 16).toUpper());
    // 检测是否已经进入游戏
    if(Gplayer.p_gamecontext == 0){
        // 尚未进入游戏，
        DebugOutput::out("等待进入游戏中...");
        Helper::console("等待进入游戏中...");
        return pointer;
    }

    return Gplayer.p_gamecontext;
}

quint64 Data::Game::mapEntity()
{
    quint64 p_gamecontext = gamecontext();
    quint64 p_mapsentity = readPointer(p_gamecontext, BASE::GameContext::MapsEntity);
    return p_mapsentity;
}

quint64 Data::Game::uiEntity()
{
    quint64 p_gamecontext = gamecontext();
    quint64 p_uientity = readPointer(p_gamecontext, BASE::GameContext::UIEntity);
    return p_uientity;
}

void Data::Game::roomMapItems()
{
    quint64 map_entity = mapEntity();

    Item target_item;
    target_item.entity = 0;

    quint64 p_curmapentity = readPointer(map_entity,
    BASE::MODEntity::children,
    BASE::List::InternalArray,
    BASE::InternalArray::Head + 0 * 8);

    quint64 map_entity_count = readInt(readLong(p_curmapentity + BASE::MODEntity::children) + BASE::List::size);
    quint64 entity_head = readLong(readLong(p_curmapentity + BASE::MODEntity::children) + BASE::List::InternalArray) + BASE::InternalArray::Head;

    MapBounding map_bounding = RoomMap::getMapBounding(p_curmapentity);

    qDebug() << "====== 物品列表 ======";
    bool is_item_start = false;
    for(int i = 0; i < map_entity_count; i++){

        quint64 cur_entity_address = entity_head + i * 8;
        quint64 entity = readLong(cur_entity_address);

        QString name = GameStruct::readEntityName(entity);
        // qDebug() << name;
        if(name == "EndObj"){
            is_item_start = true;
            continue;
        }

        if(!is_item_start){
            continue;
        }

        if(name != "PoolObject"){
            continue;
        }

        qDebug() << "物品:" << QString::number(entity,16).toUpper();
        float item_x = readFloat(readLong(entity + BASE::MODEntity::TransformComponent)+BASE::TransformComponent::PositionX);
        float item_y = readFloat(readLong(entity + BASE::MODEntity::TransformComponent)+BASE::TransformComponent::PositionY);
        qDebug() << "浮点坐标:" << item_x << "," << item_y;

        int x = RoomMap::toMapPosX(item_x, map_bounding.x_offset);
        int y = RoomMap::toMapPosY(item_y, map_bounding.y_offset,map_bounding.height);

        cv::Point item_pos = RoomMap::getItemLinePos(Gplayer.curmapinfo.map_mat,{x, y});
        if(item_pos.x == -1){
            qDebug() << "无法转换平台坐标，已忽略";
            continue;
        }

        qDebug() << "物品坐标" << x << "," << y;

        int line_id = RoomMap::getPosPlatform(item_pos, Gplayer.curmapinfo.platform_list, Gplayer.curmapinfo.map_mat);

        qDebug() << "物品平台" << line_id;

        if(line_id == -1){
            qDebug() << "找不到平台，已忽略";
            continue;
        }

        qDebug() << "--- --- --- ---";

        target_item.pos = item_pos;
        target_item.entity = entity;
        target_item.line_id = line_id;
    }
}

void Data::Game::roomMapCollision()
{
    // 服务列表
    quint64 p_gamecontext = gamecontext();
    quint64 p_allServices = Mem::readPointer(p_gamecontext, BASE::GameContext::allServices);

    // 碰撞服务
    quint64 p_collisionservice = Helper::findService(p_allServices,"CollisionService");

    int simulator_count = readInt(readLong(p_collisionservice + BASE::CollisionService::simulatorByMap) + BASE::Dictionary::count);
    quint64 p_coolision = 0;
    quint64 p_coolision_head = readPointer(p_collisionservice, BASE::CollisionService::simulatorByMap,BASE::Dictionary::InternalArray_entries)
        +BASE::InternalArray::Head;

    // 找到当前地图的碰撞数组
    for(int i = 0; i < simulator_count; i++){
        p_coolision = readLong(p_coolision_head + i * 0x18 + 0x10);
        if(p_coolision != 0){
            break;
        }
    }

    if(p_coolision == 0){
        DebugOutput::out("找不到碰撞数据");
        return;
    }

    quint64 p_MultipleDynamicTreeBroadPhase = readLong(readLong(p_coolision + BASE::CollisionSimulator::World)+BASE::World::MultipleDynamicTreeBroadPhase);
    // DebugOutput::out("MultipleDynamicTreeBroadPhase:" + QString::number(p_MultipleDynamicTreeBroadPhase, 16).toUpper());

    // 遍历数组索引
    int index_count = readInt(readLong(p_MultipleDynamicTreeBroadPhase + BASE::MultipleDynamicTreeBroadPhase::numbersByProxyId) + BASE::Dictionary::count);
    quint64 index_head = readLong(readLong(p_MultipleDynamicTreeBroadPhase
                                           + BASE::MultipleDynamicTreeBroadPhase::numbersByProxyId)
                                  + BASE::Dictionary::InternalArray_entries)
                         + BASE::InternalArray::Head;
    std::set<std::pair<int, int>> uniquePairs;
    for(int index_i = 0; index_i < index_count; index_i++,index_head += 3 * 8 ){
        int index1 = readInt(index_head + 0xC); // 数组索引
        int index2 = readInt(index_head + 0x10); // 数组索引
        uniquePairs.insert({index1, index2});
    }

    // 遍历碰撞坐标原始数据
    int rect_array_2_count = readInt(readLong(readLong(p_MultipleDynamicTreeBroadPhase + BASE::MultipleDynamicTreeBroadPhase::trees) + 0x10) + 0x10);
    quint64 rect_head = readLong(p_MultipleDynamicTreeBroadPhase + BASE::MultipleDynamicTreeBroadPhase::trees) + BASE::InternalArray::Head;

    for (const auto& pair : uniquePairs) {
        quint64 cur_address = rect_head + pair.first * rect_array_2_count * 8 + pair.second * 8;
        int node_count = readInt(readLong(cur_address) + BASE::DynamicTree::_nodeCount);
        quint64 node_head = readLong(readLong(cur_address) + BASE::DynamicTree::_nodes) + BASE::InternalArray::Head;

        qDebug() << "=== === === === === ===";
        qDebug() << "碰撞组：" << QString::number(cur_address,16).toUpper();
        for(int node_i = 0; node_i < node_count; node_i++,node_head += 0x60){ // 0x60碰撞信息大小，暂时无法找到基址
            quint64 p_climbableComponent = readLong(node_head + 0x28); // 攀爬信息偏移，暂时无法找到基址
            if(!p_climbableComponent) continue;

            qDebug() << "--- --- ---";
            qDebug() << "p_coolision：" << QString::number(node_head,16).toUpper() << ":" << QString::number(p_climbableComponent,16).toUpper();

            ClimbOrigin cur_climb;
            cur_climb.left = readFloat(node_head + 0x30); // 攀爬信息
            cur_climb.top = readFloat(node_head + 0x34); // 攀爬信息
            cur_climb.factor1 = readFloat(node_head + 0x48); // 攀爬信息
            cur_climb.factor2 = readFloat(node_head + 0x4C); // 攀爬信息

            FloatRect float_rect;
            Math::ComputeClimbRect(&cur_climb, float_rect);
            qDebug() << "浮点碰撞位置rect：" << float_rect.x_left << "," << float_rect.y_top << " " << float_rect.x_right << "," << float_rect.y_bottom;

            ClimbRect cur_rect;
            // cur_rect.id = climb_id;

            cur_rect.x_left = RoomMap::toMapPosX(float_rect.x_left, Gplayer.curmapinfo.map_bounding.x_offset);
            cur_rect.x_right = RoomMap::toMapPosX(float_rect.x_right, Gplayer.curmapinfo.map_bounding.x_offset);
            cur_rect.y_top = RoomMap::toMapPosY(float_rect.y_top, Gplayer.curmapinfo.map_bounding.y_offset, Gplayer.curmapinfo.map_bounding.height);
            cur_rect.y_bottom = RoomMap::toMapPosY(float_rect.y_bottom, Gplayer.curmapinfo.map_bounding.y_offset, Gplayer.curmapinfo.map_bounding.height);

            qDebug() << "转换碰撞位置rect：" << cur_rect.x_left << "," << cur_rect.y_top << " " << cur_rect.x_right << "," << cur_rect.y_bottom;
        }
    }
}

void Data::Game::runeUISprites()
{
    quint64 ui_entity = mapEntity();


    quint64 alarm_group = Helper::findChild(ui_entity, "LocalUserAlarmGroup");
    quint64 rune_ui = Helper::findChild(alarm_group,"RuneUI");

    if(rune_ui == 0){
        DebugOutput::out("不存在符文封印");
        return;
    }

    DebugOutput::out("RuneUI:" + QString::number(rune_ui,16).toUpper());

    quint64 sprite1 = Helper::findChild(rune_ui,"Sprite1");
    quint64 sprite2 = Helper::findChild(rune_ui,"Sprite2");
    quint64 sprite3 = Helper::findChild(rune_ui,"Sprite3");
    quint64 sprite4 = Helper::findChild(rune_ui,"Sprite4");

    DebugOutput::out("Sprite1:" + QString::number(sprite1,16).toUpper());
    DebugOutput::out("Sprite2:" + QString::number(sprite2,16).toUpper());
    DebugOutput::out("Sprite3:" + QString::number(sprite3,16).toUpper());
    DebugOutput::out("Sprite4:" + QString::number(sprite4,16).toUpper());

    // 找到图片ID
    QString sprite1_dataid = readString(readLong(readLong(readLong(
        sprite1 + BASE::MODEntity::SpriteGUIRendererComponent)
        +BASE::SpriteGUIRendererComponent::MODDataRef)
        +BASE::MODDataRef::dataId) + BASE::String::text, 100);

    QString sprite2_dataid = readString(readLong(readLong(readLong(
        sprite2 + BASE::MODEntity::SpriteGUIRendererComponent)
        +BASE::SpriteGUIRendererComponent::MODDataRef)
        +BASE::MODDataRef::dataId) + BASE::String::text, 100);

    QString sprite3_dataid = readString(readLong(readLong(readLong(
        sprite3 + BASE::MODEntity::SpriteGUIRendererComponent)
        +BASE::SpriteGUIRendererComponent::MODDataRef)
        +BASE::MODDataRef::dataId) + BASE::String::text, 100);

    QString sprite4_dataid = readString(readLong(readLong(readLong(
        sprite4 + BASE::MODEntity::SpriteGUIRendererComponent)
        +BASE::SpriteGUIRendererComponent::MODDataRef)
        +BASE::MODDataRef::dataId) + BASE::String::text, 100);

    DebugOutput::out("sprite1_dataid:" + sprite1_dataid);
    DebugOutput::out("sprite2_dataid:" + sprite2_dataid);
    DebugOutput::out("sprite3_dataid:" + sprite3_dataid);
    DebugOutput::out("sprite4_dataid:" + sprite4_dataid);
}























