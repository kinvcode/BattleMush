#ifndef HELPER_H
#define HELPER_H

#include <windows.h>
#include <QString>
#include <QDateTime>
#include <QDebug>
#include "global/struct.h"
#include <unordered_set>

class Helper
{
public:
    // 搜索Entity的子组件
    static quint64 findChild(quint64 parent, QString name);

    // 搜索服务指针
    static quint64 findService(quint64 allservice, QString name);

    // 获取Entity的文本组件字符串
    static QString getEntityTextComponentString(quint64 entity);

    // 分割字符串
    static bool parseHPString(const QString& Text, int& left, int& right);

    static bool isSamePos(const std::pair<float,float>& a,
                                  const std::pair<float,float>& b);
    // 判断是否有相同的坐标
    static void posExists(std::vector<std::pair<float, float>> &move_pos_list,std::pair<float, float> pos);
};

class DebugOutput
{
public:
    // 格式化输出
    static void out(const QString& message);
};

class Math{
public:
    // 计算攀爬物体RECT
    static void ComputeClimbRect(const ClimbOrigin* climb, FloatRect& climb_rect);

    // 保留2位小数点
    static float roundHalfAwayFromZero_bit(float value, int decimalPlaces = 2);
};

class ImageTool{
public:
    static void saveMatImage(cv::Mat image,QString path);

    static bool checkLineHasBlackPixel(const cv::Mat &img, const cv::Point &p1, const cv::Point &p2, cv::Point &p3);

    static cv::Point2i worldToImage(int width, int height, float x, float y);

    static cv::Point dropTrajectory(bool is_right, cv::Mat image, float x, float y);

    static bool isPointInWall(const cv::Point& p, const Wall& wall, int tolerance = 0);

    static bool isPointOnSegment(const cv::Point& p, const cv::Point& a, const cv::Point& b, int tolerance = 0);
};

class RoomMap{
public:
    // 获取地图内所有的攀爬组件
    static std::unordered_set<quint64> findClimbableComponentSet(quint64 cur_map);

    // 获取地图边界
    static MapBounding getMapBounding(quint64 cur_map);

    // 转换地图坐标
    static int toMapPosX(int x, int offset);
    static int toMapPosY(int y, int offset,int height);

    // 创建攀爬物体
    static std::vector<ClimbRect> makeClimbList(quint64 cur_map, quint64 collisionservice, MapBounding map_bounding);

    // 遍历瓦片
    static std::vector<std::vector<COORDINATE>> dumpTilemapList(quint64 map);

    // 创建footholds
    static std::vector<FootholdRect> makeFootholdList(quint64 cur_map, MapBounding map_bounding);

    // 创建墙体列表
    static std::vector<Wall> makeWallList(std::vector<FootholdRect> footholds, std::unordered_set<int> &id_set);

    // 创建平台列表
    static std::vector<Line> makeLineList(std::vector<FootholdRect> footholds, std::vector<Wall> wall_list, std::unordered_set<int> &id_set);

    // 创建房间地图
    static cv::Mat makeRoomMap(std::vector<ClimbRect> climb_list, std::vector<FootholdRect> foothold_list, MapBounding map_bounding);

    // 创建寻路节点
    static void makeNodes(std::vector<Line> line_list, std::vector<Wall> wall_list, cv::Mat cur_room_map, MapBounding map_bounding);



};

















































#endif // HELPER_H
