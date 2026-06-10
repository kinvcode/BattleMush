#include "realtracker.h"
#include <QDebug>
#include <numeric>
#include <ranges>

RealTargetDetector::RealTargetDetector() {
    reset();
}

void RealTargetDetector::reset() {
    real_target_id_ = -1;
    frame_count_ = 0;
    stability_counter_ = 0;
    target_lost_counter_ = 0;
    switch_counter_ = 0;
    last_switch_candidate_ = -1;
    last_valid_target_id_ = -1;
    background_angle_ = -1.0;
    last_angle_ = -1.0;
    last_speed_ = -1.0;
    lock_transferred_ = false;
    track_history_.clear();
    target_scores_.clear();
    position_history_.clear();
}

std::map<int, int> RealTargetDetector::countDirectionsByCategory(const std::vector<TrackInfo> &tracks)
{
    std::map<int, int> direction_counts;
    // 方向分类: 0=上, 1=右上, 2=右, 3=右下, 4=下, 5=左下, 6=左, 7=左上
    const int direction_bins = 8;

    std::vector<QString> dir_names = {"上", "右上", "右", "右下", "下", "左下", "左", "左上"};

    for (const auto& track : tracks) {
        auto [angle, speed, _] = calculateMovementMetrics2(track.track_id);
        // if (angle < 0) continue;

        // 将角度(0-360)映射到8个方向
        int direction = static_cast<int>((angle + 22.5) / 45.0) % 8;

        // 简化方向描述
        qDebug() << "Track" << track.track_id << "方向:" << dir_names[direction]
                 << QString("(%1°)").arg(angle, 0, 'f', 1);

        direction_counts[direction]++;
    }

    return direction_counts;
}

int RealTargetDetector::getBackgroundDirection(const std::map<int, int> &direction_counts)
{
    if (direction_counts.empty()) return -1;

    int max_direction = -1;
    int max_count = 0;

    for (const auto& [dir, count] : direction_counts) {
        if (count > max_count) {
            max_count = count;
            max_direction = dir;
        }
    }

    return max_direction;
}

bool RealTargetDetector::isDifferentFromBackground(int track_id, int background_direction, const std::vector<TrackInfo> &tracks)
{
    for (const auto& track : tracks) {
        if (track.track_id != track_id) continue;

        auto [angle, speed, _] = calculateMovementMetrics(track.track_id);
        if (angle < 0) return false;

        int track_direction = static_cast<int>((angle + 22.5) / 45.0) % 8;

        // 计算方向差异（考虑循环）
        int diff = std::abs(track_direction - background_direction);
        diff = std::min(diff, 8 - diff);

        // 方向差异大于2个象限才算不同（90度以上）
        return diff > 2;
    }
    return false;
}

void RealTargetDetector::setLockCoordinate(const cv::Point2i& coord) {
    lock_coord_ = coord;
    lock_transferred_ = false;
    qDebug() << "设置锁定坐标: (" << coord.x << ", " << coord.y << ")";
}

void RealTargetDetector::updateTracks(const std::vector<TrackInfo>& tracks) {
    std::map<int, cv::Point2f> current_positions;

    for (const auto& track : tracks) {
        current_positions[track.track_id] = track.center;

        track_history_[track.track_id].push_back(track.center);
        if (static_cast<int>(track_history_[track.track_id].size()) > MAX_TRAJECTORY_SIZE) {
            track_history_[track.track_id].pop_front();
        }

        position_history_[track.track_id].push_back(track.center);
        if (static_cast<int>(position_history_[track.track_id].size()) > MAX_TRAJECTORY_SIZE) {
            position_history_[track.track_id].pop_front();
        }
    }

    std::vector<int> inactive_ids;
    for (const auto& id : track_history_ | std::views::keys) {
        if (current_positions.find(id) == current_positions.end()) {
            inactive_ids.push_back(id);
        }
    }
    for (int id : inactive_ids) {
        track_history_.erase(id);
        position_history_.erase(id);
        target_scores_.erase(id);
    }
}

