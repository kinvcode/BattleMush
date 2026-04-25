#include "controller.h"
#include "global/globalconfig.h"
#include "global/globalplayer.h"
#include "global/gameoffset.h"
#include "driver/mem.h";
#include "utils/helper.h"

using namespace Mem;

AttackInfo Controller::m_attack_info = {1,{50, 25}};

cv::Point Controller::m_attack_range_general = {50,25};

cv::Point Controller::m_attack_range_skill1 = {150,40};

std::vector<MapRoomPath> Controller::m_clean_room_list = {};

std::vector<MapRoomPath> Controller::m_rest_point_list = {};

std::vector<NPCQuest> Controller::m_quest_list = {};

std::vector<Task> Controller::m_tasks = {};

bool Controller::m_disable_attack = false;

void Controller::updateAttackInfo()
{
    cv::Point atk_range;

    int attack_type = 1; // 0普通攻击 1技能攻击1
    int cur_mp_rate = Gplayer.MP * 100 / Gplayer.MAX_MP;
    if(cur_mp_rate < 10){
        attack_type = 0;
        atk_range = m_attack_range_general;
    }else{
        atk_range = m_attack_range_skill1;
    }

    m_attack_info.type = attack_type;
    m_attack_info.range = atk_range;
    // qDebug() << "攻击类型:" << m_attack_info.type << " 范围:" << m_attack_info.range.x << "," <<m_attack_info.range.y;
}

