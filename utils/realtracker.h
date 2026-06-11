#ifndef REALTRACKER_H
#define REALTRACKER_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <map>
#include <set>
#include <deque>
#include <cmath>
#include <algorithm>

struct TrackInfo {
    int track_id;
    cv::Point2f center;
    cv::Rect2d bbox;
    double score;
    std::vector<cv::Point2f> trajectory;
};

struct TargetScore {
    double angle;
    double angle_score;
    double speed;
    double speed_score;
    double combined_score;
    cv::Point2f position;
};

// 两元素间直线关系（距离 + 方位角）
struct ElementLink {
    int peer_local_index = -1;
    double distance = 0.0;
    double angle = 0.0;
};

// 单帧观测样本（世界偏移 + 与同帧其它元素的直线关系）
struct FrameElementSample {
    int track_id = -1;
    cv::Point2f world_offset;
    std::vector<ElementLink> links;
};

// 与另一世界节点的稳定直线关系
struct ElementRelationRecord {
    double distance = 0.0;
    double angle = 0.0;
    int count = 0;
};

// 世界节点：通过直线关系轮廓合并，finalize 后计算 layout_pos
struct WorldElementNode {
    int node_id = -1;
    cv::Point2f avg_world_offset{0.f, 0.f};
    int world_offset_count = 0;
    std::map<int, ElementRelationRecord> links;
    int observation_count = 0;
    cv::Point2f layout_pos{0.f, 0.f};
    bool layout_pos_valid = false;
};

struct CollectedElement {
    cv::Point2f world_pos;
    int slot_id = -1;
    int observation_count = 0;
};

class RealTargetDetector {
public:
    RealTargetDetector();

    void setLockCoordinate(const cv::Point2i& coord);
    void updateTracks(const std::vector<TrackInfo>& tracks);
    int identifyRealTarget(const std::vector<TrackInfo>& tracks,
                          const cv::Point2i& currentPos,
                          double aspectRatio,
                          const cv::Rect& detectionRegion);

    std::tuple<double, double, double> calculateMovementMetrics(int trackId);
    std::tuple<double, double, double> calculateMovementMetrics2(int trackId);
    double calculateAngleDifference(double angle1, double angle2) const;
    bool checkMovementStability(double currentAngle, double currentSpeed);

    std::tuple<bool, std::string> isNearBoundary(const TrackInfo& target,
                                                   const cv::Rect& region,
                                                   double aspectRatio,
                                                   int margin = 20);

    void reset();
    int getRealTargetId() const { return real_target_id_; }
    int getTargetLostCounter() const { return target_lost_counter_; }
    bool isLockTransferred() const { return lock_transferred_; }

    const std::vector<CollectedElement>& getCollectedElements() const { return collected_elements_; }
    cv::Rect2f getWorldCanvasBounds() const;
    void finalizeElementCollection();
    bool saveElementMapImage(const std::string& path, int rectSize = 70) const;
    bool saveWorldNodesToJson(const std::string& path) const;
    bool loadWorldNodesFromJson(const std::string& path);
    void setWorldNodesCachePath(const std::string& path) { world_nodes_cache_path_ = path; }
    bool hasWorldNodes() const { return !world_nodes_.empty(); }

    std::map<int, int> countDirectionsByCategory(const std::vector<TrackInfo>& tracks);
    int getBackgroundDirection(const std::map<int, int>& direction_counts);

private:
    static double pointDistance(const cv::Point2f& p1, const cv::Point2f& p2);

    void updateBackgroundAngle(const std::map<int, TargetScore>& candidateScores);
    std::tuple<int, double> findClosestTargetToLock(const std::map<int, cv::Point2f>& currentPositions);
    bool maintainTargetContinuity(const std::map<int, cv::Point2f>& currentPositions);
    double calculateAngle(const cv::Point2f& from, const cv::Point2f& to, double aspectRatio = 1.0);

    void collectFrameWorldElements(const std::vector<TrackInfo>& tracks,
                                   const cv::Rect& detectionRegion);
    cv::Point2f computeBackgroundMotion(const std::vector<TrackInfo>& tracks) const;
    void expandWorldCanvasByMotion(const cv::Point2f& bgMotion, int backgroundDirection);
    double angleFromPoint(const cv::Point2f& from, const cv::Point2f& to) const;

