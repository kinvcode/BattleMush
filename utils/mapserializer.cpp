#include "utils/mapserializer.h"
#include <QJsonValue>
#include <QByteArray>
#include "utils/helper.h"

// ==================== 主序列化函数 ====================

QJsonObject MapSerializer::serializeMapInfo(const MapInfo& mapInfo, const QString& filePath) {
    QJsonObject json;

    // 原子数据
    json["id"] = mapInfo.id;
    json["name"] = mapInfo.name;
    json["map_bounding"] = serializeMapBounding(mapInfo.map_bounding);

    // 序列化 climbs
    QJsonArray climbsArray;
    for (const auto& climb : mapInfo.climbs) {
        climbsArray.append(serializeClimbRect(climb));
    }
    json["climbs"] = climbsArray;

    // 序列化 footholds
    QJsonArray footholdsArray;
    for (const auto& foothold : mapInfo.footholds) {
        footholdsArray.append(serializeFootholdRect(foothold));
    }
    json["footholds"] = footholdsArray;

    // 序列化 wall_list
    QJsonArray wallsArray;
    for (const auto& wall : mapInfo.wall_list) {
        wallsArray.append(serializeWall(wall));
    }
    json["wall_list"] = wallsArray;

    // 序列化 platform_list
    QJsonArray platformsArray;
    for (const auto& platform : mapInfo.platform_list) {
        platformsArray.append(serializePlatform(platform));
    }
    json["platform_list"] = platformsArray;

    // 序列化 path_nodes (unordered_map)
    QJsonObject pathNodesObject;
    for (const auto& [id, node] : mapInfo.path_nodes) {
        pathNodesObject[QString::number(id)] = serializePathNode(node);
    }
    json["path_nodes"] = pathNodesObject;

    // 序列化地图图片
    if (!mapInfo.map_mat.empty()) {
        QString img_path = filePath + ".png";
        ImageTool::saveMatImage(mapInfo.map_mat, img_path);
        // json["map_mat"] = serializeMat(mapInfo.map_mat);
    }

    return json;
}

bool MapSerializer::saveToFile(const MapInfo& mapInfo, const QString& filePath) {
    QString json_file = filePath + ".json";
    QJsonObject json = serializeMapInfo(mapInfo, filePath);
    QJsonDocument doc(json);

    QFile file(json_file);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to open file for writing:" << json_file;
        return false;
    }

    QByteArray data = doc.toJson(QJsonDocument::Indented);
    qint64 bytesWritten = file.write(data);
    file.close();

    if (bytesWritten != data.size()) {
        qDebug() << "Failed to write all data to file";
        return false;
    }

    qDebug() << "Successfully saved map info to:" << filePath;
    return true;
}

// ==================== 主反序列化函数 ====================

MapInfo MapSerializer::deserializeMapInfo(const QJsonObject& json, const QString& filePath) {
    MapInfo mapInfo;

    // 原子数据
    mapInfo.id = json["id"].toString();
    mapInfo.name = json["name"].toString();
    mapInfo.map_bounding = deserializeMapBounding(json["map_bounding"].toObject());

    // 反序列化 climbs
    QJsonArray climbsArray = json["climbs"].toArray();
    for (const auto& value : climbsArray) {
        mapInfo.climbs.push_back(deserializeClimbRect(value.toObject()));
    }

    // 反序列化 footholds
    QJsonArray footholdsArray = json["footholds"].toArray();
    for (const auto& value : footholdsArray) {
        mapInfo.footholds.push_back(deserializeFootholdRect(value.toObject()));
    }

    // 反序列化 wall_list
    QJsonArray wallsArray = json["wall_list"].toArray();
    for (const auto& value : wallsArray) {
        mapInfo.wall_list.push_back(deserializeWall(value.toObject()));
    }

    // 反序列化 platform_list
    QJsonArray platformsArray = json["platform_list"].toArray();
    for (const auto& value : platformsArray) {
        mapInfo.platform_list.push_back(deserializePlatform(value.toObject()));
    }

    // 反序列化 path_nodes
    QJsonObject pathNodesObject = json["path_nodes"].toObject();
    for (auto it = pathNodesObject.begin(); it != pathNodesObject.end(); ++it) {
        int id = it.key().toInt();
        mapInfo.path_nodes[id] = deserializePathNode(it.value().toObject());
    }

    // 反序列化地图图片
    if (json.contains("map_mat")) {
        QString img_file = filePath + ".png";
        cv::Mat loaded_image = cv::imread(img_file.toStdString(), cv::IMREAD_COLOR);
        if (loaded_image.empty()) {
            mapInfo.map_mat = cv::Mat();
        }else{
            mapInfo.map_mat = loaded_image;
        }
    }

    return mapInfo;
}

