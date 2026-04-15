#include "controller.h"
#include "global/globalconfig.h"
#include "global/globalplayer.h"
#include "global/gameoffset.h"
#include "driver/mem.h";

using namespace Mem;

AttackInfo Controller::m_attack_info = {1,{50, 25}};

cv::Point Controller::m_attack_range_general = {50,25};

cv::Point Controller::m_attack_range_skill1 = {150,40};

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
}
