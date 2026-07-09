#include "helper.h"
#include "driver/mem.h"
#include <set>
#include <utility>  // for std::pair
#include <unordered_set>
#include "global/gameoffset.h"
#include "global/globalplayer.h"
#include <QElapsedTimer>
#include "utils/mapserializer.h"
#include "mainwindow.h"
#include "utils/gamestruct.h"
#include "driver/msdk/msdkDriver.h"

using namespace Mem;

void DebugOutput::out(const QString& message){
    QString prefix = "[myprint] ";
    QString formattedMsg = prefix + message;
    QString output = formattedMsg + "\n";

    // 输出到 DebugView
    OutputDebugStringW(output.toStdWString().c_str());
}

Helper::Helper()
{
    connect(this, &Helper::sendWarning, MainWindow::getInstance(), &MainWindow::warning,Qt::UniqueConnection);
}

void Helper::console(const QString &msg)
{
    // MainWindow::getInstance()->console(msg);
    QMetaObject::invokeMethod(MainWindow::getInstance(), [msg]() {
        MainWindow::getInstance()->console(msg);
    }, Qt::QueuedConnection);
}

void Helper::warning(QString msg)
{
    Gplayer.show_warning = true;
    emit sendWarning(msg);
    // while(Gplayer.show_warning){
    //     DebugOutput::out("警报播放中");
    //     ProcessHelper::programDelay(1000);
    //     QApplication::processEvents();
    // }
}

void Helper::warningSound()
{
    Beep(800, 500); // 播放 800Hz 的声音，持续 0.5 秒
}

quint64 Helper::findChild(quint64 parent, QString name)
{
    quint64 child_entity = 0;

    quint64 list = Mem::readLong(parent + BASE::MODEntity::children);
    int list_num = Mem::readLong(list + BASE::List::size);
    if(list_num == 0){
        // qDebug() << "不存在子模块" << name;
        return child_entity;
    }

    quint64 list_head = Mem::readLong(list + BASE::List::InternalArray);
    int max = Mem::readInt(list_head + BASE::InternalArray::max_size);
    list_head = list_head + BASE::InternalArray::Head;
    for(int i = 0; i < max; i++,list_head += 0x8){
        quint64 cur_entity = Mem::readLong(list_head);
        if(cur_entity == 0){
            break;
        }

        QString entity_name = readString(readLong(cur_entity + BASE::MODEntity::name) + BASE::String::text, 100);
        // qDebug() << "当前模块:" << entity_name;
        if(entity_name == name){
            return cur_entity;
        }
    }

    return child_entity;
}

quint64 Helper::findService(quint64 allservice, QString name)
{
    quint64 service_pointer = 0;

    int count = readInt(allservice + BASE::List::size);
    quint64 head = readLong(allservice + BASE::List::InternalArray) + BASE::InternalArray::Head;
    for(int i = 0; i < count; i++){
        quint64 cur_service = readLong(head);
        if(!cur_service) break;

        QString cur_name = readAnsiString(readLong(readLong(cur_service) + 0x10), 100);
        if(cur_name == name){
            return cur_service;
        }

        head += 8;
    }

    return service_pointer;
}

std::unordered_set<quint64> RoomMap::findClimbableComponentSet(quint64 cur_map)
{
    std::unordered_set<quint64> pointers;

    // QString map_uuid = readString(readLong(cur_map + 0x2A0)+0x14, 100);
    // qDebug() << "当前地图UUID：" << map_uuid;

    int list_count = readInt(readLong(cur_map + BASE::MODEntity::children) + BASE::List::size);
    quint64 list_p = readLong(readLong(cur_map + BASE::MODEntity::children) + BASE::List::InternalArray);
    quint64 list_head = list_p + BASE::InternalArray::Head;

    for(int i = 0; i < list_count; i++,list_head += 8){
        quint64 cur_p = readLong(list_head);
        if(!cur_p) continue;

        quint64 ClimbableComponent = readLong(cur_p + BASE::MODEntity::ClimbableComponent);
        if(!ClimbableComponent) continue;

        pointers.insert(ClimbableComponent);
    }

    return pointers;
}

MapBounding RoomMap::getMapBounding(quint64 cur_map)
{
    // 获取地图边界
    quint64 p_MapComponent = Mem::readLong(cur_map + BASE::MODEntity::MapComponent);
    float map_x_left = Mem::readFloat(p_MapComponent + BASE::MapComponent::MoveBoundingMin);
    float map_y_bottom = Mem::readFloat(p_MapComponent + BASE::MapComponent::MoveBoundingMin + 4);
    float map_x_right = Mem::readFloat(p_MapComponent + BASE::MapComponent::MoveBoundingMax);
    float map_y_top = Mem::readFloat(p_MapComponent + BASE::MapComponent::MoveBoundingMax + 4);
    // qDebug() << "地图边界:" << map_x_left << "," << map_y_bottom << " " << map_x_right << "," << map_y_top;

    map_x_left = Math::roundHalfAwayFromZero_bit(map_x_left);
    map_y_bottom = Math::roundHalfAwayFromZero_bit(map_y_bottom);
    map_x_right = Math::roundHalfAwayFromZero_bit(map_x_right);
    map_y_top = Math::roundHalfAwayFromZero_bit(map_y_top);

    // 坐标转换
    int x_offset = 0,y_offset = 0;
    if(map_x_left < 0){
        x_offset = abs(map_x_left * 100);
    }
    if(map_y_bottom < 0){
        y_offset = abs(map_y_bottom * 100);
    }

    // 地图宽度
    uint map_width = map_x_right * 100 + x_offset + 1; // 加1是因为坐标从0开始计算的。+1才是真实长度
    uint map_height = map_y_top * 100 + y_offset + 1; // 同时也是Y坐标转换的计算因子

    int x_left = toMapPosX(map_x_left,x_offset);
    int x_right = toMapPosX(map_x_right,x_offset);
    int y_top = toMapPosY(map_y_top, y_offset, map_height - 1);
    int y_bottom = toMapPosY(map_y_bottom, y_offset, map_height - 1);

    MapBounding map_bounding;
    map_bounding.x_offset = x_offset;
    map_bounding.y_offset = y_offset;
    map_bounding.x_left = x_left;
    map_bounding.x_right = x_right;
    map_bounding.y_top = y_top;
    map_bounding.y_bottom = y_bottom;
    map_bounding.width = map_width;
    map_bounding.height = map_height;

    return map_bounding;
}

QString RoomMap::getMapName()
{
    quint64 ui_entity = Gplayer.p_uientity;
    quint64 main_group = Helper::findChild(ui_entity, "MainGroup");
    quint64 minimap = Helper::findChild(main_group,"Minimap");
    quint64 background = Helper::findChild(minimap,"Minimap_Background");
    quint64 scroll_top = Helper::findChild(background,"Scroll_Top");
    quint64 middle = Helper::findChild(scroll_top,"Middle");
    quint64 map_name_p = Helper::findChild(middle,"Minimap_MinText");

    QString map_name = GameStruct::readEntityText(map_name_p);
    return map_name;
}

int RoomMap::toMapPosX(float x, int offset)
{
    return x * 100 + offset;
}

int RoomMap::toMapPosY(float y, int offset, int height)
{
    return height - (y * 100 + offset) - 1;
}

QString Helper::getEntityTextComponentString(quint64 entity)
{
    QString name = "";
    name = readString(readLong(readLong(entity + BASE::MODEntity::TextComponent) + BASE::TextComponent::_text) + BASE::String::text, 100);
    return name;
}