    std::vector<FrameElementSample> buildFrameSamples(const std::vector<TrackInfo>& tracks) const;
    void mergeBatchIntoWorldGraph(const std::vector<FrameElementSample>& batch);
    void mergeBatchWithExistingGraph(const std::vector<FrameElementSample>& batch,
                                     std::vector<int>& localToWorldNode);
    bool lineRelationMatchesRaw(double distance, double angle,
                                double otherDistance, double otherAngle) const;
    bool lineRelationMatches(double distance, double angle,
                             const ElementRelationRecord& record) const;
    int countMatchingLinks(const FrameElementSample& a, const FrameElementSample& b) const;
    bool allLinksMatchProfile(const FrameElementSample& a, const FrameElementSample& b) const;
    int requiredMatchingLinks(int linkCount) const;
    bool profilesCompatible(const FrameElementSample& a, const FrameElementSample& b) const;
    FrameElementSample buildLinkProfileSample(const WorldElementNode& node) const;
    void assignLocalToWorld(std::vector<int>& localToWorld, int localIdx, int worldId);
    bool linksSubsetOf(const FrameElementSample& subset, const FrameElementSample& superset) const;
    void matchElementsViaLastFrame(const std::vector<FrameElementSample>& batch,
                                   std::vector<int>& localToWorld);
    void mergeSameProfileInFrame(const std::vector<FrameElementSample>& batch,
                                 std::vector<int>& localToWorld);
    void mergeFrameLinksToWorld(const std::vector<FrameElementSample>& batch,
                                const std::vector<int>& localToWorld);
    void saveFrameSnapshot(const std::vector<FrameElementSample>& batch,
                           const std::vector<int>& localToWorld);
    bool lineAnglesMatch(double angleA, double angleB) const;
    bool nodeHasMatchingLink(const WorldElementNode& node, double distance, double angle,
                             int preferredPeerId) const;
    int findMatchingWorldNode(const FrameElementSample& sample,
                              const std::vector<int>& localToWorldNode) const;
    int createWorldNode(const cv::Point2f& worldOffset);
    WorldElementNode* findWorldNode(int nodeId);
    void absorbWorldOffset(WorldElementNode& node, const cv::Point2f& worldOffset);
    void mergeWorldLink(WorldElementNode& node, int peerNodeId, double distance, double angle);
    void consolidateWorldNodes(int keepId, int removeId);
    void deduplicateWorldNodes();
    bool nodesShareSameRelations(const WorldElementNode& a, const WorldElementNode& b) const;
    bool worldNodesDuplicate(const WorldElementNode& a, const WorldElementNode& b) const;
    static double normalizeLineAngle(double angle);
    double alignAngleForMerge(double reference, double incoming) const;
    void buildLayoutFromGraph();
    bool ensureWorldNodesCacheLoaded();
    bool isCachedInterferenceTarget(const cv::Point2f& worldPos) const;

    double angle_threshold_ = 30.0;
    double angle_change_threshold_ = 30.0;
    double speed_change_threshold_ = 3.0;
    double stability_threshold_ = 2.0;
    int max_target_lost_frames_ = 5;
    int switch_threshold_ = 3;

    int real_target_id_ = -1;
    int frame_count_ = 0;
    int stability_counter_ = 0;
    int target_lost_counter_ = 0;
    int switch_counter_ = 0;
    int last_switch_candidate_ = -1;
    int last_valid_target_id_ = -1;

    double background_angle_ = -1.0;
    double last_angle_ = -1.0;
    double last_speed_ = -1.0;

    cv::Point2i lock_coord_;
    bool lock_transferred_ = false;
    int last_background_direction_ = -1;

    std::map<int, std::deque<cv::Point2f>> track_history_;
    std::map<int, TargetScore> target_scores_;
    std::map<int, std::deque<cv::Point2f>> position_history_;

    double merge_tolerance_ = 20.0;
    double angle_match_tolerance_ = 15.0;
    double link_match_ratio_ = 0.7;
    double interference_position_tolerance_ = 35.0;
    int max_world_nodes_ = 999;//35

    cv::Point2f world_offset_{0.f, 0.f};
    cv::Point2f world_canvas_min_{0.f, 0.f};
    cv::Point2f world_canvas_max_{500.f, 300.f};
    bool world_origin_initialized_ = false;
    int collection_frame_index_ = 0;
    int next_node_id_ = 0;
    double viewport_width_ = 500.0;
    double viewport_height_ = 300.0;
    double estimated_bg_width_ = 1000.0;
    double estimated_bg_height_ = 600.0;

    std::vector<WorldElementNode> world_nodes_;
    std::vector<FrameElementSample> last_frame_samples_;
    std::vector<int> last_frame_node_ids_;
    std::vector<CollectedElement> collected_elements_;
    bool world_coords_confirmed_ = false;
    std::string world_nodes_cache_path_;

    static const int MAX_TRAJECTORY_SIZE = 20;
};

#endif // REALTRACKER_H
