#include "realtracker.h"
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <ranges>
#include <limits>

double RealTargetDetector::pointDistance(const cv::Point2f& p1, const cv::Point2f& p2) {
    const double dx = p1.x - p2.x;
    const double dy = p1.y - p2.y;
    return std::sqrt(dx * dx + dy * dy);
}

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
    world_offset_ = { 0.f, 0.f };
    world_canvas_min_ = { 0.f, 0.f };
    world_canvas_max_ = { 500.f, 300.f };
    world_origin_initialized_ = false;
    collection_frame_index_ = 0;
    next_node_id_ = 0;
    world_nodes_.clear();
    last_frame_samples_.clear();
    last_frame_node_ids_.clear();
    collected_elements_.clear();
    world_coords_confirmed_ = false;
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
        // qDebug() << "Track" << track.track_id << "方向:" << dir_names[direction] << QString("(%1°)").arg(angle, 0, 'f', 1);

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

double RealTargetDetector::calculateAngleDifference(double angle1, double angle2) const {
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

double RealTargetDetector::angleFromPoint(const cv::Point2f& from, const cv::Point2f& to) const {
    const double dx = to.x - from.x;
    const double dy = to.y - from.y;
    double angle = std::atan2(dx, -dy) * 180.0 / CV_PI;
    if (angle < 0) angle += 360.0;
    return angle;
}

double RealTargetDetector::normalizeLineAngle(double angle) {
    angle = std::fmod(angle, 360.0);
    if (angle < 0.0) {
        angle += 360.0;
    }
    return angle;
}

bool RealTargetDetector::lineAnglesMatch(double angleA, double angleB) const {
    const double a = normalizeLineAngle(angleA);
    const double b = normalizeLineAngle(angleB);
    const double tol = angle_match_tolerance_;

    // 同向: a ± tol
    if (calculateAngleDifference(a, b) <= tol) {
        return true;
    }

    // 直线反向: 43±15 与 (180-43)=137±15 视为同一条直线
    const double compA = 180.0 - a;
    const double compB = 180.0 - b;
    if (std::abs(b - compA) <= tol || std::abs(a - compB) <= tol) {
        return true;
    }

    // 0-360 表示下的反向（如 43° 与 223°）
    if (std::abs(calculateAngleDifference(a, b) - 180.0) <= tol) {
        return true;
    }

    return false;
}

double RealTargetDetector::alignAngleForMerge(double reference, double incoming) const {
    const double ref = normalizeLineAngle(reference);
    double inc = normalizeLineAngle(incoming);

    if (calculateAngleDifference(inc, ref) <= angle_match_tolerance_) {
        return inc;
    }

    const double compRef = 180.0 - ref;
    if (std::abs(inc - compRef) <= angle_match_tolerance_) {
        return ref;
    }

    if (std::abs(calculateAngleDifference(inc, ref) - 180.0) <= angle_match_tolerance_) {
        return ref;
    }

    return inc;
}

bool RealTargetDetector::lineRelationMatchesRaw(double distance, double angle,
                                                double otherDistance,
                                                double otherAngle) const {
    return std::abs(distance - otherDistance) <= merge_tolerance_
        && lineAnglesMatch(angle, otherAngle);
}

bool RealTargetDetector::lineRelationMatches(double distance, double angle,
                                             const ElementRelationRecord& record) const {
    return lineRelationMatchesRaw(distance, angle, record.distance, record.angle);
}

int RealTargetDetector::countMatchingLinks(const FrameElementSample& a,
                                             const FrameElementSample& b) const {
    int count = 0;
    for (const auto& linkA : a.links) {
        for (const auto& linkB : b.links) {
            if (lineRelationMatchesRaw(linkA.distance, linkA.angle,
                                       linkB.distance, linkB.angle)) {
                count++;
                break;
            }
        }
    }
    return count;
}

bool RealTargetDetector::allLinksMatchProfile(const FrameElementSample& a,
                                              const FrameElementSample& b) const {
    if (a.links.empty() || a.links.size() != b.links.size()) {
        return false;
    }
    return countMatchingLinks(a, b) == static_cast<int>(a.links.size());
}

int RealTargetDetector::requiredMatchingLinks(int linkCount) const {
    if (linkCount <= 0) {
        return 0;
    }
    if (linkCount == 1) {
        return 1;
    }
    return std::max(2, static_cast<int>(std::ceil(linkCount * link_match_ratio_)));
}

bool RealTargetDetector::profilesCompatible(const FrameElementSample& a,
                                            const FrameElementSample& b) const {
    if (a.links.empty() || b.links.empty()) {
        return false;
    }

    const int countA = static_cast<int>(a.links.size());
    const int countB = static_cast<int>(b.links.size());
    const int matched = countMatchingLinks(a, b);

    if (countA == countB) {
        return matched >= requiredMatchingLinks(countA);
    }

    if (countA < countB) {
        return matched == countA && matched >= requiredMatchingLinks(countA);
    }

    const int matchedReverse = countMatchingLinks(b, a);
    return matchedReverse == countB && matchedReverse >= requiredMatchingLinks(countB);
}

FrameElementSample RealTargetDetector::buildLinkProfileSample(const WorldElementNode& node) const {
    FrameElementSample sample;
    for (const auto& [peerId, rel] : node.links) {
        ElementLink link;
        link.peer_local_index = peerId;
        link.distance = rel.distance;
        link.angle = rel.angle;
        sample.links.push_back(link);
    }
    return sample;
}

bool RealTargetDetector::linksSubsetOf(const FrameElementSample& subset,
                                       const FrameElementSample& superset) const {
    if (subset.links.empty()) {
        return false;
    }
    return countMatchingLinks(subset, superset) == static_cast<int>(subset.links.size());
}

void RealTargetDetector::assignLocalToWorld(std::vector<int>& localToWorld,
                                            int localIdx, int worldId) {
    if (localIdx < 0 || localIdx >= static_cast<int>(localToWorld.size()) || worldId < 0) {
        return;
    }

    if (localToWorld[localIdx] < 0) {
        localToWorld[localIdx] = worldId;
    }
}

void RealTargetDetector::matchElementsViaLastFrame(const std::vector<FrameElementSample>& batch,
                                                   std::vector<int>& localToWorld) {
    if (last_frame_samples_.empty()) {
        return;
    }

    const int n = static_cast<int>(batch.size());
    const int lastCount = static_cast<int>(last_frame_samples_.size());
    std::vector<bool> curUsed(n, false);
    std::vector<bool> lastUsed(lastCount, false);

    struct MatchCandidate {
        int curIdx = -1;
        int lastIdx = -1;
        int score = 0;
    };

    std::vector<MatchCandidate> candidates;
    candidates.reserve(static_cast<size_t>(n * lastCount));

    for (int i = 0; i < n; ++i) {
        if (batch[i].links.empty()) {
            continue;
        }

        for (int j = 0; j < lastCount; ++j) {
            if (last_frame_samples_[j].links.empty()) {
                continue;
            }

            if (!profilesCompatible(batch[i], last_frame_samples_[j])) {
                continue;
            }

            MatchCandidate candidate;
            candidate.curIdx = i;
            candidate.lastIdx = j;
            candidate.score = countMatchingLinks(batch[i], last_frame_samples_[j]);
            candidates.push_back(candidate);
        }
    }

    std::sort(candidates.begin(), candidates.end(),
              [](const MatchCandidate& lhs, const MatchCandidate& rhs) {
                  return lhs.score > rhs.score;
              });

    for (const auto& candidate : candidates) {
        if (curUsed[candidate.curIdx] || lastUsed[candidate.lastIdx]) {
            continue;
        }

        const int worldId = last_frame_node_ids_[candidate.lastIdx];
        if (worldId < 0) {
            continue;
        }

        assignLocalToWorld(localToWorld, candidate.curIdx, worldId);
        curUsed[candidate.curIdx] = true;
        lastUsed[candidate.lastIdx] = true;
    }
}

void RealTargetDetector::mergeSameProfileInFrame(const std::vector<FrameElementSample>& batch,
                                                 std::vector<int>& localToWorld) {
    const int n = static_cast<int>(batch.size());
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (!allLinksMatchProfile(batch[i], batch[j])) {
                continue;
            }

            if (localToWorld[i] < 0 && localToWorld[j] < 0) {
                continue;
            }

            if (localToWorld[i] < 0) {
                assignLocalToWorld(localToWorld, i, localToWorld[j]);
            } else if (localToWorld[j] < 0) {
                assignLocalToWorld(localToWorld, j, localToWorld[i]);
            } else if (localToWorld[i] != localToWorld[j]) {
                localToWorld[j] = localToWorld[i];
            }
        }
    }
}

