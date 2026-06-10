#include "mousetrack.h"
#include "utils/realtracker.h"
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
#include "utils/matchtemp.h"

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

    // // CPU 核心优化关键设置
    // int num_cores = std::thread::hardware_concurrency();  // 获取CPU核心数
    // session_options.SetIntraOpNumThreads(num_cores);      // 内部操作线程数
    // session_options.SetInterOpNumThreads(num_cores / 2);   // 间操作线程数
    // // 启用 CPU 优化
    // session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    // session_options.EnableCpuMemArena();      // 启用 CPU 内存池
    // session_options.EnableMemPattern();       // 启用内存模式优化
    // // 设置执行模式为并行
    // session_options.SetExecutionMode(ExecutionMode::ORT_PARALLEL);

    // 加载模型
    m_session = new Ort::Session(m_env, data, dataSize, session_options);

    // 获取输入/输出名称（保存为成员变量）
    Ort::AllocatorWithDefaultOptions allocator;

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
    // auto detections = detect(frame, conf_threshold);
    // return m_tracker.update(detections);

    // auto t1 = std::chrono::high_resolution_clock::now();
    auto detections = detect(frame, conf_threshold);
    // auto t2 = std::chrono::high_resolution_clock::now();

    std::vector<Detection> results = m_tracker.update(detections);

    // auto t3 = std::chrono::high_resolution_clock::now();

    // qDebug() << QString("detect: %1ms  track: %2ms  total: %3ms")
    //                 .arg(std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count())
    //                 .arg(std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count())
    //                 .arg(std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t1).count());

    return results;
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
    // 更新为8个类别名称（根据你的 dataset.yaml）
    std::vector<std::string> class_names = {
        "circle_true",
        "circle_false",
        "rect_true",
        "rect_false",
        "star_true",
        "star_false",
        "triangle_true",
        "triangle_false"
    };

    // 为8个类别定义不同的颜色
    std::vector<cv::Scalar> colors = {
        cv::Scalar(0, 255, 0),
        cv::Scalar(0, 0, 255),
        cv::Scalar(0, 255, 0),
        cv::Scalar(0, 0, 255),
        cv::Scalar(0, 255, 0),
        cv::Scalar(0, 0, 255),
        cv::Scalar(0, 255, 0),
        cv::Scalar(0, 0, 255)
    };

    for (const auto& det : detections) {
        // if(det.class_id == 1 || det.class_id == 3 || det.class_id == 5 || det.class_id == 7){
        //     continue;
        // }

        // 检查 class_id 是否有效
        // if (det.class_id < 0 || det.class_id >= colors.size()) {
        //     qDebug() << "警告：无效的 class_id:" << det.class_id
        //              << "，有效范围: 0-" << colors.size() - 1;
        //     continue;  // 跳过这个检测
        // }

        // 绘制边界框
        cv::rectangle(frame, det.box, colors[det.class_id], 1);

        // 绘制标签
        std::string label = class_names[det.class_id] + ": " +
                            std::to_string(det.confidence).substr(0, 4);
        label = "";
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

        // if(det.class_id == 0){
        //     break;
        // }
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

MousePredictor::MousePredictor()
    : m_kalman(6, 2, 0)  // 状态维度:6, 测量维度:2, 控制维度:0
    , m_has_initialized(false)
    , m_predicted_x(0)
    , m_predicted_y(0)
{
    // 状态转移矩阵 [x, y, vx, vy, ax, ay]
    m_kalman.transitionMatrix = (cv::Mat_<float>(6, 6) <<
                                     1, 0, 1, 0, 0.5f, 0,
                                 0, 1, 0, 1, 0, 0.5f,
                                 0, 0, 1, 0, 1, 0,
                                 0, 0, 0, 1, 0, 1,
                                 0, 0, 0, 0, 1, 0,
                                 0, 0, 0, 0, 0, 1);

    // 测量矩阵（只测量位置）
    m_kalman.measurementMatrix = (cv::Mat_<float>(2, 6) <<
                                      1, 0, 0, 0, 0, 0,
                                  0, 1, 0, 0, 0, 0);

    // 过程噪声协方差
    cv::setIdentity(m_kalman.processNoiseCov, cv::Scalar::all(1e-4));

    // 测量噪声协方差
    cv::setIdentity(m_kalman.measurementNoiseCov, cv::Scalar::all(1e-2));

    // 误差协方差矩阵
    cv::setIdentity(m_kalman.errorCovPost, cv::Scalar::all(1));

    m_last_time = std::chrono::high_resolution_clock::now();

    qDebug() << "MousePredictor 初始化完成";
}

MousePredictor::~MousePredictor()
{
    qDebug() << "MousePredictor 销毁";
}

void MousePredictor::update(float x, float y)
{
    auto now = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float>(now - m_last_time).count();
    m_last_time = now;

    if (!m_has_initialized) {
        // 初始化状态
        m_kalman.statePost = (cv::Mat_<float>(6, 1) << x, y, 0, 0, 0, 0);
        m_predicted_x = x;
        m_predicted_y = y;
        m_has_initialized = true;
        return;
    }

    // 根据时间更新状态转移矩阵
    float dt2 = dt * dt / 2.0f;
    m_kalman.transitionMatrix = (cv::Mat_<float>(6, 6) <<
                                     1, 0, dt, 0, dt2, 0,
                                 0, 1, 0, dt, 0, dt2,
                                 0, 0, 1, 0, dt, 0,
                                 0, 0, 0, 1, 0, dt,
                                 0, 0, 0, 0, 1, 0,
                                 0, 0, 0, 0, 0, 1);

    // 预测
    m_kalman.predict();

    // 更新测量
    cv::Mat measurement = (cv::Mat_<float>(2, 1) << x, y);
    m_kalman.correct(measurement);

    m_predicted_x = x;
    m_predicted_y = y;
}

void MousePredictor::predict(float dt_ms)
{
    if (!m_has_initialized) return;

    float dt = dt_ms / 1000.0f;
    float dt2 = dt * dt / 2.0f;

    // 根据时间更新状态转移矩阵
    m_kalman.transitionMatrix = (cv::Mat_<float>(6, 6) <<
                                     1, 0, dt, 0, dt2, 0,
                                 0, 1, 0, dt, 0, dt2,
                                 0, 0, 1, 0, dt, 0,
                                 0, 0, 0, 1, 0, dt,
                                 0, 0, 0, 0, 1, 0,
                                 0, 0, 0, 0, 0, 1);

    cv::Mat pred = m_kalman.predict();
    m_predicted_x = pred.at<float>(0);
    m_predicted_y = pred.at<float>(1);
}

float MousePredictor::getPredictedX() const
{
    return m_predicted_x;
}

float MousePredictor::getPredictedY() const
{
    return m_predicted_y;
}

bool MousePredictor::isInitialized() const
{
    return m_has_initialized;
}

void MousePredictor::reset()
{
    m_has_initialized = false;
    m_predicted_x = 0;
    m_predicted_y = 0;
    m_kalman.statePost = cv::Mat::zeros(6, 1, CV_32F);
    m_kalman.errorCovPost = cv::Mat::eye(6, 6, CV_32F);
    m_last_time = std::chrono::high_resolution_clock::now();

    qDebug() << "MousePredictor 已重置";
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
    int box_pos_x = 399;
    int box_pos_y = 218;
    int box_width = 500;
    int box_height = 335;

    // 鼠标允许的移动范围
    cv::Rect allow_rect = {game_x + box_pos_x, game_y + box_pos_y, box_width, box_height};

    int pos_x = game_x + box_pos_x;
    int pos_y = game_y + box_pos_y;
    int detect_width = box_width;
    int detect_height = box_height;

    qDebug() << "识别区域:" << pos_x << pos_y << detect_width << detect_height;

    // 需要检测的分类列表 (0, 2, 4, 6)
    const std::vector<int> target_classes = {0, 2, 4, 6};

    // 锁定的分类（首次检测到的分类）
    std::atomic<int> locked_class(-1);
    // 锁定的追踪ID（首次检测到的track_id）
    std::atomic<int> locked_track_id(-1);

    // 分类名称映射
    std::map<int, QString> class_names = {
        {0, "circle_true"},
        {2, "rect_true"},
        {4, "star_true"},
        {6, "triangle_true"}
    };

    qDebug() << "等待检测目标分类: circle_true(0), rect_true(2), star_true(4), triangle_true(6)";

    // 创建保存图片的目录
    QString save_dir = "detection_results";
    QDir dir;
    if (dir.exists(save_dir)) {
        // dir.removeRecursively();
        // qDebug() << "清空目录:" << save_dir;
    }else{
        dir.mkpath(save_dir);
        qDebug() << "创建目录:" << save_dir;
    }

    // 鼠标预测器
    MousePredictor mouse_predictor;

    // 线程控制
    std::atomic<bool> has_target(false);
    std::atomic<float> latest_target_x(0);
    std::atomic<float> latest_target_y(0);
    std::atomic<bool> target_updated(false);
    std::atomic<bool> running(true);

    // 平滑移动的当前值
    float current_x = 0;
    float current_y = 0;

    // 鼠标移动线程
    std::thread mouse_thread([&]() {
        auto last_move_time = std::chrono::high_resolution_clock::now();

        while (running) {
            if (!Gplayer.mousecheck_exists) {
                qDebug() << "滑鼠界面不存在，退出线程~";
                break;
            }

            auto now = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float, std::milli>(now - last_move_time).count();
            last_move_time = now;

            if (dt > 100) dt = 16;

            float move_x, move_y;

            if (target_updated) {
                mouse_predictor.update(latest_target_x, latest_target_y);
                move_x = latest_target_x;
                move_y = latest_target_y;
                target_updated = false;
                has_target = true;
                current_x = move_x;
                current_y = move_y;
            } else if (has_target) {
                mouse_predictor.predict(dt);
                move_x = mouse_predictor.getPredictedX();
                move_y = mouse_predictor.getPredictedY();

                float step = std::min(1.0f, dt / 16.0f);
                current_x = current_x * (1 - step) + move_x * step;
                current_y = current_y * (1 - step) + move_y * step;
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
                continue;
            }

            if (current_x >= 0 && current_x <= box_width &&
                current_y >= 0 && current_y <= box_height) {

                int target_x = allow_rect.x + static_cast<int>(current_x);
                int target_y = allow_rect.y + static_cast<int>(current_y);

                if (target_x >= allow_rect.x &&
                    target_x <= allow_rect.x + allow_rect.width &&
                    target_y >= allow_rect.y &&
                    target_y <= allow_rect.y + allow_rect.height) {

                    MSDriverPRO::mouseMoveTo(target_x, target_y);
                }
            }
        }
    });

    // 获取主屏幕
    QScreen* m_pScreen = QGuiApplication::primaryScreen();

    int frame_count = 0;

    qDebug() << "开始检测目标...";

    while (true) {
        // 截取屏幕
        QPixmap cur_capture = m_pScreen->grabWindow((WId)GetDesktopWindow());
        cur_capture = cur_capture.copy(pos_x, pos_y, detect_width, detect_height);

        // 转换为 OpenCV Mat 格式
        QImage image = cur_capture.toImage();
        cv::Mat frame(image.height(), image.width(), CV_8UC4, image.bits(), image.bytesPerLine());
        cv::Mat bgr_frame;
        cv::cvtColor(frame, bgr_frame, cv::COLOR_BGRA2BGR);

        // 追踪检测
        auto detections = yolo.track(bgr_frame, 0.25);

        // 查找目标
        bool found = false;
        Detection target_det;
        int current_class = -1;

        // 如果已经锁定了分类和track_id，优先匹配track_id
        if (locked_track_id >= 0) {
            for (const auto& det : detections) {
                if (det.track_id == locked_track_id) {
                    target_det = det;
                    found = true;
                    current_class = det.class_id;
                    break;
                }
            }
            // 如果track_id丢失，尝试匹配锁定的分类
            if (!found && locked_class >= 0) {
                for (const auto& det : detections) {
                    if (det.class_id == locked_class) {
                        target_det = det;
                        found = true;
                        current_class = locked_class;
                        locked_track_id = det.track_id;  // 更新track_id
                        qDebug() << "更新锁定目标分类:" << class_names[current_class] << "(ID:" << current_class << ") 追踪ID:" << locked_track_id;
                        break;
                    }
                }
            }
        } else if (locked_class >= 0) {
            // 未锁定track_id但已锁定分类，按分类查找
            for (const auto& det : detections) {
                if (det.class_id == locked_class) {
                    target_det = det;
                    found = true;
                    current_class = locked_class;
                    locked_track_id = det.track_id;  // 锁定track_id
                    qDebug() << "锁定目标分类:" << class_names[current_class] << "(ID:" << current_class << ") 追踪ID:" << locked_track_id;
                    break;
                }
            }
        } else {
            // 未锁定时，按优先级查找目标分类 (0, 2, 4, 6)
            // 优先级顺序: circle_true(0) > rect_true(2) > star_true(4) > triangle_true(6)
            for (int target_class : target_classes) {
                for (const auto& det : detections) {
                    if (det.class_id == target_class) {
                        target_det = det;
                        found = true;
                        current_class = target_class;
                        locked_class = target_class;  // 锁定分类
                        locked_track_id = det.track_id;  // 锁定追踪ID
                        qDebug() << "锁定目标分类:" << class_names[target_class] << "(ID:" << target_class << ")";
                        break;
                    }
                }
                if (found) break;
            }
        }

        // 处理检测结果
        if (found) {
            int center_x = target_det.box.x + target_det.box.width / 2;
            int center_y = target_det.box.y + target_det.box.height / 2;

            // 更新最新目标位置
            latest_target_x = static_cast<float>(center_x);
            latest_target_y = static_cast<float>(center_y);
            target_updated = true;
            has_target = true;

            // 可选：打印调试信息（每30帧打印一次，减少日志）
            if (frame_count % 10 == 0) {
                qDebug() << "帧" << frame_count
                         << "分类:" << class_names[current_class]
                         << "ID:" << locked_track_id
                         << "置信度:" << target_det.confidence
                         << "中心点:(" << center_x << "," << center_y << ")";
            }

            // 绘制检测结果
            yolo.drawDetections(bgr_frame, detections);

            // 在图像上绘制中心点
            // if (center_x < 0) center_x = 0;
            // if (center_x >= bgr_frame.cols) center_x = bgr_frame.cols - 1;
            // if (center_y < 0) center_y = 0;
            // if (center_y >= bgr_frame.rows) center_y = bgr_frame.rows - 1;

            cv::circle(bgr_frame, cv::Point(center_x, center_y), 5, cv::Scalar(0, 0, 255), -1);

            // 添加追踪信息
            QString track_text = QString("Locked: %1 TrackID:%2")
                                     .arg(class_names[current_class])
                                     .arg(locked_track_id);
            cv::putText(bgr_frame,
                        track_text.toStdString(),
                        cv::Point(10, 60),
                        cv::FONT_HERSHEY_SIMPLEX,
                        0.7,
                        cv::Scalar(0, 255, 0),
                        2);

            // 保存识别结果图片（可选，取消注释以启用）
            QString save_path = QString("%1/frame_%2.jpg").arg(save_dir).arg(frame_count, 6, 10, QChar('0'));
            cv::imwrite(save_path.toStdString(), bgr_frame);
        } else {
            // 未检测到目标
            if (locked_class >= 0 && frame_count % 100 == 0) {
                qDebug() << "未检测到锁定目标:" << class_names[locked_class];

                // 判断滑鼠界面是否还存在
                if(frame_count > 250){
                    MatchTemp tempmatch(":assets/templat/main_pannel/mouse_move");
                    cv::Point title_coor;
                    if (!tempmatch.match(title_coor)) {
                        Gplayer.mousecheck_exists = false;
                        qDebug() << "滑鼠验证界面已消失~";
                        break;
                    }
                }
            }
        }

        // 可选：绘制所有检测结果（用于调试）
        // yolo.drawDetections(bgr_frame, detections);

        // 可选：显示画面（用于调试）
        // cv::imshow("Tracking", bgr_frame);
        // if (cv::waitKey(1) == 27) break;  // ESC 退出

        frame_count++;

        // if (frame_count % 10 == 0) {
        //     qDebug() << "已处理" << frame_count << "帧";
        // }
    }

    running = false;
    mouse_thread.join();
    qDebug() << "test() 函数退出，锁定的分类:" << class_names[locked_class];
}

void MouseTrack::realTrackTest()
{
    YOLOv8Inference yolo(":/assets/best.onnx");
    QString save_dir = QCoreApplication::applicationDirPath() + "/detection_results";

    // 获取目录名和父目录
    QDir dir(save_dir);
    QString dirName = dir.dirName();
    QDir parentDir = dir;
    parentDir.cdUp();

    // 如果目录存在，先递归删除所有内容，然后删除目录本身
    if (dir.exists()) {
        // 方法1：使用 removeRecursively() 删除整个目录树（Qt 5+）
        if (!dir.removeRecursively()) {
            qWarning() << "删除目录失败:" << save_dir;
            return;
        }

        // 重新创建空目录
        if (!parentDir.mkdir(dirName)) {
            qWarning() << "创建目录失败:" << save_dir;
            return;
        }
    } else {
        // 目录不存在，直接创建
        if (!parentDir.mkdir(dirName)) {
            qWarning() << "创建目录失败:" << save_dir;
            return;
        }
    }

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
    int box_pos_x = 399;
    int box_pos_y = 218;
    int box_width = 500;
    int box_height = 335;

    // 鼠标允许的移动范围
    cv::Rect allow_rect = {game_x + box_pos_x, game_y + box_pos_y, box_width, box_height};

    int pos_x = game_x + box_pos_x;
    int pos_y = game_y + box_pos_y;
    int detect_width = box_width;
    int detect_height = box_height;

    // 检测区域
    cv::Rect detection_region(0, 0, detect_width, detect_height);

    qDebug() << "识别区域:" << pos_x << pos_y << detect_width << detect_height;

    // 真实目标检测器
    RealTargetDetector real_target_detector;

    // 鼠标预测器
    MousePredictor mouse_predictor;

    // 线程控制
    std::atomic<bool> has_target(false);
    std::atomic<float> latest_target_x(0);
    std::atomic<float> latest_target_y(0);
    std::atomic<bool> target_updated(false);
    std::atomic<bool> running(true);
    std::atomic<int> real_track_id(-1);  // 真实目标ID

    // 平滑移动的当前值
    float current_x = 0;
    float current_y = 0;

    // 鼠标移动线程
    std::thread mouse_thread([&]() {
        auto last_move_time = std::chrono::high_resolution_clock::now();

        while (running) {
            if (!Gplayer.mousecheck_exists) {
                qDebug() << "滑鼠界面不存在，退出线程~";
                break;
            }

            auto now = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float, std::milli>(now - last_move_time).count();
            last_move_time = now;

            if (dt > 100) dt = 16;

            float move_x, move_y;

            if (target_updated) {
                mouse_predictor.update(latest_target_x, latest_target_y);
                move_x = latest_target_x;
                move_y = latest_target_y;
                target_updated = false;
                has_target = true;
                current_x = move_x;
                current_y = move_y;
            } else if (has_target) {
                mouse_predictor.predict(dt);
                move_x = mouse_predictor.getPredictedX();
                move_y = mouse_predictor.getPredictedY();

                float step = std::min(1.0f, dt / 16.0f);
                current_x = current_x * (1 - step) + move_x * step;
                current_y = current_y * (1 - step) + move_y * step;
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
                continue;
            }

            if (current_x >= 0 && current_x <= box_width &&
                current_y >= 0 && current_y <= box_height) {

                int target_x = allow_rect.x + static_cast<int>(current_x);
                int target_y = allow_rect.y + static_cast<int>(current_y);

                if (target_x >= allow_rect.x &&
                    target_x <= allow_rect.x + allow_rect.width &&
                    target_y >= allow_rect.y &&
                    target_y <= allow_rect.y + allow_rect.height) {

                    MSDriverPRO::mouseMoveTo(target_x, target_y);
                }
            }
        }
    });

    // 获取主屏幕
    QScreen* m_pScreen = QGuiApplication::primaryScreen();

    int frame_count = 0;
    int locked_frame_count = 0;  // 锁定坐标帧计数
    cv::Point2i lock_position(-1, -1);  // 锁定坐标

    qDebug() << "开始真实目标检测...";

    while (true) {
        qDebug() << "第" << frame_count << "帧";
        // 截取屏幕
        QPixmap cur_capture = m_pScreen->grabWindow((WId)GetDesktopWindow());
        cur_capture = cur_capture.copy(pos_x, pos_y, detect_width, detect_height);

        // 转换为 OpenCV Mat 格式
        QImage image = cur_capture.toImage();
        cv::Mat frame(image.height(), image.width(), CV_8UC4, image.bits(), image.bytesPerLine());
        cv::Mat bgr_frame;
        cv::cvtColor(frame, bgr_frame, cv::COLOR_BGRA2BGR);

        // 追踪检测
        auto detections = yolo.track(bgr_frame, 0.25);

        // 绘制所有检测
        yolo.drawDetections(bgr_frame, detections);

        // 转换为跟踪结果格式
        std::vector<TrackInfo> track_results;
        for (const auto& det : detections) {
            if (det.track_id < 0) continue;

            TrackInfo info;
            info.track_id = det.track_id;
            info.center = cv::Point2f(
                static_cast<float>(det.box.x + det.box.width / 2),
                static_cast<float>(det.box.y + det.box.height / 2)
            );
            info.bbox = cv::Rect2d(det.box.x, det.box.y, det.box.width, det.box.height);
            info.score = det.confidence;
            track_results.push_back(info);
        }

        // 轮廓检测阶段 - 寻找初始锁定坐标
        if (locked_frame_count < 30) {
            cv::Mat gray, binary;
            cv::cvtColor(bgr_frame, gray, cv::COLOR_BGR2GRAY);
            cv::threshold(gray, binary, 180, 255, cv::THRESH_BINARY);

            cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));
            cv::erode(binary, binary, kernel, cv::Point(-1, -1), 3);
            cv::dilate(binary, binary, kernel, cv::Point(-1, -1), 3);

            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            double max_area = 0;
            cv::Point2i max_center(-1, -1);
            for (const auto& contour : contours) {
                cv::Rect rect = cv::boundingRect(contour);
                double area = rect.width * rect.height;
                qDebug() << "area:" << area;
                // if (area > 2000 && area > max_area) {
                if (area > 800 && area > max_area) {
                    float ratio = std::max(rect.width, rect.height) / static_cast<float>(std::min(rect.width, rect.height));
                    if (ratio > 1.5f) continue;

                    max_area = area;
                    max_center = cv::Point2i(rect.x + rect.width / 2, rect.y + rect.height / 2);
                }
            }

            if (max_center.x > 0 && max_center.y > 0) {
                if (max_area < 6000) {
                    lock_position = max_center;
                    real_target_detector.setLockCoordinate(lock_position);
                    locked_frame_count = 31;
                    qDebug() << "锁定坐标确定:" << lock_position.x << "," << lock_position.y;
                    // 绘制锁定坐标
                    if (lock_position.x > 0 && lock_position.y > 0) {
                        cv::circle(bgr_frame, lock_position, 6, cv::Scalar(255, 255, 0), -1);
                    }
                }
            }
            locked_frame_count++;
        }

        // 真实目标识别
        if (locked_frame_count >= 31) {
            int current_pos_x = static_cast<int>(latest_target_x);
            int current_pos_y = static_cast<int>(latest_target_y);
            cv::Point2i current_pos(current_pos_x, current_pos_y);

            int identified_id = real_target_detector.identifyRealTarget(
                track_results, current_pos, 1.0, detection_region);

            if (frame_count % 30 == 0) {
                qDebug() << "真实目标ID:" << identified_id
                         << "锁定转移:" << real_target_detector.isLockTransferred()
                         << "丢失计数:" << real_target_detector.getTargetLostCounter();
            }

            // 查找真实目标
            if (identified_id >= 0) {
                for (const auto& det : detections) {
                    if (det.track_id == identified_id) {
                        int center_x = det.box.x + det.box.width / 2;
                        int center_y = det.box.y + det.box.height / 2;

                        latest_target_x = static_cast<float>(center_x);
                        latest_target_y = static_cast<float>(center_y);
                        target_updated = true;
                        has_target = true;

                        // 绘制真实目标
                        // cv::rectangle(bgr_frame, det.box, cv::Scalar(0, 0, 255), 2);
                        cv::circle(bgr_frame, cv::Point(center_x, center_y), 5, cv::Scalar(0, 0, 255), -1);

                        QString track_text = QString("ID:%1").arg(identified_id);
                        cv::putText(bgr_frame,
                                    track_text.toStdString(),
                                    cv::Point(det.box.x, det.box.y - 10),
                                    cv::FONT_HERSHEY_SIMPLEX,
                                    0.6,
                                    cv::Scalar(0, 0, 255),
                                    2);

                        real_track_id = identified_id;
                        break;
                    }
                }
            }
        }


        // 检测滑鼠界面是否消失
        if (frame_count > 155) {
            MatchTemp tempmatch(":assets/templat/main_pannel/mouse_move");
            cv::Point title_coor;
            if (!tempmatch.match(title_coor)) {
                Gplayer.mousecheck_exists = false;
                qDebug() << "滑鼠验证界面已消失~";
                break;
            }
        }

        QString save_path = QString("%1/frame_%2.jpg").arg(save_dir).arg(frame_count, 6, 10, QChar('0'));
        cv::imwrite(save_path.toStdString(), bgr_frame);
        frame_count++;
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    running = false;
    mouse_thread.join();
    qDebug() << "realTrackTest() 函数退出，真实目标ID:" << real_track_id.load();
}
