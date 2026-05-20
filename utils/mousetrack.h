#ifndef MOUSETRACK_H
#define MOUSETRACK_H

#include <opencv2/opencv.hpp>
#include <onnxruntime_cxx_api.h>
#include <global/struct.h>

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


class MouseTrack{
public:
    static void test();
};

#endif // MOUSETRACK_H