void RealTargetDetector::mergeFrameLinksToWorld(const std::vector<FrameElementSample>& batch,
                                                const std::vector<int>& localToWorld) {
    const int n = static_cast<int>(batch.size());
    for (int i = 0; i < n; ++i) {
        const int nodeId = localToWorld[i];
        if (nodeId < 0) {
            continue;
        }

        WorldElementNode* node = findWorldNode(nodeId);
        if (!node) {
            continue;
        }

        for (const auto& link : batch[i].links) {
            if (link.peer_local_index < 0 || link.peer_local_index <= i) {
                continue;
            }

            const int peerNodeId = localToWorld[link.peer_local_index];
            if (peerNodeId < 0 || peerNodeId == nodeId) {
                continue;
            }

            mergeWorldLink(*node, peerNodeId, link.distance, link.angle);

            WorldElementNode* peerNode = findWorldNode(peerNodeId);
            if (!peerNode) {
                continue;
            }

            const double reverseAngle = angleFromPoint(batch[link.peer_local_index].world_offset,
                                                     batch[i].world_offset);
            mergeWorldLink(*peerNode, nodeId, link.distance, reverseAngle);
        }
    }
}

void RealTargetDetector::saveFrameSnapshot(const std::vector<FrameElementSample>& batch,
                                             const std::vector<int>& localToWorld) {
    last_frame_samples_ = batch;
    last_frame_node_ids_ = localToWorld;
}

