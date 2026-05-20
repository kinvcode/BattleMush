#include "mousetrack.h"
#include <QResource>
#include <QScreen>
#include <opencv2/dnn.hpp>
#include <opencv2/video/tracking.hpp>
#include "utils/gamestruct.h"
#include "global/gameoffset.h"
#include "global/globalplayer.h"
#include "driver/msdriverpro.h"
#include "driver/mskeycode.h"
#include "utils/helper.h"

YOLOv8Inference::YOLOv8Inference(const QString &model_path)
: m_tracker(0.3, 30, 0.1, 0.5)  // IoU阈值, 最大丢失帧数, 低置信度阈值, 高置信度阈值
{
    QResource resource(model_path);

    // 创建 ONNX Runtime 环境
    m_env = Ort::Env(ORT_LOGGING_LEVEL_WARNING, "YOLOv8");

    // 获取资源数据（直接指向内存，无需拷贝）
    const uchar* data = resource.data();
    size_t dataSize = resource.size();

    // 配置 Session 选项
    Ort::SessionOptions session_options;
    session_options.SetIntraOpNumThreads(1);
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

    // 加载模型
    m_session = new Ort::Session(m_env, data, dataSize, session_options);

    // 获取输入/输出名称（保存为成员变量）
    Ort::AllocatorWithDefaultOptions allocator;

    // auto input_name_ptr = m_session->GetInputNameAllocated(0, allocator);
    // auto output_name_ptr = m_session->GetOutputNameAllocated(0, allocator);
    // m_input_name = input_name_ptr.get();
    // m_output_name = output_name_ptr.get();

    // // 获取输入形状
    // Ort::TypeInfo input_type_info = m_session->GetInputTypeInfo(0);
    // auto input_tensor_info = input_type_info.GetTensorTypeAndShapeInfo();
    // m_input_shape = input_tensor_info.GetShape();

    try {
        auto input_name_ptr = m_session->GetInputNameAllocated(0, allocator);
        auto output_name_ptr = m_session->GetOutputNameAllocated(0, allocator);

        // 拷贝到成员变量（char*）
        size_t input_len = strlen(input_name_ptr.get()) + 1;
        m_input_name = new char[input_len];
        strcpy_s(m_input_name, input_len, input_name_ptr.get());

        size_t output_len = strlen(output_name_ptr.get()) + 1;
        m_output_name = new char[output_len];
        strcpy_s(m_output_name, output_len, output_name_ptr.get());

        qDebug() << "输入节点名称:" << m_input_name;
        qDebug() << "输出节点名称:" << m_output_name;
    } catch (const Ort::Exception& e) {
        qCritical() << "获取节点名称失败:" << e.what();
        throw;
    }

    // 获取输入形状
    try {
        Ort::TypeInfo input_type_info = m_session->GetInputTypeInfo(0);
        auto input_tensor_info = input_type_info.GetTensorTypeAndShapeInfo();
        m_input_shape = input_tensor_info.GetShape();

        QString shapeStr;
        for (auto dim : m_input_shape) {
            shapeStr += QString::number(dim) + " ";
        }
        qDebug() << "模型期望输入形状:" << shapeStr;
    } catch (const Ort::Exception& e) {
        qCritical() << "获取输入形状失败:" << e.what();
        throw;
    }

    qDebug() << "模型加载成功!";
}

// std::vector<Detection> YOLOv8Inference::detect(const cv::Mat &frame, float conf_threshold, float iou_threshold)
// {
//     // 1. 预处理
//     cv::Mat resized_frame;
//     cv::resize(frame, resized_frame, cv::Size(m_input_shape[3], m_input_shape[2]));

//     cv::Mat blob;
//     cv::dnn::blobFromImage(resized_frame, blob, 1.0 / 255.0,
//                            cv::Size(m_input_shape[3], m_input_shape[2]),
//                            cv::Scalar(), true, false);

//     // 2. 准备输入数据
//     std::vector<float> input_data(blob.begin<float>(), blob.end<float>());

//     std::vector<int64_t> input_shape = {1, 3, m_input_shape[2], m_input_shape[3]};
//     Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
//     Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
//         memory_info, input_data.data(), input_data.size(),
//         input_shape.data(), input_shape.size()
//         );

//     // 3. 运行推理
//     auto start_time = std::chrono::high_resolution_clock::now();

//     const char* input_name_ptr = m_input_name;   // char* 直接赋值给 const char*
//     const char* output_name_ptr = m_output_name; // char* 直接赋值给 const char*