MapInfo MapSerializer::loadFromFile(const QString& filePath) {
    QString json_file = filePath + ".json";
    QFile file(json_file);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open file for reading:" << json_file;
        return MapInfo();
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qDebug() << "Failed to parse JSON from file:" << json_file;
        return MapInfo();
    }

    return deserializeMapInfo(doc.object(), filePath);
}

bool MapSerializer::isValidSaveFile(const QString& filePath) {
    QString json_file = filePath + ".json";
    QFile file(json_file);
    if (!file.exists()) return false;
    if (!file.open(QIODevice::ReadOnly)) return false;

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    return !doc.isNull() && doc.isObject();
}

// ==================== 各结构体序列化实现 ====================

QJsonObject MapSerializer::serializeFloatRect(const FloatRect& rect) {
    QJsonObject json;
    json["x_left"] = rect.x_left;
    json["y_top"] = rect.y_top;
    json["x_right"] = rect.x_right;
    json["y_bottom"] = rect.y_bottom;
    return json;
}

FloatRect MapSerializer::deserializeFloatRect(const QJsonObject& json) {
    FloatRect rect;
    rect.x_left = json["x_left"].toDouble();
    rect.y_top = json["y_top"].toDouble();
    rect.x_right = json["x_right"].toDouble();
    rect.y_bottom = json["y_bottom"].toDouble();
    return rect;
}

QJsonObject MapSerializer::serializeClimbOrigin(const ClimbOrigin& origin) {
    QJsonObject json;
    json["left"] = origin.left;
    json["top"] = origin.top;
    json["factor1"] = origin.factor1;
    json["factor2"] = origin.factor2;
    return json;
}

ClimbOrigin MapSerializer::deserializeClimbOrigin(const QJsonObject& json) {
    ClimbOrigin origin;
    origin.left = json["left"].toDouble();
    origin.top = json["top"].toDouble();
    origin.factor1 = json["factor1"].toDouble();
    origin.factor2 = json["factor2"].toDouble();
    return origin;
}

QJsonObject MapSerializer::serializeClimbRect(const ClimbRect& rect) {
    QJsonObject json;
    json["x_left"] = rect.x_left;
    json["y_top"] = rect.y_top;
    json["x_right"] = rect.x_right;
    json["y_bottom"] = rect.y_bottom;
    return json;
}

ClimbRect MapSerializer::deserializeClimbRect(const QJsonObject& json) {
    ClimbRect rect;
    rect.x_left = json["x_left"].toInt();
    rect.y_top = json["y_top"].toInt();
    rect.x_right = json["x_right"].toInt();
    rect.y_bottom = json["y_bottom"].toInt();
    return rect;
}

QJsonObject MapSerializer::serializeFootholdRect(const FootholdRect& rect) {
    QJsonObject json;
    json["id"] = rect.id;
    json["x_left"] = rect.x_left;
    json["y_left"] = rect.y_left;
    json["x_right"] = rect.x_right;
    json["y_right"] = rect.y_right;
    return json;
}

FootholdRect MapSerializer::deserializeFootholdRect(const QJsonObject& json) {
    FootholdRect rect;
    rect.id = json["id"].toInt();
    rect.x_left = json["x_left"].toInt();
    rect.y_left = json["y_left"].toInt();
    rect.x_right = json["x_right"].toInt();
    rect.y_right = json["y_right"].toInt();
    return rect;
}