bool RealTargetDetector::nodeHasMatchingLink(const WorldElementNode& node, double distance,
                                               double angle, int preferredPeerId) const {
    if (preferredPeerId >= 0) {
        const auto preferredIt = node.links.find(preferredPeerId);
        if (preferredIt != node.links.end()
            && lineRelationMatches(distance, angle, preferredIt->second)) {
            return true;
        }
    }

    for (const auto& [peerId, rel] : node.links) {
        if (peerId == preferredPeerId) {
            continue;
        }
        if (lineRelationMatches(distance, angle, rel)) {
            return true;
        }
    }

    return false;
}

WorldElementNode* RealTargetDetector::findWorldNode(int nodeId) {
    for (auto& node : world_nodes_) {
        if (node.node_id == nodeId) {
            return &node;
        }
    }
    return nullptr;
}

int RealTargetDetector::createWorldNode(const cv::Point2f& worldOffset) {
    WorldElementNode node;
    node.node_id = next_node_id_++;
    node.avg_world_offset = worldOffset;
    node.world_offset_count = 0;
    node.observation_count = 0;
    world_nodes_.push_back(node);
    return node.node_id;
}

void RealTargetDetector::absorbWorldOffset(WorldElementNode& node, const cv::Point2f& worldOffset) {
    node.avg_world_offset.x = (node.avg_world_offset.x * node.world_offset_count + worldOffset.x)
                              / (node.world_offset_count + 1);
    node.avg_world_offset.y = (node.avg_world_offset.y * node.world_offset_count + worldOffset.y)
                              / (node.world_offset_count + 1);
    node.world_offset_count++;
    node.observation_count++;
}

void RealTargetDetector::mergeWorldLink(WorldElementNode& node, int peerNodeId,
                                        double distance, double angle) {
    auto& rel = node.links[peerNodeId];
    if (rel.count == 0) {
        rel.distance = distance;
        rel.angle = angle;
        rel.count = 1;
        return;
    }

    rel.distance = (rel.distance * rel.count + distance) / (rel.count + 1);
    const double aligned = alignAngleForMerge(rel.angle, angle);
    rel.angle = (rel.angle * rel.count + aligned) / (rel.count + 1);
    rel.count++;
}

void RealTargetDetector::consolidateWorldNodes(int keepId, int removeId) {
    if (keepId == removeId) {
        return;
    }

    WorldElementNode* keep = findWorldNode(keepId);
    WorldElementNode* remove = findWorldNode(removeId);
    if (!keep || !remove) {
        return;
    }

    const int totalOffset = keep->world_offset_count + remove->world_offset_count;
    if (totalOffset > 0) {
        keep->avg_world_offset.x = (keep->avg_world_offset.x * keep->world_offset_count
                                    + remove->avg_world_offset.x * remove->world_offset_count)
                                   / totalOffset;
        keep->avg_world_offset.y = (keep->avg_world_offset.y * keep->world_offset_count
                                    + remove->avg_world_offset.y * remove->world_offset_count)
                                   / totalOffset;
        keep->world_offset_count = totalOffset;
    }
    keep->observation_count += remove->observation_count;

    for (const auto& [peerId, rel] : remove->links) {
        const int remappedPeer = peerId == removeId ? keepId : peerId;
        if (remappedPeer == keepId) {
            continue;
        }
        mergeWorldLink(*keep, remappedPeer, rel.distance, rel.angle);
    }

    for (auto& node : world_nodes_) {
        if (node.node_id == removeId) {
            continue;
        }

        auto removeIt = node.links.find(removeId);
        if (removeIt != node.links.end()) {
            if (node.node_id != keepId) {
                mergeWorldLink(node, keepId, removeIt->second.distance, removeIt->second.angle);
            }
            node.links.erase(removeIt);
        }
    }

    world_nodes_.erase(
        std::remove_if(world_nodes_.begin(), world_nodes_.end(),
                       [removeId](const WorldElementNode& node) {
                           return node.node_id == removeId;
                       }),
        world_nodes_.end());
}

bool RealTargetDetector::nodesShareSameRelations(const WorldElementNode& a,
                                                 const WorldElementNode& b) const {
    if (a.node_id == b.node_id) {
        return true;
    }

    int sharedLinks = 0;

    for (const auto& [peerId, relA] : a.links) {
        if (peerId == b.node_id) {
            continue;
        }

        const auto relBIt = b.links.find(peerId);
        if (relBIt == b.links.end()) {
            return false;
        }

        if (!lineRelationMatches(relA.distance, relA.angle, relBIt->second)) {
            return false;
        }
        sharedLinks++;
    }

    for (const auto& [peerId, relB] : b.links) {
        if (peerId == a.node_id) {
            continue;
        }
        if (a.links.find(peerId) == a.links.end()) {
            return false;
        }
    }

    return sharedLinks > 0;
}

