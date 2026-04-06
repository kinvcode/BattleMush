#include "helper.h"
#include "driver/mem.h"
#include <set>
#include <utility>  // for std::pair
#include <unordered_set>
#include "global/gameoffset.h"

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

    quint64 list = Mem::readLong(parent + 0x3A8);
    int list_num = Mem::readLong(list + 0x18);
    if(list_num == 0){
        qDebug() << "不存在子模块";
        return child_entity;
    }

    quint64 list_head = Mem::readLong(list + 0x10);
    int max = Mem::readInt(list_head + 0x18);
    list_head = list_head + 0x20;
    for(int i = 0; i < max; i++,list_head += 0x8){
        quint64 cur_entity = Mem::readLong(list_head);
        if(cur_entity == 0){
            break;
        }

        QString entity_name = readString(readLong(cur_entity + 0x270) + 0x14, 100);
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

    int count = readInt(allservice + 0x18);
    quint64 head = readLong(allservice + 0x10) + 0x20;
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

std::unordered_set<quint64> Helper::findClimbableComponentSet(quint64 cur_map)
{
    std::unordered_set<quint64> pointers;

    QString map_uuid = readString(readLong(cur_map + 0x2A0)+0x14, 100);
    qDebug() << "当前地图UUID：" << map_uuid;

    int list_count = readInt(readLong(cur_map + 0x3A8) + 0x18);
    quint64 list_p = readLong(readLong(cur_map + 0x3A8) + 0x10);
    quint64 list_head = list_p + 0x20;

    for(int i = 0; i < list_count; i++,list_head += 8){
        quint64 cur_p = readLong(list_head);
        if(!cur_p) continue;

        quint64 ClimbableComponent = readLong(cur_p + 0xC8);
        if(!ClimbableComponent) continue;

        pointers.insert(ClimbableComponent);
    }

    return pointers;
}

QString Helper::getEntityTextComponentString(quint64 entity)
{
    QString name = "";
    name = readString(readLong(readLong(entity + 0x1F8) + 0xF0) + 0x14, 100);
    return name;
}

std::vector<ClimbRect> Helper::dumpClimbList(quint64 cur_map, quint64 collisionservice)
{
    std::vector<ClimbRect> climb_list;

    // 遍历地图，搜索所有的攀爬Entity
    std::unordered_set<quint64> climb_pointers = findClimbableComponentSet(cur_map);

    // 找碰撞数据
    quint64 p_MultipleDynamicTreeBroadPhase = Mem::readPointer(collisionservice, 0x38, 0x18, 0x30, 0x30, 0x18);
    DebugOutput::out("MultipleDynamicTreeBroadPhase:" + QString::number(p_MultipleDynamicTreeBroadPhase, 16).toUpper());

    // 遍历数组索引
    int index_count = readInt(readLong(p_MultipleDynamicTreeBroadPhase + 0x20) + 0x20);
    quint64 index_head = readLong(readLong(p_MultipleDynamicTreeBroadPhase + 0x20) + 0x18) + 0x20;
    std::set<std::pair<int, int>> uniquePairs;
    for(int index_i = 0; index_i < index_count; index_i++,index_head += 3*8){
        int index1 = readInt(index_head + 0xC);
        int index2 = readInt(index_head + 0x10);
        uniquePairs.insert({index1, index2});
    }

    // 遍历碰撞坐标原始数据
    int rect_count = readInt(readLong(p_MultipleDynamicTreeBroadPhase + 0x10) + 0x18);
    int rect_array_1_count = readInt(readLong(readLong(p_MultipleDynamicTreeBroadPhase + 0x10) + 0x10) + 0);
    int rect_array_2_count = readInt(readLong(readLong(p_MultipleDynamicTreeBroadPhase + 0x10) + 0x10) + 10);
    quint64 rect_head = readLong(p_MultipleDynamicTreeBroadPhase + 0x10) + 0x20;

    for (const auto& pair : uniquePairs) {
        quint64 cur_address = rect_head + pair.first * 0x20 * 8 + pair.second * 8;
        int node_count = readInt(readLong(cur_address) + 0x1C);
        quint64 node_head = readLong(readLong(cur_address) + 0x10) + 0x20;

        qDebug() << "--------------------------------------------------------";
        for(int node_i = 0; node_i < node_count; node_i++,node_head += 0x60){
            quint64 p_climbableComponent = readLong(node_head + 0x28);
            if(!p_climbableComponent) continue;

            qDebug() << "p_climbableComponent：" << QString::number(p_climbableComponent,16).toUpper();
            // 如果不是当前地图的攀爬物体,则跳过
            if (climb_pointers.find(p_climbableComponent) == climb_pointers.end()) {
                break;
            }
            ClimbOrigin cur_climb;
            cur_climb.left = readFloat(node_head + 0x30);
            cur_climb.top = readFloat(node_head + 0x34);
            cur_climb.factor1 = readFloat(node_head + 0x48);
            cur_climb.factor2 = readFloat(node_head + 0x4C);
            ClimbRect cur_rect;
            Math::ComputeClimbRect(&cur_climb, cur_rect);

            climb_list.push_back(cur_rect);

            // qDebug() << cur_rect.x_left << cur_rect.y_top << cur_rect.x_right << cur_rect.y_bottom;
        }
    }

    return climb_list;
}

std::vector<std::vector<COORDINATE>> Helper::dumpTilemapList(quint64 map)
{
    std::vector<std::vector<COORDINATE>> tile_list;

    int list_count = readInt(readLong(map + 0x3A8) + 0x18);
    quint64 list_head = readLong(readLong(map + 0x3A8) + 0x10) + 0x20;

    for(int i = 0; i < list_count; i++,list_head += 0x8){
        quint64 cur_entity = Mem::readLong(list_head);
        if(cur_entity == 0) break;

        // 判断是否有TileMapComponent
        quint64 tilemapcomponet = readLong(cur_entity + 0x88);
        if(tilemapcomponet == 0) continue;
        QString entity_name = readString(readLong(cur_entity + 0x270) + 0x14, 100);
        qDebug() << "当前模块:" << entity_name << " 指针：" << QString::number(tilemapcomponet,16).toUpper();

        // 遍历TileContext
        quint64 tilecontext_list = readLong(tilemapcomponet + 0x120);
        int tilecontext_list_count = readInt(tilecontext_list + 0x18);
        quint64 tilecontext_list_head = readLong(tilecontext_list + 0x10) + 0x20;

        // 遍历tilecontext_list
        for(int j = 0; j < tilecontext_list_count; j++,tilecontext_list_head += 0x8){
            quint64 convexPoints = readLong(readLong(tilecontext_list_head) + 0x20);
            int convexPoints_count = readInt(convexPoints + 0x18);
            quint64 convexPoints_head = readLong(convexPoints + 0x10) + 0x20;

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

std::vector<FootholdRect> Helper::dumpFootholdList(quint64 cur_map)
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
        int group = Mem::readInt(readLong(foot_head) + BASE::Foothold::groupID);
        qDebug() << "group:" << group;
        int id = readInt(readLong(foot_head) + BASE::Foothold::ID);
        float x_left = Math::roundHalfAwayFromZero_bit(readFloat(readLong(foot_head) + BASE::Foothold::StartPoint));
        float y_left = Math::roundHalfAwayFromZero_bit(readFloat(readLong(foot_head) + BASE::Foothold::StartPoint + 4));
        float x_right = Math::roundHalfAwayFromZero_bit(readFloat(readLong(foot_head) + BASE::Foothold::EndPoint));
        float y_right = Math::roundHalfAwayFromZero_bit(readFloat(readLong(foot_head) + BASE::Foothold::EndPoint + 4));
        qDebug() << x_left << "," << y_left << " " << x_right << "," << y_right;
        qDebug() << "------------------------";

        foothold_list.push_back({id, x_left, y_left, x_right, y_right});
    }

    return foothold_list;
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

void Math::ComputeClimbRect(const ClimbOrigin *climb, ClimbRect &climb_rect)
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
