#ifndef REALTRACKER_H
#define REALTRACKER_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <map>
#include <deque>
#include <cmath>
#include <algorithm>

struct TrackInfo {
    int track_id;
    cv::Point2f center;
    cv::Rect2d bbox;
    double score;
    std::vector<cv::Point2f> trajectory;  // 轨迹历史
};

struct TargetScore {
    double angle;
    double angle_score;
    double speed;
    double speed_score;
    double combined_score;
    cv::Point2f position;
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

    // 移动指标计算
    std::tuple<double, double, double> calculateMovementMetrics(int trackId);
    std::tuple<double, double, double> calculateMovementMetrics2(int trackId);
    double calculateAngleDifference(double angle1, double angle2);
    bool checkMovementStability(double currentAngle, double currentSpeed);

    // 边界检测
    std::tuple<bool, std::string> isNearBoundary(const TrackInfo& target,
                                                   const cv::Rect& region,
                                                   double aspectRatio,
                                                   int margin = 20);

    // 工具方法
    void reset();
    int getRealTargetId() const { return real_target_id_; }
    int getTargetLostCounter() const { return target_lost_counter_; }
    bool isLockTransferred() const { return lock_transferred_; }

    // 统计当前帧各方向数量
    std::map<int, int> countDirectionsByCategory(const std::vector<TrackInfo>& tracks);

    // 获取最多的方向（背景运动方向）
    int getBackgroundDirection(const std::map<int, int>& direction_counts);

    // 检查候选目标是否与背景方向不同
    bool isDifferentFromBackground(int track_id,int background_direction,const std::vector<TrackInfo>& tracks);

private:
    void updateBackgroundAngle(const std::map<int, TargetScore>& candidateScores);
    std::tuple<int, double> findClosestTargetToLock(const std::map<int, cv::Point2f>& currentPositions);
    bool maintainTargetContinuity(const std::map<int, cv::Point2f>& currentPositions);

    double calculateAngle(const cv::Point2f& from, const cv::Point2f& to, double aspectRatio = 1.0);

    // 配置参数
    double angle_threshold_ = 30.0;
    double angle_change_threshold_ = 30.0;
    double speed_change_threshold_ = 3.0;
    double stability_threshold_ = 2.0;
    int max_target_lost_frames_ = 5;
    int switch_threshold_ = 3;

    // 状态变量
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

    // 方向属性
    int reset_real_count_ = 0;  // 重置计数器
    int last_background_direction_ = -1;  // 上一帧背景方向
    int consecutive_same_direction_ = 0;  // 连续相同方向帧数

    // 轨迹历史
    std::map<int, std::deque<cv::Point2f>> track_history_;
    std::map<int, TargetScore> target_scores_;
    std::map<int, std::deque<cv::Point2f>> position_history_;

    static const int MAX_TRAJECTORY_SIZE = 20;
};

#endif // REALTRACKER_H