std::tuple<double, double, double> RealTargetDetector::calculateMovementMetrics(int trackId) {
    auto it = track_history_.find(trackId);
    if (it == track_history_.end() || it->second.size() < 5) {
        return { -1.0, -1.0, -1.0 };
    }

    const auto& positions = it->second;
    double angle = calculateAngle(positions.front(), positions.back());

    double total_distance = 0.0;
    std::vector<double> speeds;
    for (size_t i = 1; i < positions.size(); ++i) {
        double dx = positions[i].x - positions[i-1].x;
        double dy = positions[i].y - positions[i-1].y;
        double dist = std::sqrt(dx * dx + dy * dy);
        total_distance += dist;
        speeds.push_back(dist);
    }

    double avg_speed = total_distance / speeds.size();
    double speed_variance = 0.0;
    if (!speeds.empty()) {
        for (double s : speeds) {
            speed_variance += (s - avg_speed) * (s - avg_speed);
        }
        speed_variance /= speeds.size();
    }

    return { angle, avg_speed, speed_variance };
}

std::tuple<double, double, double> RealTargetDetector::calculateMovementMetrics2(int trackId)
{
    auto it = track_history_.find(trackId);
    if (it == track_history_.end() || it->second.size() < 2) {
        return { -1.0, -1.0, -1.0 };
    }

    const auto& positions = it->second;

    // 使用最近两帧计算运动方向
    const auto& current = positions.back();
    const auto& prev = positions[positions.size() - 2];

    double dx = current.x - prev.x;
    double dy = current.y - prev.y;

    double angle = -1.0;
    if (std::abs(dx) > 0.01 || std::abs(dy) > 0.01) {
        angle = std::atan2(dx, -dy) * 180.0 / CV_PI;
        if (angle < 0) angle += 360.0;
    }

    // 计算速度（像素/帧）
    double distance = std::sqrt(dx*dx + dy*dy);
    double avg_speed = distance;

    // 速度方差（需要至少3个点）
    double speed_variance = -1.0;
    if (positions.size() >= 3) {
        const auto& prev2 = positions[positions.size() - 3];
        double dx2 = prev.x - prev2.x;
        double dy2 = prev.y - prev2.y;
        double dist2 = std::sqrt(dx2*dx2 + dy2*dy2);
        double avg = (distance + dist2) / 2.0;
        speed_variance = ((distance - avg)*(distance - avg) + (dist2 - avg)*(dist2 - avg)) / 2.0;
    }

    return { angle, avg_speed, speed_variance };
}

double RealTargetDetector::calculateAngle(const cv::Point2f& from, const cv::Point2f& to, double aspectRatio) {
    double dx = (to.x - from.x) * aspectRatio;
    double dy = to.y - from.y;
    double angle_rad = std::atan2(dx, -dy);
    double angle_deg = angle_rad * 180.0 / CV_PI;
    if (angle_deg < 0) {
        angle_deg += 360.0;
    }
    return angle_deg;
}

double RealTargetDetector::calculateAngleDifference(double angle1, double angle2) {
    double diff = std::abs(angle1 - angle2);
    return std::min(diff, 360.0 - diff);
}

bool RealTargetDetector::checkMovementStability(double currentAngle, double currentSpeed) {
    if (last_angle_ < 0 || last_speed_ < 0) {
        last_angle_ = currentAngle;
        last_speed_ = currentSpeed;
        return true;
    }

    double angle_diff = calculateAngleDifference(currentAngle, last_angle_);
    double speed_diff = std::abs(currentSpeed - last_speed_);

    if (angle_diff > angle_change_threshold_ || speed_diff > speed_change_threshold_) {
        stability_counter_++;
    } else {
        stability_counter_ = std::max(0, stability_counter_ - 1);
    }

    last_angle_ = currentAngle;
    last_speed_ = currentSpeed;
    return stability_counter_ < stability_threshold_;
}