bool RealTargetDetector::worldNodesDuplicate(const WorldElementNode& a,
                                             const WorldElementNode& b) const {
    const FrameElementSample profileA = buildLinkProfileSample(a);
    const FrameElementSample profileB = buildLinkProfileSample(b);
    if (profileA.links.empty() || profileB.links.empty()) {
        return false;
    }

    const int countA = static_cast<int>(profileA.links.size());
    const int countB = static_cast<int>(profileB.links.size());
    const int minCount = std::min(countA, countB);
    const int maxCount = std::max(countA, countB);

    const FrameElementSample& smaller = countA <= countB ? profileA : profileB;
    const FrameElementSample& larger = countA <= countB ? profileB : profileA;
    const int matched = countMatchingLinks(smaller, larger);

    if (matched < requiredMatchingLinks(minCount)) {
        return false;
    }

    if (matched < static_cast<int>(std::ceil(minCount * link_match_ratio_))) {
        return false;
    }

    // 允许较大节点多 1~2 条不稳定连线（如偶发误检）
    const int extraLinksAllowed = std::max(1, static_cast<int>(std::ceil(maxCount * 0.15)));
    return (maxCount - minCount) <= extraLinksAllowed
        && (matched >= minCount - 1 || matched >= requiredMatchingLinks(maxCount));
}

void RealTargetDetector::deduplicateWorldNodes() {
    bool merged = true;
    while (merged) {
        merged = false;

        for (size_t i = 0; i < world_nodes_.size() && !merged; ++i) {
            for (size_t j = 0; j < i && !merged; ++j) {
                if (!worldNodesDuplicate(world_nodes_[i], world_nodes_[j])) {
                    continue;
                }

                const int keepId = world_nodes_[j].observation_count >= world_nodes_[i].observation_count
                    ? world_nodes_[j].node_id
                    : world_nodes_[i].node_id;
                const int removeId = keepId == world_nodes_[j].node_id
                    ? world_nodes_[i].node_id
                    : world_nodes_[j].node_id;

                consolidateWorldNodes(keepId, removeId);
                merged = true;
            }
        }
    }
}

int RealTargetDetector::findMatchingWorldNode(const FrameElementSample& sample,
                                              const std::vector<int>& localToWorldNode) const {
    if (sample.links.empty()) {
        return -1;
    }

    int bestNodeId = -1;
    int bestScore = -1;

    for (const auto& worldNode : world_nodes_) {
        const FrameElementSample profile = buildLinkProfileSample(worldNode);
        if (!profilesCompatible(sample, profile)) {
            continue;
        }

        const int score = countMatchingLinks(sample, profile);
        if (score > bestScore) {
            bestScore = score;
            bestNodeId = worldNode.node_id;
        }
    }

    return bestNodeId;
}

std::vector<FrameElementSample> RealTargetDetector::buildFrameSamples(
    const std::vector<TrackInfo>& tracks) const {

    std::vector<FrameElementSample> batch;
    batch.reserve(tracks.size());

    for (const auto& track : tracks) {
        if (real_target_id_ >= 0 && track.track_id == real_target_id_) {
            continue;
        }

        FrameElementSample sample;
        sample.track_id = track.track_id;
        sample.world_offset = track.center + world_offset_;
        batch.push_back(sample);
    }

    const int n = static_cast<int>(batch.size());
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            const double dist = pointDistance(batch[i].world_offset, batch[j].world_offset);
            const double angleIJ = angleFromPoint(batch[i].world_offset, batch[j].world_offset);
            const double angleJI = angleFromPoint(batch[j].world_offset, batch[i].world_offset);

            ElementLink linkIJ;
            linkIJ.peer_local_index = j;
            linkIJ.distance = dist;
            linkIJ.angle = angleIJ;
            batch[i].links.push_back(linkIJ);

            ElementLink linkJI;
            linkJI.peer_local_index = i;
            linkJI.distance = dist;
            linkJI.angle = angleJI;
            batch[j].links.push_back(linkJI);
        }
    }

    return batch;
}

void RealTargetDetector::mergeBatchWithExistingGraph(const std::vector<FrameElementSample>& batch,
                                                   std::vector<int>& localToWorldNode) {
    const int n = static_cast<int>(batch.size());

    if (!last_frame_samples_.empty()) {
        matchElementsViaLastFrame(batch, localToWorldNode);
    }

    for (int i = 0; i < n; ++i) {
        if (localToWorldNode[i] >= 0) {
            continue;
        }

        int nodeId = findMatchingWorldNode(batch[i], localToWorldNode);
        if (nodeId < 0) {
            if (static_cast<int>(world_nodes_.size()) >= max_world_nodes_) {
                qDebug() << "已达世界节点上限" << max_world_nodes_
                         << "，跳过 track" << batch[i].track_id;
                continue;
            }
            nodeId = createWorldNode(batch[i].world_offset);
        }

        assignLocalToWorld(localToWorldNode, i, nodeId);
    }

    mergeSameProfileInFrame(batch, localToWorldNode);

    for (int i = 0; i < n; ++i) {
        if (localToWorldNode[i] < 0) {
            continue;
        }
        WorldElementNode* node = findWorldNode(localToWorldNode[i]);
        if (node) {
            absorbWorldOffset(*node, batch[i].world_offset);
        }
    }

    mergeFrameLinksToWorld(batch, localToWorldNode);
    saveFrameSnapshot(batch, localToWorldNode);
    deduplicateWorldNodes();
}

