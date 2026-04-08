#include "helper.h"
#include "driver/mem.h"
#include <set>
#include <utility>  // for std::pair
#include <unordered_set>
#include "global/gameoffset.h"
#include "global/globalplayer.h"

using namespace Mem;

void DebugOutput::out(const QString& message){
    QString prefix = "[myprint] ";
    QString formattedMsg = prefix + message;
    QString output = formattedMsg + "\n";

    // 输出到 DebugView
    OutputDebugStringW(output.toStdWString().c_str());
}



quint64 Helper::findChild(quint64 parent, QString name)
{
    quint64 child_entity = 0;

    quint64 list = Mem::readLong(parent + BASE::MODEntity::children);
    int list_num = Mem::readLong(list + BASE::List::size);
    if(list_num == 0){
        qDebug() << "不存在子模块";
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
    int y_top = toMapPosY(map_y_top,y_offset,map_height);
    int y_bottom = toMapPosY(map_y_bottom,y_offset,map_height);

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

int RoomMap::toMapPosX(float x, int offset)
{
    return x * 100 + offset;
}

int RoomMap::toMapPosY(float y, int offset, int height)
{
    return height - (y * 100 + offset);
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

    // 找碰撞数据
    quint64 p_MultipleDynamicTreeBroadPhase = Mem::readPointer(collisionservice,
                                                                BASE::CollisionService::simulatorByMap,
                                                                BASE::Dictionary::InternalArray_entries,
                                                                BASE::InternalArray::Head + 0x10,
                                                                BASE::CollisionSimulator::World,
                                                                BASE::World::MultipleDynamicTreeBroadPhase);
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
    // int rect_count = readInt(readLong(p_MultipleDynamicTreeBroadPhase + 0x10) + 0x18);
    // int rect_array_1_count = readInt(readLong(readLong(p_MultipleDynamicTreeBroadPhase + 0x10) + 0x10) + 0);
    int rect_array_2_count = readInt(readLong(readLong(p_MultipleDynamicTreeBroadPhase + BASE::MultipleDynamicTreeBroadPhase::trees) + 0x10) + 0x10);
    quint64 rect_head = readLong(p_MultipleDynamicTreeBroadPhase + BASE::MultipleDynamicTreeBroadPhase::trees) + BASE::InternalArray::Head;

    int climb_id = 0; // 生成一个ID
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
            cur_rect.id = climb_id;

            cur_rect.x_left = toMapPosX(float_rect.x_left, map_bounding.x_offset);
            cur_rect.x_right = toMapPosX(float_rect.x_right, map_bounding.x_offset);
            cur_rect.y_top = toMapPosY(float_rect.y_top, map_bounding.y_offset, map_bounding.height);
            cur_rect.y_bottom = toMapPosY(float_rect.y_bottom, map_bounding.y_offset, map_bounding.height);

            climb_list.push_back(cur_rect);
            climb_id++;
        }
    }

    return climb_list;
}