std::tuple<int, double> RealTargetDetector::findClosestTargetToLock(
    const std::map<int, cv::Point2f>& currentPositions) {

    if (lock_coord_.x < 0 || currentPositions.empty()) {
        return { -1, std::numeric_limits<double>::infinity() };
    }

    int closest_target = -1;
    double min_distance = std::numeric_limits<double>::infinity();

    for (const auto& pos : currentPositions) {
        double dx = pos.second.x - lock_coord_.x;
        double dy = pos.second.y - lock_coord_.y;
        double distance = std::sqrt(dx * dx + dy * dy);
        if (distance < min_distance) {
            min_distance = distance;
            closest_target = pos.first;
        }
    }

    return { closest_target, min_distance };
}

bool RealTargetDetector::maintainTargetContinuity(const std::map<int, cv::Point2f>& currentPositions) {
    if (last_valid_target_id_ >= 0 &&
        currentPositions.find(last_valid_target_id_) != currentPositions.end()) {
        if (real_target_id_ != last_valid_target_id_) {
            qDebug() << "恢复上次有效目标:" << last_valid_target_id_;
            real_target_id_ = last_valid_target_id_;
            target_lost_counter_ = 0;
            return true;
        }
    }
    return false;
}

std::tuple<bool, std::string> RealTargetDetector::isNearBoundary(
    const TrackInfo& target, const cv::Rect& region, double aspectRatio, int margin) {

    double x1 = target.bbox.x;
    double y1 = target.bbox.y;
    double x2 = target.bbox.x + target.bbox.width;
    double y2 = target.bbox.y + target.bbox.height;

    double w = target.bbox.width;
    double h = target.bbox.height;
    double ratio = std::max(w, h) / std::min(w, h);

    if (aspectRatio < 1) {
        if (x1 - region.x <= margin || region.x + region.width - x2 <= margin ||
            y1 - region.y <= margin || region.y + region.height - y2 <= margin) {
            return { true, "边界目标" };
        }
    }

    if (ratio <= 2.5 && w >= 80 && h >= 80) {
        return { false, "尺寸过滤" };
    }

    if (x1 - region.x <= margin) return { true, "左边界" };
    if (region.x + region.width - x2 <= margin) return { true, "右边界" };
    if (y1 - region.y <= margin) return { true, "上边界" };
    if (region.y + region.height - y2 <= margin) return { true, "下边界" };

    return { false, "" };
}

