#include "mousetrack.h"
#include <QResource>
#include <opencv2/dnn.hpp>


YOLOv8Inference::YOLOv8Inference(const QString &model_path)
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

std::vector<Detection> YOLOv8Inference::detect(const cv::Mat &frame, float conf_threshold, float iou_threshold)
{
    // 1. 预处理
    cv::Mat resized_frame;
    cv::resize(frame, resized_frame, cv::Size(m_input_shape[3], m_input_shape[2]));

    cv::Mat blob;
    cv::dnn::blobFromImage(resized_frame, blob, 1.0 / 255.0,
                           cv::Size(m_input_shape[3], m_input_shape[2]),
                           cv::Scalar(), true, false);

    // 2. 准备输入数据
    std::vector<float> input_data(blob.begin<float>(), blob.end<float>());

    std::vector<int64_t> input_shape = {1, 3, m_input_shape[2], m_input_shape[3]};
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info, input_data.data(), input_data.size(),
        input_shape.data(), input_shape.size()
        );

    // 3. 运行推理
    auto start_time = std::chrono::high_resolution_clock::now();

    const char* input_name_ptr = m_input_name;   // char* 直接赋值给 const char*
    const char* output_name_ptr = m_output_name; // char* 直接赋值给 const char*

    // 在 m_session->Run 之前添加
    qDebug() << "===";
    qDebug() << "=== detect 函数中 ===";
    qDebug() << "输入节点名称:" << m_input_name;
    qDebug() << "输出节点名称:" << m_output_name;
    qDebug() << "输入形状: 1 3" << m_input_shape[2] << m_input_shape[3];
    qDebug() << "输入数据大小:" << input_data.size();

    try {
        auto output_tensors = m_session->Run(
            Ort::RunOptions{nullptr},
            &input_name_ptr, &input_tensor, 1,
            &output_name_ptr, 1
            );
        qDebug() << "推理成功!";
    } catch (const Ort::Exception& e) {
        // qCritical() << "ORT 错误码:" << e.code();
        qCritical() << "ORT 错误信息:" << e.what();
        throw;
    }

    auto output_tensors = m_session->Run(
        Ort::RunOptions{nullptr},
        &input_name_ptr, &input_tensor, 1,
        &output_name_ptr, 1
        );

    auto end_time = std::chrono::high_resolution_clock::now();
    auto inference_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "推理时间: " << inference_time << " ms" << std::endl;

    // 4. 后处理
    float* output_data = output_tensors[0].GetTensorMutableData<float>();
    std::vector<int64_t> output_shape = output_tensors[0].GetTensorTypeAndShapeInfo().GetShape();

    // YOLOv8 输出格式: [batch, 84, num_detections]
    int num_detections = output_shape[2];  // 通常是 8400
    int num_classes = output_shape[1] - 4; // 84 - 4 = 80

    std::vector<Detection> detections;

    for (int i = 0; i < num_detections; ++i) {
        float* detection_ptr = output_data + i * (num_classes + 4);

        // 获取边界框坐标 (中心x, 中心y, 宽度, 高度) - 归一化坐标
        float cx = detection_ptr[0];
        float cy = detection_ptr[1];
        float w = detection_ptr[2];
        float h = detection_ptr[3];

        // 确保宽度和高度为正数
        if (w <= 0 || h <= 0) {
            continue;
        }

        // 转换为绝对坐标
        int x = static_cast<int>((cx - w / 2) * frame.cols);
        int y = static_cast<int>((cy - h / 2) * frame.rows);
        int width = static_cast<int>(w * frame.cols);
        int height = static_cast<int>(h * frame.rows);

        // 确保宽度和高度为正数
        if (width <= 0 || height <= 0) {
            continue;
        }

        // 边界修正（防止越界）
        x = std::max(0, x);
        y = std::max(0, y);

        // 确保框不超出图像边界
        if (x + width > frame.cols) {
            width = frame.cols - x;
        }
        if (y + height > frame.rows) {
            height = frame.rows - y;
        }

        // 再次检查宽度和高度有效性
        if (width <= 0 || height <= 0) {
            continue;
        }

        // 获取类别置信度
        float* class_scores = detection_ptr + 4;

        // 找到最大置信度的类别
        int class_id = 0;
        float max_score = class_scores[0];
        for (int j = 1; j < num_classes; ++j) {
            if (class_scores[j] > max_score) {
                max_score = class_scores[j];
                class_id = j;
            }
        }

        // 应用置信度阈值
        if (max_score > conf_threshold) {
            Detection det;
            det.class_id = class_id;
            det.confidence = max_score;
            det.box = cv::Rect(x, y, width, height);
            detections.push_back(det);
        }
    }

    // 5. NMS 过滤重叠框
    std::vector<Detection> filtered_detections;
    std::vector<int> indices;
    std::vector<cv::Rect> boxes;
    std::vector<float> scores;

    for (const auto& det : detections) {
        boxes.push_back(det.box);
        scores.push_back(det.confidence);
    }

    cv::dnn::NMSBoxes(boxes, scores, conf_threshold, iou_threshold, indices);

    for (int idx : indices) {
        filtered_detections.push_back(detections[idx]);
    }

    return filtered_detections;
}

