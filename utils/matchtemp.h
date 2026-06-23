#pragma once
#ifndef MATCHTEMP_H
#define MATCHTEMP_H

#include <QGuiApplication>
#include <QScreen>
#include <QUuid>
#include <QPixmap>
#include <QFile>
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"

class MatchTemp : public QObject
{
    Q_OBJECT
private:
    MatchTemp();
public:
    // 初始化
    MatchTemp(QString tmp);

    // 清理
    ~MatchTemp();

    // 截取当前界面
    void getScreenShot();

    // 当前截图
    QPixmap cur_capture;

    // 模板资源路径
    QString tmp_src;

    // 重新匹配次数
    int retry_times = 0;

    // 禁止重置鼠标
    bool mouse_reset = true;

    // 截图处理
    void cutPicture();

    // 将图片导出为文件（图片转换代替方案）
    void exportImg();

    // 执行模板匹配
    bool match(cv::Point &coor);

    // 模板匹配图片
    bool matchImage(cv::Point &coor,QString& path);

    // 执行形状匹配
    bool shapeMatch(cv::Point &coor);

    std::vector<cv::Point> extractArrowContour(const cv::Mat& templateImg);

    void quantizeImage(cv::Mat& image, const std::vector<cv::Vec3b>& targetColors);

    bool isTargetColor(const cv::Vec3b& color, const std::vector<cv::Vec3b>& targetColors);

    // 将Qpixmap转为OpenCV Mat;
    cv::Mat QPixmapToMat(const QPixmap& pixmap);

    // 设置截图位置
    void setMatchSize(int x, int y, int width, int height);

    // 灰度设置
    void setGrayscale();

    // 阈值设置
    void setThreshold(double thresh,double maxval);

    // 模板宽度
    int tmpWidth();

    // 模板高度
    int tmpHeight();
protected:
    // 模板匹配区域X
    int x_offset = 0;

    // 模板匹配区域Y
    int y_offset = 0;

    // 模板匹配区域宽度
    int x_length = 0;

    // 模板匹配区域高度
    int y_length = 0;

    // 模板宽度
    int tmp_width;

    // 模板高度
    int tmp_height;

    // 截图宽度
    int img_width;

    // 截图高度
    int img_height;

    // 是否裁剪
    bool iscut = false;

    // 是否灰度处理
    bool grayscale = false;

    bool threshold = false;

    double threshold_val;
    double threshold_max;
};

#endif // MATCHTEMP_H