//     // 在 m_session->Run 之前添加
//     // qDebug() << "=== detect 函数中 ===";
//     // qDebug() << "输入节点名称:" << m_input_name;
//     // qDebug() << "输出节点名称:" << m_output_name;
//     // qDebug() << "输入形状: 1 3" << m_input_shape[2] << m_input_shape[3];
//     // qDebug() << "输入数据大小:" << input_data.size();

//     auto output_tensors = m_session->Run(
//         Ort::RunOptions{nullptr},
//         &input_name_ptr, &input_tensor, 1,
//         &output_name_ptr, 1
//         );

//     auto end_time = std::chrono::high_resolution_clock::now();
//     auto inference_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
//     // qDebug() << "推理时间: " << inference_time << " ms";

//     // 4. 后处理
//     float* output_data = output_tensors[0].GetTensorMutableData<float>();
//     std::vector<int64_t> output_shape = output_tensors[0].GetTensorTypeAndShapeInfo().GetShape();

//     // YOLOv8 输出格式: [batch, 84, num_detections]
//     int num_detections = output_shape[2];  // 通常是 8400
//     int num_classes = output_shape[1] - 4; // 84 - 4 = 80

//     std::vector<Detection> detections;

//     for (int i = 0; i < num_detections; ++i) {
//         float* detection_ptr = output_data + i * (num_classes + 4);

//         // 获取边界框坐标 (中心x, 中心y, 宽度, 高度) - 归一化坐标
//         float cx = detection_ptr[0];
//         float cy = detection_ptr[1];
//         float w = detection_ptr[2];
//         float h = detection_ptr[3];

//         // 确保宽度和高度为正数
//         if (w <= 0 || h <= 0) {
//             continue;
//         }

//         // 转换为绝对坐标
//         int x = static_cast<int>((cx - w / 2) * frame.cols);
//         int y = static_cast<int>((cy - h / 2) * frame.rows);
//         int width = static_cast<int>(w * frame.cols);
//         int height = static_cast<int>(h * frame.rows);

//         // 确保宽度和高度为正数
//         if (width <= 0 || height <= 0) {
//             continue;
//         }

//         // 边界修正（防止越界）
//         x = std::max(0, x);
//         y = std::max(0, y);

//         // 确保框不超出图像边界
//         if (x + width > frame.cols) {
//             width = frame.cols - x;
//         }
//         if (y + height > frame.rows) {
//             height = frame.rows - y;
//         }

//         // 再次检查宽度和高度有效性
//         if (width <= 0 || height <= 0) {
//             continue;
//         }

//         // 获取类别置信度
//         float* class_scores = detection_ptr + 4;

//         // 找到最大置信度的类别
//         int class_id = 0;
//         float max_score = class_scores[0];
//         for (int j = 1; j < num_classes; ++j) {
//             if (class_scores[j] > max_score) {
//                 max_score = class_scores[j];
//                 class_id = j;
//             }
//         }

//         // 应用置信度阈值
//         if (max_score > conf_threshold) {
//             Detection det;
//             det.class_id = class_id;
//             det.confidence = max_score;
//             det.box = cv::Rect(x, y, width, height);
//             detections.push_back(det);
//         }
//     }

//     // 5. NMS 过滤重叠框
//     std::vector<Detection> filtered_detections;
//     std::vector<int> indices;
//     std::vector<cv::Rect> boxes;
//     std::vector<float> scores;

//     for (const auto& det : detections) {
//         boxes.push_back(det.box);
//         scores.push_back(det.confidence);
//     }

//     cv::dnn::NMSBoxes(boxes, scores, conf_threshold, iou_threshold, indices);

//     for (int idx : indices) {
//         filtered_detections.push_back(detections[idx]);
//     }

//     return filtered_detections;
// }

