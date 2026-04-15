#ifndef HELPER_H
#define HELPER_H

#include <windows.h>
#include <QString>
#include <QDateTime>
#include <QDebug>
#include "global/struct.h"
#include <unordered_set>

class Helper : public QObject
{
    Q_OBJECT
public:
    Helper();

    static void console(const QString& msg);

    void warning(QString msg);

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

    // 合并node节点(到达平台)
    static std::unordered_map<int, std::vector<LineSegment>> mergeFootPoint(
        const std::unordered_map<int, std::vector<TargetPoints>> &target_list
    );

    // 合并node节点(到达攀爬)
    static std::unordered_map<int, std::vector<LineSegment>> mergeClimbPoint(
        const std::unordered_map<int, std::vector<TargetPoints>> &target_list
    );

signals:
    void sendWarning(QString msg);
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

    // 找到最小差值
    static Monster getNearMonster(const std::vector<Monster> &monsters,int x);

    // 计算坐标是否在范围内（用于找怪）
    static bool posInRange(bool to_left, cv::Point posA, cv::Point posB, cv::Point range);
};

class ImageTool{
public:
    static void saveMatImage(cv::Mat image,QString path);

    static bool checkLineHasBlackPixel(const cv::Mat &img, const cv::Point &p1, const cv::Point &p2, cv::Point &p3);

    static bool checkLineHasPlatformPixel(const cv::Mat &img, const cv::Point &p1, const cv::Point &p2, cv::Point &p3);

    static bool checkLineHasColorPixel(const cv::Mat &img, const cv::Point &p1, const cv::Point &p2, cv::Vec3b color, cv::Point &target);

    static cv::Point2i worldToImage(int width, int height, float x, float y);

    static cv::Point dropTrajectory(bool is_right, const cv::Mat &image, float x, float y);

    static std::pair<cv::Point,cv::Point> jumpTrajectory(bool is_right, const cv::Mat &image, float x, float y);

    static std::pair<cv::Point,cv::Point> climbJumpTrajectory(bool is_right,
                                                        const cv::Mat &image,
                                                        ClimbRect climb_rect,
                                                        float x, float y);

    static bool isPointInWall(const cv::Point& p, const Wall& wall, int tolerance = 0);

    static bool isPointOnSegment(const cv::Point& p, const cv::Point& a, const cv::Point& b, int tolerance = 0);

    static bool isPointOnFoot(const cv::Point& pos);

    static bool isPointOnLine(const cv::Mat &map, const cv::Point &pos, const cv::Point &start, const cv::Point &end);
};

class RoomMap{
public:
    // 获取地图内所有的攀爬组件
    static std::unordered_set<quint64> findClimbableComponentSet(quint64 cur_map);

    // 获取地图边界
    static MapBounding getMapBounding(quint64 cur_map);

    // 转换地图坐标
    static int toMapPosX(float x, int offset);
    static int toMapPosY(float y, int offset,int height);

    // 创建攀爬物体
    static std::vector<ClimbRect> makeClimbList(quint64 cur_map, quint64 collisionservice, MapBounding map_bounding);

    // 创建footholds
    static std::vector<FootholdRect> makeFootholdList(quint64 cur_map, MapBounding map_bounding);

    // 创建墙体列表
    static std::vector<Wall> makeWallList(const std::vector<FootholdRect> &footholds, std::unordered_set<int> &id_set);

    // 创建平台列表
    static std::vector<Platform> makePlatformList(
        const std::vector<FootholdRect> &footholds,
        const std::vector<Wall> &wall_list,
        const std::vector<ClimbRect> &climb_list,
        MapBounding map_bounding,
        std::unordered_set<int> &id_set
    );

    // 创建房间地图
    static cv::Mat makeRoomMap(
        const std::vector<ClimbRect> &climb_list,
        const std::vector<FootholdRect> &foothold_list,
        const std::vector<Wall> &wall_list,
        const MapBounding &map_bounding
    );

    // 创建寻路节点
    static std::unordered_map<int, PathNode> makeNodes(
        const std::vector<Platform> &platform_list,
        const std::vector<Wall> &wall_list,
        const cv::Mat &cur_room_map,
        const MapBounding &map_bounding
    );

    // 计算物品所在平台坐标
    static cv::Point getItemLinePos(const cv::Mat &map, cv::Point pos);

    // 计算地板所在平台
    static int getFootPlatform(int foothold, const std::vector<Platform>& platform_list);

    // 计算坐标所在平台
    static int getPosPlatform(cv::Point pos, const std::vector<Platform> &platform_list, const cv::Mat &room_map);

    // 计算坐标所在攀爬平台
    static int getClimbPlatform(cv::Point pos, const std::vector<Platform> &platform_list, const cv::Mat &room_map);

    // 获取平台的类型
    static int getPlatformType(int platform_id, const std::vector<Platform> &platform_list);

    // 从缓存中加载地图
    static void loadLocalMap(const QString map_id);
};

class PathAlgo{
public:
    // 搜索A*路径
    static std::vector<int> findAStarPath(int platformA, int platformB, std::unordered_map<int, PathNode> &path_nodes);

    static void walkToLeft(
        const std::vector<Platform> &platform_list,
        const cv::Mat &cur_room_map,
        std::unordered_map<int, PathNode> &path_nodes
    );

    static void walkToRight(
        const std::vector<Platform> &platform_list,
        const cv::Mat &cur_room_map,
        const MapBounding &map_bounding,
        std::unordered_map<int, PathNode> &path_nodes
    );

    static void jumpLeft(
        const std::vector<Platform> &platform_list,
        const cv::Mat &room_map,
        std::unordered_map<int, PathNode> &path_nodes
    );

    static void jumpRight(
        const std::vector<Platform> &platform_list,
        const cv::Mat &room_map,
        std::unordered_map<int, PathNode> &path_nodes
    );

    static void jump_up(
        const std::vector<Platform> &platform_list,
        const cv::Mat &room_map,
        std::unordered_map<int, PathNode> &path_nodes
    );

    static void jump_down(
        const std::vector<Platform> &platform_list,
        const cv::Mat &room_map,
        const MapBounding &map_bounding,
        std::unordered_map<int, PathNode> &path_nodes
    );

    static void up(
        const std::vector<Platform> &platform_list,
        const cv::Mat &room_map,
        std::unordered_map<int, PathNode> &path_nodes
    );

    static void down(
        const std::vector<Platform> &platform_list,
        const cv::Mat &room_map,
        const MapBounding &map_bounding,
        std::unordered_map<int, PathNode> &path_nodes
    );

    static void teleport();

    static void skill();
};

















































#endif // HELPER_H