int RealTargetDetector::identifyRealTarget(
    const std::vector<TrackInfo>& tracks,
    const cv::Point2i& currentPos,
    double aspectRatio,
    const cv::Rect& detectionRegion) {

    if (tracks.empty()) {
        if (real_target_id_ >= 0) {
            target_lost_counter_++;
            if (target_lost_counter_ > max_target_lost_frames_) {
                qDebug() << "目标" << real_target_id_ << "丢失超过" << max_target_lost_frames_ << "帧，放弃保持";
                real_target_id_ = -1;
                switch_counter_ = 0;
            } else {
                qDebug() << "目标" << real_target_id_ << "暂时丢失，保持中... (第" << target_lost_counter_ << "帧)";
            }
        }
        return real_target_id_;
    }

    // 当前帧所有目标的位置
    std::map<int, cv::Point2f> current_positions;
    // 真实目标附近的目标
    std::map<int, TrackInfo> near_targets;
    for (const auto& track : tracks) {
        current_positions[track.track_id] = track.center;
        // 保存真实目标附近的所有目标
        if (real_target_id_ >= 0) {
             // 50范围内搜索ID
            double min_distance = 50.0;
            double dx = track.center.x - currentPos.x;
            double dy = track.center.y - currentPos.y;
            double distance = std::sqrt(dx*dx + dy*dy);
            if (distance <= min_distance){
                near_targets[track.track_id] = track;
                qDebug() << "附近目标:" << track.track_id << "距离" << distance;
            }
        }
    }

    updateTracks(tracks);

    frame_count_++;
    target_lost_counter_ = 0;

    if (!lock_transferred_) {
        auto [closest_target, min_distance] = findClosestTargetToLock(current_positions);
        const double DISTANCE_THRESHOLD = 80.0;

        if (closest_target >= 0 && min_distance < DISTANCE_THRESHOLD) {
            real_target_id_ = closest_target;
            last_valid_target_id_ = closest_target;
            lock_transferred_ = true;
            switch_counter_ = 0;
            qDebug() << "锁定坐标已移交到目标" << closest_target << ", 距离:" << min_distance << "像素";

            auto [angle, avg_speed, _] = calculateMovementMetrics2(closest_target);
            if (angle >= 0) {
                last_angle_ = angle;
                last_speed_ = avg_speed;
                stability_counter_ = 0;
            }
            return real_target_id_;
        } else {
            qDebug() << "未找到合适目标进行交接, 最近目标距离:" << min_distance << "像素";
        }
    }

    std::vector<QString> dir_names = {"上", "右上", "右", "右下", "下", "左下", "左", "左上"};

    // 统计当前帧所有目标的方向
    auto direction_counts = countDirectionsByCategory(tracks);
    // 获取当前背景的运动方向
    int background_direction = getBackgroundDirection(direction_counts);
    qDebug() << "背景运动方向：" << background_direction;

    // 寻找 真实ID与背景运动相同且附近存在与背景运动相反的目标
    std::vector<int> real_near_id_list;
    if (background_direction >= 0) {
        // 2. 检查真实ID的运动方向
        if (real_target_id_ >= 0) {
            auto real_it = std::find_if(tracks.begin(), tracks.end(),
                [this](const TrackInfo& t) { return t.track_id == real_target_id_; });
            if (real_it != tracks.end()) {
                auto [real_angle, _, __] = calculateMovementMetrics2(real_target_id_);
                int real_direction = static_cast<int>((real_angle + 22.5) / 45.0) % 8;
                bool similar_direction = real_direction == background_direction;  // 45度内算相似
                qDebug() << "真实ID" << real_target_id_
                         << "方向:" << dir_names[real_direction]
                         << "与背景" << (similar_direction ? "一致" : "不同");
            }
        }
    }

    std::map<int, TargetScore> candidate_scores;

    qDebug() << "-----------------------------------";
    qDebug() << "track_id | 方向 | 与背景方向相同 | 综合分";
    for (const auto& track : tracks) {
        auto [angle, avg_speed, speed_variance] = calculateMovementMetrics2(track.track_id);
        // if (angle < 0 && track.track_id != real_target_id_) continue;

        auto [near_boundary, boundary_type] = isNearBoundary(track, detectionRegion, aspectRatio);
        // if (near_boundary) continue;

        int direction = static_cast<int>((angle + 22.5) / 45.0) % 8;
        bool similar_direction = direction == background_direction;

        double angle_score = 0.0;
        if (background_angle_ >= 0) {
            double angle_diff = calculateAngleDifference(angle, background_angle_);
            angle_score = std::min(angle_diff / 180.0, 1.0);
        }

        double speed_score = std::min(avg_speed / 3.0, 1.0);
        double combined_score = angle_score;

        TargetScore ts;
        ts.angle = angle;
        ts.angle_score = angle_score;
        ts.speed = avg_speed;
        ts.speed_score = speed_score;
        ts.combined_score = combined_score;
        ts.position = track.center;

        candidate_scores[track.track_id] = ts;
        qDebug() << track.track_id << " | " << dir_names[direction] << " | " << (similar_direction ? "相同" : "不同") << " | " << combined_score;
    }
    qDebug() << "-----------------------------------";

    updateBackgroundAngle(candidate_scores);

    if (!candidate_scores.empty()) {

        // 筛选出最佳备选目标
        auto best_target = std::max_element(
            candidate_scores.begin(), candidate_scores.end(),
            [](const auto& a, const auto& b) {
                return a.second.combined_score < b.second.combined_score;
            });
        int best_id = best_target->first;
        qDebug() << "best_id" << best_id;
        const auto& best_scores = best_target->second;

        bool should_switch = false;
        bool distance_check_passed = true;

        // 计算备选目标与真实目标位置的距离
        cv::Point2f candidate_position = best_scores.position;
        double dx = candidate_position.x - currentPos.x;
        double dy = candidate_position.y - currentPos.y;
        double distance = std::sqrt(dx * dx + dy * dy);

        const double MAX_ALLOWED_DISTANCE = 100.0; //350
        distance_check_passed = distance <= MAX_ALLOWED_DISTANCE;

        if (!distance_check_passed) {
            qDebug() << "距离检查失败: 候选目标" << best_id << "距离当前目标" << real_target_id_ << "为" << distance << "像素 >" << MAX_ALLOWED_DISTANCE << "像素";
        }

        // 如果备选目标不是真实目标
        if (real_target_id_ != best_id) {
            if (maintainTargetContinuity(current_positions)) {
                qDebug() << "备选目标不是真实目标，上次有效目标存在本帧中。切换统计归零";
                should_switch = false;
                switch_counter_ = 0;
            } else {
                qDebug() << "备选目标不是真实目标，上次有效目标不在本帧中";
                if (last_switch_candidate_ == best_id) {
                    switch_counter_++;
                    qDebug() << "上次切换的目标与备选目标一致，切换统计增加";
                } else {
                    switch_counter_ = 1;
                    last_switch_candidate_ = best_id;
                    qDebug() << "上次切换的目标与备选目标不一致，切换统计设置1";
                }
                should_switch = (switch_counter_ >= switch_threshold_) && distance_check_passed;
            }
        } else {
            // 如果备选目标就是真实目标，检测真实目标运动是否稳定
            bool is_stable = checkMovementStability(best_scores.angle, best_scores.speed);
            if (!is_stable) {
                qDebug() << "真实目标运动不稳定，切换统计增加";
                should_switch = true;
                switch_counter_++;
            } else {
                qDebug() << "真实目标运动稳定，切换统计归零";
                switch_counter_ = 0;
            }
        }

        // 如果备选目标综合分大于0.25，并且应该切换目标，并且切换次数大于3次，并且符合距离。则将备选目标切换为真实目标
        if (best_scores.combined_score > 0.25 && should_switch &&
            switch_counter_ >= switch_threshold_ && distance_check_passed) {
            qDebug() << "备选目标" << best_id << "替换目标" << real_target_id_ << "为真实目标";
            int old_target = real_target_id_;
            real_target_id_ = best_id;
            last_valid_target_id_ = best_id;
            target_scores_[best_id] = best_scores;

            stability_counter_ = 0;
            switch_counter_ = 0;
            last_switch_candidate_ = -1;
        } else {
            // 如果当前帧存在真实目标，记录上次有效的目标
            if (real_target_id_ >= 0 &&
                current_positions.find(real_target_id_) != current_positions.end()) {
                qDebug() << "将真实目标设置为上次有效目标";
                last_valid_target_id_ = real_target_id_;
            }
        }
    } else {
        if (!maintainTargetContinuity(current_positions)) {
            switch_counter_ = 0;
            qDebug() << "没有合适目标且无法保持上次目标";
        }
    }

    qDebug() << "最终运算真实ID：" << real_target_id_;
    if (real_target_id_ >= 0 && current_positions.find(real_target_id_) == current_positions.end()) {
        qDebug() << "目标不存在2";
    }

    return real_target_id_;
}

void RealTargetDetector::updateBackgroundAngle(const std::map<int, TargetScore>& candidateScores) {
    if (candidateScores.size() < 2) return;

    std::vector<double> background_angles;
    for (const auto& score : candidateScores) {
        if (score.first != real_target_id_) {
            background_angles.push_back(score.second.angle);
        }
    }

    if (background_angles.empty()) return;

    double avg_sin = 0.0, avg_cos = 0.0;
    for (double angle : background_angles) {
        double rad = angle * CV_PI / 180.0;
        avg_sin += std::sin(rad);
        avg_cos += std::cos(rad);
    }
    avg_sin /= background_angles.size();
    avg_cos /= background_angles.size();

    double avg_angle_rad = std::atan2(avg_sin, avg_cos);
    background_angle_ = std::fmod(avg_angle_rad * 180.0 / CV_PI + 360.0, 360.0);
}