QJsonObject MapSerializer::serializeWall(const Wall& wall) {
    QJsonObject json;
    json["foot_id"] = wall.foot_id;
    json["x"] = wall.x;
    json["bottom"] = wall.bottom;
    json["top"] = wall.top;
    return json;
}

Wall MapSerializer::deserializeWall(const QJsonObject& json) {
    Wall wall;
    wall.foot_id = json["foot_id"].toInt();
    wall.x = json["x"].toInt();
    wall.bottom = json["bottom"].toInt();
    wall.top = json["top"].toInt();
    return wall;
}

QJsonObject MapSerializer::serializePoint(const cv::Point& point) {
    QJsonObject json;
    json["x"] = point.x;
    json["y"] = point.y;
    return json;
}

cv::Point MapSerializer::deserializePoint(const QJsonObject& json) {
    return cv::Point(json["x"].toInt(), json["y"].toInt());
}

QJsonObject MapSerializer::serializeLineChild(const LineChild& line) {
    QJsonObject json;
    json["foot_id"] = line.foot_id;
    json["start"] = serializePoint(line.start);
    json["end"] = serializePoint(line.end);
    json["length"] = line.length;
    return json;
}

LineChild MapSerializer::deserializeLineChild(const QJsonObject& json) {
    LineChild line;
    line.foot_id = json["foot_id"].toInt();
    line.start = deserializePoint(json["start"].toObject());
    line.end = deserializePoint(json["end"].toObject());
    line.length = json["length"].toInt();
    return line;
}

QJsonObject MapSerializer::serializePlatform(const Platform& platform) {
    QJsonObject json;
    json["id"] = platform.id;
    json["type"] = platform.type;
    json["start"] = serializePoint(platform.start);
    json["end"] = serializePoint(platform.end);
    json["hasLeftWall"] = platform.hasLeftWall;
    json["hasRightWall"] = platform.hasRightWall;

    QJsonArray lineChildren;
    for (const auto& child : platform.line_child) {
        lineChildren.append(serializeLineChild(child));
    }
    json["line_child"] = lineChildren;

    return json;
}

Platform MapSerializer::deserializePlatform(const QJsonObject& json) {
    Platform platform;
    platform.id = json["id"].toInt();
    platform.type = json["type"].toInt();
    platform.start = deserializePoint(json["start"].toObject());
    platform.end = deserializePoint(json["end"].toObject());
    platform.hasLeftWall = json["hasLeftWall"].toBool();
    platform.hasRightWall = json["hasRightWall"].toBool();

    QJsonArray lineChildren = json["line_child"].toArray();
    for (const auto& value : lineChildren) {
        platform.line_child.push_back(deserializeLineChild(value.toObject()));
    }

    return platform;
}

QJsonObject MapSerializer::serializeMapBounding(const MapBounding& bounding) {
    QJsonObject json;
    json["x_left"] = bounding.x_left;
    json["y_bottom"] = bounding.y_bottom;
    json["x_right"] = bounding.x_right;
    json["y_top"] = bounding.y_top;
    json["x_offset"] = bounding.x_offset;
    json["y_offset"] = bounding.y_offset;
    json["width"] = bounding.width;
    json["height"] = bounding.height;
    return json;
}

MapBounding MapSerializer::deserializeMapBounding(const QJsonObject& json) {
    MapBounding bounding;
    bounding.x_left = json["x_left"].toInt();
    bounding.y_bottom = json["y_bottom"].toInt();
    bounding.x_right = json["x_right"].toInt();
    bounding.y_top = json["y_top"].toInt();
    bounding.x_offset = json["x_offset"].toInt();
    bounding.y_offset = json["y_offset"].toInt();
    bounding.width = json["width"].toInt();
    bounding.height = json["height"].toInt();
    return bounding;
}