void Controller::initTaskList()
{
    /* ============【 ✅️維多利亞港外圍 】============= */
    /* ============【 ❌️空屋 】============= */
    /* ============【 ✅️菇菇山丘 】===================== */
    /* ============【 ❌️地铁一号线1区 】============= */
    /* ============【 ❌️北方工地顶端 】============= */
    /* ============【 ❌️蓝菇菇树林 】============= */
    /* ============【 ✅️遗迹发掘地1 】============= */
    /* ============【 ❌️黑肥肥领土 】============= */
    /* ============【 ❌️黑肥肥领土2 】============= */
    /* ============【 ❌️遗迹发掘地3 】============= */
    /* ============【 ❌️蚂蚁洞穴2 】============= */
    /* ============【 ❌️蚂蚁洞穴3 】============= */
    /* ============【 ❌️火焰之地 】============= */
    /* ============【 ❌️仙人掌沙漠2 】============= */
    /* ============【 ❌️东方岩石山6 】============= */
    /* ============【 ❌️云彩露台1 】============= */
    /* ============【 ❌️云彩露台3 】============= */
    /* ============【 ❌️红螃蟹海滩2 】============= */
    /* ============【 ❌️猴子深林1 】============= */
    /* ============【 ❌️遗迹之墓1 】============= */
    /* ============【 ❌️青螃蟹海滩2 】============= */
    /* ============【 ❌️危险的峡谷 】============= */
    /* ============【 ❌️龙族打猎场 】============= */
    /* ============【 ❌️巫婆深林2 】============= */
    /* ============【 ❌️石人寺院2 】============= */
    /* ============【 ❌️巨人之森 】============= */
    /* ============【 ❌️第一军营 】============= */
    /* ============【 ❌️雪域王冠之地 】============= */
    /* ============【 ❌️魔龙领土 】============= */
    /* ============【 ❌️死亡之林2 】============= */
    /* ============【 ❌️时尚大道西部区域 】============= */
    /* ============【 ❌️都會廣場東部區域 】============= */
    /* ============【 ❌️東邊森林 】============= */
    /* ============【 ❌️龍族之穴 】============= */
    /* ============【 ❌️名人大道南部區域 】============= */
    /* ============【 ❌️都會廣場南部區域 】============= */
    /* ============【 ❌️被遺忘的時間之路 2 】============= */
    /* ============【 ❌️彎曲的時間 】============= */
    /* ============【 ❌️森林岔道 】============= */
    /* ============【 ❌️暗黑半人馬領土 】============= */
    /* ============【 ❌️神木村 - 火 】============= */
    /* ============【 ❌️焰半人馬領土 】============= */
    /* ============【 ❌️寒冰半人馬領土 】============= */
    /* ============【 ❌️山羊峽谷 】============= */
    /* ============【 ❌️彎曲地獄路 】============= */
    /* ============【 ❌️深海峽谷 2 】============= */
    /* ============【 ❌️扭曲的時間之路 4 】============= */
    /* ============【 ❌️受難船的墓地 】============= */
    /* ============【 ❌️與大佛的邂逅 】============= */
    /* ============【 ❌️死龍巢穴 】============= */
    /* ============【 ❌️遭破壞的龍之巢穴 】============= */
    /* ============【 ❌️空屋 】============= */
    /* ============【 ❌️空屋 】============= */
    /* ============【 ❌️空屋 】============= */
    /* ============【 ❌️空屋 】============= */
    /* ============【 ❌️空屋 】============= */
    /* ============【 ❌️空屋 】============= */
    /* ============【 ❌️空屋 】============= */
    /* ============【 ❌️空屋 】============= */
    /* ============【 ❌️空屋 】============= */

    // 初始化刷图列表
    /**
     * 維多利亞港外圍 8ded358a-8cd6-4dde-8b93-018b5650b5c0
     * 弓箭手村訓練場1 a73c7664-2f12-4caf-be19-94f8b65175f2
     * 弓箭手村西部小山 60d5d28d-7379-4af0-afc0-e1716d9ef474
     * 海岸草叢2 cf13d314-1437-49f4-83e8-739c3ee5465d
     * 海岸草叢3 641b82aa-da1e-46b3-8b67-0aeda141383e
     * 菇菇山丘 5624c829-11e1-4544-91b1-a9220c654f5a
     * 三岔路 52579f62-8ba2-4c56-b0c8-f0069b119494
     *
    **/

    /* ============【 菇 菇 山 丘 】===================== */
    MapRoomPath map0000;
    map0000.id = 0;
    map0000.name = "菇菇山丘";
    map0000.uuid = "5624c829-11e1-4544-91b1-a9220c654f5a";

    RoomRest map0000_rest00;
    map0000_rest00.id = 0;
    map0000_rest00.name = "【休息点A】左边第一平台";
    map0000_rest00.platform = 1;

    RoomPath map0000_path00;
    map0000_path00.id = 0;
    map0000_path00.name = "自动规划路线";

    RoomPath map0000_path01;
    map0000_path01.id = 1;
    map0000_path01.name = "【路线A】只刷下面一层";
    map0000_path01.platforms = {0};

    RoomPath map0000_path02;
    map0000_path02.id = 2;
    map0000_path02.name = "【路线B】只刷上面一层";
    map0000_path02.platforms = {5};

    RoomPath map0000_path03;
    map0000_path03.id = 3;
    map0000_path03.name = "【路线C】上下两层循环刷";
    map0000_path03.platforms = {0,7,5,10};

    map0000.paths.push_back(map0000_path00);
    map0000.paths.push_back(map0000_path01);
    map0000.paths.push_back(map0000_path02);
    map0000.paths.push_back(map0000_path03);

    map0000.rests.push_back(map0000_rest00);

    /* ============【 維 多 利 亞 港 外 圍 】============= */
    MapRoomPath map0001;
    map0001.id = 1;
    map0001.name = "維多利亞港外圍";
    map0001.uuid = "8ded358a-8cd6-4dde-8b93-018b5650b5c0";

    RoomRest map0001_rest00;
    map0001_rest00.id = 0;
    map0001_rest00.name = "【休息点A】地图左上角";
    map0001_rest00.platform = 0;

    RoomPath map0001_path00;
    map0001_path00.id = 0;
    map0001_path00.name = "自动规划路线";

    RoomPath map0001_path01;
    map0001_path01.id = 1;
    map0001_path01.name = "【路线A】刷全图";
    map0001_path01.platforms = {0,2,3,4,5,6,7,8};

    map0001.paths.push_back(map0001_path00);
    map0001.paths.push_back(map0001_path01);

    map0001.rests.push_back(map0001_rest00);

    /* ============【 遺 跡 發 掘 地 1 】============= */
    MapRoomPath map0002;
    map0002.id = 2;
    map0002.name = "遺跡發掘地1";
    map0002.uuid = "d26fc64e-2ff3-4f66-978e-f606f286ddf9";

    RoomRest map0002_rest00;
    map0002_rest00.id = 0;
    map0002_rest00.name = "【休息点A】地图顶端";
    map0002_rest00.platform = 15;

    RoomPath map0002_path00;
    map0002_path00.id = 0;
    map0002_path00.name = "自动规划路线";

    RoomPath map0002_path01;
    map0002_path01.id = 1;
    map0002_path01.name = "【路线A】刷全图";
    map0002_path01.platforms = {0,24,10,22,11,12,7,20,13,27};

    map0002.paths.push_back(map0002_path00);
    map0002.paths.push_back(map0002_path01);

    map0002.rests.push_back(map0002_rest00);

    /* ============【 加 入 刷 图 列 表】============= */
    m_clean_room_list.push_back(map0000);
    m_clean_room_list.push_back(map0001);
    m_clean_room_list.push_back(map0002);


    /* ============【 加 入 休 息 列 表】============= */
    m_rest_point_list.push_back(map0000);
    m_rest_point_list.push_back(map0001);
    m_rest_point_list.push_back(map0002);


    // 初始化NPC任务列表
    /**
     * 任务UUID 8ded358a-8cd6-4dde-8b93-018b5650b5c0
     *
    **/
    NPCQuest quest0000;
    quest0000.id = 0;
    quest0000.name = "收集100万个猪头";

    NPCQuest quest0001;
    quest0001.id = 1;
    quest0001.name = "击杀10亿只蜗牛";

    m_quest_list.push_back(quest0000);
    m_quest_list.push_back(quest0001);
}

