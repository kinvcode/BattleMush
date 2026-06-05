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

    std::map<int, cv::Point2f> current_positions;
    for (const auto& track : tracks) {
        current_positions[track.track_id] = track.center;
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

            auto [angle, avg_speed, _] = calculateMovementMetrics(closest_target);
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

    std::map<int, TargetScore> candidate_scores;

    for (const auto& track : tracks) {
        auto [angle, avg_speed, speed_variance] = calculateMovementMetrics(track.track_id);
        if (angle < 0) continue;

        auto [near_boundary, boundary_type] = isNearBoundary(track, detectionRegion, aspectRatio);
        if (near_boundary) continue;

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
    }

    updateBackgroundAngle(candidate_scores);

    if (!candidate_scores.empty()) {
        auto best_target = std::max_element(
            candidate_scores.begin(), candidate_scores.end(),
            [](const auto& a, const auto& b) {
                return a.second.combined_score < b.second.combined_score;
            });
        int best_id = best_target->first;
        const auto& best_scores = best_target->second;

        bool should_switch = false;
        bool distance_check_passed = true;

        cv::Point2f candidate_position = best_scores.position;
        double dx = candidate_position.x - currentPos.x;
        double dy = candidate_position.y - currentPos.y;
        double distance = std::sqrt(dx * dx + dy * dy);

        const double MAX_ALLOWED_DISTANCE = 350.0;
        distance_check_passed = distance <= MAX_ALLOWED_DISTANCE;

        if (!distance_check_passed) {
            qDebug() << "距离检查失败: 候选目标" << best_id << "距离当前目标" << real_target_id_ << "为" << distance << "像素 >" << MAX_ALLOWED_DISTANCE << "像素";
        }

        if (real_target_id_ != best_id) {
            if (maintainTargetContinuity(current_positions)) {
                should_switch = false;
                switch_counter_ = 0;
            } else {
                if (last_switch_candidate_ == best_id) {
                    switch_counter_++;
                } else {
                    switch_counter_ = 1;
                    last_switch_candidate_ = best_id;
                }
                should_switch = (switch_counter_ >= switch_threshold_) && distance_check_passed;
            }
        } else {
            bool is_stable = checkMovementStability(best_scores.angle, best_scores.speed);
            if (!is_stable) {
                should_switch = true;
                switch_counter_++;
            } else {
                switch_counter_ = 0;
            }
        }

        if (best_scores.combined_score > 0.25 && should_switch &&
            switch_counter_ >= switch_threshold_ && distance_check_passed) {

            int old_target = real_target_id_;
            real_target_id_ = best_id;
            last_valid_target_id_ = best_id;
            target_scores_[best_id] = best_scores;

            stability_counter_ = 0;
            switch_counter_ = 0;
            last_switch_candidate_ = -1;
        } else {
            if (real_target_id_ >= 0 &&
                current_positions.find(real_target_id_) != current_positions.end()) {
                last_valid_target_id_ = real_target_id_;
            }
        }
    } else {
        if (!maintainTargetContinuity(current_positions)) {
            switch_counter_ = 0;
            qDebug() << "没有合适目标且无法保持上次目标";
        }
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