std::vector<Detection> YOLOv8Inference::detect(const cv::Mat &frame, float conf_threshold, float iou_threshold)
{
    int original_width = frame.cols;
    int original_height = frame.rows;

    int target_height = static_cast<int>(m_input_shape[2]);  // 736
    int target_width = static_cast<int>(m_input_shape[3]);   // 1280

    // 计算缩放和偏移（与之前相同）
    float scale = static_cast<float>(target_width) / original_width;  // 1.0
    int new_width = original_width;
    int new_height = static_cast<int>(original_height * scale);

    if (new_height > target_height) {
        scale = static_cast<float>(target_height) / original_height;
        new_width = static_cast<int>(original_width * scale);
        new_height = original_height;
    }

    new_width = std::min(new_width, target_width);
    new_height = std::min(new_height, target_height);

    // 调整大小
    cv::Mat resized;
    cv::resize(frame, resized, cv::Size(new_width, new_height));

    // Letterbox 填充
    cv::Scalar pad_color(114, 114, 114);
    cv::Mat letterboxed(target_height, target_width, frame.type(), pad_color);

    int pad_x = (target_width - new_width) / 2;
    int pad_y = (target_height - new_height) / 2;

    resized.copyTo(letterboxed(cv::Rect(pad_x, pad_y, new_width, new_height)));

    // 创建 blob
    cv::Mat blob;
    cv::dnn::blobFromImage(letterboxed, blob, 1.0 / 255.0,
                           cv::Size(target_width, target_height),
                           cv::Scalar(), true, false);

    // 准备输入
    std::vector<float> input_data(blob.begin<float>(), blob.end<float>());
    std::vector<int64_t> input_shape = {1, 3, target_height, target_width};

    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info, input_data.data(), input_data.size(),
        input_shape.data(), input_shape.size()
        );

    // 推理
    const char* input_name_ptr = m_input_name;
    const char* output_name_ptr = m_output_name;

    auto output_tensors = m_session->Run(
        Ort::RunOptions{nullptr},
        &input_name_ptr, &input_tensor, 1,
        &output_name_ptr, 1
        );

    // 解析输出 - 新格式 [batch, max_detections, 6]
    float* output_data = output_tensors[0].GetTensorMutableData<float>();
    std::vector<int64_t> output_shape = output_tensors[0].GetTensorTypeAndShapeInfo().GetShape();

    int max_detections = static_cast<int>(output_shape[1]);  // 300
    int values_per_detection = static_cast<int>(output_shape[2]);  // 6

    // qDebug() << "最大检测框数量:" << max_detections;

    std::vector<Detection> detections;

    for (int i = 0; i < max_detections; i++) {
        float* ptr = output_data + i * values_per_detection;

        float x1 = ptr[0];
        float y1 = ptr[1];
        float x2 = ptr[2];
        float y2 = ptr[3];
        float confidence = ptr[4];
        int class_id = static_cast<int>(ptr[5]);

        // 置信度为 0 表示无效检测，跳过
        if (confidence == 0.0f) {
            continue;
        }

        if (confidence < conf_threshold) {
            continue;
        }

        // 坐标转换回原始图像
        float original_x1 = (x1 - pad_x) / scale;
        float original_y1 = (y1 - pad_y) / scale;
        float original_x2 = (x2 - pad_x) / scale;
        float original_y2 = (y2 - pad_y) / scale;

        int x = static_cast<int>(std::max(0.0f, original_x1));
        int y = static_cast<int>(std::max(0.0f, original_y1));
        int width = static_cast<int>(original_x2 - original_x1);
        int height = static_cast<int>(original_y2 - original_y1);

        width = std::min(width, original_width - x);
        height = std::min(height, original_height - y);

        if (width > 0 && height > 0) {
            Detection det;
            det.class_id = class_id;
            det.confidence = confidence;
            det.box = cv::Rect(x, y, width, height);
            detections.push_back(det);

            // qDebug() << "检测到 class:" << class_id << "conf:" << confidence << "box:[" << x << "," << y << "," << x+width << "," << y+height << "]";
        }
    }

    return detections;
}

std::vector<Detection> YOLOv8Inference::track(const cv::Mat &frame, float conf_threshold)
{
    auto detections = detect(frame, conf_threshold);
    return m_tracker.update(detections);
}

double YOLOv8Inference::calculateIoU(const cv::Rect &box1, const cv::Rect &box2)
{
    int x1 = std::max(box1.x, box2.x);
    int y1 = std::max(box1.y, box2.y);
    int x2 = std::min(box1.x + box1.width, box2.x + box2.width);
    int y2 = std::min(box1.y + box1.height, box2.y + box2.height);

    int intersection = std::max(0, x2 - x1) * std::max(0, y2 - y1);
    int area1 = box1.width * box1.height;
    int area2 = box2.width * box2.height;
    int union_area = area1 + area2 - intersection;

    return union_area > 0 ? static_cast<double>(intersection) / union_area : 0.0;
}