void Controller::initRoomNodes()
{
    std::unordered_map<QString, RoomNode> roomMap;

    // 读取地图文件
    QString json_file = ":/assets/maps/room_node.json";
    QFile file(json_file);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open file for reading:" << json_file;
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qDebug() << "Failed to parse JSON from file:" << json_file;
        return;
    }

    // QJsonObject json = doc.object();

    QJsonArray rootArray = doc.array();

    // 遍历每个房间节点

    for (const QJsonValue& roomValue : rootArray) {
        if (!roomValue.isObject()) continue;

        QJsonObject roomObj = roomValue.toObject();
        RoomNode room;

        room.uuid = roomObj["uuid"].toString();
        room.name = roomObj["name"].toString();
        room.type = roomObj["type"].toInt();

        // 解析 targets
        if (roomObj.contains("targets") && roomObj["targets"].isArray()) {
            QJsonArray targetsArray = roomObj["targets"].toArray();

            for (const QJsonValue& targetValue : targetsArray) {
                if (!targetValue.isObject()) continue;

                QJsonObject targetObj = targetValue.toObject();
                TargetRoom target;

                target.uuid = targetObj["uuid"].toString();
                target.name = targetObj["name"].toString();
                target.type = targetObj["type"].toInt();
                target.platform = targetObj["platform"].toInt();

                if (targetObj.contains("area") && targetObj["area"].isArray()) {
                    target.area = IntRect::fromJsonArray(targetObj["area"].toArray());
                }

                room.targets.push_back(target);
            }
        }

        // 插入 map，如果 UUID 重复会覆盖（实际应该不会重复）
        roomMap[room.uuid] = room;
    }

    Gplayer.room_nodes = roomMap;
}

void Controller::saveTaskConfig()
{
    // 数据保存逻辑，保存为json文件
    QJsonArray task_list;
    for(auto const &task : Gcontroller.m_tasks){
        QJsonObject task_data;
        task_data["id"] = task.id;
        task_data["type"] = task.type;
        task_data["option1_key"] = task.option1_key;
        task_data["option2_key"] = task.option2_key;
        task_data["runTime"] = task.runTime;
        task_list.append(task_data);
    }

    QString currentDir = QCoreApplication::applicationDirPath();
    QString config_path = currentDir + "/task.json";
    Config* config = Config::instance();
    config->setFilePath(config_path);
    config->m_jsonArray = task_list;
    config->saveArray();
    DebugOutput::out("更新计划任务配置文件");
}

void Controller::loadTaskConfig()
{
    // 读取本地配置
    // Gcontroller.m_tasks.push_back(t);

    QString currentDir = QCoreApplication::applicationDirPath();
    QString task_path = currentDir + "/task.json";

    QFile task_file(task_path);
    if (!task_file.exists()) {
        qDebug() << "计划任务文件不存在！";
        return;
    }

    QFile file(task_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Couldn't open file:" << task_path;
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);

    if (error.error != QJsonParseError::NoError) {
        qDebug() << "JSON 解析错误:" << error.errorString();
        return;
    }

    if (!doc.isArray()) {
        qDebug() << "文件不是 JSON 数组格式";
        return;
    }

    QJsonArray taskArray = doc.array();

    Gcontroller.m_tasks.clear();

    for (const QJsonValue &value : taskArray) {
        QJsonObject taskObj = value.toObject();

        Task task;
        task.id = taskObj["id"].toInt();
        task.type = static_cast<TaskType>(taskObj["type"].toInt());
        task.option1_key = taskObj["option1_key"].toInt();
        task.option2_key = taskObj["option2_key"].toInt();
        task.runTime = taskObj["runTime"].toInt();

        if(task.type == 0){
            task.option1_value = Gcontroller.m_clean_room_list.at(task.option1_key).name;
            task.option2_value = Gcontroller.m_clean_room_list.at(task.option1_key).paths.at(task.option2_key).name;
        }else if(task.type == 1){
            task.option1_value = Gcontroller.m_rest_point_list.at(task.option1_key).name;
            task.option2_value = Gcontroller.m_rest_point_list.at(task.option1_key).paths.at(task.option2_key).name;
        }else if(task.type == 2){
            task.option1_value = Gcontroller.m_quest_list.at(task.option1_key).name;
        }

        Gcontroller.m_tasks.push_back(task);

        qDebug() << "=================================";
        qDebug() << task.id;
        qDebug() << task.type;
        qDebug() << task.option1_key;
        qDebug() << task.option1_value;
        qDebug() << task.option2_key;
        qDebug() << task.option2_value;
        qDebug() << task.runTime;
    }
}

void Controller::reSortTask()
{
    for(int i = 0; i < m_tasks.size(); i++){
        m_tasks[i].id = i;
    }
}
