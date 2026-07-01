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

std::vector<QString> Controller::m_black_monster = {};

std::vector<Task> Controller::m_tasks = {};

bool Controller::m_disable_attack = false;

bool Controller::m_disable_recive = false;

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
    map0000.name = "(Lv8~15)菇菇山丘";
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
    map0001.name = "(Lv1~5)維多利亞港外圍";
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
    map0002.name = "(Lv19~23)遺跡發掘地1";
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

    /* ============【 弓 箭 手 村 訓 練 場 2 】============= */
    MapRoomPath map0003;
    map0003.id = 3;
    map0003.name = "(Lv8~15)弓箭手村訓練場2";
    map0003.uuid = "1f234900-61d7-45d0-8ad0-871a119b183f";

    RoomRest map0003_rest00;
    map0003_rest00.id = 0;
    map0003_rest00.name = "【休息点A】地图左下角平台";
    map0003_rest00.platform = 1;

    RoomPath map0003_path00;
    map0003_path00.id = 0;
    map0003_path00.name = "自动规划路线";

    RoomPath map0003_path01;
    map0003_path01.id = 1;
    map0003_path01.name = "【路线A】刷全图";
    map0003_path01.platforms = {0,3,4,11,12};

    map0003.paths.push_back(map0003_path00);
    map0003.paths.push_back(map0003_path01);

    map0003.rests.push_back(map0003_rest00);

    /* ============【 黑 肥 肥 領 土 2 】============= */
    MapRoomPath map0004;
    map0004.id = 4;
    map0004.name = "(Lv25~27)黑肥肥領土2";
    map0004.uuid = "dc87557c-2165-4962-bcbb-370e9a1f38a8";

    RoomRest map0004_rest00;
    map0004_rest00.id = 0;
    map0004_rest00.name = "【休息点A】地图中间平台";
    map0004_rest00.platform = 1;

    RoomPath map0004_path00;
    map0004_path00.id = 0;
    map0004_path00.name = "自动规划路线";

    RoomPath map0004_path01;
    map0004_path01.id = 1;
    map0004_path01.name = "【路线A】刷全图";
    map0004_path01.platforms = {0,1,2,3};

    map0004.paths.push_back(map0004_path00);
    map0004.paths.push_back(map0004_path01);

    map0004.rests.push_back(map0004_rest00);

    /* ============【 遺 跡 發 掘 地 2 】============= */
    MapRoomPath map0005;
    map0005.id = 5;
    map0005.name = "(Lv23~25)遺跡發掘地2";
    map0005.uuid = "ccbb98d2-61f6-4b4a-8e88-67cfa921fc32";

    RoomRest map0005_rest00;
    map0005_rest00.id = 0;
    map0005_rest00.name = "【休息点A】地图左上角平台";
    map0005_rest00.platform = 4;

    RoomPath map0005_path00;
    map0005_path00.id = 0;
    map0005_path00.name = "自动规划路线";

    RoomPath map0005_path01;
    map0005_path01.id = 1;
    map0005_path01.name = "【路线A】刷全图";
    map0005_path01.platforms = {0,21,14,20,22,3,12};

    map0005.paths.push_back(map0005_path00);
    map0005.paths.push_back(map0005_path01);

    map0005.rests.push_back(map0005_rest00);

    /* ============【 火 焰 之 地 1 】============= */
    MapRoomPath map0006;
    map0006.id = 6;
    map0006.name = "(Lv32~35)火焰之地1";
    map0006.uuid = "c9349a80-5a2d-401d-8cdb-8e5ff414de71";

    RoomRest map0006_rest00;
    map0006_rest00.id = 0;
    map0006_rest00.name = "【休息点A】地图右边平台";
    map0006_rest00.platform = 18;

    RoomPath map0006_path00;
    map0006_path00.id = 0;
    map0006_path00.name = "自动规划路线";

    RoomPath map0006_path01;
    map0006_path01.id = 1;
    map0006_path01.name = "【路线A】刷下面";
    map0006_path01.platforms = {13,0,12};

    map0006.paths.push_back(map0006_path00);
    map0006.paths.push_back(map0006_path01);

    map0006.rests.push_back(map0006_rest00);

    /* ============【 2號線<1區> 】============= */
    MapRoomPath map0007;
    map0007.id = 7;
    map0007.name = "(Lv35~37)2號線<1區>";
    map0007.uuid = "e1b1a71f-0a41-4657-9579-7f90807c00bc";

    RoomRest map0007_rest00;
    map0007_rest00.id = 0;
    map0007_rest00.name = "【休息点A】地图左边梯子";
    map0007_rest00.platform = 35;

    RoomPath map0007_path00;
    map0007_path00.id = 0;
    map0007_path00.name = "自动规划路线";

    RoomPath map0007_path01;
    map0007_path01.id = 1;
    map0007_path01.name = "【路线A】刷全图";
    map0007_path01.platforms = {13,26,0,16,19,21,22,15,1};

    map0007.paths.push_back(map0007_path00);
    map0007.paths.push_back(map0007_path01);

    map0007.rests.push_back(map0007_rest00);

    /* ============【 遺 跡 發 掘 地 3 】============= */
    MapRoomPath map0008;
    map0008.id = 8;
    map0008.name = "(Lv23~25)遺跡發掘地3";
    map0008.uuid = "258b9b4d-dd3f-4147-8aae-b31272c57e91";

    RoomRest map0008_rest00;
    map0008_rest00.id = 0;
    map0008_rest00.name = "【休息点A】地图顶部平台";
    map0008_rest00.platform = 19;

    RoomPath map0008_path00;
    map0008_path00.id = 0;
    map0008_path00.name = "自动规划路线";

    RoomPath map0008_path01;
    map0008_path01.id = 1;
    map0008_path01.name = "【路线A】刷全图";
    map0008_path01.platforms = {0,4,8,11,13,6,5};

    map0008.paths.push_back(map0008_path00);
    map0008.paths.push_back(map0008_path01);

    map0008.rests.push_back(map0008_rest00);

    /* ============【 玩 具 工 廠 < 主 工 程 A > 】============= */
    MapRoomPath map0009;
    map0009.id = 9;
    map0009.name = "(Lv36~38)玩具工廠<主工程A>";
    map0009.uuid = "2c865a87-555b-4302-a328-f8d7b07ecc23";

    RoomRest map0009_rest00;
    map0009_rest00.id = 0;
    map0009_rest00.name = "【休息点A】地图入口右侧平台";
    map0009_rest00.platform = 8;

    RoomPath map0009_path00;
    map0009_path00.id = 0;
    map0009_path00.name = "自动规划路线";

    RoomPath map0009_path01;
    map0009_path01.id = 1;
    map0009_path01.name = "【路线A】只刷下面一层";
    map0009_path01.platforms = {22};

    map0009.paths.push_back(map0009_path00);
    map0009.paths.push_back(map0009_path01);

    map0009.rests.push_back(map0009_rest00);

    /* ============【 時 間 分 岔 道 】============= */
    MapRoomPath map0010;
    map0010.id = 10;
    map0010.name = "(Lv40~42)時間分岔道";
    map0010.uuid = "3150a2a0-128c-42c6-bbc9-d2305546fd18";

    RoomRest map0010_rest00;
    map0010_rest00.id = 0;
    map0010_rest00.name = "【休息点A】地图顶部平台";
    map0010_rest00.platform = 15;

    RoomPath map0010_path00;
    map0010_path00.id = 0;
    map0010_path00.name = "自动规划路线";

    RoomPath map0010_path01;
    map0010_path01.id = 1;
    map0010_path01.name = "【路线A】刷全图";
    map0010_path01.platforms = {11,2,1,9,8,0,4,5,6};

    map0010.paths.push_back(map0010_path00);
    map0010.paths.push_back(map0010_path01);

    map0010.rests.push_back(map0010_rest00);

    /* ============【 雲 彩 公 園 3 】============= */
    MapRoomPath map0011;
    map0011.id = 11;
    map0011.name = "(Lv45~47)雲彩公園3";
    map0011.uuid = "3f0fd016-a8b8-4d85-a06b-cfe30eebc336";

    RoomRest map0011_rest00;
    map0011_rest00.id = 0;
    map0011_rest00.name = "【休息点A】地图左下角平台";
    map0011_rest00.platform = 13;

    RoomPath map0011_path00;
    map0011_path00.id = 0;
    map0011_path00.name = "自动规划路线";

    RoomPath map0011_path01;
    map0011_path01.id = 1;
    map0011_path01.name = "【路线A】刷全图";
    map0011_path01.platforms = {22,24,19,20,21};

    map0011.paths.push_back(map0011_path00);
    map0011.paths.push_back(map0011_path01);

    map0011.rests.push_back(map0011_rest00);

    /* ============【 名 人 大 道 南 部 區 域 】============= */
    MapRoomPath map0012;
    map0012.id = 12;
    map0012.name = "(Lv76~78)名人大道南部區域";
    map0012.uuid = "b4cf7e17-50e3-44db-affa-df917364c7de";

    RoomRest map0012_rest00;
    map0012_rest00.id = 0;
    map0012_rest00.name = "【休息点A】地图上方平台";
    map0012_rest00.platform = 8;

    RoomPath map0012_path00;
    map0012_path00.id = 0;
    map0012_path00.name = "自动规划路线";

    RoomPath map0012_path01;
    map0012_path01.id = 1;
    map0012_path01.name = "【路线A】刷全图";
    map0012_path01.platforms = {6,5,4};

    map0012.paths.push_back(map0012_path00);
    map0012.paths.push_back(map0012_path01);

    map0012.rests.push_back(map0012_rest00);

    /* ============【 石 巨 人 寺 院 入 口 】============= */
    MapRoomPath map0013;
    map0013.id = 13;
    map0013.name = "(Lv55~57)石巨人寺院入口";
    map0013.uuid = "ff51af0c-3daf-4b60-8158-be6d16d30374";

    // RoomRest map0013_rest00;
    // map0013_rest00.id = 0;
    // map0013_rest00.name = "【休息点A】地图上方平台";
    // map0013_rest00.platform = 0;

    RoomPath map0013_path00;
    map0013_path00.id = 0;
    map0013_path00.name = "自动规划路线";

    RoomPath map0013_path01;
    map0013_path01.id = 1;
    map0013_path01.name = "【路线A】刷全图";
    map0013_path01.platforms = {3,4,1,0,2};

    map0013.paths.push_back(map0013_path00);
    map0013.paths.push_back(map0013_path01);

    // map0013.rests.push_back(map0013_rest00);

    /* ============【 鋼 之 黑 肥 肥 之 地 】============= */
    MapRoomPath map0014;
    map0014.id = 14;
    map0014.name = "(Lv45~47)鋼之黑肥肥之地";
    map0014.uuid = "294ed65c-b4e2-46ae-b868-4f052a903006";

    RoomRest map0014_rest00;
    map0014_rest00.id = 0;
    map0014_rest00.name = "【休息点A】地图左上方平台";
    map0014_rest00.platform = 10;

    RoomPath map0014_path00;
    map0014_path00.id = 0;
    map0014_path00.name = "自动规划路线";

    RoomPath map0014_path01;
    map0014_path01.id = 1;
    map0014_path01.name = "【路线A】刷全图";
    map0014_path01.platforms = {29,6,0,5};

    map0014.paths.push_back(map0014_path00);
    map0014.paths.push_back(map0014_path01);

    map0014.rests.push_back(map0014_rest00);

    /* ============【 巨 人 之 森 】============= */
    MapRoomPath map0015;
    map0015.id = 15;
    map0015.name = "(Lv58~62)巨人之森";
    map0015.uuid = "9521b27c-d545-4ba8-bf22-fd1d6246d2f8";

    RoomRest map0015_rest00;
    map0015_rest00.id = 0;
    map0015_rest00.name = "【休息点A】地图上方平台";
    map0015_rest00.platform = 0;

    RoomPath map0015_path00;
    map0015_path00.id = 0;
    map0015_path00.name = "自动规划路线";

    RoomPath map0015_path01;
    map0015_path01.id = 1;
    map0015_path01.name = "【路线A】刷全图";
    map0015_path01.platforms = {2,3,4};

    map0015.paths.push_back(map0015_path00);
    map0015.paths.push_back(map0015_path01);

    map0015.rests.push_back(map0015_rest00);

    /* ============【 青 螃 蟹 海 灘 Ⅱ 】============= */
    MapRoomPath map0016;
    map0016.id = 16;
    map0016.name = "(Lv48~50)青螃蟹海灘Ⅱ";
    map0016.uuid = "9cef013d-5021-4314-92a7-f535bb27f3fd";

    RoomRest map0016_rest00;
    map0016_rest00.id = 0;
    map0016_rest00.name = "【休息点A】地图上方平台";
    map0016_rest00.platform = 14;

    RoomPath map0016_path00;
    map0016_path00.id = 0;
    map0016_path00.name = "自动规划路线";

    RoomPath map0016_path01;
    map0016_path01.id = 1;
    map0016_path01.name = "【路线A】刷全图";
    map0016_path01.platforms = {2,10,5,13,3,11,4,12};

    map0016.paths.push_back(map0016_path00);
    map0016.paths.push_back(map0016_path01);

    map0016.rests.push_back(map0016_rest00);

    /* ============【 龍 族 狩 獵 場 】============= */
    MapRoomPath map0017;
    map0017.id = 17;
    map0017.name = "(Lv50~52)龍族狩獵場";
    map0017.uuid = "27d886ad-d3d7-416e-b246-e49d3738e3dd";

    RoomRest map0017_rest00;
    map0017_rest00.id = 0;
    map0017_rest00.name = "【休息点A】地图左方平台";
    map0017_rest00.platform = 0;

    RoomPath map0017_path00;
    map0017_path00.id = 0;
    map0017_path00.name = "自动规划路线";

    RoomPath map0017_path01;
    map0017_path01.id = 1;
    map0017_path01.name = "【路线A】刷全图";
    map0017_path01.platforms = {9,10,11,1};

    map0017.paths.push_back(map0017_path00);
    map0017.paths.push_back(map0017_path01);

    map0017.rests.push_back(map0017_rest00);

    /* ============【 魔 龍 領 土 】============= */
    MapRoomPath map0018;
    map0018.id = 18;
    map0018.name = "(Lv62~64)魔龍領土";
    map0018.uuid = "8b2384ca-5ada-4464-b482-887e1fc99205";

    RoomRest map0018_rest00;
    map0018_rest00.id = 0;
    map0018_rest00.name = "【休息点A】地图下方平台";
    map0018_rest00.platform = 1;

    RoomPath map0018_path00;
    map0018_path00.id = 0;
    map0018_path00.name = "自动规划路线";

    RoomPath map0018_path01;
    map0018_path01.id = 1;
    map0018_path01.name = "【路线A】刷全图";
    map0018_path01.platforms = {2,1};

    map0018.paths.push_back(map0018_path00);
    map0018.paths.push_back(map0018_path01);

    map0018.rests.push_back(map0018_rest00);

    /* ============【 時 間 之 路 <2> 】============= */
    MapRoomPath map0019;
    map0019.id = 19;
    map0019.name = "(Lv37~37)時間之路<2>";
    map0019.uuid = "96c299d1-c1bd-490a-86da-06f2ab231986";

    RoomRest map0019_rest00;
    map0019_rest00.id = 0;
    map0019_rest00.name = "【休息点A】地图右侧传送阵下方平台";
    map0019_rest00.platform = 26;

    RoomPath map0019_path00;
    map0019_path00.id = 0;
    map0019_path00.name = "自动规划路线";

    RoomPath map0019_path01;
    map0019_path01.id = 1;
    map0019_path01.name = "【路线A】刷全图";
    map0019_path01.platforms = {28,65,66,36};

    map0019.paths.push_back(map0019_path00);
    map0019.paths.push_back(map0019_path01);

    map0019.rests.push_back(map0019_rest00);

    /* ============【 時 間 之 路 <1> 】============= */
    MapRoomPath map0020;
    map0020.id = 20;
    map0020.name = "(Lv41~41)時間之路<1>";
    map0020.uuid = "63ff8f01-0b70-49e5-affd-cd008406075a";

    RoomRest map0020_rest00;
    map0020_rest00.id = 0;
    map0020_rest00.name = "【休息点A】地图右侧传送阵左方小平台";
    map0020_rest00.platform = 20;

    RoomPath map0020_path00;
    map0020_path00.id = 0;
    map0020_path00.name = "自动规划路线";

    RoomPath map0020_path01;
    map0020_path01.id = 1;
    map0020_path01.name = "【路线A】刷全图";
    map0020_path01.platforms = {46,47,48};

    map0020.paths.push_back(map0020_path00);
    map0020.paths.push_back(map0020_path01);

    map0020.rests.push_back(map0020_rest00);

    /* ============【 2號線<2區> 】============= */
    MapRoomPath map0021;
    map0021.id = 21;
    map0021.name = "(Lv48~48)2號線<2區>";
    map0021.uuid = "63ff8f01-0b70-49e5-affd-cd008406075a";

    RoomRest map0021_rest00;
    map0021_rest00.id = 0;
    map0021_rest00.name = "【休息点A】地图左侧上方平台";
    map0021_rest00.platform = 14;

    RoomPath map0021_path00;
    map0021_path00.id = 0;
    map0021_path00.name = "自动规划路线";

    RoomPath map0021_path01;
    map0021_path01.id = 1;
    map0021_path01.name = "【路线A】刷全图";
    map0021_path01.platforms = {0,23,22,12,20,19,17,15};

    map0021.paths.push_back(map0021_path00);
    map0021.paths.push_back(map0021_path01);

    map0021.rests.push_back(map0021_rest00);

    /* ============【 石 巨 人 寺 院 2 】============= */
    MapRoomPath map0022;
    map0022.id = 22;
    map0022.name = "(Lv55~55)石巨人寺院2";
    map0022.uuid = "9b433712-50e3-4a6a-96d3-44be696c16be";

    RoomRest map0022_rest00;
    map0022_rest00.id = 0;
    map0022_rest00.name = "【休息点A】地图上方平台";
    map0022_rest00.platform = 3;

    RoomPath map0022_path00;
    map0022_path00.id = 0;
    map0022_path00.name = "自动规划路线";

    RoomPath map0022_path01;
    map0022_path01.id = 1;
    map0022_path01.name = "【路线A】刷全图";
    map0022_path01.platforms = {0,1,2,4};

    map0022.paths.push_back(map0022_path00);
    map0022.paths.push_back(map0022_path01);

    map0022.rests.push_back(map0022_rest00);

    /* ============【 黑 肥 肥 領 土 】============= */
    MapRoomPath map0023;
    map0023.id = 23;
    map0023.name = "(Lv25~27)黑肥肥領土";
    map0023.uuid = "f6078a0d-d85d-404c-add5-7b728fd5b88e";

    RoomRest map0023_rest00;
    map0023_rest00.id = 0;
    map0023_rest00.name = "【休息点A】地图右侧平台";
    map0023_rest00.platform = 23;

    RoomPath map0023_path00;
    map0023_path00.id = 0;
    map0023_path00.name = "自动规划路线";

    RoomPath map0023_path01;
    map0023_path01.id = 1;
    map0023_path01.name = "【路线A】刷全图";
    map0023_path01.platforms = {12,30,0};

    map0023.paths.push_back(map0023_path00);
    map0023.paths.push_back(map0023_path01);

    map0023.rests.push_back(map0023_rest00);

    /* ============【 加 入 刷 图 列 表】============= */
    m_clean_room_list.push_back(map0000); // 菇菇山丘
    m_clean_room_list.push_back(map0001); // 維多利亞港外圍
    m_clean_room_list.push_back(map0002); // 遺跡發掘地1
    m_clean_room_list.push_back(map0003); // 弓箭手村訓練場2
    m_clean_room_list.push_back(map0004); // 黑肥肥領土2
    m_clean_room_list.push_back(map0005); // 遺跡發掘地2
    m_clean_room_list.push_back(map0006); // 火焰之地1
    m_clean_room_list.push_back(map0007); // 2號線<1區>
    m_clean_room_list.push_back(map0008); // 遺跡發掘地3
    m_clean_room_list.push_back(map0009); // 玩具工廠<主工程A>
    m_clean_room_list.push_back(map0010); // 時間分岔道
    m_clean_room_list.push_back(map0011); // 雲彩公園3
    m_clean_room_list.push_back(map0012); // 名人大道南部區域
    m_clean_room_list.push_back(map0013); // 石巨人寺院入口
    m_clean_room_list.push_back(map0014); // 鋼之黑肥肥之地
    m_clean_room_list.push_back(map0015); // 巨人之森
    m_clean_room_list.push_back(map0016); // 青螃蟹海灘Ⅱ
    m_clean_room_list.push_back(map0017); // 龍族狩獵場
    m_clean_room_list.push_back(map0018); // 魔龍領土
    m_clean_room_list.push_back(map0019); // 時間之路<2>
    m_clean_room_list.push_back(map0020); // 時間之路<1>
    m_clean_room_list.push_back(map0021); // 2號線<2區>
    m_clean_room_list.push_back(map0022); // 石巨人寺院2
    m_clean_room_list.push_back(map0023); // 黑肥肥領土



    /* ============【 加 入 休 息 列 表】============= */
    m_rest_point_list.push_back(map0000);
    m_rest_point_list.push_back(map0001);
    m_rest_point_list.push_back(map0002);
    m_rest_point_list.push_back(map0003);
    m_rest_point_list.push_back(map0004);
    m_rest_point_list.push_back(map0005);
    m_rest_point_list.push_back(map0006);
    m_rest_point_list.push_back(map0007);
    m_rest_point_list.push_back(map0008);
    m_rest_point_list.push_back(map0009);
    m_rest_point_list.push_back(map0010);
    m_rest_point_list.push_back(map0011);
    m_rest_point_list.push_back(map0012);
    // m_rest_point_list.push_back(map0013); // 没有休息点
    m_rest_point_list.push_back(map0014);
    m_rest_point_list.push_back(map0015);
    m_rest_point_list.push_back(map0016);
    m_rest_point_list.push_back(map0017);
    m_rest_point_list.push_back(map0018);
    m_rest_point_list.push_back(map0019);
    m_rest_point_list.push_back(map0020);
    m_rest_point_list.push_back(map0021);
    m_rest_point_list.push_back(map0022);
    m_rest_point_list.push_back(map0023);


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
    QString json_file = ":/assets/room_node.json";
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
            task.option2_value = Gcontroller.m_rest_point_list.at(task.option1_key).rests.at(task.option2_key).name;
        }else if(task.type == 2){
            task.option1_value = Gcontroller.m_quest_list.at(task.option1_key).name;
        }

        Gcontroller.m_tasks.push_back(task);

        qDebug() << "=================================";
        qDebug() << "task_id" << task.id;
        qDebug() << "task_type" << task.type;
        qDebug() << "task_option1_key" << task.option1_key;
        qDebug() << "task_option1_value" << task.option1_value;
        qDebug() << "task_option2_key" << task.option2_key;
        qDebug() << "task_option2_value" << task.option2_value;
        qDebug() << "task_runTime" << task.runTime;
    }
}

void Controller::reSortTask()
{
    for(int i = 0; i < m_tasks.size(); i++){
        m_tasks[i].id = i;
    }
}