QJsonObject MapSerializer::serializeNodeVisit(const NodeVisit& visit) {
    QJsonObject json;
    json["visit_type"] = static_cast<int>(visit.visit_type);

    QJsonArray areaArray;
    for (const auto& segment : visit.area) {
        QJsonArray segmentArray;
        segmentArray.append(segment.first);
        segmentArray.append(segment.second);
        areaArray.append(segmentArray);
    }
    json["area"] = areaArray;

    return json;
}

NodeVisit MapSerializer::deserializeNodeVisit(const QJsonObject& json) {
    NodeVisit visit;
    visit.visit_type = static_cast<VISIT_TYPE>(json["visit_type"].toInt());

    QJsonArray areaArray = json["area"].toArray();
    for (const auto& value : areaArray) {
        QJsonArray segment = value.toArray();
        if (segment.size() == 2) {
            visit.area.push_back({segment[0].toInt(), segment[1].toInt()});
        }
    }

    return visit;
}

QJsonObject MapSerializer::serializeTargetNode(const TargetNode& target) {
    QJsonObject json;
    json["id"] = target.id;
    json["type"] = target.type;

    QJsonArray visitList;
    for (const auto& visit : target.visit_list) {
        visitList.append(serializeNodeVisit(visit));
    }
    json["visit_list"] = visitList;

    return json;
}

TargetNode MapSerializer::deserializeTargetNode(const QJsonObject& json) {
    TargetNode target;
    target.id = json["id"].toInt();
    target.type = json["type"].toInt();

    QJsonArray visitList = json["visit_list"].toArray();
    for (const auto& value : visitList) {
        target.visit_list.push_back(deserializeNodeVisit(value.toObject()));
    }

    return target;
}

QJsonObject MapSerializer::serializePathNode(const PathNode& node) {
    QJsonObject json;
    json["id"] = node.id;
    json["type"] = node.type;

    QJsonObject targetsObject;
    for (const auto& [targetId, target] : node.targets) {
        targetsObject[QString::number(targetId)] = serializeTargetNode(target);
    }
    json["targets"] = targetsObject;

    return json;
}

PathNode MapSerializer::deserializePathNode(const QJsonObject& json) {
    PathNode node;
    node.id = json["id"].toInt();
    node.type = json["type"].toInt();

    QJsonObject targetsObject = json["targets"].toObject();
    for (auto it = targetsObject.begin(); it != targetsObject.end(); ++it) {
        int targetId = it.key().toInt();
        node.targets[targetId] = deserializeTargetNode(it.value().toObject());
    }

    return node;
}

// OpenCV Mat 序列化（将图像数据转换为Base64）
QJsonArray MapSerializer::serializeMat(const cv::Mat& mat) {
    QJsonArray array;

    if (mat.empty()) {
        return array;
    }

    // 保存图像参数
    array.append(mat.rows);
    array.append(mat.cols);
    array.append(mat.type());

    // 将图像数据转换为Base64字符串
    size_t totalBytes = mat.total() * mat.elemSize();
    std::vector<uchar> buffer(mat.data, mat.data + totalBytes);
    QString base64Data = QByteArray(reinterpret_cast<const char*>(buffer.data()),
                                    static_cast<int>(buffer.size())).toBase64();
    array.append(base64Data);

    return array;
}

cv::Mat MapSerializer::deserializeMat(const QJsonArray& json) {
    if (json.size() < 4) {
        return cv::Mat();
    }

    int rows = json[0].toInt();
    int cols = json[1].toInt();
    int type = json[2].toInt();
    QString base64Data = json[3].toString();

    // 从Base64解码
    QByteArray decodedData = QByteArray::fromBase64(base64Data.toUtf8());

    // 创建Mat
    cv::Mat mat(rows, cols, type);
    if (decodedData.size() == mat.total() * mat.elemSize()) {
        memcpy(mat.data, decodedData.data(), decodedData.size());
    } else {
        qDebug() << "Warning: Decoded data size mismatch!";
    }

    return mat;
}