void RealTargetDetector::mergeBatchIntoWorldGraph(const std::vector<FrameElementSample>& batch) {
    const int n = static_cast<int>(batch.size());
    if (n <= 0) {
        return;
    }

    std::vector<int> localToWorldNode(n, -1);

    if (last_frame_samples_.empty() && world_nodes_.empty()) {
        for (int i = 0; i < n; ++i) {
            if (static_cast<int>(world_nodes_.size()) >= max_world_nodes_) {
                qDebug() << "已达世界节点上限" << max_world_nodes_
                         << "，跳过 track" << batch[i].track_id;
                continue;
            }
            localToWorldNode[i] = createWorldNode(batch[i].world_offset);
        }

        for (int i = 0; i < n; ++i) {
            if (localToWorldNode[i] < 0) {
                continue;
            }
            WorldElementNode* node = findWorldNode(localToWorldNode[i]);
            if (node) {
                absorbWorldOffset(*node, batch[i].world_offset);
            }
        }

        mergeFrameLinksToWorld(batch, localToWorldNode);
        saveFrameSnapshot(batch, localToWorldNode);
        return;
    }

    mergeBatchWithExistingGraph(batch, localToWorldNode);
}

namespace {

cv::Point2f offsetFromLink(double distance, double angleDeg) {
    const double rad = angleDeg * CV_PI / 180.0;
    return cv::Point2f(static_cast<float>(distance * std::sin(rad)),
                       static_cast<float>(-distance * std::cos(rad)));
}

} // namespace

void RealTargetDetector::buildLayoutFromGraph() {
    if (world_nodes_.empty()) {
        return;
    }

    for (auto& node : world_nodes_) {
        node.layout_pos_valid = false;
    }

    WorldElementNode* anchor = &world_nodes_[0];
    for (auto& node : world_nodes_) {
        if (node.avg_world_offset.x < anchor->avg_world_offset.x
            || (node.avg_world_offset.x == anchor->avg_world_offset.x
                && node.avg_world_offset.y < anchor->avg_world_offset.y)) {
            anchor = &node;
        }
    }

    anchor->layout_pos = anchor->avg_world_offset;
    anchor->layout_pos_valid = true;

    std::vector<int> queue;
    queue.push_back(anchor->node_id);

    while (!queue.empty()) {
        const int currentId = queue.front();
        queue.erase(queue.begin());

        WorldElementNode* current = findWorldNode(currentId);
        if (!current || !current->layout_pos_valid) {
            continue;
        }

        for (const auto& [peerId, rel] : current->links) {
            WorldElementNode* peer = findWorldNode(peerId);
            if (!peer) {
                continue;
            }

            const cv::Point2f delta = offsetFromLink(rel.distance, rel.angle);
            const cv::Point2f expectedPos = current->layout_pos + delta;

            if (!peer->layout_pos_valid) {
                peer->layout_pos = expectedPos;
                peer->layout_pos_valid = true;
                queue.push_back(peerId);
            }
        }
    }

    for (auto& node : world_nodes_) {
        if (!node.layout_pos_valid) {
            node.layout_pos = node.avg_world_offset;
            node.layout_pos_valid = true;
        }
    }
}

cv::Point2f RealTargetDetector::computeBackgroundMotion(const std::vector<TrackInfo>& tracks) const {
    cv::Point2f sum(0.f, 0.f);
    int count = 0;

    for (const auto& track : tracks) {
        if (real_target_id_ >= 0 && track.track_id == real_target_id_) {
            continue;
        }

        const auto it = track_history_.find(track.track_id);
        if (it == track_history_.end() || it->second.size() < 2) {
            continue;
        }

        const auto& hist = it->second;
        const cv::Point2f motion = hist.back() - hist[static_cast<int>(hist.size()) - 2];
        if (std::abs(motion.x) < 0.01f && std::abs(motion.y) < 0.01f) {
            continue;
        }

        sum += motion;
        count++;
    }

    if (count == 0) {
        return { 0.f, 0.f };
    }

    return sum * (1.f / count);
}

void RealTargetDetector::expandWorldCanvasByMotion(const cv::Point2f& bgMotion, int backgroundDirection) {
    const double motionMag = std::sqrt(bgMotion.x * bgMotion.x + bgMotion.y * bgMotion.y);
    if (motionMag < 0.5) {
        return;
    }

    const cv::Point2f cameraMotion(-bgMotion.x, -bgMotion.y);
    world_offset_ += cameraMotion;

    const float expandStep = static_cast<float>(motionMag);
    switch (backgroundDirection) {
    case 0: world_canvas_min_.y -= expandStep; break;
    case 1: world_canvas_max_.x += expandStep; world_canvas_min_.y -= expandStep; break;
    case 2: world_canvas_max_.x += expandStep; break;
    case 3: world_canvas_max_.x += expandStep; world_canvas_max_.y += expandStep; break;
    case 4: world_canvas_max_.y += expandStep; break;
    case 5: world_canvas_min_.x -= expandStep; world_canvas_max_.y += expandStep; break;
    case 6: world_canvas_min_.x -= expandStep; break;
    case 7: world_canvas_min_.x -= expandStep; world_canvas_min_.y -= expandStep; break;
    default:
        world_canvas_min_.x -= expandStep;
        world_canvas_min_.y -= expandStep;
        world_canvas_max_.x += expandStep;
        world_canvas_max_.y += expandStep;
        break;
    }

    const float canvasWidth = world_canvas_max_.x - world_canvas_min_.x;
    const float canvasHeight = world_canvas_max_.y - world_canvas_min_.y;
    if (canvasWidth > estimated_bg_width_) {
        world_canvas_max_.x = world_canvas_min_.x + static_cast<float>(estimated_bg_width_);
    }
    if (canvasHeight > estimated_bg_height_) {
        world_canvas_max_.y = world_canvas_min_.y + static_cast<float>(estimated_bg_height_);
    }
}

