#include "realtracker.h"
#include <QDebug>
#include <QString>
#include <limits>
#include <set>

namespace {

bool isValidTrackSnapshot(const TrackInfo& track) {
    return track.score >= 0.0;
}

TrackInfo makeMissingTrackSnapshot(int trackId) {
    TrackInfo info;
    info.track_id = trackId;
    info.center = cv::Point2f(-1.f, -1.f);
    info.bbox = cv::Rect2d(-1, -1, -1, -1);
    info.score = -1.0;
    return info;
}

bool findLastTwoValidCenters(const std::deque<TrackPositionRecord>& history,
                             cv::Point2f& current,
                             cv::Point2f& prev) {
    int validCount = 0;
    for (auto it = history.rbegin(); it != history.rend(); ++it) {
        if (!isValidTrackSnapshot(it->track)) {
            continue;
        }
        if (validCount == 0) {
            current = it->track.center;
            validCount = 1;
        } else {
            prev = it->track.center;
            return true;
        }
    }
    return false;
}

bool findLastValidCenter(const std::deque<TrackPositionRecord>& history, cv::Point2f& center) {
    for (auto it = history.rbegin(); it != history.rend(); ++it) {
        if (!isValidTrackSnapshot(it->track)) {
            continue;
        }
        center = it->track.center;
        return true;
    }
    return false;
}

bool isPointInRect(const cv::Point2f& point, const cv::Rect& rect) {
    return point.x >= rect.x
        && point.x < rect.x + rect.width
        && point.y >= rect.y
        && point.y < rect.y + rect.height;
}

} // namespace

RealTargetDetector::RealTargetDetector() {
    reset();
}

void RealTargetDetector::reset() {
    real_target_id_ = -1;
    frame_count_ = 0;
    target_lost_counter_ = 0;
    switch_counter_ = 0;
    last_switch_candidate_ = -1;
    last_valid_target_id_ = -1;
    background_angle_ = -1.0;
    lock_transferred_ = false;
    track_history_.clear();
    target_scores_.clear();
    predict_target_rect_valid_ = false;
    real_target_direction_ = -1;
    real_target_sameback_count_ = 0;
    last_log_sameback_count_ = 0;
}

std::map<int, int> RealTargetDetector::countDirectionsByCategory(const std::vector<TrackInfo>& tracks)
{
    std::map<int, int> direction_counts;

    for (const auto& track : tracks) {
        auto [angle, speed] = calculateMovementMetrics(track.track_id);
        Q_UNUSED(speed);

        int direction = static_cast<int>((angle + 22.5) / 45.0) % 8;

        if (track.track_id == real_target_id_) {
            real_target_direction_ = direction;
        }

        direction_counts[direction]++;
    }

    return direction_counts;
}

int RealTargetDetector::getBackgroundDirection(const std::map<int, int>& direction_counts)
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

void RealTargetDetector::setLockCoordinate(const cv::Point2i& coord) {
    lock_coord_ = coord;
    lock_transferred_ = false;
    qDebug() << "设置锁定坐标: (" << coord.x << ", " << coord.y << ")";
}

void RealTargetDetector::updateTracks(const std::vector<TrackInfo>& tracks) {
    frame_count_++;
    const int current_frame = frame_count_;

    std::set<int> current_ids;
    for (const auto& track : tracks) {
        current_ids.insert(track.track_id);
        TrackPositionRecord record;
        record.frame = current_frame;
        record.track = track;
        track_history_[track.track_id].push_back(record);
    }

    for (auto& [track_id, history] : track_history_) {
        if (current_ids.count(track_id) > 0) {
            continue;
        }
        TrackPositionRecord record;
        record.frame = current_frame;
        record.track = makeMissingTrackSnapshot(track_id);
        history.push_back(record);
    }
}

