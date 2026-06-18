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
};

struct TrackPositionRecord {
    int frame = 0;
    TrackInfo track;
};

struct TargetScore {
    bool   near_boundary;
    int    direction;
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
    int identifyRealTarget(const std::vector<TrackInfo>& tracks,
                          const cv::Point2i& currentPos,
                          double aspectRatio,
                          const cv::Rect& detectionRegion);

    int getRealTargetId() const { return real_target_id_; }

    cv::Rect getPredictTargetRect() const { return predict_target_rect_; }
    bool hasPredictTargetRect() const { return predict_target_rect_valid_; }

private:
    void reset();
    void updateTracks(const std::vector<TrackInfo>& tracks);
    std::tuple<double, double> calculateMovementMetrics(int trackId);
    double calculateAngleDifference(double angle1, double angle2) const;
    bool checkMovementStability(double currentAngle, double currentSpeed);

    void updateBackgroundAngle(const std::map<int, TargetScore>& candidateScores);
    std::tuple<int, double> findClosestTargetToLock(const std::map<int, cv::Point2f>& currentPositions);
    bool maintainTargetContinuity(const std::map<int, cv::Point2f>& currentPositions);
    std::tuple<bool, std::string> isNearBoundary(const TrackInfo& target,
                                                   const cv::Rect& region,
                                                   double aspectRatio,
                                                   int margin = 20);
    void updatePredictTargetRect(const std::map<int, cv::Point2f>& currentPositions);

    std::map<int, int> countDirectionsByCategory(const std::vector<TrackInfo>& tracks);
    int getBackgroundDirection(const std::map<int, int>& direction_counts);

    int max_target_lost_frames_ = 5;
    int switch_threshold_ = 3;

    int real_target_id_ = -1;
    int real_target_direction_ = -1;
    int real_target_sameback_count_ = 0;
    int frame_count_ = 0;
    int target_lost_counter_ = 0;
    int switch_counter_ = 0;
    int last_switch_candidate_ = -1;
    int last_valid_target_id_ = -1;
    int last_log_sameback_count_ = 0;

    double background_angle_ = -1.0;

    double last_angle_ = -1.0;
    double last_speed_ = -1.0;
    double angle_change_threshold_ = 30.0;
    double speed_change_threshold_ = 3.0;
    double stability_threshold_ = 2.0;
    int stability_counter_ = 0;

    cv::Point2i lock_coord_;
    bool lock_transferred_ = false;

    std::map<int, std::deque<TrackPositionRecord>> track_history_;
    std::map<int, TargetScore> target_scores_;

    cv::Rect predict_target_rect_;
    bool predict_target_rect_valid_ = false;
};

#endif // REALTRACKER_H
