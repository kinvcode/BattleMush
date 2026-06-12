#ifndef MOUSETRACK_H
#define MOUSETRACK_H

#include <opencv2/opencv.hpp>
#include <onnxruntime_cxx_api.h>
#include <global/struct.h>
#include <chrono>

class ByteTracker {
public:
    ByteTracker(float iou_threshold = 0.3,
                int max_missed_frames = 30,
                float conf_threshold_low = 0.1,
                float conf_threshold_high = 0.5);

    std::vector<Detection> update(const std::vector<Detection>& detections);

private:
    void initKalmanFilter(TrackState& track, const cv::Rect& box);
    cv::Rect predictKalman(TrackState& track);
    void updateKalman(TrackState& track, const cv::Rect& box);
    double calculateIoU(const cv::Rect& box1, const cv::Rect& box2);

    float m_iou_threshold;
    int m_max_missed_frames;
    float m_conf_threshold_low;
    float m_conf_threshold_high;
    int m_next_id;

    std::vector<TrackState> m_tracks;
};

class YOLOv8Inference
{
public:
    YOLOv8Inference(const QString &model_path);

    Ort::Env m_env{nullptr};
    Ort::Session* m_session;
    ByteTracker m_tracker;
    char* m_input_name;       // 使用原始指针
    char* m_output_name;      // 使用原始指针

    std::vector<int64_t> m_input_shape;

    // void YOLOv8Inference(const QString &model_path);

    std::vector<Detection> detect(const cv::Mat& frame, float conf_threshold = 0.25, float iou_threshold = 0.45);

    // 带跟踪的检测（简化版）
    std::vector<Detection> track(const cv::Mat& frame, float conf_threshold = 0.25);

double calculateIoU(const cv::Rect& box1, const cv::Rect& box2);

// 绘制检测结果（支持 Qt 或 OpenCV）

void drawDetections(cv::Mat& frame, const std::vector<Detection>& detections);

};

class MousePredictor
{
public:
    MousePredictor();
    ~MousePredictor();

    /**
     * @brief 更新卡尔曼滤波器（有新的检测结果时调用）
     * @param x 检测到的目标 X 坐标
     * @param y 检测到的目标 Y 坐标
     */
    void update(float x, float y);

    /**
     * @brief 预测目标位置（无新检测时调用）
     * @param dt_ms 距离上次预测的时间（毫秒）
     */
    void predict(float dt_ms);

    /**
     * @brief 获取预测的 X 坐标
     */
    float getPredictedX() const;

    /**
     * @brief 获取预测的 Y 坐标
     */
    float getPredictedY() const;

    /**
     * @brief 检查是否已初始化
     */
    bool isInitialized() const;

    /**
     * @brief 重置预测器
     */
    void reset();

private:
    cv::KalmanFilter m_kalman;
    bool m_has_initialized;
    float m_predicted_x;
    float m_predicted_y;
    std::chrono::high_resolution_clock::time_point m_last_time;
};

namespace MouseTrackConfig {
    constexpr int ROI_W = 500;
    constexpr int ROI_H = 335;
    constexpr int TRAIN_IMG_W = 416;
    constexpr int TRAIN_IMG_H = 256;
}

class MouseTrack{
private:
    double avg_fps_;
public:
    static void test();
    void realTrackTest();
    double getfps();
};

#endif // MOUSETRACK_H