std::vector<ClimbRect> RoomMap::makeClimbList(quint64 cur_map, quint64 collisionservice, MapBounding map_bounding)
{
    std::vector<ClimbRect> climb_list;

    // 遍历地图，搜索所有的攀爬Entity
    std::unordered_set<quint64> climb_pointers = findClimbableComponentSet(cur_map);

    // BASE::InternalArray::Head + 0x20, // 这里的数据是这样：
    /**
     * +0  固定FF开头
     * +8  是地图名称字符串
     * +10 是地图碰撞数据（梯子，人物，怪物，技能等碰撞信息）
     * +18 下一个元素
     * 一个数据结构是3*8大小
     *
    * head+0  FFFFFFFF6E8FA80C head+8  0000023992DD7D80
    * head+10 00000239A7605C00 head+18 FFFFFFFFFFFFFFFF
    **/
    // 找碰撞数据

    int simulator_count = readInt(readLong(collisionservice + BASE::CollisionService::simulatorByMap) + BASE::Dictionary::count);
    quint64 p_coolision = 0;
    quint64 p_coolision_head = readPointer(collisionservice,
                                            BASE::CollisionService::simulatorByMap,
                                            BASE::Dictionary::InternalArray_entries) +
                                            BASE::InternalArray::Head;
    for(int i = 0; i < simulator_count; i++){
        p_coolision = readLong(p_coolision_head + i * 0x18 + 0x10);
        if(p_coolision != 0){
            break;
        }
    }

    if(p_coolision == 0){
        DebugOutput::out("找不到碰撞数据");
        return climb_list;
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

    // int climb_id = 0; // 生成一个ID
    for (const auto& pair : uniquePairs) {
        quint64 cur_address = rect_head + pair.first * rect_array_2_count * 8 + pair.second * 8;
        int node_count = readInt(readLong(cur_address) + BASE::DynamicTree::_nodeCount);
        quint64 node_head = readLong(readLong(cur_address) + BASE::DynamicTree::_nodes) + BASE::InternalArray::Head;

        // qDebug() << "--------------------------------------------------------";
        for(int node_i = 0; node_i < node_count; node_i++,node_head += 0x60){ // 0x60碰撞信息大小，暂时无法找到基址
            quint64 p_climbableComponent = readLong(node_head + 0x28); // 攀爬信息偏移，暂时无法找到基址
            if(!p_climbableComponent) continue;

            // qDebug() << "p_climbableComponent：" << QString::number(p_climbableComponent,16).toUpper();
            // 如果不是当前地图的攀爬物体,则跳过
            if (climb_pointers.find(p_climbableComponent) == climb_pointers.end()) {
                break;
            }
            ClimbOrigin cur_climb;
            cur_climb.left = readFloat(node_head + 0x30); // 攀爬信息
            cur_climb.top = readFloat(node_head + 0x34); // 攀爬信息
            cur_climb.factor1 = readFloat(node_head + 0x48); // 攀爬信息
            cur_climb.factor2 = readFloat(node_head + 0x4C); // 攀爬信息

            FloatRect float_rect;
            Math::ComputeClimbRect(&cur_climb, float_rect);


            ClimbRect cur_rect;
            // cur_rect.id = climb_id;

            cur_rect.x_left = toMapPosX(float_rect.x_left, map_bounding.x_offset);
            cur_rect.x_right = toMapPosX(float_rect.x_right, map_bounding.x_offset);
            cur_rect.y_top = toMapPosY(float_rect.y_top, map_bounding.y_offset, map_bounding.height);
            cur_rect.y_bottom = toMapPosY(float_rect.y_bottom, map_bounding.y_offset, map_bounding.height);

            climb_list.push_back(cur_rect);
            // climb_id++;
        }
    }

    return climb_list;
}

std::vector<FootholdRect> RoomMap::makeFootholdList(quint64 cur_map, MapBounding map_bounding)
{
    std::vector<FootholdRect> foothold_list;

    quint64 p_footholds = readPointer(cur_map,
                                      BASE::MODEntity::MapComponent,
                                      BASE::MapComponent::FootholdComponent,
                                      BASE::FootholdComponent::FootholdImpl,
                                      BASE::FootholdImpl::footHoldList,
                                      BASE::List::InternalArray);
    // qDebug() << "地板指针：" << QString::number(p_footholds,16).toUpper();
    int max_count = readInt(p_footholds + BASE::InternalArray::max_size);
    quint64 foot_head = p_footholds + BASE::InternalArray::Head;

    for(int i = 0; i < max_count; i++,foot_head +=8){
        if(readLong(foot_head) == 0) break;
        // int group = Mem::readInt(readLong(foot_head) + BASE::Foothold::groupID);
        // qDebug() << "group:" << group;

        int id = readInt(readLong(foot_head) + BASE::Foothold::ID);
        float x_left = Math::roundHalfAwayFromZero_bit(readFloat(readLong(foot_head) + BASE::Foothold::StartPoint));
        float y_left = Math::roundHalfAwayFromZero_bit(readFloat(readLong(foot_head) + BASE::Foothold::StartPoint + 4));
        float x_right = Math::roundHalfAwayFromZero_bit(readFloat(readLong(foot_head) + BASE::Foothold::EndPoint));
        float y_right = Math::roundHalfAwayFromZero_bit(readFloat(readLong(foot_head) + BASE::Foothold::EndPoint + 4));
        // qDebug() << "Float数据__footid:" << id << " " << x_left << "," << y_left << " " << x_right << "," << y_right;
        // qDebug() << "------------------------";

        int cur_x_left = toMapPosX(x_left,map_bounding.x_offset);
        int cur_y_left = toMapPosY(y_left,map_bounding.y_offset,map_bounding.height);
        int cur_x_right = toMapPosX(x_right,map_bounding.x_offset);
        int cur_y_right = toMapPosY(y_right,map_bounding.y_offset,map_bounding.height);

        if(cur_x_left < map_bounding.x_left){
            cur_x_left = map_bounding.x_left;
        }
        if(cur_x_left > map_bounding.x_right){
            cur_x_left = map_bounding.x_right;
        }
        if(cur_x_right < map_bounding.x_left){
            cur_x_right = map_bounding.x_left;
        }
        if(cur_x_right > map_bounding.x_right){
            cur_x_right = map_bounding.x_right;
        }
        if(cur_y_left < map_bounding.y_top){
            cur_y_left = map_bounding.y_top;
        }
        if(cur_y_left > map_bounding.y_bottom){
            cur_y_left = map_bounding.y_bottom;
        }
        if(cur_y_right < map_bounding.y_top){
            cur_y_right = map_bounding.y_top;
        }
        if(cur_y_right > map_bounding.y_bottom){
            cur_y_right = map_bounding.y_bottom;
        }

        if(cur_x_left > cur_x_right){
            uint tmp_x = cur_x_left;
            uint tmp_y = cur_y_left;
            cur_x_left = cur_x_right;
            cur_y_left = cur_y_right;
            cur_x_right = tmp_x;
            cur_y_right = tmp_y;
        }

        // // 过滤掉不需要的墙体
        // if(cur_x_left == 230 && cur_x_right == 230){
        //     continue;
        // }

        // if(cur_x_left == 1220 && cur_x_right == 1220){
        //     continue;
        // }
        // qDebug() << "Int数据__footid:" << id << " " << cur_x_left << "," << cur_y_left << " " << cur_x_right << "," << cur_y_right;
        // qDebug() << "黑色:" << cur_x_left << "," << cur_y_left << " " << cur_x_right << "," << cur_y_right;
        foothold_list.push_back({id,cur_x_left,cur_y_left,cur_x_right,cur_y_right});
    }

    return foothold_list;
}

std::vector<Wall> RoomMap::makeWallList(const std::vector<FootholdRect> &footholds, std::unordered_set<int> &id_set)
{
    std::vector<Wall> wall_list;
    for (auto item : footholds) {
        if (id_set.find(item.id) == id_set.end()) {
            continue;
        }

        // 如果item是一个墙体(x相同，y不同)
        if (item.x_left == item.x_right && item.y_left != item.y_right) {

            // 过虑墙体
            // if(item.x_left == 231 || item.x_left == 1221 || item.x_left == 1446){
            //     continue;
            // }

            Wall wall;
            wall.foot_id = item.id;
            wall.x = item.x_left;
            wall.bottom = std::max(item.y_left, item.y_right);
            wall.top = std::min(item.y_left, item.y_right);
            wall_list.push_back(wall);
            // qDebug() << wall.foot_id << wall.x << wall.top << wall.bottom;
            id_set.erase(item.id);
            continue;
        }

        // 如果是一个斜墙
        if(item.x_left != item.x_right && item.y_left != item.y_right){
            int height = abs(item.y_right - item.y_left);
            if(height > 15){
                int width = item.x_right - item.x_left;
                int radio = 1.0 * width / height * 100;
                if(radio < 30){
                    // qDebug() << "墙体：" << item.x_left << "," << item.y_left << item.x_right << "," << item.y_right << "radio" << radio;
                    Wall wall;
                    wall.foot_id = item.id;
                    wall.x = item.x_left;
                    wall.bottom = std::max(item.y_left, item.y_right);
                    wall.top = std::min(item.y_left, item.y_right);
                    wall_list.push_back(wall);
                    // qDebug() << wall.foot_id << wall.x << wall.top << wall.bottom;

                    wall.x = item.x_right;
                    wall_list.push_back(wall); // 插入第二个墙
                    id_set.erase(item.id);
                }
            }
        }
    }

    return wall_list;
}

std::vector<Platform> RoomMap::makePlatformList(
    const std::vector<FootholdRect> &footholds,
    const std::vector<Wall> &wall_list,
    const std::vector<ClimbRect> &climb_list,
    MapBounding map_bounding,
    std::unordered_set<int> &id_set)
{
    // std::vector<Line> line_list;
    std::vector<Platform> platform_list;

    int platform_id = 0;

    // 先处理地板列表，再处理攀爬体
    for (auto item : footholds) {
        // 如果元素x的id不存在于id_set中，则进入下一层循环
        if (id_set.find(item.id) == id_set.end()) {
            continue;
        }

        // 将该item的id从id_set里面移除
        id_set.erase(item.id);

        // 如果是直线（y相同）或者是斜线（x_left与x_right不同且y不同）
        bool is_horizon = item.y_left == item.y_right;
        bool is_slash = item.x_left != item.x_right && item.y_left != item.y_right;
        if(!is_horizon && !is_slash){
            continue;
        }

        if(item.x_left > item.x_right){
            qDebug() << "数据不正确，退出计算~";
            platform_list.clear();
            return platform_list;
        }

        // Line tmp_line;
        Platform tmp_platform;
        tmp_platform.hasLeftWall = false;
        tmp_platform.hasRightWall = false;

        // 创建第一个line_child
        LineChild first_child;
        first_child.foot_id = item.id;
        first_child.start = cv::Point(item.x_left,item.y_left);
        first_child.end = cv::Point(item.x_right,item.y_right);
        if(is_slash){
            int dx = item.x_right - item.x_left;
            int dy = item.y_right - item.y_left;
            first_child.length = static_cast<int>(std::sqrt(dx*dx + dy*dy));
        }else{
            first_child.length = qAbs(item.x_right - item.x_left) + qAbs(item.y_right - item.y_left);
        }

        tmp_platform.type = 0; // 地板平台
        tmp_platform.line_child.push_back(first_child);
        tmp_platform.start = cv::Point(item.x_left,item.y_left);
        tmp_platform.end = cv::Point(item.x_right,item.y_right);
        tmp_platform.hasLeftWall = false;
        tmp_platform.hasRightWall = false;

        // 合并地板
        for (auto& other_item : footholds) {
            if (id_set.find(other_item.id) == id_set.end()) {
                continue;
            }

            bool other_is_horizon = other_item.y_left == other_item.y_right;
            bool other_is_slash = other_item.x_left != other_item.x_right && other_item.y_left != other_item.y_right;
            if(!other_is_horizon && !other_is_slash){
                continue;
            }

            // 拼接当前平台左边的地板
            // if (cv::Point(other_item.x_right,other_item.y_right) == tmp_platform.start) {
            if (other_item.y_right == tmp_platform.start.y
                && other_item.x_left < tmp_platform.start.x
                && other_item.x_right >= tmp_platform.start.x
            ) {
                id_set.erase(other_item.id);

                LineChild child;
                child.foot_id = other_item.id;
                child.start = cv::Point(other_item.x_left,other_item.y_left);
                child.end = cv::Point(other_item.x_right,other_item.y_right);
                if(other_is_slash){
                    int dx = other_item.x_right - other_item.x_left;
                    int dy = other_item.y_right - other_item.y_left;
                    first_child.length = static_cast<int>(std::sqrt(dx*dx + dy*dy));
                }else{
                    child.length = qAbs(other_item.x_right - other_item.x_left) + qAbs(other_item.y_right - other_item.y_left);
                }

                // 将line_child插入到容器tmp_platform.line_child前面
                tmp_platform.line_child.insert(tmp_platform.line_child.begin(), child);
                tmp_platform.start = cv::Point(other_item.x_left,other_item.y_left);
                continue;
            }

            // 拼接当前平台右边的地板
            // if (cv::Point(other_item.x_left,other_item.y_left) == tmp_platform.end) {
            if (other_item.y_left == tmp_platform.end.y
                && other_item.x_right > tmp_platform.end.x
                && other_item.x_left <= tmp_platform.end.x
                ) {
                id_set.erase(other_item.id);

                LineChild child;
                child.foot_id = other_item.id;
                child.start = cv::Point(other_item.x_left,other_item.y_left);
                child.end = cv::Point(other_item.x_right,other_item.y_right);
                if(other_is_slash){
                    int dx = other_item.x_right - other_item.x_left;
                    int dy = other_item.y_right - other_item.y_left;
                    first_child.length = static_cast<int>(std::sqrt(dx*dx + dy*dy));
                }else{
                    child.length = qAbs(other_item.x_right - other_item.x_left) + qAbs(other_item.y_right - other_item.y_left);
                }

                // 将line_child插入到容器tmp_platform.line_child后面
                tmp_platform.line_child.push_back(child);
                tmp_platform.end = cv::Point(other_item.x_right,other_item.y_right);
                continue;
            }
        }

        // 注意：需要确保start的x最小
        if (tmp_platform.start.x > tmp_platform.end.x) {
            qDebug() << "数据不正确2，退出计算~";
            platform_list.clear();
            return platform_list;
        }

        // 创建平台的墙体属性
        for (const auto& wall : wall_list) {
            // 检查左墙
            if (tmp_platform.start.x <= 0 || (wall.x == tmp_platform.start.x && wall.bottom == tmp_platform.start.y)) {
                tmp_platform.hasLeftWall = true;
            }
            // 检查右墙
            if (tmp_platform.end.x >= map_bounding.x_right || (wall.x == tmp_platform.end.x && wall.bottom == tmp_platform.end.y)) {
                tmp_platform.hasRightWall = true;
            }
        }

        tmp_platform.id = platform_id;
        platform_id++;
        platform_list.push_back(tmp_platform);

    }

    // for (auto item : climb_list) {
    //     Platform tmp_platform;
    //     tmp_platform.type = 1; // 攀爬体
    //     tmp_platform.hasLeftWall = false;
    //     tmp_platform.hasRightWall = false;

    //     tmp_platform.start = {item.x_left, item.y_top};
    //     tmp_platform.end = {item.x_right, item.y_bottom};

    //     tmp_platform.id = platform_id;
    //     platform_id++;
    //     platform_list.push_back(tmp_platform);
    // }

    // 1. 将所有地板ID放入到容器 footid_set里面
    struct ClimbPlat{
        int id;
        ClimbRect rect;
    };

    std::vector<ClimbPlat> climbplat_list;
    std::unordered_set<int> climb_set;
    int climbplat_index = 0;
    for (auto const item : climb_list) {
        climbplat_list.push_back({climbplat_index,item});
        climb_set.insert(climbplat_index);
        climbplat_index++;
    }

    for (auto item : climbplat_list) {
        if (climb_set.find(item.id) == climb_set.end()) {
            continue;
        }

        climb_set.erase(item.id);

        Platform tmp_platform;
        tmp_platform.type = 1; // 攀爬体
        tmp_platform.hasLeftWall = false;
        tmp_platform.hasRightWall = false;

        // 创建第一个line_child
        LineChild first_child;
        first_child.foot_id = -1;
        first_child.start = cv::Point(item.rect.x_left,item.rect.y_top);
        first_child.end = cv::Point(item.rect.x_right,item.rect.y_bottom);
        first_child.length = qAbs(item.rect.y_bottom - item.rect.y_top);
        tmp_platform.line_child.push_back(first_child);

        tmp_platform.start = {item.rect.x_left, item.rect.y_top};
        tmp_platform.end = {item.rect.x_right, item.rect.y_bottom};
        tmp_platform.id = platform_id;

        retryCombine:
        for (auto& other_item : climbplat_list) {

            if (climb_set.find(other_item.id) == climb_set.end()) {
                continue;
            }

            // 如果梯子重叠
            if( Math::hasIntersection(tmp_platform.start.x,tmp_platform.end.x,other_item.rect.x_left,other_item.rect.x_right) &&
                Math::hasIntersection(tmp_platform.start.y,tmp_platform.end.y,other_item.rect.y_top,other_item.rect.y_bottom)
            )
            {
                climb_set.erase(other_item.id);

                LineChild child;
                child.foot_id = -1;
                child.start = cv::Point(other_item.rect.x_left,other_item.rect.y_top);
                child.end = cv::Point(other_item.rect.x_right,other_item.rect.y_bottom);
                child.length = qAbs(other_item.rect.y_bottom - other_item.rect.y_top);
                tmp_platform.line_child.push_back(child);

                // 拼接上面的梯子
                if(other_item.rect.y_top < tmp_platform.start.y && other_item.rect.y_bottom >= tmp_platform.start.y)
                {
                    tmp_platform.start = cv::Point(other_item.rect.x_left,other_item.rect.y_top);
                }

                // 拼接下面的梯子
                if(other_item.rect.y_bottom > tmp_platform.end.y && other_item.rect.y_top <= tmp_platform.end.y)
                {
                    tmp_platform.end = cv::Point(other_item.rect.x_right,other_item.rect.y_bottom);
                }

                // 重新检测一遍
                goto retryCombine;
            }
        }

        platform_id++;
        platform_list.push_back(tmp_platform);
    }


    return platform_list;
}

MapPlat RoomMap::makeRoomMap(
    const std::vector<Platform> &platform_list,
    const std::vector<ClimbRect> &climb_list,
    const std::vector<FootholdRect> &foothold_list,
    const std::vector<Wall> &wall_list,
    const MapBounding &map_bounding)
{
    // 创建地图文件
    cv::Mat cur_room_map(map_bounding.height, map_bounding.width, CV_8UC3, COLOR_AIR);

    // 第一维：x（width），第二维：y（height）
    std::vector<std::vector<PlatCoor>> plat_map_coor(
        map_bounding.width,
        std::vector<PlatCoor>(map_bounding.height, {-1, -1, -1, -1}));

    // 填充攀爬
    for(const auto& item : climb_list){
        int x_width = item.x_right - item.x_left;
        int y_height = item.y_bottom - item.y_top;
        cv::Rect rect(item.x_left, item.y_top, x_width, y_height);

        int platform_id = -1;
        for(const auto& platform : platform_list) {
            bool find_id = false;
            for(const auto& child : platform.line_child){
                if(child.start.x == item.x_left && child.end.x == item.x_right &&
                    child.start.y == item.y_top && child.end.y == item.y_bottom){
                    platform_id = platform.id;
                    find_id = true;
                    break;
                }
            }
            if(find_id){
                break;
            }
        }

        for(int y = rect.y; y < rect.y + rect.height; y++) {
            for(int x = rect.x; x < rect.x + rect.width; x++) {
                try {
                    // 使用 [x][y] 访问
                    plat_map_coor.at(x).at(y).platform_id = platform_id;
                    plat_map_coor.at(x).at(y).type = 1;
                    plat_map_coor.at(x).at(y).x = x;
                    plat_map_coor.at(x).at(y).y = y;
                    cur_room_map.at<cv::Vec3b>(y, x) = COLOR_CLIMB;
                } catch (const std::out_of_range& e) {
                    continue;
                }
            }
        }
    }

    // 绘制地板
    for(const auto& item : foothold_list){
        if(item.x_left == item.x_right){
            continue;
        }

        int platform_id = -1;
        for(const auto& platform : platform_list) {
            // 缩小X轴范围
            if( item.x_left < platform.start.x || item.x_right > platform.end.x){
                continue;
            }
            // 搜索地板列表
            bool find_id = false;
            for(const auto& child : platform.line_child){
                if(item.id == child.foot_id){
                    platform_id = platform.id;
                    find_id = true;
                    break;
                }
            }
            if(find_id){
                break;
            }
        }

        int connectivity = (item.y_left != item.y_right) ? 4 : 8;
        cv::LineIterator iterator(cur_room_map,
                                  cv::Point(item.x_left, item.y_left),
                                  cv::Point(item.x_right, item.y_right),
                                  connectivity);

        for(int i = 0; i < iterator.count; i++, ++iterator) {
            cv::Point pt = iterator.pos();
            try {
                // 使用 [x][y] 访问
                plat_map_coor.at(pt.x).at(pt.y).platform_id = platform_id;
                plat_map_coor.at(pt.x).at(pt.y).type = 0;
                plat_map_coor.at(pt.x).at(pt.y).x = pt.x;
                plat_map_coor.at(pt.x).at(pt.y).y = pt.y;
                cur_room_map.at<cv::Vec3b>(pt.y, pt.x) = COLOR_LINE;
            } catch (const std::out_of_range& e) {
                continue;
            }
        }
    }

    // 绘制墙体
    for(const auto& item : wall_list){
        cv::LineIterator iterator(cur_room_map,
                                  cv::Point(item.x, item.top),
                                  cv::Point(item.x, item.bottom),
                                  8);
        for(int i = 0; i < iterator.count; i++, ++iterator) {
            cv::Point pt = iterator.pos();
            try {
                // 使用 [x][y] 访问
                plat_map_coor.at(pt.x).at(pt.y).type = 2;
                plat_map_coor.at(pt.x).at(pt.y).x = pt.x;
                plat_map_coor.at(pt.x).at(pt.y).y = pt.y;
                cur_room_map.at<cv::Vec3b>(pt.y, pt.x) = COLOR_WALL;
            } catch (const std::out_of_range& e) {
                continue;
            }
        }
    }

    MapPlat result;
    result.room_map = cur_room_map;
    result.plat_map = std::move(plat_map_coor);
    return result;
}

std::unordered_map<int, PathNode> RoomMap::makeNodes(
    const std::vector<Platform> &platform_list,
    const std::vector<ClimbRect> &climb_list,
    const std::vector<Wall> &wall_list,
    const cv::Mat &cur_room_map,
    const MapBounding &map_bounding
)
{
    // 算法开始时间
    QElapsedTimer begin_time;
    begin_time.start();

    std::unordered_map<int, PathNode> path_nodes;
    path_nodes.reserve(platform_list.size());
    for(const auto &platform : platform_list){
        path_nodes[platform.id].id = platform.id;
        path_nodes[platform.id].type = platform.type;
    }

    // 可以将地图中的每个像素坐标映射到平台

    PathAlgo::walkToLeft(platform_list, cur_room_map, path_nodes);

    PathAlgo::walkToRight(platform_list, cur_room_map, map_bounding, path_nodes);

    PathAlgo::jumpLeft(platform_list, cur_room_map, path_nodes);

    PathAlgo::jumpRight(platform_list, cur_room_map, path_nodes);

    PathAlgo::jump_up(platform_list, cur_room_map, path_nodes);

    PathAlgo::jump_down(platform_list, climb_list, cur_room_map, map_bounding, path_nodes);

    PathAlgo::up(platform_list, cur_room_map, path_nodes);

    PathAlgo::down(platform_list, cur_room_map, map_bounding, path_nodes);

    // 对节点结果再次过滤
    for(auto &node : path_nodes){
        for(auto &target : node.second.targets){
            // 如果是平台到平台
            if(node.second.type == 0 && target.second.type == 0){
                // 存在jumpdown，jumpup,walk。就直接删掉jumpleft，right
                bool is_exists = false;
                for(const auto &visit : target.second.visit_list){
                    if( visit.visit_type == JUMP_UP ||
                        visit.visit_type == JUMP_DOWN ||
                        visit.visit_type == WALK_LEFT ||
                        visit.visit_type == WALK_RIGHT
                        ){
                        is_exists = true;
                    }
                }
                if(is_exists){
                    for(auto it = target.second.visit_list.begin(); it != target.second.visit_list.end(); ) {
                        if(it->visit_type == JUMP_LEFT || it->visit_type == JUMP_RIGHT){
                            it = target.second.visit_list.erase(it);
                        }else{
                            ++it;
                        }
                    }
                }
            }

            // 如果是平台到攀爬物体
            if(node.second.type == 0 && target.second.type == 1)
            {
                // 存在down的时候，直接删掉jumpleft，right,jumpup
                bool is_exists = false;
                for(const auto &visit : target.second.visit_list){
                    if(visit.visit_type == DOWN) {
                        is_exists = true;
                    }
                }
                if(is_exists){
                    for(auto it = target.second.visit_list.begin(); it != target.second.visit_list.end(); ) {
                        if( it->visit_type == JUMP_LEFT ||
                            it->visit_type == JUMP_RIGHT ||
                            it->visit_type == JUMP_UP
                        ){
                            it = target.second.visit_list.erase(it);
                        }else{
                            ++it;
                        }
                    }
                }

                // 存在jumpup的时候，直接删掉jumpleft，right
                // 但是雪域地图不能这样，否则会一直跳不上去
                is_exists = false;
                for(const auto &visit : target.second.visit_list){
                    if(visit.visit_type == JUMP_UP) {
                        is_exists = true; // 如果是雪域地图，则注释该行
                    }
                }
                if(is_exists){
                    for(auto it = target.second.visit_list.begin(); it != target.second.visit_list.end(); ) {
                        if( it->visit_type == JUMP_LEFT ||
                            it->visit_type == JUMP_RIGHT
                            ){
                            it = target.second.visit_list.erase(it);
                        }else{
                            ++it;
                        }
                    }
                }
            }


            // 如果是攀爬物到平台：
            if(node.second.type == 1 && target.second.type == 0){
                // 存在up，down直接删掉jumpleft,right
                bool is_exists = false;
                for(const auto &visit : target.second.visit_list){
                    if( visit.visit_type == UP || visit.visit_type == DOWN) {
                        is_exists = true;
                    }
                }
                if(is_exists){
                    for(auto it = target.second.visit_list.begin(); it != target.second.visit_list.end(); ) {
                        if(it->visit_type == JUMP_LEFT || it->visit_type == JUMP_RIGHT){
                            it = target.second.visit_list.erase(it);
                        }else{
                            ++it;
                        }
                    }
                }
            }
        }
    }

    quint64 total_time = begin_time.elapsed();
    qDebug() << "平台寻路算法用时:" << total_time;

    return path_nodes;
}

bool Helper::parseHPString(const QString &Text, int &left, int &right) {
    QStringList parts = Text.split("/");
    if (parts.size() == 2) {
        left = parts[0].trimmed().toInt();
        right = parts[1].trimmed().toInt();
        return true;
    }
    return false;
}

int Helper::coinToInt(QString coid)
{
    QString cleanStr = coid;
    cleanStr.remove(',');  // 移除所有逗号
    return cleanStr.toInt();
}

bool Helper::isSamePos(const std::pair<float,float>& a,
               const std::pair<float,float>& b)
{
    const float eps = 0.001f;
    return fabs(a.first - b.first) < eps &&
           fabs(a.second - b.second) < eps;
}

void Helper::posExists(std::vector<std::pair<float, float> > &move_pos_list, std::pair<float, float> pos)
{
    // if(std::find(move_pos_list.begin(), move_pos_list.end(), pos) != move_pos_list.end()){
    //     return;
    // }

    auto it = std::find_if(move_pos_list.begin(), move_pos_list.end(),
                           [&](const std::pair<float,float>& p)
                           {
                               return isSamePos(p, pos);
                           });

    if (it != move_pos_list.end())
        return;

    move_pos_list.push_back(pos);
}

// std::vector<LineSegment> Helper::mergeFootPoint(const std::vector<TargetPoints> &target_list)
std::unordered_map<int, std::vector<LineSegment>> Helper::mergeFootPoint(
    const std::unordered_map<int, std::vector<TargetPoints>> &target_list
)
{
    // std::vector<LineSegment> line_set;
    std::unordered_map<int, std::vector<LineSegment>> line_set;
    for(const auto &target : target_list){
        int platform_count = target.second.size();
        line_set[target.first].reserve(platform_count);

        for(int platform_id = 0; platform_id < platform_count; platform_id++) {

            // qDebug() << "到达线路：" << platform_id;

            TargetPoints points = target.second.at(platform_id);
            if (points.empty()) {
                line_set[target.first].emplace_back(); // 添加空段
                continue;
            }

            // 提取并排序所有x坐标
            std::vector<int> x_coords;
            x_coords.reserve(points.size());
            for (const auto& pt : points) {
                x_coords.push_back(pt.x);
            }
            std::sort(x_coords.begin(), x_coords.end());

            // 合并相邻的x坐标（差值 <= 1）
            LineSegment segments;
            int start_x = x_coords[0];
            int end_x = x_coords[0];

            for (size_t i = 1; i < x_coords.size(); ++i) {
                if (x_coords[i] - x_coords[i-1] <= 3) {
                    // 相邻，扩展当前段
                    end_x = x_coords[i];
                } else {
                    // 不连续，保存当前段，开始新段
                    segments.emplace_back(start_x, end_x);
                    start_x = x_coords[i];
                    end_x = x_coords[i];
                }
            }

            // 保存最后一个段
            segments.emplace_back(start_x, end_x);

            line_set[target.first].push_back(std::move(segments));
        }
    }

    return line_set;
}

// std::vector<LineSegment> Helper::mergeClimbPoint(const std::vector<TargetPoints> &target_list)
std::unordered_map<int, std::vector<LineSegment>> Helper::mergeClimbPoint(
    const std::unordered_map<int, std::vector<TargetPoints>> &target_list
    )
{
    // std::vector<LineSegment> line_set;
    // int climb_count = target_list.size();
    // line_set.reserve(climb_count);

    std::unordered_map<int, std::vector<LineSegment>> line_set;
    for(const auto &target : target_list){
        int climb_count = target.second.size();
        // int platform_count =
        line_set[target.first].reserve(climb_count);

        for(int climb_id = 0; climb_id < climb_count; climb_id++) {

            // qDebug() << "到达攀爬ID：" << climb_id;

            TargetPoints points = target.second.at(climb_id);
            if (points.empty()) {
                line_set[target.first].emplace_back(); // 添加空段
                continue;
            }

            // 提取并排序所有x坐标
            std::vector<int> y_coords;
            y_coords.reserve(points.size());
            for (const auto& pt : points) {
                y_coords.push_back(pt.y);
            }
            std::sort(y_coords.begin(), y_coords.end());

            // 合并相邻的x坐标（差值 <= 1）
            LineSegment segments;
            int start_y = y_coords[0];
            int end_y = y_coords[0];

            for (size_t i = 1; i < y_coords.size(); ++i) {
                if (y_coords[i] - y_coords[i-1] <= 3) {
                    // 相邻，扩展当前段
                    end_y = y_coords[i];
                } else {
                    // 不连续，保存当前段，开始新段
                    segments.emplace_back(start_y, end_y);
                    start_y = y_coords[i];
                    end_y = y_coords[i];
                }
            }

            // 保存最后一个段
            segments.emplace_back(start_y, end_y);

            line_set[target.first].push_back(std::move(segments));
        }
    }

    return line_set;
}

QString Helper::timestampToHI(int timestamp)
{
    int hours = timestamp / 3600;
    int minutes = (timestamp % 3600) / 60;
    return QString("%1小时:%2分钟")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'));
}

void Helper::updateGameWindow()
{
    HWND game_hwnd = FindWindow(NULL,TEXT("MapleStory Worlds-Artale (繁體中文版)"));
    if(game_hwnd == NULL){
        return;
    }

    RECT game_rect;
    GetWindowRect(game_hwnd, &game_rect);

    Gplayer.game_window.x          = game_rect.left;
    Gplayer.game_window.y          = game_rect.top;
    Gplayer.game_window.width      = game_rect.right - game_rect.left;
    Gplayer.game_window.height     = game_rect.bottom - game_rect.top;

    // 如果游戏窗口大小不是1296*759,调整至1296*759
    int game_width      = game_rect.right - game_rect.left;
    int game_height     = game_rect.bottom - game_rect.top;
    if (game_width != BASE::game_width || game_height != BASE::game_height) {
        // 调整窗口大小和位置（可选保持原位置）
        SetWindowPos(game_hwnd,  // 窗口句柄
            NULL,                // 不改变Z顺序
            game_rect.left,      // X位置（保持不变）
            game_rect.top,       // Y位置（保持不变）
            BASE::game_width,    // 新宽度
            BASE::game_height,   // 新高度
            SWP_NOZORDER | SWP_NOACTIVATE);  // 标志位

        GetWindowRect(game_hwnd, &game_rect);
        Gplayer.game_window.x          = game_rect.left;
        Gplayer.game_window.y          = game_rect.top;
        Gplayer.game_window.width      = game_rect.right - game_rect.left;
        Gplayer.game_window.height     = game_rect.bottom - game_rect.top;
    }
    qDebug() << "更新游戏窗口";
    qDebug() << Gplayer.game_window.x << Gplayer.game_window.y << Gplayer.game_window.width << Gplayer.game_window.height;
}

double Helper::getSystemGDI()
{
    // 获取窗口当前显示的监视器
    HWND hWnd = GetDesktopWindow();//根据需要可以替换成自己程序的句柄
    HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

    // 获取监视器逻辑宽度与高度
    MONITORINFOEX miex;
    miex.cbSize = sizeof(miex);
    GetMonitorInfo(hMonitor, &miex);
    int cxLogical = (miex.rcMonitor.right - miex.rcMonitor.left);
    int cyLogical = (miex.rcMonitor.bottom - miex.rcMonitor.top);

    // 获取监视器物理宽度与高度
    DEVMODE dm;
    dm.dmSize = sizeof(dm);
    dm.dmDriverExtra = 0;
    EnumDisplaySettings(miex.szDevice, ENUM_CURRENT_SETTINGS, &dm);
    int cxPhysical = dm.dmPelsWidth;
    int cyPhysical = dm.dmPelsHeight;

    //缩放比例计算
    double horzScale = ((double)cxPhysical / (double)cxLogical);
    double vertScale = ((double)cyPhysical / (double)cyLogical);
    if(horzScale == vertScale){
        return horzScale;
    }else{
        return 0;
    }
}

void Math::ComputeClimbRect(const ClimbOrigin *climb, FloatRect &climb_rect)
{
    const float kHalf = 0.5f;

    float halfFactor2 = climb->factor2 * kHalf;
    climb_rect.y_top = halfFactor2 + climb->top;

    float halfFactor1 = climb->factor1 * kHalf;
    climb_rect.x_left = climb->left - halfFactor1;
    climb_rect.x_right = climb->left + halfFactor1;
    climb_rect.y_bottom = climb->top - halfFactor2;

    climb_rect.x_left = roundHalfAwayFromZero_bit(climb_rect.x_left);
    climb_rect.y_top = roundHalfAwayFromZero_bit(climb_rect.y_top);
    climb_rect.x_right = roundHalfAwayFromZero_bit(climb_rect.x_right);
    climb_rect.y_bottom = roundHalfAwayFromZero_bit(climb_rect.y_bottom);
}

float Math::roundHalfAwayFromZero_bit(float value, int decimalPlaces)
{
    float multiplier = std::pow(10.0f, decimalPlaces);
    float scaled = value * multiplier;

    // 分离符号和绝对值
    bool negative = scaled < 0;
    float absScaled = std::fabs(scaled);

    // 取整数部分和小数部分
    float intPart;
    float fracPart = std::modf(absScaled, &intPart);

    // 远离0的舍入逻辑
    if (fracPart > 0.5f) {
        intPart += 1.0f;
    } else if (std::fabs(fracPart - 0.5f) < 1e-6f) {
        // 恰好0.5，远离0舍入
        intPart += 1.0f;
    }

    float result = negative ? -intPart : intPart;
    return result / multiplier;
}

Monster Math::getNearMonster(const std::vector<Monster> &monsters, int x)
{
    Monster monster;
    monster.entity = 0;
    int diff = 9999;
    for(const auto &item : monsters){
        if(item.pos.x == 0 && item.pos.y == 0) continue;

        // 判断怪物是否存在
        bool exists = readByte( readLong( item.entity +
            BASE::MODEntity::RigidbodyComponent) +
            BASE::RigidbodyComponent::exists);

        if(!exists){
            continue;
        }

        QString status = readString( readLong( readLong(
            item.entity + BASE::MODEntity::StateComponent) +
            BASE::StateComponent::_CurrentStateName) +
            BASE::String::text, 10);

        if(status == "DEAD"){
            continue;
        }

        int distance = abs(item.pos.x - x);
        if( distance < diff){
            diff = distance;
            monster = item;
        }
    }

    return monster;
}

bool Math::posInRange(bool to_left,cv::Point posA, cv::Point posB, cv::Point range)
{
    // qDebug() << "====== 范围内是否有怪物 ======";
    // qDebug() << "检测范围:" << range.x << "," << range.y;

    ClimbRect attack;
    if(to_left){
        attack = {
            posA.x - range.x,
            posA.y - range.y / 2,
            posA.x,
            posA.y + range.y / 2,
        };
    }else{
        attack = {
            posA.x,
            posA.y - range.y / 2,
            posA.x + range.x,
            posA.y + range.y / 2,
        };
    }

    // qDebug() << "在左边:" << to_left;
    // qDebug() << "人物:" << posA.x << "," << posA.y;
    // qDebug() << "检测框RECT：" << attack.x_left << "," << attack.y_top << " " << attack.x_right << "," << attack.y_bottom;
    // qDebug() << "目标位置:" << posB.x << "," << posB.y;

    if( posB.x >= attack.x_left &&
        posB.x <= attack.x_right &&
        posB.y >= attack.y_top  &&
        posB.y <= attack.y_bottom )
    {
        // qDebug() << "====== 范围内是否有怪物 存在 ======";
        return true;
    }
    // qDebug() << "====== 范围内是否有怪物 没有 ======";
    return false;
}

bool Math::hasIntersection(float a1, float a2, float b1, float b2)
{
    // 确保区间正确（如果传入可能未排序）
    if (a1 > a2) std::swap(a1, a2);
    if (b1 > b2) std::swap(b1, b2);

    return a1 <= b2 && b1 <= a2;
}


void ImageTool::saveMatImage(cv::Mat image, QString path)
{
    if (image.empty())
    {
        return;
    }

    std::string outputPath = path.toStdString();

    // PNG格式的压缩参数，0表示无压缩，9表示最大压缩
    std::vector<int> compression_params;
    compression_params.push_back(cv::IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(0); // 无压缩

    cv::imwrite(outputPath, image, compression_params);
}

bool ImageTool::checkLineHasBlackPixel(const cv::Mat &img, const cv::Point &p1, const cv::Point &p2, cv::Point &p3)
{
    // 使用OpenCV的LineIterator遍历直线像素
    cv::LineIterator it(img, p1, p2, 8); // 8邻域
    for(int i = 0; i < it.count; i++, ++it)
    {
        cv::Vec3b color = img.at<cv::Vec3b>(it.pos());
        if(color == COLOR_LINE)
        {
            p3 = it.pos();
            return true;
        }
    }
    return false; // 没有黑色像素
}

bool ImageTool::checkLineHasPlatformPixel(const cv::Mat &img, const cv::Point &p1, const cv::Point &p2, cv::Point &p3)
{
    cv::LineIterator it(img, p1, p2, 8); // 8邻域
    for(int i = 0; i < it.count; i++, ++it)
    {
        cv::Vec3b color = img.at<cv::Vec3b>(it.pos());
        if(color == COLOR_LINE || color == COLOR_CLIMB)
        {
            p3 = it.pos();
            return true;
        }
    }

    return false;
}

bool ImageTool::checkLineHasColorPixel(const cv::Mat &img, const cv::Point &p1, const cv::Point &p2, cv::Vec3b color, cv::Point &target)
{
    cv::LineIterator it(img, p1, p2, 8); // 8邻域
    for(int i = 0; i < it.count; i++, ++it)
    {
        cv::Vec3b cur_color = img.at<cv::Vec3b>(it.pos());
        if(cur_color == color)
        {
            target = it.pos();
            return true;
        }
    }
    return false;
}

cv::Point2i ImageTool::worldToImage(int width, int height, float x, float y)
{
    int col = static_cast<int>(std::round(x));
    int row = static_cast<int>(std::round(y));

    col = std::clamp(col, 0, width - 1);
    row = std::clamp(row, 0, height - 1);

    return cv::Point2i(col, row);
}

cv::Point ImageTool::dropTrajectory(bool is_right, const cv::Mat &image, float x, float y)
{
    cv::Point start_pos = {static_cast<int>(std::round(x)),static_cast<int>(std::round(y))};
    cv::Point target_pos = {-1,-1};
    float vx;
    if(is_right){
        vx = 4.7f;
    }else{
        vx = -4.7f;
    }

    float vy = 0.0f;
    float g  = 2.0f;

    std::vector<cv::Point> drawPoints;
    cv::Point last(-1, -1);

    cv::Vec3b wall_color = COLOR_WALL;

    while (y < image.rows)
    {
        cv::Point p = worldToImage(image.cols, image.rows, x, y);

        if (p != last)
        {
            drawPoints.push_back(p);
            last = p;
        }

        x += vx;
        y += vy;
        vy += g;

        if (x >= image.cols) break;
    }

    for (size_t i = 1; i < drawPoints.size(); i++)
    {
        // 检测到墙壁
        if(checkLineHasColorPixel(image, drawPoints[i-1], drawPoints[i], wall_color,target_pos) && i != 1)
        {
            target_pos = {-1,-1};
            break;
        }

        if(checkLineHasPlatformPixel(image, drawPoints[i-1], drawPoints[i], target_pos))
        {
            if(target_pos == start_pos || target_pos.x == -1){
                continue;
            }
            return target_pos;
        }
    }

    return target_pos;
}

std::pair<cv::Point,cv::Point> ImageTool::jumpTrajectory(
    bool is_right,
    const cv::Mat &image,
    float x,
    float y)
{
    // Mat img(ROWS, COLS, CV_8UC3, Scalar(0,0,0));
    cv::Point start_pos = {static_cast<int>(std::round(x)),static_cast<int>(std::round(y))};
    // start_pos.y += 2; // 给个高度试试

    cv::Point target_pos = {-1,-1};
    float vx; // 水平移动快 → 落点越远
    if(is_right){
        vx = 3.1f;
    }else{
        vx = -3.1f;
    }

    // ==== 参数（通过数据拟合） ====
    float g  = 0.8f;  //0.5        // 越大 → 下落快 → 空中停留时间短 → 跳得更近
    float vy = -10.9f; //-15.0      // 越大 → 跳得更高，空中停留时间越长 → 跳得更远

    std::vector<cv::Point> drawPoints;
    cv::Point last(-1,-1);

    std::pair<cv::Point,cv::Point> result = {target_pos, target_pos};

    cv::Vec3b wall_color = COLOR_WALL;
    cv::Vec3b climb_color = COLOR_CLIMB;
    cv::Vec3b line_color = COLOR_LINE;

    while(y <= image.rows)
    {
        cv::Point p = worldToImage(image.cols, image.rows, x, y);
        if(p != last){
            drawPoints.push_back(p);
            last = p;
        }

        x += vx;
        y += vy;
        vy += g;

        if(x >= image.cols) break;
    }

    for (size_t i = 1; i < drawPoints.size(); i++)
    {
        // 检测到墙壁
        if(checkLineHasColorPixel(image, drawPoints[i-1], drawPoints[i], wall_color,target_pos))
        {
            target_pos = {-1,-1};
            break;
        }

        // 检测攀爬物
        if(checkLineHasColorPixel(image, drawPoints[i-1], drawPoints[i], climb_color,target_pos))
        {
            if(start_pos.y - target_pos.y > 2){
                result.first = target_pos;
                target_pos = {-1,-1};
            }
        }

        // 检测到地面（可以跳跃到上层）
        if(checkLineHasColorPixel(image, drawPoints[i-1], drawPoints[i], line_color,target_pos) && i != 1)
        {
            result.second = target_pos;
            break;
        }
    }

    return result;
}

std::pair<cv::Point,cv::Point> ImageTool::climbJumpTrajectory(
    bool is_right,
    const cv::Mat &image,
    ClimbRect climb_rect,
    float x,
    float y)
{
    cv::Point start_pos = {static_cast<int>(std::round(x)),static_cast<int>(std::round(y))};

    cv::Point target_pos = {-1,-1};
    float vx; // 水平移动快 → 落点越远
    if(is_right){
        vx = 3.1f;
    }else{
        vx = -3.1f;
    }

    // ==== 参数（通过数据拟合） ====
    float g  = 0.8f;  //0.5        // 越大 → 下落快 → 空中停留时间短 → 跳得更近
    float vy = -5.5f; //-15.0      // 越大 → 跳得更高，空中停留时间越长 → 跳得更远

    std::vector<cv::Point> drawPoints;
    cv::Point last(-1,-1);

    std::pair<cv::Point,cv::Point> result = {target_pos, target_pos};

    cv::Vec3b wall_color = COLOR_WALL;
    cv::Vec3b climb_color = COLOR_CLIMB;
    cv::Vec3b line_color = COLOR_LINE;

    while(y <= image.rows)
    {
        cv::Point p = worldToImage(image.cols, image.rows, x, y);
        if(p != last){
            drawPoints.push_back(p);
            last = p;
        }

        x += vx;
        y += vy;
        vy += g;

        if(x >= image.cols) break;
    }

    for (size_t i = 1; i < drawPoints.size(); i++)
    {
        // 检测到墙壁
        if(checkLineHasColorPixel(image, drawPoints[i-1], drawPoints[i], wall_color,target_pos))
        {
            target_pos = {-1,-1};
            break;
        }

        // 检测攀爬物
        if(checkLineHasColorPixel(image, drawPoints[i-1], drawPoints[i], climb_color,target_pos))
        {
            // 只有不是在自己体内的时候才算找到攀爬物
            if (!(target_pos.x >= climb_rect.x_left && target_pos.x <= climb_rect.x_right &&
                  target_pos.y >= climb_rect.y_top && target_pos.y <= climb_rect.y_bottom)) {
                result.first = target_pos;
                target_pos = {-1, -1};
            }
        }

        // 检测到地面（可以跳跃到上层）
        if(checkLineHasColorPixel(image, drawPoints[i-1], drawPoints[i], cv::Vec3b(0,0,0),target_pos) && i != 1)
        {
            result.second = target_pos;
            break;

        }
    }

    return result;
}

bool ImageTool::isPointInWall(const cv::Point &p, const Wall &wall, int tolerance)
{
    // 判断 x 是否在墙附近（允许误差 tolerance）
    if (abs(p.x - wall.x) > tolerance)
        return false;

    // 判断 y 是否在范围内
    if (p.y < wall.top - tolerance || p.y > wall.bottom + tolerance)
        return false;

    return true;
}

bool ImageTool::isPointOnSegment(const cv::Point &p, const cv::Point &a, const cv::Point &b, int tolerance)
{
    // int dx1 = p.x - a.x;
    // int dy1 = p.y - a.y;
    // int dx2 = b.x - a.x;
    // int dy2 = b.y - a.y;

    // // ① 叉积（判断是否共线）
    // int cross = dx1 * dy2 - dy1 * dx2;
    // if (abs(cross) > tolerance)
    //     return false;

    // // ② 点积（判断是否在线段范围内）
    // int dot = dx1 * dx2 + dy1 * dy2;
    // if (dot < -tolerance)
    //     return false;

    // int lenSq = dx2 * dx2 + dy2 * dy2;
    // if (dot > lenSq + tolerance)
    //     return false;

    // return true;

    // 当前算法存在BUG，如果点位于一个斜线当中结果会返回false

    double dx1 = p.x - a.x;
    double dy1 = p.y - a.y;
    double dx2 = b.x - a.x;
    double dy2 = b.y - a.y;

    // ① 叉积（面积）
    double cross = dx1 * dy2 - dy1 * dx2;

    // ② 线段长度
    double length = std::sqrt(dx2 * dx2 + dy2 * dy2);
    if (length == 0) return false;

    // ⭐ 关键：转成“点到直线距离”
    double distance = std::abs(cross) / length;

    if (distance > tolerance)
        return false;

    // ③ 投影判断（是否在线段范围内）
    double dot = dx1 * dx2 + dy1 * dy2;
    if (dot < 0) return false;

    double lenSq = dx2 * dx2 + dy2 * dy2;
    if (dot > lenSq) return false;

    return true;
}

bool ImageTool::isPointOnFoot(const cv::Point &pos)
{

    return false;
}

bool ImageTool::isPointOnLine(const cv::Mat &map, const cv::Point &pos, const cv::Point &start, const cv::Point &end)
{
    cv::LineIterator it(map, start, end, 8);  // 8连通线段
    if(start.y != end.y){
        cv::LineIterator it(map, start, end, 4);  // 4连通线段
    }

    for (int i = 0; i < it.count; i++, ++it) {
        int it_x = it.pos().x;
        int it_y = it.pos().y;
        cv::Point coor = {it_x,it_y};
        if (it.pos() == pos) {
            return true;
        }
    }
    return false;
}

bool ImageTool::isPointInClimbRect(const cv::Point &point, const ClimbRect &rect)
{
    return point.x >= rect.x_left && point.x <= rect.x_right &&
           point.y >= rect.y_top && point.y <= rect.y_bottom;
}

void ImageTool::screenshot()
{
    HWND game_hwnd = FindWindow(NULL,TEXT("MapleStory Worlds-Artale (繁體中文版)"));
    if(game_hwnd == NULL){
        DebugOutput::out("游戏窗口不存在");
        return;
    }
    RECT game_rect;
    GetWindowRect(game_hwnd, &game_rect);

    QScreen* m_pScreen  = QGuiApplication::primaryScreen();
    QPixmap cur_capture = m_pScreen->grabWindow((WId)GetDesktopWindow());
    int game_x          = game_rect.left;
    int game_y          = game_rect.top;
    int game_width      = game_rect.right - game_rect.left;
    int game_height     = game_rect.bottom - game_rect.top;
    cur_capture         = cur_capture.copy(game_x,game_y,game_width,game_height);
    Gplayer.game_window.x = game_x;
    Gplayer.game_window.y = game_y;
    Gplayer.game_window.width = game_width;
    Gplayer.game_window.height = game_height;

    // 如果游戏窗口大小不是1296*759,调整至1296*759
    if (game_width != BASE::game_width || game_height != BASE::game_height) {
        // 调整窗口大小和位置（可选保持原位置）
        SetWindowPos(game_hwnd,  // 窗口句柄
            NULL,                // 不改变Z顺序
            game_rect.left,      // X位置（保持不变）
            game_rect.top,       // Y位置（保持不变）
            BASE::game_width,    // 新宽度
            BASE::game_height,   // 新高度
            SWP_NOZORDER | SWP_NOACTIVATE);  // 标志位

        GetWindowRect(game_hwnd, &game_rect);
        m_pScreen  = QGuiApplication::primaryScreen();
        cur_capture = m_pScreen->grabWindow((WId)GetDesktopWindow());
        game_x          = game_rect.left;
        game_y          = game_rect.top;
        game_width      = game_rect.right - game_rect.left;
        game_height     = game_rect.bottom - game_rect.top;
        cur_capture     = cur_capture.copy(game_x,game_y,game_width,game_height);
        Gplayer.game_window.x = game_x;
        Gplayer.game_window.y = game_y;
        Gplayer.game_window.width = game_width;
        Gplayer.game_window.height = game_height;
    }

    qDebug() << "窗口大小：" << game_width << "," << game_height;
    qDebug() << "窗口位置:" << game_x << "," << game_y;

    QString timestamp = QString::number(QDateTime::currentMSecsSinceEpoch());
    QString savePath = QCoreApplication::applicationDirPath() + "/screenshot_" + timestamp + ".png";
    cur_capture.save(savePath);
    DebugOutput::out("截图已保存在程序目录");
}

cv::Point RoomMap::getItemLinePos(const cv::Mat &map, cv::Point pos)
{
    cv::Point target(-1, -1);
    if (map.empty() || map.channels() != 3) {
        return target;
    }

    cv::Point end(pos.x, pos.y + 30);
    if(ImageTool::checkLineHasBlackPixel(map, pos, end, target)){
        return target;
    }

    return target;
}

int RoomMap::getFootPlatform(int foothold_id, const std::vector<Platform>& platform_list)
{
    for(const auto &item : platform_list){
        for(const auto &foot : item.line_child){
            if(foot.foot_id == foothold_id){
                return item.id;
            }
        }
    }
    return -1;
}

int RoomMap::getPosPlatformByMAP(cv::Point pos)
{
    if(Gplayer.plat_map.empty()){
        return -1;
    }

    try {
        int plat_id = Gplayer.plat_map.at(pos.x).at(pos.y).platform_id;
        return plat_id;
    } catch (const std::out_of_range& e) {
        return -1;
    }

    return -1;
}

int RoomMap::getClimbPlatform(cv::Point pos, const std::vector<Platform> &platform_list, const cv::Mat &room_map)
{
    // 遍历平台
    for(const auto &platform : platform_list){
        if(platform.type == 1){
            if(pos.x >= platform.start.x
                && pos.x <= platform.end.x
                && pos.y >= (platform.start.y - 40)
                && pos.y <= (platform.end.y + 40)
                ){
                return platform.id;
            }
        }
    }

    return -1;
}

int RoomMap::getRectPlatform(IntRect rect, const std::vector<Platform> &platform_list)
{
    int x = rect.x_left;
    for(int i = rect.y_bottom; i > rect.y_top; i--){
        for(const auto &platform : platform_list){
            for(const auto &foot : platform.line_child){
                if(i == foot.start.y && i == foot.end.y){
                    if(x >= foot.start.x && x <= foot.end.x){
                        return platform.id;
                    }
                }
            }
        }
    }

    return -1;
}

int RoomMap::getPlatformType(int platform_id, const std::vector<Platform> &platform_list)
{
    for(const auto &item : platform_list){
        if(item.id == platform_id){
            return item.type;
        }
    }
    return -1;
}

void RoomMap::loadLocalMap(const QString map_id)
{
    Gplayer.curmapinfo.id = map_id;
    Gplayer.curmapinfo.name = "";
    Gplayer.curmapinfo.map_bounding = {};
    std::vector<ClimbRect>().swap(Gplayer.curmapinfo.climbs);
    std::vector<FootholdRect>().swap(Gplayer.curmapinfo.footholds);
    std::vector<Wall>().swap(Gplayer.curmapinfo.wall_list);
    std::vector<Platform>().swap(Gplayer.curmapinfo.platform_list);
    std::unordered_map<int, PathNode>().swap(Gplayer.curmapinfo.path_nodes);
    Gplayer.curmapinfo.map_mat = cv::Mat();

    QString jsonPath = ":/assets/maps/map_" + map_id;
    qDebug() << "加载内置地图：" << jsonPath;
    if(MapSerializer::isValidSaveFile(jsonPath)){
        qDebug() << "加载内置地图数据！";
        Gplayer.curmapinfo = MapSerializer::loadFromFile(jsonPath);
        return;
    }else{
        qDebug() << "内置地图不存在，即将加载本地缓存文件";
    }

    QString filePath = QCoreApplication::applicationDirPath() + "/maps/map_" + map_id;

    if(!MapSerializer::isValidSaveFile(filePath)) return;

    qDebug() << "加载缓存地图文件:" << filePath;
    Gplayer.curmapinfo = MapSerializer::loadFromFile(filePath);

    // Gplayer.curmapinfo.map_bounding  = mapInfo.map_bounding;
    // Gplayer.curmapinfo.climbs        = mapInfo.climbs;
    // Gplayer.curmapinfo.footholds     = mapInfo.footholds;
    // Gplayer.curmapinfo.wall_list     = mapInfo.wall_list;
    // Gplayer.curmapinfo.platform_list = mapInfo.platform_list;
    // Gplayer.curmapinfo.path_nodes    = mapInfo.path_nodes;
    // Gplayer.curmapinfo.map_mat       = mapInfo.map_mat;

}

cv::Point RoomMap::npcScreenCoor(QString npc_id)
{
    cv::Point npc_coor = {-1,-1};

    quint64 p_curmapentity = readPointer(Gplayer.p_curmapentity,
        BASE::MODEntity::children,
        BASE::List::InternalArray,
        BASE::InternalArray::Head + 0 * 8);

    quint64 map_entity_count = readInt(readLong(Gplayer.p_curmapentity + BASE::MODEntity::children) + BASE::List::size);
    quint64 entity_head = readLong(readLong(Gplayer.p_curmapentity + BASE::MODEntity::children) + BASE::List::InternalArray) + BASE::InternalArray::Head;

    MapBounding map_bounding = Gplayer.curmapinfo.map_bounding;

    quint64 npc_entity = 0;
    for(int i = 0; i < map_entity_count; i++){

        quint64 cur_entity_address = entity_head + i * 8;
        quint64 entity = readLong(cur_entity_address);

        QString name = GameStruct::readEntityName(entity);

        if(name == npc_id){
            npc_entity = entity;
        }
    }

    if(npc_entity == 0){
        return npc_coor;
    }

    quint64 p_camera = readPointer(Gplayer.p_assemblybase,
        BASE::Camera_base,
        BASE::Camera_offset_lv1,
        BASE::Camera_offset_lv2,
        BASE::MODPlayerMain::CinemachineBrain,
        BASE::CinemachineBrain::Camera, 0x10);

    float npc_x = readFloat(readLong(npc_entity + BASE::MODEntity::TransformComponent)+BASE::TransformComponent::PositionX);
    float npc_y = readFloat(readLong(npc_entity + BASE::MODEntity::TransformComponent)+BASE::TransformComponent::PositionY);

    COORDINATE pos{npc_x, npc_y, 0.0f};
    // qDebug() << "NPC坐标:" << npc_x << "," << npc_y;

    cv::Point screen;

    if (CameraTool::WorldToScreen(p_camera, pos, screen))
    {
        qDebug() << "屏幕坐标:" << screen.x << screen.y;
        npc_coor.x = screen.x + 10;
        npc_coor.y = screen.y - 10;
    }
    else
    {
        qDebug() << "相机转换错误~，可能在相机背面";
    }

    return npc_coor;
}

std::vector<int> PathAlgo::findAStarPath(int platformA, int platformB, std::unordered_map<int, PathNode> &path_nodes)
{
    // std::vector<int> path;

    // // 开启列表(最小堆)
    // std::priority_queue<int, std::vector<int>, std::greater<int>> open_list;

    // // 平台map
    // std::unordered_map<int, PathNode> came_from;

    // // 用于记录已经访问过的节点
    // std::unordered_set<int> closed_list;

    // // 初始化起始平台
    // PathNode start_node = path_nodes[platformA];
    // open_list.push(start_node.id);


    // while (!open_list.empty()) {
    //     int cur_plat = open_list.top();
    //     PathNode current = path_nodes[cur_plat]; // 获取第一个节点
    //     open_list.pop(); // 从堆中删除

    //     // 如果找到目标平台，生成路径
    //     if (current.id == platformB) {
    //         while (came_from.find(current.id) != came_from.end()) {
    //             path.push_back(current.id);
    //             current = came_from[current.id];
    //         }
    //         path.push_back(platformA);
    //         std::reverse(path.begin(), path.end());
    //         return path; // 可以返回路径信息了
    //     }

    //     // 记录当前节点（如果没到终点，把当前节点放入到关闭列表中）
    //     closed_list.insert(current.id);

    //     // 遍历当前节点的所有目标平台
    //     for (const auto& target : current.targets) {
    //         if (closed_list.find(target.first) != closed_list.end()) {
    //             continue;  // 如果目标已在关闭列表中，跳过
    //         }

    //         // 如果目标平台不在开启列表中，加入
    //         if (came_from.find(target.first) == came_from.end()) {
    //             open_list.push(target.first);
    //             came_from[target.first] = current;
    //         }
    //     }
    // }

    // return path;

    std::vector<int> path;

    if (path_nodes.find(platformA) == path_nodes.end() ||
        path_nodes.find(platformB) == path_nodes.end()) {
        return path;
    }

    if (platformA == platformB) {
        path.push_back(platformA);
        return path;
    }

    // BFS 使用普通队列（更简单，边权为1时效率一样）
    std::queue<int> open_list;
    std::unordered_map<int, int> came_from;
    std::unordered_set<int> visited;

    open_list.push(platformA);
    visited.insert(platformA);

    while (!open_list.empty()) {
        int current_id = open_list.front();
        open_list.pop();

        if (current_id == platformB) {
            int cur = platformB;
            while (came_from.find(cur) != came_from.end()) {
                path.push_back(cur);
                cur = came_from[cur];
            }
            path.push_back(platformA);
            std::reverse(path.begin(), path.end());
            return path;
        }

        PathNode& current_node = path_nodes[current_id];

        for (const auto& target : current_node.targets) {
            int target_id = target.first;

            if (visited.find(target_id) != visited.end()) {
                continue;
            }

            visited.insert(target_id);
            came_from[target_id] = current_id;
            open_list.push(target_id);
        }
    }

    return path;
}

std::vector<TargetRoom> PathAlgo::findAstarRoom(const QString &roomA, const QString &roomB)
{
    std::vector<TargetRoom> path;

    if(Gplayer.room_nodes.empty()) return path;

    // 检查房间是否存在
    if (Gplayer.room_nodes.find(roomA) == Gplayer.room_nodes.end() ||
        Gplayer.room_nodes.find(roomB) == Gplayer.room_nodes.end()) {
        return path;
    }

    // 如果起点等于终点
    if (roomA == roomB) {
        return path;
    }

    // 开启列表
    std::queue<QString> open_list;

    // 记录路径：key=目标房间UUID, value=从哪里来以及使用的传送门
    std::unordered_map<QString, std::pair<QString, TargetRoom>> came_from;

    // 用于记录已经访问过的节点
    std::unordered_set<QString> closed_list;

    // 初始化起始平台
    RoomNode start_node = Gplayer.room_nodes[roomA];
    open_list.push(start_node.uuid);


    while (!open_list.empty()) {
        QString cur_room = open_list.front();
        RoomNode current = Gplayer.room_nodes[cur_room]; // 获取第一个节点
        open_list.pop(); // 从堆中删除

        // 如果节点已在关闭列表中，跳过
        if (closed_list.find(current.uuid) != closed_list.end()) {
            continue;
        }

        // 如果找到目标平台，生成路径
        if (current.uuid == roomB) {
            QString cur = roomB;
            while (came_from.find(cur) != came_from.end()) {
                path.push_back(came_from[cur].second);  // 添加传送门信息
                cur = came_from[cur].first;             // 回溯到上一个房间
            }
            std::reverse(path.begin(), path.end());  // 反转得到正确顺序
            return path;
        }

        // 记录当前节点（如果没到终点，把当前节点放入到关闭列表中）
        closed_list.insert(current.uuid);

        // 遍历当前节点的所有目标平台
        for (const auto& target : current.targets) {
            if (closed_list.find(target.uuid) != closed_list.end()) {
                continue;  // 如果目标已在关闭列表中，跳过
            }

            // 如果目标平台不在开启列表中，加入
            if (came_from.find(target.uuid) == came_from.end()) {
                open_list.push(target.uuid);
                came_from[target.uuid] = {current.uuid, target};
            }
        }
    }

    return path;
}

std::vector<TargetRoom> PathAlgo::findPotionRoom(const QString &roomA)
{
    std::vector<TargetRoom> path;

    if(Gplayer.room_nodes.empty()) return path;

    if (Gplayer.room_nodes.find(roomA) == Gplayer.room_nodes.end()) {
        return path;
    }

    if (Gplayer.room_nodes[roomA].type == 2) {
        return path;
    }

    // BFS 使用普通队列（边权都为1时效率更高）
    std::queue<QString> open_list;
    std::unordered_map<QString, std::pair<QString, TargetRoom>> came_from;
    std::unordered_set<QString> visited;

    open_list.push(roomA);
    visited.insert(roomA);

    while (!open_list.empty()) {
        QString current_uuid = open_list.front();
        open_list.pop();

        RoomNode& current_room = Gplayer.room_nodes[current_uuid];

        if (current_room.type == 2) {
            QString cur = current_uuid;
            while (came_from.find(cur) != came_from.end()) {
                path.push_back(came_from[cur].second);
                cur = came_from[cur].first;
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        for (const auto& target : current_room.targets) {
            if (target.uuid.isEmpty()) continue;
            if (visited.find(target.uuid) != visited.end()) continue;

            visited.insert(target.uuid);
            came_from[target.uuid] = {current_uuid, target};
            open_list.push(target.uuid);
        }
    }

    return path;
}

void PathAlgo::walkToLeft(
    const std::vector<Platform> &platform_list,
    const cv::Mat &cur_room_map,
    std::unordered_map<int, PathNode> &path_nodes
)
{
    // 遍历线路
    // qDebug() << "----------向左走寻路----------";
    for(auto item : platform_list){
        // qDebug() << "搜索线路：" << item.id;
        if(item.type == 1){
            continue;
        }
        // 判断左边是否有墙
        if(item.hasLeftWall){
            // qDebug() << "左边有墙，退出寻路";
            continue;
        }

        if(item.start.x == 0){
            // qDebug() << "左边是边界，退出寻路";
            continue;
        }

        cv::Point to_left_pos = ImageTool::dropTrajectory(false,cur_room_map,item.start.x,item.start.y);
        if(to_left_pos.x == -1){
            continue;
        }

        int target_platform_id = RoomMap::getPosPlatformByMAP(to_left_pos);
        if(target_platform_id == -1){
            // 未找到线路
            continue;
        }

        // 找到线路之后，存放节点信息
        // qDebug() << "连接到目标路线:" << target_platform_id;

        LineSegment line_segment;
        line_segment.push_back(std::pair(item.start.x, item.start.x));

        NodeVisit node_visit;
        node_visit.visit_type = VISIT_TYPE::WALK_LEFT;
        node_visit.area = line_segment;

        path_nodes[item.id].targets[target_platform_id].id = target_platform_id;
        path_nodes[item.id].targets[target_platform_id].type = path_nodes[target_platform_id].type;
        path_nodes[item.id].targets[target_platform_id].visit_list.push_back(node_visit);
    }
}

void PathAlgo::walkToRight(
    const std::vector<Platform> &platform_list,
    const cv::Mat &cur_room_map,
    const MapBounding &map_bounding,
    std::unordered_map<int, PathNode> &path_nodes
    )
{
    // qDebug() << "----------向右走寻路----------";
    for(auto item : platform_list){
        // qDebug() << "搜索线路：" << item.id;
        if(item.type == 1){
            continue;
        }
        // 判断右边是否有墙
        if(item.hasRightWall){
            continue;
        }

        if(item.end.x == map_bounding.width){
            continue;
        }

        cv::Point to_right_pos = ImageTool::dropTrajectory(true,cur_room_map,item.end.x,item.end.y);
        if(to_right_pos.x == -1){
            continue;
        }

        int target_platform_id = RoomMap::getPosPlatformByMAP(to_right_pos);
        if(target_platform_id == -1){
            // 未找到线路
            continue;
        }

        // qDebug() << "连接到目标路线:" << target_platform_id;

        LineSegment line_segment;
        line_segment.push_back(std::pair(item.end.x, item.end.x));

        NodeVisit node_visit;
        node_visit.visit_type = VISIT_TYPE::WALK_RIGHT;
        node_visit.area = line_segment;

        path_nodes[item.id].targets[target_platform_id].id = target_platform_id;
        path_nodes[item.id].targets[target_platform_id].type = path_nodes[target_platform_id].type;
        path_nodes[item.id].targets[target_platform_id].visit_list.push_back(node_visit);
    }
}

void PathAlgo::jumpLeft(
    const std::vector<Platform> &platform_list,
    const cv::Mat &room_map,
    std::unordered_map<int, PathNode> &path_nodes
)
{
    // 遍历线路
    // qDebug() << "----------跳跃左边寻路----------";

    for(const auto &platform : platform_list){

        // qDebug() << "搜索平台（地板）：" << platform.id;

        if(platform.type == 0){  // 如果是线类型
            // std::vector<TargetPoints> targets;
            std::unordered_map<int, std::vector<TargetPoints>> targets;
            targets[0].resize(platform_list.size());
            targets[1].resize(platform_list.size());

            // 需要遍历所有的地板(从地板开始跳跃)
            for(auto foot : platform.line_child){
                // 是直线还是斜线地板
                cv::LineIterator iterator(room_map, foot.start, foot.end, 8);  // 8连通线段
                for (int i = 0; i < iterator.count; i++, ++iterator) {
                    std::pair<cv::Point, cv::Point> target = ImageTool::jumpTrajectory(false, room_map, iterator.pos().x, iterator.pos().y);
                    if(target.first.x == -1 && target.second.x == -1){
                        continue;
                    }
                    if(target.first.x != -1){
                        int climb_platform = RoomMap::getPosPlatformByMAP(target.first);
                        // tmp_node.target_climb.at(climb_id).push_back(iterator.pos());
                        if(climb_platform != -1){
                            targets[1].at(climb_platform).push_back(iterator.pos());
                        }
                    }
                    if(target.second.x != -1) {
                        // 如果目标地点是本平台，则直接忽略
                        int line_platform = RoomMap::getPosPlatformByMAP(target.second);
                        if(line_platform == platform.id || line_platform == -1){
                            continue;
                        }
                        targets[0].at(line_platform).push_back(iterator.pos());
                    }
                }
            }

            // qDebug() << "准备合并线路：" << platform.id;
            // std::vector<LineSegment> foot_segment = Helper::mergeFootPoint(targets);
            std::unordered_map<int, std::vector<LineSegment>> foot_segment = Helper::mergeFootPoint(targets);

            // for(int target_id = 0; target_id < foot_segment.size(); target_id++){
            //     if(foot_segment.at(target_id).empty()){
            //         continue;
            //     }
            //     NodeVisit node_visit;
            //     node_visit.visit_type = VISIT_TYPE::JUMP_LEFT;
            //     node_visit.area = foot_segment.at(target_id);
            //     // path_node.second.targets[target_id].visit_list.push_back(node_visit);
            //     path_nodes[platform.id].targets[target_id].visit_list.push_back(node_visit);
            // }
            for(const auto &segment : foot_segment){
                for(int target_id = 0; target_id < segment.second.size(); target_id++){
                    if(segment.second.at(target_id).empty()){
                        continue;
                    }
                    NodeVisit node_visit;
                    node_visit.visit_type = VISIT_TYPE::JUMP_LEFT;
                    node_visit.area = foot_segment[segment.first].at(target_id);
                    path_nodes[platform.id].targets[target_id].id = target_id;
                    path_nodes[platform.id].targets[target_id].type = path_nodes[target_id].type;
                    path_nodes[platform.id].targets[target_id].visit_list.push_back(node_visit);
                }
            }

            // for(int target_id = 0; target_id < foot_segment[1].size(); target_id++){
            //     if(foot_segment[1].at(target_id).empty()){
            //         continue;
            //     }
            // }
        }

        if(platform.type == 1){ // 如果是攀爬体类型
            // qDebug() << "搜索平台(攀爬)：" << platform.id;

            // std::vector<TargetPoints> targets;
            // targets.resize(platform_list.size());
            std::unordered_map<int, std::vector<TargetPoints>> targets;
            targets[0].resize(platform_list.size());
            targets[1].resize(platform_list.size());

            cv::Point climb_start(platform.start.x + 25, platform.start.y + 3);
            cv::Point climb_end(platform.start.x + 25, platform.end.y);
            cv::LineIterator iterator(room_map, climb_start, climb_end, 8);  // 8连通线段

            ClimbRect cur_climb_rect;
            cur_climb_rect.x_left = platform.start.x;
            cur_climb_rect.y_top = platform.start.y;
            cur_climb_rect.x_right = platform.end.x;
            cur_climb_rect.y_bottom = platform.end.y;

            for (int i = 0; i < iterator.count; i++, ++iterator) {
                std::pair<cv::Point, cv::Point> target = ImageTool::climbJumpTrajectory(false, room_map, cur_climb_rect, iterator.pos().x, iterator.pos().y);
                if(target.first.x == -1 && target.second.x == -1){
                    continue;
                }
                if(target.first.x != -1){
                    int climb_platform = RoomMap::getPosPlatformByMAP(target.first);
                    if(climb_platform != -1){
                        targets[1].at(climb_platform).push_back(iterator.pos());
                    }
                }
                if(target.second.x != -1){
                    // 如果目标地点是本平台，则直接忽略
                    int line_platform = RoomMap::getPosPlatformByMAP(target.second);
                    if(line_platform == platform.id || line_platform == -1){
                        continue;
                    }
                    targets[0].at(line_platform).push_back(iterator.pos());
                }
            }

            // qDebug() << "准备合并线路：" << platform.id;
            // std::vector<LineSegment> climb_segment = Helper::mergeClimbPoint(targets);
            std::unordered_map<int, std::vector<LineSegment>> climb_segment = Helper::mergeClimbPoint(targets);

            // for(int target_id = 0; target_id < climb_segment.size(); target_id++){
            //     if(climb_segment.at(target_id).empty()){
            //         continue;
            //     }
            //     NodeVisit node_visit;
            //     node_visit.visit_type = VISIT_TYPE::JUMP_LEFT;
            //     node_visit.area = climb_segment.at(target_id);
            //     path_nodes[platform.id].targets[target_id].visit_list.push_back(node_visit);
            //     break;
            // }
            for(const auto &segment : climb_segment){
                for(int target_id = 0; target_id < segment.second.size(); target_id++){
                    if(segment.second.at(target_id).empty()){
                        continue;
                    }
                    NodeVisit node_visit;
                    node_visit.visit_type = VISIT_TYPE::JUMP_LEFT;
                    node_visit.area = climb_segment[segment.first].at(target_id);
                    path_nodes[platform.id].targets[target_id].id = target_id;
                    path_nodes[platform.id].targets[target_id].type = path_nodes[target_id].type;
                    path_nodes[platform.id].targets[target_id].visit_list.push_back(node_visit);
                }
            }
        }
    }

    // qDebug() << "跳跃左边寻路完毕~";
}

void PathAlgo::jumpRight(
    const std::vector<Platform> &platform_list,
    const cv::Mat &room_map,
    std::unordered_map<int, PathNode> &path_nodes
)
{
    // 遍历线路
    // qDebug() << "----------跳跃右边寻路----------";

    for(const auto &platform : platform_list){
        if(platform.type == 0){  // 如果是线类型
            std::unordered_map<int, std::vector<TargetPoints>> targets;
            targets[0].resize(platform_list.size());
            targets[1].resize(platform_list.size());

            // 需要遍历所有的地板(从地板开始跳跃)
            for(auto foot : platform.line_child){
                // 是直线还是斜线地板
                cv::LineIterator iterator(room_map, foot.start, foot.end, 8);  // 8连通线段
                for (int i = 0; i < iterator.count; i++, ++iterator) {
                    std::pair<cv::Point, cv::Point> target = ImageTool::jumpTrajectory(true, room_map, iterator.pos().x, iterator.pos().y);
                    if(target.first.x == -1 && target.second.x == -1){
                        continue;
                    }
                    if(target.first.x != -1){
                        int climb_platform = RoomMap::getPosPlatformByMAP(target.first);
                        // tmp_node.target_climb.at(climb_id).push_back(iterator.pos());
                        if(climb_platform != -1){
                            targets[1].at(climb_platform).push_back(iterator.pos());
                        }
                    }
                    if(target.second.x != -1) {
                        // 如果目标地点是本平台，则直接忽略
                        int line_platform = RoomMap::getPosPlatformByMAP(target.second);
                        if(line_platform == platform.id || line_platform == -1){
                            continue;
                        }
                        targets[0].at(line_platform).push_back(iterator.pos());
                    }
                }
            }

            std::unordered_map<int, std::vector<LineSegment>> foot_segment = Helper::mergeFootPoint(targets);

            for(const auto &segment : foot_segment){
                for(int target_id = 0; target_id < segment.second.size(); target_id++){
                    if(segment.second.at(target_id).empty()){
                        continue;
                    }
                    NodeVisit node_visit;
                    node_visit.visit_type = VISIT_TYPE::JUMP_RIGHT;
                    node_visit.area = foot_segment[segment.first].at(target_id);
                    path_nodes[platform.id].targets[target_id].id = target_id;
                    path_nodes[platform.id].targets[target_id].type = path_nodes[target_id].type;
                    path_nodes[platform.id].targets[target_id].visit_list.push_back(node_visit);
                }
            }
        }

        if(platform.type == 1){ // 如果是攀爬体类型
            std::unordered_map<int, std::vector<TargetPoints>> targets;
            targets[0].resize(platform_list.size());
            targets[1].resize(platform_list.size());

            cv::Point climb_start(platform.start.x + 25, platform.start.y + 3);
            cv::Point climb_end(platform.start.x + 25, platform.end.y);
            cv::LineIterator iterator(room_map, climb_start, climb_end, 8);  // 8连通线段

            ClimbRect cur_climb_rect;
            cur_climb_rect.x_left = platform.start.x;
            cur_climb_rect.y_top = platform.start.y;
            cur_climb_rect.x_right = platform.end.x;
            cur_climb_rect.y_bottom = platform.end.y;

            for (int i = 0; i < iterator.count; i++, ++iterator) {
                std::pair<cv::Point, cv::Point> target = ImageTool::climbJumpTrajectory(true, room_map, cur_climb_rect, iterator.pos().x, iterator.pos().y);
                if(target.first.x == -1 && target.second.x == -1){
                    continue;
                }
                if(target.first.x != -1){
                    int climb_platform = RoomMap::getPosPlatformByMAP(target.first);
                    if(climb_platform != -1){
                        targets[1].at(climb_platform).push_back(iterator.pos());
                    }
                }
                if(target.second.x != -1){
                    // 如果目标地点是本平台，则直接忽略
                    int line_platform = RoomMap::getPosPlatformByMAP(target.second);
                    if(line_platform == platform.id || line_platform == -1){
                        continue;
                    }
                    targets[0].at(line_platform).push_back(iterator.pos());
                }
            }

            std::unordered_map<int, std::vector<LineSegment>> climb_segment = Helper::mergeClimbPoint(targets);

            for(const auto &segment : climb_segment){
                for(int target_id = 0; target_id < segment.second.size(); target_id++){
                    if(segment.second.at(target_id).empty()){
                        continue;
                    }
                    NodeVisit node_visit;
                    node_visit.visit_type = VISIT_TYPE::JUMP_RIGHT;
                    node_visit.area = climb_segment[segment.first].at(target_id);
                    path_nodes[platform.id].targets[target_id].id = target_id;
                    path_nodes[platform.id].targets[target_id].type = path_nodes[target_id].type;
                    path_nodes[platform.id].targets[target_id].visit_list.push_back(node_visit);
                }
            }
        }
    }
}

void PathAlgo::jump_up(
    const std::vector<Platform> &platform_list,
    const cv::Mat &room_map,
    std::unordered_map<int, PathNode> &path_nodes
)
{
    for(const auto &platform : platform_list){
        if(platform.type == 1){
            continue;
        }

        std::unordered_map<int, std::vector<TargetPoints>> targets;
        targets[0].resize(platform_list.size());
        targets[1].resize(platform_list.size());

        for(auto foot : platform.line_child){

            cv::LineIterator iterator(room_map, foot.start, foot.end, 8);
            for (int i = 0; i < iterator.count; i++, ++iterator) {

                // 从下向上跳跃80像素，需要增加到100，可能人物碰撞盒体积是20*20
                // 但这样可能会导致：计算结果包含了跳不上去的平台
                cv::Point start = iterator.pos();
                // start.y -= 1;
                cv::Point end = start;
                end.y -= 110;

                // 检测攀爬物
                cv::Point target_pos = {-1, -1};
                cv::LineIterator jump_iterator(room_map, start, end, 8);
                for(int j = 0; j < jump_iterator.count; j++, ++jump_iterator)
                {
                    cv::Point cur_pos = jump_iterator.pos();
                    cv::Vec3b cur_color = room_map.at<cv::Vec3b>(jump_iterator.pos());
                    if(cur_color == COLOR_CLIMB)
                    {
                        if(start.y - cur_pos.y > 2){
                            int climb_platform = RoomMap::getPosPlatformByMAP(cur_pos);
                            if(climb_platform != -1){
                                targets[1].at(climb_platform).push_back(iterator.pos());
                            }
                        }
                    }
                    if(cur_color == COLOR_LINE)
                    {
                        if(start.y - cur_pos.y > 1){
                            // 如果目标地点是本平台，则直接忽略
                            int line_platform = RoomMap::getPosPlatformByMAP(cur_pos);
                            if(line_platform == platform.id || line_platform == -1){
                                continue;
                            }
                            targets[0].at(line_platform).push_back(iterator.pos());
                        }
                    }
                }
            }
        }

        std::unordered_map<int, std::vector<LineSegment>> foot_segment = Helper::mergeFootPoint(targets);
        for(const auto &segment : foot_segment){
            for(int target_id = 0; target_id < segment.second.size(); target_id++){
                if(segment.second.at(target_id).empty()){
                    continue;
                }
                NodeVisit node_visit;
                node_visit.visit_type = VISIT_TYPE::JUMP_UP;
                node_visit.area = foot_segment[segment.first].at(target_id);
                path_nodes[platform.id].targets[target_id].id = target_id;
                path_nodes[platform.id].targets[target_id].type = path_nodes[target_id].type;
                path_nodes[platform.id].targets[target_id].visit_list.push_back(node_visit);
            }
        }
    }
}

void PathAlgo::jump_down(
    const std::vector<Platform> &platform_list,
    const std::vector<ClimbRect> &climb_list,
    const cv::Mat &room_map,
    const MapBounding &map_bounding,
    std::unordered_map<int, PathNode> &path_nodes)
{
    for(const auto &platform : platform_list){
        if(platform.type == 1){
            continue;
        }

        // std::vector<TargetPoints> targets;
        // targets.resize(platform_list.size());
        std::unordered_map<int, std::vector<TargetPoints>> targets;
        targets[0].resize(platform_list.size());
        targets[1].resize(platform_list.size());

        // 遍历平台的地板
        for(auto foot : platform.line_child){

            cv::LineIterator iterator(room_map, foot.start, foot.end, 8);
            // 遍历地板的每一个像素
            for (int i = 0; i < iterator.count; i++, ++iterator) {
                cv::Point start = iterator.pos();
                cv::Point check_clib = start;
                // 检测攀爬物,判断是否在任何一个 ClimbRect 中
                bool inClimb = false;
                // 遍历10个像素
                for(int down_line = 1; down_line <= 10; down_line ++){
                    check_clib.y = start.y + down_line;
                    if(check_clib.y >= map_bounding.height){
                        check_clib.y = map_bounding.height - 1;
                    }
                    // 直接判断这个位置的像素是否是攀爬
                    cv::Vec3b cur_color = room_map.at<cv::Vec3b>(check_clib);
                    if(cur_color == COLOR_CLIMB)
                    {
                        inClimb = true;
                        break;
                    }
                }

                if (inClimb) {
                    continue;
                }

                // 跳跃到下一层
                cv::Point end = start;
                end.y = map_bounding.y_bottom;

                // 预测下跳轨迹
                cv::Point target_pos = {-1, -1};
                // 遍历地板像素点到地图底部的所有像素
                cv::LineIterator jump_iterator(room_map, start, end, 8);
                for(int j = 0; j < jump_iterator.count; j++, ++jump_iterator)
                {
                    cv::Point cur_pos = jump_iterator.pos();
                    cv::Vec3b cur_color = room_map.at<cv::Vec3b>(jump_iterator.pos());

                    // 下跳时不找攀爬物体。如果遇到黑色像素就是找到平台了。
                    if(cur_color == COLOR_LINE)
                    {
                        // qDebug() << cur_pos.x << "," << cur_pos.y;
                        if(cur_pos.y - start.y > 4){ // 其实大于2就行
                            // 如果目标地点是本平台，则直接忽略
                            int line_platform = RoomMap::getPosPlatformByMAP(cur_pos);
                            if(line_platform == platform.id || line_platform == -1){
                                break;
                            }
                            targets[0].at(line_platform).push_back(iterator.pos());
                            break;
                        }
                    }
                }
            }
        }

        // std::vector<LineSegment> foot_segment = Helper::mergeFootPoint(targets);
        std::unordered_map<int, std::vector<LineSegment>> foot_segment = Helper::mergeFootPoint(targets);

        // for(int target_id = 0; target_id < foot_segment.size(); target_id++){
        //     if(foot_segment.at(target_id).empty()){
        //         continue;
        //     }
        //     NodeVisit node_visit;
        //     node_visit.visit_type = VISIT_TYPE::JUMP_DOWN;
        //     node_visit.area = foot_segment.at(target_id);
        //     path_nodes[platform.id].targets[target_id].visit_list.push_back(node_visit);
        // }

        for(const auto &segment : foot_segment){
            for(int target_id = 0; target_id < segment.second.size(); target_id++){
                if(segment.second.at(target_id).empty()){
                    continue;
                }
                NodeVisit node_visit;
                node_visit.visit_type = VISIT_TYPE::JUMP_DOWN;
                node_visit.area = foot_segment[segment.first].at(target_id);
                path_nodes[platform.id].targets[target_id].id = target_id;
                path_nodes[platform.id].targets[target_id].type = path_nodes[target_id].type;
                path_nodes[platform.id].targets[target_id].visit_list.push_back(node_visit);
            }
        }
    }
}

void PathAlgo::up(
    const std::vector<Platform> &platform_list,
    const cv::Mat &room_map,
    std::unordered_map<int, PathNode> &path_nodes
    )
{
    for(const auto &platform : platform_list){

        // 人物碰撞范围内是否有梯子
        if(platform.type == 0){
            // std::vector<TargetPoints> targets;
            // targets.resize(platform_list.size());
            std::unordered_map<int, std::vector<TargetPoints>> targets;
            targets[0].resize(platform_list.size());
            targets[1].resize(platform_list.size());

            for(auto foot : platform.line_child){

                cv::LineIterator iterator(room_map, foot.start, foot.end, 8);
                for (int i = 0; i < iterator.count; i++, ++iterator) {

                    // 向上找攀爬物体
                    cv::Point start = iterator.pos();
                    start.y -= 1;
                    cv::Point end = start;
                    end.y -= 22; // 目前发现最大值是23

                    // 直接判断这个位置的像素是否是攀爬
                    cv::Vec3b cur_color = room_map.at<cv::Vec3b>(end);
                    if(cur_color == COLOR_CLIMB)
                    {
                        int climb_platform = RoomMap::getPosPlatformByMAP(end);
                        if(climb_platform != -1){
                            targets[1].at(climb_platform).push_back(iterator.pos());
                        }
                    }
                }
            }

            // std::vector<LineSegment> foot_segment = Helper::mergeFootPoint(targets);
            std::unordered_map<int, std::vector<LineSegment>> foot_segment = Helper::mergeFootPoint(targets);

            // for(int target_id = 0; target_id < foot_segment.size(); target_id++){
            //     if(foot_segment.at(target_id).empty()){
            //         continue;
            //     }
            //     NodeVisit node_visit;
            //     node_visit.visit_type = VISIT_TYPE::UP;
            //     node_visit.area = foot_segment.at(target_id);
            //     path_nodes[platform.id].targets[target_id].visit_list.push_back(node_visit);
            // }
            for(const auto &segment : foot_segment){
                for(int target_id = 0; target_id < segment.second.size(); target_id++){
                    if(segment.second.at(target_id).empty()){
                        continue;
                    }
                    NodeVisit node_visit;
                    node_visit.visit_type = VISIT_TYPE::UP;
                    node_visit.area = foot_segment[segment.first].at(target_id);
                    path_nodes[platform.id].targets[target_id].id = target_id;
                    path_nodes[platform.id].targets[target_id].type = path_nodes[target_id].type;
                    path_nodes[platform.id].targets[target_id].visit_list.push_back(node_visit);
                }
            }
        }

        // 向上找地板
        if(platform.type == 1){
            // 直接定位攀爬的最上面那个像素
            int climb_x = platform.start.x + 25;
            cv::Point start(climb_x, platform.end.y); // 从下面的像素开始

            cv::Point end = start;
            end.y = platform.start.y - 10; // 目前发现如果攀爬体距离地面有10的距离也可以到达地面

            cv::Point target_pos = {-1, -1};
            cv::LineIterator iterator(room_map, start, end, 8);
            for(int j = 0; j < iterator.count; j++, ++iterator)
            {
                cv::Point cur_pos = iterator.pos();
                cv::Vec3b cur_color = room_map.at<cv::Vec3b>(iterator.pos());

                // 找到地板就退出
                if(cur_color == COLOR_LINE)
                {
                    // 如果目标地点是本平台，则直接忽略
                    int line_platform = RoomMap::getPosPlatformByMAP(cur_pos);
                    if( line_platform == -1){
                        continue;
                    }

                    LineSegment line_segment;
                    line_segment.push_back(std::pair(cur_pos.y, cur_pos.y));

                    NodeVisit node_visit;
                    node_visit.visit_type = VISIT_TYPE::UP;
                    node_visit.area = line_segment;

                    path_nodes[platform.id].targets[line_platform].id = line_platform;
                    path_nodes[platform.id].targets[line_platform].type = path_nodes[line_platform].type;
                    path_nodes[platform.id].targets[line_platform].visit_list.push_back(node_visit);
                    break;
                }
            }
        }
    }
}

void PathAlgo::down(const std::vector<Platform> &platform_list,
    const cv::Mat &room_map,
    const MapBounding &map_bounding,
    std::unordered_map<int, PathNode> &path_nodes)
{
    for(const auto &platform : platform_list){

        // 先处理从地板向下攀爬
        if(platform.type == 0){
            // std::vector<TargetPoints> targets;
            // targets.resize(platform_list.size());
            std::unordered_map<int, std::vector<TargetPoints>> targets;
            targets[0].resize(platform_list.size());
            targets[1].resize(platform_list.size());

            for(auto foot : platform.line_child){

                cv::LineIterator iterator(room_map, foot.start, foot.end, 8);
                for (int i = 0; i < iterator.count; i++, ++iterator) {

                    // 向下攀爬
                    cv::Point start = iterator.pos();
                    // start.y -= 1;
                    cv::Point end = start;

                    // 遍历10个像素
                    for(int down_line = 1; down_line <= 10; down_line++){
                        end.y = start.y + down_line;
                        if(end.y >= map_bounding.height){
                            end.y = map_bounding.height - 1;
                        }

                        // 直接判断这个位置的像素是否是攀爬
                        cv::Vec3b cur_color = room_map.at<cv::Vec3b>(end);
                        if(cur_color == COLOR_CLIMB)
                        {
                            int climb_platform = RoomMap::getPosPlatformByMAP(end);
                            if(climb_platform != -1){
                                targets[1].at(climb_platform).push_back(iterator.pos());
                            }
                            break;
                        }
                    }
                }
            }

            // std::vector<LineSegment> foot_segment = Helper::mergeFootPoint(targets);
            std::unordered_map<int, std::vector<LineSegment>> foot_segment = Helper::mergeFootPoint(targets);

            // for(int target_id = 0; target_id < foot_segment.size(); target_id++){
            //     if(foot_segment.at(target_id).empty()){
            //         continue;
            //     }
            //     NodeVisit node_visit;
            //     node_visit.visit_type = VISIT_TYPE::DOWN;
            //     node_visit.area = foot_segment.at(target_id);
            //     path_nodes[platform.id].targets[target_id].visit_list.push_back(node_visit);
            // }

            for(const auto &segment : foot_segment){
                for(int target_id = 0; target_id < segment.second.size(); target_id++){
                    if(segment.second.at(target_id).empty()){
                        continue;
                    }
                    NodeVisit node_visit;
                    node_visit.visit_type = VISIT_TYPE::DOWN;
                    node_visit.area = foot_segment[segment.first].at(target_id);
                    path_nodes[platform.id].targets[target_id].id = target_id;
                    path_nodes[platform.id].targets[target_id].type = path_nodes[target_id].type;
                    path_nodes[platform.id].targets[target_id].visit_list.push_back(node_visit);
                }
            }
        }

        // 如果是从攀爬物体向下爬
        if(platform.type == 1){ // 如果是攀爬体类型

            // 直接定位攀爬的最下面那个像素
            int climb_x = platform.start.x + 25;
            cv::Point start(climb_x, platform.end.y + 1); // 从下面的像素开始

            cv::Point end = start;
            end.y = map_bounding.y_bottom;

            cv::Point target_pos = {-1, -1};
            cv::LineIterator jump_iterator(room_map, start, end, 8);
            for(int j = 0; j < jump_iterator.count; j++, ++jump_iterator)
            {
                cv::Point cur_pos = jump_iterator.pos();
                cv::Vec3b cur_color = room_map.at<cv::Vec3b>(jump_iterator.pos());

                // 找到下面的地板
                if(cur_color == COLOR_LINE)
                {
                    // 如果目标地点是本平台，则直接忽略
                    int line_platform = RoomMap::getPosPlatformByMAP(cur_pos);
                    if( line_platform == -1){
                        continue;
                    }

                    LineSegment line_segment;
                    line_segment.push_back(std::pair(cur_pos.y, cur_pos.y));

                    NodeVisit node_visit;
                    node_visit.visit_type = VISIT_TYPE::DOWN;
                    node_visit.area = line_segment;

                    path_nodes[platform.id].targets[line_platform].id = line_platform;
                    path_nodes[platform.id].targets[line_platform].type = path_nodes[line_platform].type;
                    path_nodes[platform.id].targets[line_platform].visit_list.push_back(node_visit);
                    break;
                }
            }
        }
    }
}

void PathAlgo::skillteleport(const std::vector<Platform> &platform_list, const cv::Mat &room_map, const MapBounding &map_bounding, std::unordered_map<int, PathNode> &path_nodes)
{
    for(const auto &platform : platform_list){
        // 目标平台不可以是攀爬体
        if(platform.type == 1){
            continue;
        }

        std::unordered_map<int, std::vector<TargetPoints>> targets;
        targets[0].resize(platform_list.size());
        targets[1].resize(platform_list.size());

        // 向上或向下2*180范围
        // 向左或向右2*130范围，且高低不超过2*80

        // 向上传送
        for(auto foot : platform.line_child){
            cv::LineIterator iterator(room_map, foot.start, foot.end, 8);
            for (int i = 0; i < iterator.count; i++, ++iterator) {
                cv::Point start = iterator.pos();
                cv::Point end = start;
                end.y -= 180;

                cv::Point target_pos = {-1, -1};
                cv::LineIterator jump_iterator(room_map, start, end, 8);

                for(int j = 0; j < jump_iterator.count; j++, ++jump_iterator)
                {
                    cv::Point cur_pos = jump_iterator.pos();
                    cv::Vec3b cur_color = room_map.at<cv::Vec3b>(jump_iterator.pos());
                    if(cur_color == COLOR_LINE)
                    {
                        if(start.y - cur_pos.y > 1){
                            // 如果目标地点是本平台，则直接忽略
                            int line_platform = RoomMap::getPosPlatformByMAP(cur_pos);
                            if(line_platform == platform.id || line_platform == -1){
                                continue;
                            }
                            targets[0].at(line_platform).push_back(iterator.pos());
                        }
                    }
                }
            }
        }

        std::unordered_map<int, std::vector<LineSegment>> foot_segment = Helper::mergeFootPoint(targets);
        for(const auto &segment : foot_segment){
            for(int target_id = 0; target_id < segment.second.size(); target_id++){
                if(segment.second.at(target_id).empty()){
                    continue;
                }
                NodeVisit node_visit;
                node_visit.visit_type = VISIT_TYPE::SKILL;
                node_visit.area = foot_segment[segment.first].at(target_id);
                path_nodes[platform.id].targets[target_id].id = target_id;
                path_nodes[platform.id].targets[target_id].type = path_nodes[target_id].type;
                path_nodes[platform.id].targets[target_id].visit_list.push_back(node_visit);
            }
        }
    }
}

Matrix4x4 CameraTool::ReadMatrix(quint64 addr)
{
    Matrix4x4 mat{};
    Mem::readMemory(addr, reinterpret_cast<PBYTE>(mat.m), sizeof(mat.m));
    return mat;
}

void CameraTool::WorldToClip(const COORDINATE &pos, const Matrix4x4 &view, const Matrix4x4 &proj, float &cx, float &cy, float &cz, float &cw)
{
    float x = pos.x;
    float y = pos.y;
    float z = pos.z;

    // View
    float vx = x*view.m[0] + y*view.m[4] + z*view.m[8]  + view.m[12];
    float vy = x*view.m[1] + y*view.m[5] + z*view.m[9]  + view.m[13];
    float vz = x*view.m[2] + y*view.m[6] + z*view.m[10] + view.m[14];
    float vw = x*view.m[3] + y*view.m[7] + z*view.m[11] + view.m[15];

    // Projection
    cx = vx*proj.m[0] + vy*proj.m[4] + vz*proj.m[8]  + vw*proj.m[12];
    cy = vx*proj.m[1] + vy*proj.m[5] + vz*proj.m[9]  + vw*proj.m[13];
    cz = vx*proj.m[2] + vy*proj.m[6] + vz*proj.m[10] + vw*proj.m[14];
    cw = vx*proj.m[3] + vy*proj.m[7] + vz*proj.m[11] + vw*proj.m[15];
/*
1280 * 720的分辨率，矩阵数据如下。不能直接用，-41.02 2.84这两个数据是会变化的
ViewMatrix
1.00 0.00 0.00 0.00
0.00 1.00 0.00 0.00
0.00 0.00 -1.00 0.00
-41.02 2.84 -30.00 1.00

ProjectionMatrix
0.16 0.00 0.00 0.00
0.00 0.28 0.00 0.00
0.00 0.00 0.00 0.00
0.00 0.00 -1.00 1.00
*/
}

bool CameraTool::WorldToScreen(uintptr_t cam, const COORDINATE &pos, cv::Point &out)
{
    uintptr_t viewBase = cam + 0x80;
    uintptr_t projBase = cam + 0xC0;

    Matrix4x4 view = ReadMatrix(viewBase);
    Matrix4x4 proj = ReadMatrix(projBase);

    float cx, cy, cz, cw;
    WorldToClip(pos, view, proj, cx, cy, cz, cw);

    if (cw < 0.1f)
        return false;

    float ndcX = cx / cw;
    float ndcY = cy / cw;

    // 👉 这里你写死的分辨率
    float width  = Gplayer.camera_width;
    float height = Gplayer.camera_height;

    out.x = (ndcX + 1.0f) * 0.5f * width;
    out.y = (1.0f - ndcY) * 0.5f * height;

    return true;
}

// void MSKM::humanMove(int targetX, int targetY)
// {
//     targetX = Gplayer.game_window.x + targetX;
//     targetY = Gplayer.game_window.y + targetY;

//     QPoint currentPos = QCursor::pos();
//     int currentX = currentPos.x();
//     int currentY = currentPos.y();

//     // 计算直径
//     int length = qSqrt(qPow((currentX-targetX),2)+qPow((currentY-targetY),2));
//     int controllValue = 5;
//     int rand_value = 15;
//     if(length < 300)
//     {
//         // 控制弧度
//         controllValue = 5;
//         // 控制抖动
//         rand_value = 3;
//     }

//     // 随机化控制点1的偏移量
//     int control1OffsetX = QRandomGenerator::global()->bounded(-controllValue, controllValue);
//     int control1OffsetY = QRandomGenerator::global()->bounded(-controllValue, controllValue);

//     // 随机化控制点2的偏移量
//     int control2OffsetX = QRandomGenerator::global()->bounded(-controllValue, controllValue);
//     int control2OffsetY = QRandomGenerator::global()->bounded(-controllValue, controllValue);

//     // 控制点1坐标
//     int control1X = currentX + (targetX - currentX) / 3 + control1OffsetX;
//     int control1Y = currentY - qAbs(targetY - currentY) / 2 - controllValue + control1OffsetY; // 调整控制点1的Y坐标偏移量
//     control1X = targetX;
//     control1Y = targetY;

//     // 控制点2坐标
//     int control2X = currentX + 2 * (targetX - currentX) / 3 + control2OffsetX;
//     int control2Y = targetY + qAbs(targetY - currentY) / 2 + controllValue + control2OffsetY; // 调整控制点2的Y坐标偏移量
//     control2Y = targetY;

//     // 贝塞尔曲线轨迹
//     for (double t = 0; t <= 1; t += 0.03) {
//         // 计算当前点的坐标
//         qreal x = qPow(1 - t, 3) * currentX + 3 * qPow(1 - t, 2) * t * control1X + 3 * (1 - t) * qPow(t, 2) * control2X + qPow(t, 3) * targetX;
//         qreal y = qPow(1 - t, 3) * currentY + 3 * qPow(1 - t, 2) * t * control1Y + 3 * (1 - t) * qPow(t, 2) * control2Y + qPow(t, 3) * targetY;

//         // 添加较低频率的随机抖动效果
//         if (QRandomGenerator::global()->bounded(100) < rand_value) {
//             int jitterX = QRandomGenerator::global()->bounded(-5, 5);
//             int jitterY = QRandomGenerator::global()->bounded(-5, 5);
//             x += jitterX;
//             y += jitterY;
//         }

//         // 移动鼠标
//         MSDriverPRO::mouseMoveTo(qRound(x), qRound(y));

//         // 等待一段时间
//         QThread::msleep(QRandomGenerator::global()->bounded(5, 15));
//     }

//     MSDriverPRO::mouseMoveTo(targetX, targetY);
// }

void MSKM::inputNum(int num)
{
    if(num <= 0){
        return;
    }

    // 定义键盘枚举数组
    const int keyboardKeys[] = {
        Keyboard_0, Keyboard_1, Keyboard_2, Keyboard_3, Keyboard_4,
        Keyboard_5, Keyboard_6, Keyboard_7, Keyboard_8, Keyboard_9
    };

    // 转换为QString并逐字符处理
    QString numStr = QString::number(num);

    for (int i = 0; i < numStr.length(); ++i) {
        QChar ch = numStr.at(i);

        if (ch == '-') {
            continue;
        }

        int digit = ch.digitValue(); // QChar转数字，-1表示非数字
        if (digit >= 0 && digit <= 9) {
            MSDK::keyPress(keyboardKeys[digit], 1);
        }
    }
}