std::vector<Detection> YOLOv8Inference::track(const cv::Mat &frame, float conf_threshold)
{
    auto detections = detect(frame, conf_threshold);

    // 简单的跟踪：为每个检测分配 ID
    // 实际应用中应使用卡尔曼滤波等更复杂的方法
    static int next_id = 0;
    static std::vector<std::pair<int, cv::Rect>> last_boxes;

    for (auto& det : detections) {
        bool matched = false;

        // 简单匹配：找到最近的框
        for (auto& last : last_boxes) {
            double iou = calculateIoU(det.box, last.second);
            if (iou > 0.5) {
                det.track_id = last.first;
                matched = true;
                break;
            }
        }

        if (!matched) {
            det.track_id = next_id++;
        }
    }

    // 更新上一帧的框
    last_boxes.clear();
    for (const auto& det : detections) {
        last_boxes.push_back({det.track_id, det.box});
    }

    return detections;
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

        // 输出坐标到控制台
        std::cout << "检测到 " << class_names[det.class_id]
                  << " ID:" << det.track_id
                  << " 置信度:" << det.confidence
                  << " 坐标:(" << det.box.x << "," << det.box.y
                  << "," << det.box.x + det.box.width << "," << det.box.y + det.box.height << ")"
                  << std::endl;
    }
}


void MouseTrack::test()
{
    YOLOv8Inference yolo(":/assets/best.onnx");

    // 2. 打开视频
    std::string video_path = "D:\\CodeRepos\\Pythons\\mousemove\\2.mp4";  // 修改为你的视频路径
    cv::VideoCapture cap(video_path);

    if (!cap.isOpened()) {
        std::cerr << "无法打开视频文件: " << video_path;
        return;
    }

    // 获取视频信息
    double fps = cap.get(cv::CAP_PROP_FPS);
    int frame_width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int frame_height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    int total_frames = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));

    qDebug() << "视频信息:";
    qDebug() << "  分辨率: " << frame_width << "x" << frame_height;
    qDebug() << "  帧率: " << fps << " fps";
    qDebug() << "  总帧数: " << total_frames;

    // 3. 处理视频
    cv::Mat frame;
    int frame_count = 0;
    auto total_start_time = std::chrono::high_resolution_clock::now();

    while (cap.read(frame)) {
        frame_count++;

        // 检测（每帧都检测）
        auto detections = yolo.detect(frame, 0.25, 0.45);

        // 绘制结果
        yolo.drawDetections(frame, detections);

        // 显示 FPS
        auto current_time = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - total_start_time).count();
        if (elapsed > 0) {
            double current_fps = frame_count * 1000.0 / elapsed;
            cv::putText(frame, "FPS: " + std::to_string(static_cast<int>(current_fps)),
                        cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
        }

        // 显示帧数
        cv::putText(frame, "Frame: " + std::to_string(frame_count) + "/" + std::to_string(total_frames),
                    cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);

        // 每100帧输出一次进度
        if (frame_count % 100 == 0) {
            double progress = frame_count * 100.0 / total_frames;
            qDebug() << "进度: " << progress << "% (" << frame_count << "/" << total_frames << ")" ;
        }
    }

    // 释放资源
    cap.release();
    // cv::destroyAllWindows();

    auto total_end_time = std::chrono::high_resolution_clock::now();
    auto total_elapsed = std::chrono::duration_cast<std::chrono::seconds>(total_end_time - total_start_time).count();

    qDebug() << "处理完成!" ;
    qDebug() << "总帧数: " << frame_count ;
    qDebug() << "总耗时: " << total_elapsed << " 秒" ;
    qDebug() << "平均 FPS: " << frame_count / total_elapsed ;

}