void YOLOv8Inference::drawDetections(cv::Mat &frame, const std::vector<Detection> &detections)
{
    // 类别名称（根据你的 dataset.yaml 修改）
    std::vector<std::string> class_names = {"circle_true", "circle_false"};
    std::vector<cv::Scalar> colors = {
        cv::Scalar(0, 255, 0),   // green for circle_true
        cv::Scalar(0, 0, 255)    // red for circle_false
    };

    for (const auto& det : detections) {
        if(det.class_id == 1){
            continue;
        }
        // 绘制边界框
        cv::rectangle(frame, det.box, colors[det.class_id], 2);

        // 绘制标签
        std::string label = class_names[det.class_id] + ": " +
                            std::to_string(det.confidence).substr(0, 4);
        if (det.track_id >= 0) {
            label += " ID:" + std::to_string(det.track_id);
        }

        int baseline;
        cv::Size label_size = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline);

        cv::rectangle(frame,
                      cv::Point(det.box.x, det.box.y - label_size.height - 5),
                      cv::Point(det.box.x + label_size.width, det.box.y),
                      colors[det.class_id], -1);

        cv::putText(frame, label,
                    cv::Point(det.box.x, det.box.y - 5),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);

        if(det.class_id == 0){
            break;
        }
    }
}



// ==================== ByteTracker 实现 ====================

ByteTracker::ByteTracker(float iou_threshold, int max_missed_frames,
                         float conf_threshold_low, float conf_threshold_high)
    : m_iou_threshold(iou_threshold)
    , m_max_missed_frames(max_missed_frames)
    , m_conf_threshold_low(conf_threshold_low)
    , m_conf_threshold_high(conf_threshold_high)
    , m_next_id(0)
{
}

void ByteTracker::initKalmanFilter(TrackState& track, const cv::Rect& box)
{
    // 状态维度: x, y, w, h, vx, vy, vw, vh (8维)
    // 测量维度: x, y, w, h (4维)
    track.kf = cv::KalmanFilter(8, 4, 0);

    // 状态转移矩阵
    track.kf.transitionMatrix = (cv::Mat_<float>(8, 8) <<
                                     1, 0, 0, 0, 1, 0, 0, 0,
                                 0, 1, 0, 0, 0, 1, 0, 0,
                                 0, 0, 1, 0, 0, 0, 1, 0,
                                 0, 0, 0, 1, 0, 0, 0, 1,
                                 0, 0, 0, 0, 1, 0, 0, 0,
                                 0, 0, 0, 0, 0, 1, 0, 0,
                                 0, 0, 0, 0, 0, 0, 1, 0,
                                 0, 0, 0, 0, 0, 0, 0, 1);

    // 测量矩阵
    cv::setIdentity(track.kf.measurementMatrix);

    // 过程噪声协方差
    cv::setIdentity(track.kf.processNoiseCov, cv::Scalar::all(1e-2));

    // 测量噪声协方差
    cv::setIdentity(track.kf.measurementNoiseCov, cv::Scalar::all(1e-1));

    // 误差协方差矩阵
    cv::setIdentity(track.kf.errorCovPost, cv::Scalar::all(1));

    // 初始状态
    track.kf.statePost = (cv::Mat_<float>(8, 1) <<
                              box.x + box.width/2.0f,   // center x
                          box.y + box.height/2.0f,  // center y
                          box.width,                 // width
                          box.height,                // height
                          0, 0, 0, 0);              // 初始速度为0
}

cv::Rect ByteTracker::predictKalman(TrackState& track)
{
    cv::Mat prediction = track.kf.predict();

    float cx = prediction.at<float>(0);
    float cy = prediction.at<float>(1);
    float w = prediction.at<float>(2);
    float h = prediction.at<float>(3);

    int x = static_cast<int>(cx - w / 2);
    int y = static_cast<int>(cy - h / 2);
    int width = static_cast<int>(w);
    int height = static_cast<int>(h);

    // 边界限制
    x = std::max(0, x);
    y = std::max(0, y);
    width = std::max(1, width);
    height = std::max(1, height);

    return cv::Rect(x, y, width, height);
}

void ByteTracker::updateKalman(TrackState& track, const cv::Rect& box)
{
    cv::Mat measurement = (cv::Mat_<float>(4, 1) <<
                               box.x + box.width/2.0f,
                           box.y + box.height/2.0f,
                           box.width,
                           box.height);

    track.kf.correct(measurement);
}

