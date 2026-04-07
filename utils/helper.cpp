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

std::unordered_set<quint64> Helper::findClimbableComponentSet(quint64 cur_map)
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

QString Helper::getEntityTextComponentString(quint64 entity)
{
    QString name = "";
    name = readString(readLong(readLong(entity + BASE::MODEntity::TextComponent) + BASE::TextComponent::_text) + BASE::String::text, 100);
    return name;
}

std::vector<ClimbRect> Helper::dumpClimbList(quint64 cur_map, quint64 collisionservice)
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

    // int array_lv2_count =
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
            ClimbRect cur_rect;
            Math::ComputeClimbRect(&cur_climb, cur_rect);

            climb_list.push_back(cur_rect);
        }
    }

    return climb_list;
}

std::vector<std::vector<COORDINATE>> Helper::dumpTilemapList(quint64 map)
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
        qDebug() << "当前模块:" << entity_name << " 指针：" << QString::number(tilemapcomponet,16).toUpper();

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