void RealTargetDetector::collectFrameWorldElements(
    const std::vector<TrackInfo>& tracks,
    const cv::Rect& detectionRegion) {

    if (tracks.empty()) {
        return;
    }

    collection_frame_index_++;
    viewport_width_ = detectionRegion.width > 0 ? detectionRegion.width : viewport_width_;
    viewport_height_ = detectionRegion.height > 0 ? detectionRegion.height : viewport_height_;

    if (!world_origin_initialized_) {
        world_canvas_min_ = { 0.f, 0.f };
        world_canvas_max_ = { static_cast<float>(viewport_width_), static_cast<float>(viewport_height_) };
        world_offset_ = { 0.f, 0.f };
        world_origin_initialized_ = true;
    }

    int backgroundDirection = -1;
    cv::Point2f bgMotion(0.f, 0.f);

    if (collection_frame_index_ >= 2) {
        const auto directionCounts = countDirectionsByCategory(tracks);
        backgroundDirection = getBackgroundDirection(directionCounts);
        bgMotion = computeBackgroundMotion(tracks);
        expandWorldCanvasByMotion(bgMotion, backgroundDirection);
        last_background_direction_ = backgroundDirection;
    }

    const auto batch = buildFrameSamples(tracks);
    if (batch.empty()) {
        return;
    }

    mergeBatchIntoWorldGraph(batch);

    if (collection_frame_index_ == 1) {
        qDebug() << "元素数量:" << tracks.size();
        qDebug() << "元素收集: 第1帧，记录直线关系，暂无背景方向，世界节点数" << world_nodes_.size()
                 << "本帧观测" << batch.size();
    } else {
        qDebug() << "元素收集: 第" << collection_frame_index_ << "帧"
                 << "背景方向 =" << backgroundDirection
                 << "世界偏移 =" << world_offset_.x << world_offset_.y
                 << "世界节点数" << world_nodes_.size();
    }
}

cv::Rect2f RealTargetDetector::getWorldCanvasBounds() const {
    return cv::Rect2f(world_canvas_min_, world_canvas_max_);
}

void RealTargetDetector::finalizeElementCollection() {
    deduplicateWorldNodes();
    buildLayoutFromGraph();

    collected_elements_.clear();
    world_canvas_min_ = { 0.f, 0.f };
    world_canvas_max_ = { static_cast<float>(viewport_width_), static_cast<float>(viewport_height_) };

    for (const auto& node : world_nodes_) {
        CollectedElement element;
        element.slot_id = node.node_id;
        element.world_pos = node.layout_pos;
        element.observation_count = node.observation_count;
        collected_elements_.push_back(element);

        world_canvas_min_.x = std::min(world_canvas_min_.x, element.world_pos.x);
        world_canvas_min_.y = std::min(world_canvas_min_.y, element.world_pos.y);
        world_canvas_max_.x = std::max(world_canvas_max_.x, element.world_pos.x);
        world_canvas_max_.y = std::max(world_canvas_max_.y, element.world_pos.y);
    }

    world_coords_confirmed_ = !collected_elements_.empty();

    if (collected_elements_.empty()) {
        qDebug() << "元素收集完成: 无干扰元素";
        return;
    }

    qDebug() << "元素世界坐标已确认: 共" << collected_elements_.size() << "个干扰元素"
             << "画布范围 [(" << world_canvas_min_.x << "," << world_canvas_min_.y
             << ") - (" << world_canvas_max_.x << "," << world_canvas_max_.y << ")]";
    for (const auto& e : collected_elements_) {
        qDebug() << "  节点" << e.slot_id
                 << "布局坐标(" << e.world_pos.x << "," << e.world_pos.y << ")"
                 << "观测" << e.observation_count << "次";
    }
}

namespace {

QJsonObject pointToJson(const cv::Point2f& point) {
    QJsonObject obj;
    obj["x"] = point.x;
    obj["y"] = point.y;
    return obj;
}

cv::Point2f pointFromJson(const QJsonObject& obj) {
    return cv::Point2f(static_cast<float>(obj["x"].toDouble()),
                         static_cast<float>(obj["y"].toDouble()));
}

} // namespace