double ByteTracker::calculateIoU(const cv::Rect& box1, const cv::Rect& box2)
{
    int x1 = std::max(box1.x, box2.x);
    int y1 = std::max(box1.y, box2.y);
    int x2 = std::min(box1.x + box1.width, box2.x + box2.width);
    int y2 = std::min(box1.y + box1.height, box2.y + box2.height);

    int intersection = std::max(0, x2 - x1) * std::max(0, y2 - y1);
    int area1 = box1.width * box1.height;
    int area2 = box2.width * box2.height;
    int union_area = area1 + area2 - intersection;

    return union_area > 0 ? static_cast<double>(intersection) / union_area : 0.0;
}

std::vector<Detection> ByteTracker::update(const std::vector<Detection>& detections)
{
    // 1. 分离高置信度和低置信度检测框
    std::vector<Detection> high_conf_dets;
    std::vector<Detection> low_conf_dets;

    for (const auto& det : detections) {
        if (det.confidence >= m_conf_threshold_high) {
            high_conf_dets.push_back(det);
        } else if (det.confidence >= m_conf_threshold_low) {
            low_conf_dets.push_back(det);
        }
    }

    // 2. 卡尔曼滤波预测
    for (auto& track : m_tracks) {
        if (track.active) {
            track.box = predictKalman(track);
            track.missed_frames++;
            track.age++;
        }
    }

    // 3. IoU 匹配（高置信度检测框 vs 现有轨迹）
    int num_tracks = static_cast<int>(m_tracks.size());
    int num_high = static_cast<int>(high_conf_dets.size());

    std::vector<bool> track_matched(num_tracks, false);
    std::vector<bool> det_matched(num_high, false);

    // 构建 IoU 矩阵并执行贪婪匹配
    std::vector<std::tuple<float, int, int>> iou_matches;  // iou, det_idx, track_idx

    for (int i = 0; i < num_high; i++) {
        for (int j = 0; j < num_tracks; j++) {
            if (!m_tracks[j].active || m_tracks[j].missed_frames > m_max_missed_frames) {
                continue;
            }
            double iou = calculateIoU(high_conf_dets[i].box, m_tracks[j].box);
            if (iou > m_iou_threshold) {
                iou_matches.push_back({static_cast<float>(iou), i, j});
            }
        }
    }

    // 按 IoU 降序排序
    std::sort(iou_matches.begin(), iou_matches.end(),
              [](const auto& a, const auto& b) { return std::get<0>(a) > std::get<0>(b); });

    // 执行匹配
    for (const auto& match : iou_matches) {
        int det_idx = std::get<1>(match);
        int track_idx = std::get<2>(match);

        if (!det_matched[det_idx] && !track_matched[track_idx]) {
            det_matched[det_idx] = true;
            track_matched[track_idx] = true;

            high_conf_dets[det_idx].track_id = m_tracks[track_idx].id;
            updateKalman(m_tracks[track_idx], high_conf_dets[det_idx].box);
            m_tracks[track_idx].box = high_conf_dets[det_idx].box;
            m_tracks[track_idx].missed_frames = 0;
            m_tracks[track_idx].confidence = high_conf_dets[det_idx].confidence;
        }
    }

    // 4. 处理未匹配的高置信度检测框（创建新轨迹）
    for (int i = 0; i < num_high; i++) {
        if (!det_matched[i]) {
            TrackState new_track;
            new_track.id = m_next_id++;
            new_track.box = high_conf_dets[i].box;
            new_track.missed_frames = 0;
            new_track.age = 0;
            new_track.active = true;
            new_track.confidence = high_conf_dets[i].confidence;
            initKalmanFilter(new_track, high_conf_dets[i].box);
            m_tracks.push_back(new_track);

            high_conf_dets[i].track_id = new_track.id;
        }
    }

    // ========== 关键修复：重新获取 track_matched 的大小 ==========
    // 扩展 track_matched 以匹配新的 m_tracks 大小
    int old_num_tracks = static_cast<int>(track_matched.size());
    int new_num_tracks = static_cast<int>(m_tracks.size());

    if (new_num_tracks > old_num_tracks) {
        track_matched.resize(new_num_tracks, false);
    }

    // 5. 低置信度检测框匹配
    int num_low = static_cast<int>(low_conf_dets.size());
    std::vector<bool> low_matched(num_low, false);

    // 找出未匹配的轨迹（现在 track_matched 大小与 m_tracks 一致了）
    std::vector<bool> track_unmatched(new_num_tracks, false);
    for (int j = 0; j < new_num_tracks; j++) {
        // 确保 j 在 track_matched 范围内
        bool is_matched = (j < old_num_tracks) ? track_matched[j] : false;
        track_unmatched[j] = (!is_matched && m_tracks[j].active);
    }

    for (int i = 0; i < num_low; i++) {
        int best_idx = -1;
        float best_iou = m_iou_threshold;

        for (int j = 0; j < num_tracks; j++) {
            if (!track_unmatched[j]) continue;

            double iou = calculateIoU(low_conf_dets[i].box, m_tracks[j].box);
            if (iou > best_iou) {
                best_iou = static_cast<float>(iou);
                best_idx = j;
            }
        }

        if (best_idx >= 0) {
            low_matched[i] = true;
            track_unmatched[best_idx] = false;
            low_conf_dets[i].track_id = m_tracks[best_idx].id;
            updateKalman(m_tracks[best_idx], low_conf_dets[i].box);
            m_tracks[best_idx].missed_frames = 0;
        }
    }

    // 6. 清理丢失的轨迹
    for (auto& track : m_tracks) {
        if (track.missed_frames > m_max_missed_frames) {
            track.active = false;
        }
    }

    m_tracks.erase(std::remove_if(m_tracks.begin(), m_tracks.end(),
                                  [](const TrackState& t) { return !t.active; }), m_tracks.end());

    // 7. 合并结果
    std::vector<Detection> results;
    for (const auto& det : high_conf_dets) {
        results.push_back(det);
    }
    for (const auto& det : low_conf_dets) {
        if (det.track_id >= 0) {
            results.push_back(det);
        }
    }

    return results;
}

