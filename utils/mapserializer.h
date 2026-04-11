#ifndef MAPSERIALIZER_H
#define MAPSERIALIZER_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDebug>
#include <opencv2/opencv.hpp>
#include <vector>
#include <unordered_map>
#include "global/struct.h"


class MapSerializer {
public:
    // 序列化主函数
    static QJsonObject serializeMapInfo(const MapInfo& mapInfo, const QString& filePath);
    static bool saveToFile(const MapInfo& mapInfo, const QString& filePath);

    // 反序列化主函数
    static MapInfo deserializeMapInfo(const QJsonObject& json, const QString& filePath);
    static MapInfo loadFromFile(const QString& filePath);

    // 检查文件是否存在且有效
    static bool isValidSaveFile(const QString& filePath);

private:
    // 基础类型序列化
    static QJsonObject serializeFloatRect(const FloatRect& rect);
    static FloatRect deserializeFloatRect(const QJsonObject& json);

    static QJsonObject serializeClimbOrigin(const ClimbOrigin& origin);
    static ClimbOrigin deserializeClimbOrigin(const QJsonObject& json);

    static QJsonObject serializeClimbRect(const ClimbRect& rect);
    static ClimbRect deserializeClimbRect(const QJsonObject& json);

    static QJsonObject serializeFootholdRect(const FootholdRect& rect);
    static FootholdRect deserializeFootholdRect(const QJsonObject& json);

    static QJsonObject serializeWall(const Wall& wall);
    static Wall deserializeWall(const QJsonObject& json);

    static QJsonObject serializeLineChild(const LineChild& line);
    static LineChild deserializeLineChild(const QJsonObject& json);

    static QJsonObject serializePlatform(const Platform& platform);
    static Platform deserializePlatform(const QJsonObject& json);

    static QJsonObject serializeMapBounding(const MapBounding& bounding);
    static MapBounding deserializeMapBounding(const QJsonObject& json);

    static QJsonObject serializeNodeVisit(const NodeVisit& visit);
    static NodeVisit deserializeNodeVisit(const QJsonObject& json);

    static QJsonObject serializeTargetNode(const TargetNode& target);
    static TargetNode deserializeTargetNode(const QJsonObject& json);

    static QJsonObject serializePathNode(const PathNode& node);
    static PathNode deserializePathNode(const QJsonObject& json);

    // OpenCV Mat 序列化
    static QJsonArray serializeMat(const cv::Mat& mat);
    static cv::Mat deserializeMat(const QJsonArray& json);

    // Point 序列化
    static QJsonObject serializePoint(const cv::Point& point);
    static cv::Point deserializePoint(const QJsonObject& json);
};

#endif // MAPSERIALIZER_H