bool RealTargetDetector::saveWorldNodesToJson(const std::string& path) const {
    if (world_nodes_.empty()) {
        qWarning() << "世界节点为空，跳过保存:" << path.c_str();
        return false;
    }

    QJsonObject root;
    root["version"] = 1;
    root["next_node_id"] = next_node_id_;
    root["world_canvas_min"] = pointToJson(world_canvas_min_);
    root["world_canvas_max"] = pointToJson(world_canvas_max_);
    root["world_offset"] = pointToJson(world_offset_);
    root["viewport_width"] = viewport_width_;
    root["viewport_height"] = viewport_height_;

    QJsonArray nodesArray;
    for (const auto& node : world_nodes_) {
        QJsonObject nodeObj;
        nodeObj["node_id"] = node.node_id;
        nodeObj["avg_world_offset"] = pointToJson(node.avg_world_offset);
        nodeObj["world_offset_count"] = node.world_offset_count;
        nodeObj["observation_count"] = node.observation_count;
        nodeObj["layout_pos"] = pointToJson(node.layout_pos);
        nodeObj["layout_pos_valid"] = node.layout_pos_valid;

        QJsonArray linksArray;
        for (const auto& [peerId, rel] : node.links) {
            QJsonObject linkObj;
            linkObj["peer_id"] = peerId;
            linkObj["distance"] = rel.distance;
            linkObj["angle"] = rel.angle;
            linkObj["count"] = rel.count;
            linksArray.append(linkObj);
        }
        nodeObj["links"] = linksArray;
        nodesArray.append(nodeObj);
    }
    root["nodes"] = nodesArray;

    QFile file(QString::fromStdString(path));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "世界节点 JSON 保存失败，无法打开文件:" << path.c_str();
        return false;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    qDebug() << "世界节点已保存:" << path.c_str() << "共" << world_nodes_.size() << "个节点";
    return true;
}

bool RealTargetDetector::loadWorldNodesFromJson(const std::string& path) {
    QFile file(QString::fromStdString(path));
    if (!file.exists()) {
        qDebug() << "世界节点缓存不存在，将从头收集:" << path.c_str();
        return false;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "世界节点 JSON 读取失败，无法打开文件:" << path.c_str();
        return false;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        qWarning() << "世界节点 JSON 格式无效:" << path.c_str();
        return false;
    }

    const QJsonObject root = doc.object();
    if (!root.contains("nodes") || !root["nodes"].isArray()) {
        qWarning() << "世界节点 JSON 缺少 nodes 数组:" << path.c_str();
        return false;
    }

    world_nodes_.clear();
    last_frame_samples_.clear();
    last_frame_node_ids_.clear();
    collected_elements_.clear();
    world_coords_confirmed_ = false;

    next_node_id_ = root["next_node_id"].toInt(0);
    world_canvas_min_ = pointFromJson(root["world_canvas_min"].toObject());
    world_canvas_max_ = pointFromJson(root["world_canvas_max"].toObject());
    world_offset_ = pointFromJson(root["world_offset"].toObject());
    viewport_width_ = root["viewport_width"].toDouble(viewport_width_);
    viewport_height_ = root["viewport_height"].toDouble(viewport_height_);
    world_origin_initialized_ = true;
    collection_frame_index_ = 0;

    const QJsonArray nodesArray = root["nodes"].toArray();
    world_nodes_.reserve(nodesArray.size());

    for (const QJsonValue& nodeValue : nodesArray) {
        const QJsonObject nodeObj = nodeValue.toObject();
        WorldElementNode node;
        node.node_id = nodeObj["node_id"].toInt(-1);
        node.avg_world_offset = pointFromJson(nodeObj["avg_world_offset"].toObject());
        node.world_offset_count = nodeObj["world_offset_count"].toInt(0);
        node.observation_count = nodeObj["observation_count"].toInt(0);
        node.layout_pos = pointFromJson(nodeObj["layout_pos"].toObject());
        node.layout_pos_valid = nodeObj["layout_pos_valid"].toBool(false);

        const QJsonArray linksArray = nodeObj["links"].toArray();
        for (const QJsonValue& linkValue : linksArray) {
            const QJsonObject linkObj = linkValue.toObject();
            const int peerId = linkObj["peer_id"].toInt(-1);
            if (peerId < 0) {
                continue;
            }

            ElementRelationRecord rel;
            rel.distance = linkObj["distance"].toDouble();
            rel.angle = linkObj["angle"].toDouble();
            rel.count = linkObj["count"].toInt(1);
            node.links[peerId] = rel;
        }

        world_nodes_.push_back(node);
        next_node_id_ = std::max(next_node_id_, node.node_id + 1);
    }

    qDebug() << "世界节点缓存已加载:" << path.c_str()
             << "共" << world_nodes_.size() << "个节点"
             << "next_node_id =" << next_node_id_;
    return !world_nodes_.empty();
}

bool RealTargetDetector::ensureWorldNodesCacheLoaded() {
    if (!world_nodes_.empty() || world_nodes_cache_path_.empty()) {
        return !world_nodes_.empty();
    }
    return loadWorldNodesFromJson(world_nodes_cache_path_);
}

bool RealTargetDetector::isCachedInterferenceTarget(const cv::Point2f& worldPos) const {
    for (const auto& node : world_nodes_) {
        if (pointDistance(worldPos, node.avg_world_offset) <= interference_position_tolerance_) {
            return true;
        }
        if (node.layout_pos_valid
            && pointDistance(worldPos, node.layout_pos) <= interference_position_tolerance_) {
            return true;
        }
    }
    return false;
}