std::tuple<double, double> RealTargetDetector::calculateMovementMetrics(int trackId)
{
    auto it = track_history_.find(trackId);
    if (it == track_history_.end()) {
        return { -1.0, -1.0 };
    }

    cv::Point2f current;
    cv::Point2f prev;
    if (!findLastTwoValidCenters(it->second, current, prev)) {
        return { -1.0, -1.0 };
    }

    double dx = current.x - prev.x;
    double dy = current.y - prev.y;

    double angle = -1.0;
    if (std::abs(dx) > 0.01 || std::abs(dy) > 0.01) {
        angle = std::atan2(dx, -dy) * 180.0 / CV_PI;
        if (angle < 0) angle += 360.0;
    }

    const double avg_speed = std::sqrt(dx * dx + dy * dy);

    return { angle, avg_speed };
}

void RealTargetDetector::updatePredictTargetRect(const std::map<int, cv::Point2f>& currentPositions) {
    predict_target_rect_valid_ = false;
    if (real_target_id_ < 0) {
        return;
    }

    cv::Point2f position;
    const auto pos_it = currentPositions.find(real_target_id_);
    if (pos_it != currentPositions.end()) {
        position = pos_it->second;
    } else {
        const auto hist_it = track_history_.find(real_target_id_);
        if (hist_it == track_history_.end() || !findLastValidCenter(hist_it->second, position)) {
            return;
        }
    }

    const auto [angle, speed] = calculateMovementMetrics(real_target_id_);
    if (angle < 0 || speed < 0) {
        return;
    }

    const double angle_rad = angle * CV_PI / 180.0;
    const double dx = speed * std::sin(angle_rad);
    const double dy = -speed * std::cos(angle_rad);
    const cv::Point2f predicted(position.x + static_cast<float>(dx),
                                position.y + static_cast<float>(dy));

    constexpr int kPredictRectSize = 50;
    constexpr int kHalf = kPredictRectSize / 2;
    predict_target_rect_ = cv::Rect(
        static_cast<int>(std::round(predicted.x)) - kHalf,
        static_cast<int>(std::round(predicted.y)) - kHalf,
        kPredictRectSize,
        kPredictRectSize);
    predict_target_rect_valid_ = true;
}

double RealTargetDetector::calculateAngleDifference(double angle1, double angle2) const {
    double diff = std::abs(angle1 - angle2);
    return std::min(diff, 360.0 - diff);
}