void MouseTrack::test()
{
    YOLOv8Inference yolo(":/assets/best.onnx");

    // 查找游戏窗口
    HWND game_hwnd = FindWindow(NULL, TEXT("MapleStory Worlds-Artale (繁體中文版)"));
    if (game_hwnd == NULL) {
        qCritical() << "游戏窗口不存在！";
        return;
    }

    // 获取窗口位置和大小
    RECT game_rect;
    GetWindowRect(game_hwnd, &game_rect);
    int game_x = game_rect.left;
    int game_y = game_rect.top;
    int game_width = game_rect.right - game_rect.left;
    int game_height = game_rect.bottom - game_rect.top;
    int gamewindow_title_height = 32;

    qDebug() << "游戏窗口信息:";
    qDebug() << "  位置:(" << game_x << "," << game_y << ")";
    qDebug() << "  尺寸:" << game_width << "x" << game_height;

    // 获取主屏幕
    QScreen* m_pScreen = QGuiApplication::primaryScreen();

    int frame_count = 0;
    int target_fps = 2;  // 限制处理帧率
    auto last_time = std::chrono::steady_clock::now();

    while (true) {
        // 限制处理帧率
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_time).count();
        if (elapsed < 1000 / target_fps) {
            // 等待到下一帧的处理时间
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / target_fps - elapsed));
        }
        last_time = std::chrono::steady_clock::now();

        // 截取屏幕
        QPixmap cur_capture = m_pScreen->grabWindow((WId)GetDesktopWindow());
        cur_capture = cur_capture.copy(game_x, game_y, game_width, game_height);

        // 转换为 OpenCV Mat 格式
        QImage image = cur_capture.toImage();
        cv::Mat frame(image.height(), image.width(), CV_8UC4, image.bits(), image.bytesPerLine());
        cv::Mat bgr_frame;
        cv::cvtColor(frame, bgr_frame, cv::COLOR_BGRA2BGR);

        // 追踪检测
        auto detections = yolo.track(bgr_frame, 0.25);

        // 查找 circle_true (class_id == 0)
        bool found = false;
        Detection circle_true_det;

        for (const auto& det : detections) {
            if (det.class_id == 0) {
                circle_true_det = det;
                found = true;
                break;
            }
        }

        // 打印结果
        if (found) {
            // 计算中心点坐标
            int center_x = circle_true_det.box.x + circle_true_det.box.width / 2;
            int center_y = circle_true_det.box.y + circle_true_det.box.height / 2;
            qDebug() << "帧" << frame_count << "circle_true:"
                     << "ID:" << circle_true_det.track_id
                     << "置信度:" << circle_true_det.confidence
                     << "坐标:(" << circle_true_det.box.x << "," << circle_true_det.box.y
                     << ")-(" << circle_true_det.box.x + circle_true_det.box.width
                     << "," << circle_true_det.box.y + circle_true_det.box.height << ")";
            // 中心点
            MSKM::humanMove(center_x, center_y + gamewindow_title_height);
        }

        frame_count++;

        // 每100帧输出进度
        if (frame_count % 100 == 0) {
            qDebug() << "已处理" << frame_count << "帧";
        }
    }
}