bool RealTargetDetector::saveElementMapImage(const std::string& path, int rectSize) const {
    if (!world_coords_confirmed_ || collected_elements_.empty()) {
        qWarning() << "元素位置图为空或未确认，跳过保存:" << path.c_str();
        return false;
    }

    const int half = rectSize / 2;
    cv::Point2f minBound = world_canvas_min_;
    cv::Point2f maxBound = world_canvas_max_;

    for (const auto& element : collected_elements_) {
        minBound.x = std::min(minBound.x, element.world_pos.x - half);
        minBound.y = std::min(minBound.y, element.world_pos.y - half);
        maxBound.x = std::max(maxBound.x, element.world_pos.x + half);
        maxBound.y = std::max(maxBound.y, element.world_pos.y + half);
    }

    const int mapWidth = std::max(1, static_cast<int>(std::ceil(maxBound.x - minBound.x)));
    const int mapHeight = std::max(1, static_cast<int>(std::ceil(maxBound.y - minBound.y)));

    cv::Mat mapImage(mapHeight, mapWidth, CV_8UC3, cv::Scalar(30, 30, 30));

    const cv::Rect viewportRect(
        static_cast<int>(-minBound.x),
        static_cast<int>(-minBound.y),
        static_cast<int>(viewport_width_),
        static_cast<int>(viewport_height_)
    );
    cv::rectangle(mapImage, viewportRect, cv::Scalar(60, 60, 60), 1);

    for (const auto& element : collected_elements_) {
        const int cx = static_cast<int>(std::round(element.world_pos.x - minBound.x));
        const int cy = static_cast<int>(std::round(element.world_pos.y - minBound.y));

        cv::Rect rect(cx - half, cy - half, rectSize, rectSize);
        rect &= cv::Rect(0, 0, mapWidth, mapHeight);
        if (rect.width <= 0 || rect.height <= 0) {
            continue;
        }

        cv::rectangle(mapImage, rect, cv::Scalar(200, 200, 80), cv::FILLED);

        const std::string label = std::to_string(element.slot_id);
        int baseline = 0;
        const cv::Size textSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.35, 1, &baseline);
        const cv::Point textOrg(
            rect.x + (rect.width - textSize.width) / 2,
            rect.y + (rect.height + textSize.height) / 2
        );
        cv::putText(mapImage, label, textOrg,
                    cv::FONT_HERSHEY_SIMPLEX, 0.35, cv::Scalar(255, 255, 255), 1);
    }

    const bool ok = cv::imwrite(path, mapImage);
    if (ok) {
        qDebug() << "元素位置图已保存:" << path.c_str()
                 << "尺寸" << mapWidth << "x" << mapHeight
                 << "干扰元素数" << collected_elements_.size();
    } else {
        qWarning() << "元素位置图保存失败:" << path.c_str();
    }
    return ok;
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

    // 真实目标是否存在于本帧当中
    bool real_track_inframe = false;
    for (const auto& track : tracks) {
        current_positions[track.track_id] = track.center;
        // 保存真实目标附近的所有目标
        if (real_target_id_ >= 0) {
            if(real_target_id_ == track.track_id){
                real_track_inframe = true;
            }

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

    if(!real_track_inframe){
        qDebug() << "！！！真实目标不在本帧中！！！";
    }else{
        qDebug() << "真实目标存在本帧中";
    }

    updateTracks(tracks);

    frame_count_++;
    target_lost_counter_ = 0;

    bool skip_angle_logic = false;

    if (!lock_transferred_) {
        auto [closest_target, min_distance] = findClosestTargetToLock(current_positions);
        const double DISTANCE_THRESHOLD = 80.0;

        if (closest_target >= 0 && min_distance < DISTANCE_THRESHOLD) {
            real_target_id_ = closest_target;
            last_valid_target_id_ = closest_target;
            lock_transferred_ = true;
            switch_counter_ = 0;
            skip_angle_logic = true;
            qDebug() << "锁定坐标已移交到目标" << closest_target << ", 距离:" << min_distance << "像素";

            auto [angle, avg_speed, _] = calculateMovementMetrics2(closest_target);
            if (angle >= 0) {
                last_angle_ = angle;
                last_speed_ = avg_speed;
                stability_counter_ = 0;
            }
        } else {
            qDebug() << "未找到合适目标进行交接, 最近目标距离:" << min_distance << "像素";
        }
    }


    std::vector<QString> dir_names = {"上", "右上", "右", "右下", "下", "左下", "左", "左上"};
    auto direction_counts = countDirectionsByCategory(tracks);
    // 获取当前背景的运动方向
    int background_direction = getBackgroundDirection(direction_counts);
    qDebug() << "背景运动方向：" << dir_names[background_direction];

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
    qDebug() << "track_id | 方向 | 角度 | 与背景方向相同 | 综合分";
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
        qDebug() << track.track_id << " | " << dir_names[direction] << " | " << angle << " | " << (similar_direction ? "相同" : "不同") << " | " << combined_score;
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