bool RealTargetDetector::checkMovementStability(double currentAngle, double currentSpeed)
{
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

    cv::Point2d center = {x1 + w / 2, y1 + h /2};

    if (aspectRatio < 1) {
        if (x1 - region.x <= margin || region.x + region.width - x2 <= margin ||
            y1 - region.y <= margin || region.y + region.height - y2 <= margin) {
            return { true, "边界目标" };
        }
    }

    if (ratio <= 2.5 && w >= 80 && h >= 80) {
        return { false, "尺寸过滤" };
    }

    if(center.x - region.x <= margin) return { true, "左边界" };
    if(region.x + region.width - center.x <= margin) return { true, "右边界" };
    if (center.y - region.y <= margin) return { true, "上边界" };
    if (region.y + region.height - center.y <= margin) return { true, "下边界" };

    // if (x1 - region.x <= margin) return { true, "左边界" };
    // if (region.x + region.width - x2 <= margin) return { true, "右边界" };
    // if (y1 - region.y <= margin) return { true, "上边界" };
    // if (region.y + region.height - y2 <= margin) return { true, "下边界" };

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
        updatePredictTargetRect({});
        return real_target_id_;
    }

    std::map<int, cv::Point2f> current_positions;
    std::map<int, TrackInfo> near_targets;
    bool real_track_inframe = false;

    for (const auto& track : tracks) {
        current_positions[track.track_id] = track.center;
        if (real_target_id_ >= 0) {
            if (real_target_id_ == track.track_id) {
                real_track_inframe = true;
            }

            double min_distance = 50.0;
            double dx = track.center.x - currentPos.x;
            double dy = track.center.y - currentPos.y;
            double distance = std::sqrt(dx * dx + dy * dy);
            if (distance <= min_distance) {
                near_targets[track.track_id] = track;
                qDebug() << "附近目标:" << track.track_id << "距离" << distance;
            }
        }
    }

    updateTracks(tracks);
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
        } else {
            qDebug() << "未找到合适目标进行交接, 最近目标距离:" << min_distance << "像素";
        }
    }

    std::vector<QString> dir_names = {"上", "右上", "右", "右下", "下", "左下", "左", "左上"};
    auto direction_counts = countDirectionsByCategory(tracks);
    int background_direction = getBackgroundDirection(direction_counts);

    double background_speed = -1.0;
    {
        double speed_sum = 0.0;
        int speed_count = 0;
        for (const auto& track : tracks) {
            if (real_target_id_ >= 0 && track.track_id == real_target_id_) {
                continue;
            }
            auto [angle, avg_speed] = calculateMovementMetrics(track.track_id);
            Q_UNUSED(angle);
            if (avg_speed < 0 || avg_speed > 50.0) {
                continue;
            }
            speed_sum += avg_speed;
            speed_count++;
        }
        if (speed_count > 0) {
            background_speed = speed_sum / speed_count;
        }
    }

    double real_target_speed = -1.0;
    if (real_target_id_ >= 0) {
        auto [real_angle, real_speed] = calculateMovementMetrics(real_target_id_);
        Q_UNUSED(real_angle);
        if (real_speed >= 0) {
            real_target_speed = real_speed;
        }
    }

    bool same_background_direction = real_target_direction_ == background_direction;
    qDebug() << "背景运动方向：" << dir_names[background_direction]
             << "背景速度：" << background_speed;
    if (real_target_direction_ > -1 && real_target_direction_ < 8) {
        qDebug() << "真实ID" << real_target_id_
                 << "方向:" << dir_names[real_target_direction_]
                 << "速度:" << real_target_speed
                 << "与背景" << (same_background_direction ? "一致" : "不同");
    }

    struct NearTargetRank {
        int track_id = -1;
        TrackInfo track;
        double distance = 0.0;
    };

    std::vector<NearTargetRank> sorted_near_targets;
    sorted_near_targets.reserve(near_targets.size());

    const cv::Point2f real_pos(static_cast<float>(currentPos.x), static_cast<float>(currentPos.y));

    for (const auto& [track_id, track] : near_targets) {
        const double dx = track.center.x - real_pos.x;
        const double dy = track.center.y - real_pos.y;
        const double distance = std::sqrt(dx * dx + dy * dy);
        sorted_near_targets.push_back({track_id, track, distance});
    }

    std::sort(sorted_near_targets.begin(), sorted_near_targets.end(),
              [](const NearTargetRank& a, const NearTargetRank& b) {
                  return a.distance < b.distance;
              });

    if (!sorted_near_targets.empty()) {
        qDebug() << "附近目标排序(共" << sorted_near_targets.size() << "个):";
        for (size_t i = 0; i < sorted_near_targets.size(); ++i) {
            const auto& item = sorted_near_targets[i];
            qDebug() << "#" << (i + 1)
                     << "ID:" << item.track_id
                     << "距离:" << item.distance;
        }
    }

    if (!real_track_inframe) {
        qDebug() << "！！！真实目标不在本帧中！！！";
    } else {
        qDebug() << "真实目标存在本帧中";
    }

    std::map<int, TargetScore> candidate_scores;

    qDebug() << "-----------------------------------";
    qDebug() << "track_id | 方向 | 角度 | 速度 | 与背景方向相同 | 角度分 | 速度分 | 综合分";
    for (const auto& track : tracks) {
        auto [angle, avg_speed] = calculateMovementMetrics(track.track_id);

        auto [near_boundary, boundary_type] = isNearBoundary(track, detectionRegion, aspectRatio);
        Q_UNUSED(near_boundary);
        Q_UNUSED(boundary_type);
        if(near_boundary){
            qDebug() << track.track_id << "被过滤";
        }

        int direction = static_cast<int>((angle + 22.5) / 45.0) % 8;
        bool similar_direction = direction == background_direction;
        Q_UNUSED(similar_direction);

        double angle_score = 0.0;
        if (background_angle_ >= 0) {
            double angle_diff = calculateAngleDifference(angle, background_angle_);
            angle_score = std::min(angle_diff / 180.0, 1.0);
        }

        double speed_score = 0.0;
        if (background_speed >= 0 && avg_speed >= 0) {
            double speed_diff = std::abs(avg_speed - background_speed);
            if (background_speed > 0) {
                speed_score = std::min(speed_diff / background_speed, 1.0);
            } else {
                speed_score = speed_diff > 0 ? 1.0 : 0.0;
            }
        }
        double combined_score = angle_score;

        TargetScore ts;
        ts.near_boundary = near_boundary;
        ts.angle = angle;
        ts.angle_score = angle_score;
        ts.speed = avg_speed;
        ts.speed_score = speed_score;
        ts.combined_score = angle_score + speed_score;
        ts.position = track.center;
        ts.direction = direction;

        candidate_scores[track.track_id] = ts;
        qDebug() << track.track_id
                 << " | " << dir_names[direction]
                 << " | " << angle
                 << " | " << avg_speed
                 << " | " << (similar_direction ? "相同" : "不同")
                 << " | " << angle_score
                 << " | " << speed_score
                 << " | " << combined_score;
    }
    qDebug() << "-----------------------------------";

    updateBackgroundAngle(candidate_scores);

    int tmp_track_id = -1;

    // 真实ID是否
    // real_target_id_ = -1;

    if (!candidate_scores.empty()) {
        const double MAX_ALLOWED_DISTANCE = 100.0;
        std::map<int, TargetScore> eligible_scores;
        for (const auto& [id, score] : candidate_scores) {
            const double dx = score.position.x - currentPos.x;
            const double dy = score.position.y - currentPos.y;
            const double distance = std::sqrt(dx * dx + dy * dy);
            if(score.near_boundary){
                continue;
            }
            if(score.direction == background_direction){
                continue;
            }
            if (distance <= MAX_ALLOWED_DISTANCE) {
                eligible_scores[id] = score;
                qDebug() << "候选目标：" << id << "距离当前位置" << distance << "像素";
            }
        }

        if (eligible_scores.empty()) {
            qDebug() << "无符合距离要求的候选目标";
            if (!maintainTargetContinuity(current_positions)) {
                switch_counter_ = 0;
                qDebug() << "没有合适目标且无法保持上次目标";
            }
        } else {
            auto best_target = std::max_element(
                eligible_scores.begin(), eligible_scores.end(),
                [](const auto& a, const auto& b) {
                    return a.second.combined_score < b.second.combined_score;
                }
            );

            int best_id = best_target->first;
            qDebug() << "best_id" << best_id;
            const auto& best_scores = best_target->second;

            bool should_switch = false;

            if (real_track_inframe) {
                qDebug() << "真实目标在本帧中，计算";
                if (real_target_speed >= 0 && background_speed >= 0
                    && abs(real_target_speed - background_speed) < 10
                    && same_background_direction)
                {
                    if (last_log_sameback_count_ + 1 == frame_count_) {
                        real_target_sameback_count_++;
                        qDebug() << "真实目标与背景同向且速度不超过背景, 计数累计:" << real_target_sameback_count_;
                    } else {
                        real_target_sameback_count_ = 1;
                    }
                    last_log_sameback_count_ = frame_count_;
                } else {
                    real_target_sameback_count_ = 0;
                }

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
                            qDebug() << "上次切换的目标" << last_switch_candidate_ << "与备选目标不一致，切换统计设置1";
                        }
                        should_switch = switch_counter_ >= switch_threshold_;
                        if (should_switch) {
                            qDebug() << "已达到切换条件";
                        }
                    }
                } else {
                    qDebug() << "备选目标是真实目标，切换统计归零";
                    switch_counter_ = 0;

                    // 判断真实目标运动是否稳定
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
            } else {
                qDebug() << "真实目标不在本帧中,计算预测位置"; // 应该在预测框中找
                real_target_sameback_count_ = 0;

                // if (!sorted_near_targets.empty()) {
                    // qDebug() << "";
                    // tmp_track_id = sorted_near_targets[0].track_id;
                    // tmp_track_id = best_id;

                    if (last_switch_candidate_ == best_id) {
                        switch_counter_++;
                        qDebug() << "上次切换的目标与备选目标一致，切换统计增加";
                    } else {
                        switch_counter_ = 1;
                        last_switch_candidate_ = best_id;
                        qDebug() << "上次切换的目标与备选目标不一致，切换统计设置1";
                    }

                    // should_switch = switch_counter_ >= switch_threshold_;
                    // if (should_switch) {
                    //     qDebug() << "已达到切换临时目标条件";

                    //     real_target_id_ = best_id;
                    //     last_valid_target_id_ = best_id;
                    //     switch_counter_ = 0;
                    //     last_switch_candidate_ = -1;
                    //     should_switch = false;
                    // }
                // } else {
                    // qDebug() << "真实目标不存在，附近没有可用目标";
                // }
                    should_switch = switch_counter_ >= switch_threshold_;
            }

            if (real_target_sameback_count_ >= 3)
            {
                qDebug() << "真实目标与背景运动连续3帧相同";
                if (!sorted_near_targets.empty()) {
                    qDebug() << "选择就近目标";
                    const int near_id = sorted_near_targets[0].track_id;
                    qDebug() << "备选目标" << near_id << "替换目标" << real_target_id_ << "为真实目标";
                    real_target_id_ = near_id;
                    last_valid_target_id_ = near_id;
                    switch_counter_ = 0;
                    last_switch_candidate_ = -1;
                } else {
                    qDebug() << "就近目标不存在，选择备选ID";
                    switch_counter_ = real_target_sameback_count_;
                    should_switch = switch_counter_ >= switch_threshold_;
                }
                real_target_sameback_count_ = 0;
            }

            // if (real_track_inframe
            //     && hasPredictTargetRect()
            //     && same_background_direction
            //     && best_id != real_target_id_) {
            //     const cv::Point2f real_center = current_positions.at(real_target_id_);
            //     if (!isPointInRect(real_center, predict_target_rect_)) {
            //         qDebug() << "真实目标不在预测框内且与背景同向，切换为备选目标" << best_id;
            //         real_target_id_ = best_id;
            //         last_valid_target_id_ = best_id;
            //         target_scores_[best_id] = best_scores;
            //         switch_counter_ = 0;
            //         last_switch_candidate_ = -1;
            //     }
            // }
            qDebug() << "当前切换数值：" << should_switch << best_scores.combined_score << switch_counter_;

            if (best_scores.combined_score > 0.25
                && should_switch
                && switch_counter_ >= switch_threshold_
                && best_id != real_target_id_)
            {
                qDebug() << "备选目标" << best_id << "替换目标" << real_target_id_ << "为真实目标";
                real_target_id_ = best_id;
                last_valid_target_id_ = best_id;
                target_scores_[best_id] = best_scores;
                switch_counter_ = 0;
                last_switch_candidate_ = -1;
            } else {
                if (real_target_id_ >= 0 &&
                    current_positions.find(real_target_id_) != current_positions.end()) {
                    qDebug() << "将真实目标设置为上次有效目标";
                    last_valid_target_id_ = real_target_id_;
                }
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

    updatePredictTargetRect(current_positions);
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