std::vector<std::vector<COORDINATE>> RoomMap::dumpTilemapList(quint64 map)
{
    std::vector<std::vector<COORDINATE>> tile_list;

    int list_count = readInt(readLong(map + BASE::MODEntity::children) + BASE::List::size);
    quint64 list_head = readLong(readLong(map + BASE::MODEntity::children) + BASE::List::InternalArray) + BASE::InternalArray::Head;

    for(int i = 0; i < list_count; i++,list_head += 0x8){
        quint64 cur_entity = Mem::readLong(list_head);
        if(cur_entity == 0) break;

        // 判断是否有TileMapComponent
        quint64 tilemapcomponet = readLong(cur_entity + BASE::MODEntity::TileMapComponent);
        if(tilemapcomponet == 0) continue;
        QString entity_name = readString(readLong(cur_entity + BASE::MODEntity::name) + BASE::String::text, 100);
        // qDebug() << "当前模块:" << entity_name << " 指针：" << QString::number(tilemapcomponet,16).toUpper();

        // 遍历TileContext
        quint64 tilecontext_list = readLong(tilemapcomponet + BASE::TileMapComponent::TileContextList);
        int tilecontext_list_count = readInt(tilecontext_list + BASE::List::size);
        quint64 tilecontext_list_head = readLong(tilecontext_list + BASE::List::InternalArray) + BASE::InternalArray::Head;

        // 遍历tilecontext_list
        for(int j = 0; j < tilecontext_list_count; j++,tilecontext_list_head += 0x8){
            quint64 convexPoints = readLong(readLong(tilecontext_list_head) + BASE::TileContext::convexPoints);
            int convexPoints_count = readInt(convexPoints + BASE::List::size);
            quint64 convexPoints_head = readLong(convexPoints + BASE::List::InternalArray) + BASE::InternalArray::Head;

            std::vector<COORDINATE> cur_tile;
            // qDebug() << "===============";
            // 遍历坐标
            for(int k = 0; k < convexPoints_count; k++,convexPoints_head += 0x8){
                float cur_x = Math::roundHalfAwayFromZero_bit(readFloat(convexPoints_head));
                float cur_y = Math::roundHalfAwayFromZero_bit(readFloat(convexPoints_head + 0x4));
                cur_tile.push_back({cur_x, cur_y});
                // qDebug() << cur_x << "," << cur_y;
            }
            tile_list.push_back(cur_tile);
        }
        // list_head += 0x8;
    }

    return tile_list;
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

std::vector<Wall> RoomMap::makeWallList(std::vector<FootholdRect> footholds, std::unordered_set<int> &id_set)
{
    std::vector<Wall> wall_list;
    for (auto item : footholds) {
        if (id_set.find(item.id) == id_set.end()) {
            continue;
        }

        // 如果item是一个墙体(x相同，y不同)
        if (item.x_left == item.x_right && item.y_left != item.y_right) {
            Wall wall;
            wall.foot_id = item.id;
            wall.x = item.x_left;
            wall.bottom = std::max(item.y_left, item.y_right);
            wall.top = std::min(item.y_left, item.y_right);

            wall_list.push_back(wall);
            id_set.erase(item.id);
            continue;
        }
    }

    return wall_list;
}

std::vector<Line> RoomMap::makeLineList(std::vector<FootholdRect> footholds, std::vector<Wall> wall_list, std::unordered_set<int> &id_set)
{
    std::vector<Line> line_list;

    int line_id = 0;
    for (auto item : footholds) {
        // 如果元素x的id不存在于id_set中，则进入下一层循环
        if (id_set.find(item.id) == id_set.end()) {
            continue;
        }

        // 将该item的id从id_set里面移除
        id_set.erase(item.id);

        // 如果是直线（y相同）或者是斜线（x_left与x_right不同且y不同）
        if (item.y_left == item.y_right ||
            (item.x_left != item.x_right && item.y_left != item.y_right)) {


            if(item.x_left > item.x_right){
                qDebug() << "数据不正确，退出计算~";
                line_list.clear();
                return line_list;
            }

            Line tmp_line;

            // 创建第一个line_child
            LineChild first_child;
            first_child.foot_id = item.id;
            first_child.start = cv::Point(item.x_left,item.y_left);
            first_child.end = cv::Point(item.x_right,item.y_right);
            first_child.length = qAbs(item.x_right - item.x_left) + qAbs(item.y_right - item.y_left);

            tmp_line.line_child.push_back(first_child);
            tmp_line.start = cv::Point(item.x_left,item.y_left);
            tmp_line.end = cv::Point(item.x_right,item.y_right);

            for (auto& other_item : footholds) {
                if (id_set.find(other_item.id) == id_set.end()) {
                    continue;
                }

                // 拼接当前平台左边的地板
                if (cv::Point(other_item.x_right,other_item.y_right) == tmp_line.start) {
                    id_set.erase(other_item.id);

                    LineChild child;
                    child.foot_id = other_item.id;
                    child.start = cv::Point(other_item.x_left,other_item.y_left);
                    child.end = cv::Point(other_item.x_right,other_item.y_right);
                    child.length = qAbs(other_item.x_right - other_item.x_left) + qAbs(other_item.y_right - other_item.y_left);

                    // 将line_child插入到容器tmp_line.line_child前面
                    tmp_line.line_child.insert(tmp_line.line_child.begin(), child);
                    tmp_line.start = cv::Point(other_item.x_left,other_item.y_left);
                    continue;
                }

                // 拼接当前平台右边的地板
                if (cv::Point(other_item.x_left,other_item.y_left) == tmp_line.end) {
                    id_set.erase(other_item.id);

                    LineChild child;
                    child.foot_id = other_item.id;
                    child.start = cv::Point(other_item.x_left,other_item.y_left);
                    child.end = cv::Point(other_item.x_right,other_item.y_right);
                    child.length = qAbs(other_item.x_right - other_item.x_left) + qAbs(other_item.y_right - other_item.y_left);

                    // 将line_child插入到容器tmp_line.line_child后面
                    tmp_line.line_child.push_back(child);
                    tmp_line.end = cv::Point(other_item.x_right,other_item.y_right);
                    continue;
                }
            }

            // 注意：需要确保start的x最小
            if (tmp_line.start.x > tmp_line.end.x) {
                qDebug() << "数据不正确2，退出计算~";
                line_list.clear();
                return line_list;
            }

            // 创建平台的墙体属性
            for (const auto& wall : wall_list) {
                // 检查左墙
                if (wall.x == tmp_line.start.x &&
                    wall.bottom == tmp_line.start.y) {
                    tmp_line.hasLeftWall = true;
                }else{
                    tmp_line.hasLeftWall = false;
                }
                // 检查右墙
                if (wall.x == tmp_line.end.x &&
                    wall.bottom == tmp_line.end.y) {
                    tmp_line.hasRightWall = true;
                }else{
                    tmp_line.hasRightWall = false;
                }
            }

            tmp_line.lineid = line_id;
            line_id++;
            line_list.push_back(tmp_line);
        }
    }

    return line_list;
}

cv::Mat RoomMap::makeRoomMap(std::vector<ClimbRect> climb_list, std::vector<FootholdRect> foothold_list, MapBounding map_bounding)
{
    // 创建地图文件
    cv::Mat cur_room_map(map_bounding.height, map_bounding.width, CV_8UC3, cv::Scalar(255, 255, 255));

    // 填充攀爬
    for(auto item : climb_list){
        int x_width = item.x_right - item.x_left;
        int y_height = item.y_bottom - item.y_top;
        cv::Rect rect(item.x_left, item.y_top, x_width, y_height);
        cv::rectangle(cur_room_map, rect, cv::Scalar(0, 0xFF, 0), -1); // 用黄色填充矩形-攀爬
    }

    // 绘制地板
    for(auto item : foothold_list){
        if(item.x_left == item.x_right){
            // 用红色绘制墙壁
            cv::line(cur_room_map,cv::Point(item.x_left,item.y_left),cv::Point(item.x_right,item.y_right),cv::Scalar(0xFF,0,0));
        }else{
            // 用黑色绘制地板
            cv::line(cur_room_map,cv::Point(item.x_left,item.y_left),cv::Point(item.x_right,item.y_right),cv::Scalar(0,0,0));
        }

    }

    return cur_room_map;
}

void RoomMap::makeNodes(std::vector<Line> line_list, std::vector<Wall> wall_list, cv::Mat cur_room_map, MapBounding map_bounding)
{
    // 遍历线路
    qDebug() << "向左走寻路";
    for(auto item : line_list){
        qDebug() << "搜索线路：" << item.lineid;

        // 判断左边是否有墙
        if(item.hasLeftWall){
            qDebug() << "左边有墙，退出寻路";
            continue;
        }

        if(item.start.x == 0){
            qDebug() << "左边是边界，退出寻路";
            continue;
        }


        cv::Point to_left_pos = ImageTool::dropTrajectory(false,cur_room_map,item.start.x,item.start.y);
        if(to_left_pos.x != -1){
            qDebug() << "经过位置：" << to_left_pos.x << "," << to_left_pos.y;
        }

        for(auto wall_item : wall_list){
            if(ImageTool::isPointInWall(to_left_pos,wall_item)){
                qDebug() << "该点位于墙体中，不可用";
            }
        }

        bool searched_line_id = false;
        for(auto it : line_list){
            if(searched_line_id){
                break;
            }
            for (const auto& child : it.line_child)
            {
                if (ImageTool::isPointOnSegment(to_left_pos, child.start, child.end))
                {
                    // return;
                    qDebug() << "该点位于线路ID：" << it.lineid;
                    searched_line_id = true;
                    break;

                }
            }
        }
    }

    qDebug() << "向右走寻路";
    for(auto item : line_list){
        qDebug() << "搜索线路：" << item.lineid;

        // 判断右边是否有墙
        if(item.hasRightWall){
            qDebug() << "右边有墙，退出寻路";
            continue;
        }

        if(item.end.x == map_bounding.width){
            qDebug() << "右边是边界，退出寻路";
            continue;
        }

        cv::Point to_right_pos = ImageTool::dropTrajectory(true,cur_room_map,item.end.x,item.end.y);
        if(to_right_pos.x != -1){
            qDebug() << "经过位置：" << to_right_pos.x << "," << to_right_pos.y;
        }

        for(auto item : wall_list){
            if(ImageTool::isPointInWall(to_right_pos,item)){
                qDebug() << "该点位于墙体中，不可用";
            }
        }

        bool searched_line_id = false;
        for(auto it : line_list){
            if(searched_line_id){
                break;
            }
            for (const auto& child : it.line_child)
            {
                if (ImageTool::isPointOnSegment(to_right_pos, child.start, child.end))
                {
                    // return;
                    qDebug() << "该点位于线路ID：" << it.lineid;
                    searched_line_id = true;
                    break;
                }
            }
        }
    }
}

int RoomMap::getPlatformByFoothold(int foothold_id, std::vector<Line> line_list)
{
    for(auto item : line_list){
        for(auto foot : item.line_child){
            if(foot.foot_id == foothold_id){
                return item.lineid;
            }
        }
    }
    return -1;
}

int RoomMap::getClimbID(cv::Point position, std::vector<ClimbRect> climb_list)
{
    for(auto item : climb_list){
        if(position.x >= item.x_left &&
            position.x <= item.x_right &&
            position.y >= item.y_top &&
            position.y <= item.y_bottom){
            return item.id;
        }
    }
    return -1;
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

Monster Math::getNearMonster(std::vector<Monster> monsters, int x)
{
    Monster monster;

    int diff = 9999;
    for(auto item : monsters){
        int distance = abs(item.pos.x - x);
        if( distance < diff){
            diff = distance;
            monster = item;
        }
    }

    return monster;
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
        if(color == cv::Vec3b(0,0,0))
        {
            p3 = it.pos();
            return true;
        }

    }
    return false; // 没有黑色像素
}

cv::Point2i ImageTool::worldToImage(int width, int height, float x, float y)
{
    int col = static_cast<int>(std::round(x));
    int row = static_cast<int>(std::round(y));

    col = std::clamp(col, 0, width - 1);
    row = std::clamp(row, 0, height - 1);

    return cv::Point2i(col, row);
}

cv::Point ImageTool::dropTrajectory(bool is_right, cv::Mat image, float x, float y)
{
    cv::Point start_pos = {static_cast<int>(std::round(x)),static_cast<int>(std::round(y))};
    cv::Point black_pos = {-1,-1};
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
        if(checkLineHasBlackPixel(image, drawPoints[i-1], drawPoints[i], black_pos))
        {
            if(black_pos == start_pos){
                continue;
            }
            return black_pos;
            // cout << "直线 " << i-1 << "->" << i << " 遇到黑色像素！" << endl;
        }
    }

    return black_pos;
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
