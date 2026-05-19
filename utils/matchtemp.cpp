#include "matchtemp.h"
#include "process/process.h"
#include "global/globalplayer.h"

MatchTemp::MatchTemp(QString tmp)
{
    tmp_src = tmp;
}

MatchTemp::~MatchTemp()
{

}

void MatchTemp::getScreenShot()
{
    // 调试本地图片
    // QPixmap pixmap(QCoreApplication::applicationDirPath() + "/temp/game.png");
    // cur_capture = pixmap;

    QScreen* m_pScreen = QGuiApplication::primaryScreen();
    cur_capture        = m_pScreen->grabWindow((WId)GetDesktopWindow());
    cur_capture        = cur_capture.copy(
        Gplayer.game_window.x,
        Gplayer.game_window.y,
        Gplayer.game_window.width,
        Gplayer.game_window.height
    );

    img_width = cur_capture.width();
    img_height = cur_capture.height();
}

void MatchTemp::cutPicture()
{
    cur_capture = cur_capture.copy(x_offset,y_offset,x_length,y_length);
}

cv::Mat MatchTemp::QPixmapToMat(const QPixmap& pixmap) {

    QImage image = pixmap.toImage();

    if (image.format() == QImage::Format_ARGB32_Premultiplied) {
        image = image.convertToFormat(QImage::Format_ARGB32);
    }

    // 将 QImage 转换为 cv::Mat
    cv::Mat mat;
    if (image.format() == QImage::Format_RGB32 || image.format() == QImage::Format_ARGB32)
    {
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, const_cast<uchar*>(image.bits()), static_cast<size_t>(image.bytesPerLine())).clone();
    }
    else if (image.format() == QImage::Format_RGB888)
    {
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, const_cast<uchar*>(image.bits()), static_cast<size_t>(image.bytesPerLine())).clone();
        cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR); // OpenCV 使用 BGR 格式，需进行颜色通道转换
    }
    else
    {
        qDebug() << "Unsupported image format...........";
        mat = cv::Mat();
    }

    return mat;
}

void MatchTemp::setMatchSize(int x, int y, int width, int height)
{
    x_offset = x;
    y_offset = y;
    x_length = width;
    y_length = height;
    iscut    = true;
}

void MatchTemp::setGrayscale()
{
    grayscale = true;
}

void MatchTemp::setThreshold(double thresh, double maxval)
{
    threshold = true;
    threshold_val = thresh;
    threshold_max = maxval;
}

bool MatchTemp::match(cv::Point &coor)
{
    // 截图
    getScreenShot();

    if(img_width != 1296 || img_height != 759){
        qDebug() << "窗口大小不符合要求";
        return false;
    }

    begin:

    // 裁剪截图
    if(iscut){
        cutPicture();
    }

    coor.x = 0;
    coor.y = 0;

    // 载入模板图片
    QPixmap tmp_file(tmp_src);
    // qDebug() << "当前匹配模板：" << tmp_src;

    // 模板
    cv::Mat cv_templat = QPixmapToMat(tmp_file);
    if(cv_templat.empty()){
        qDebug() << "模板为空";
        return false;
    }

    tmp_width = cv_templat.cols;
    tmp_height = cv_templat.rows;
    // qDebug() << "模板大小:" << tmp_width << "," << tmp_height;

    // 截图
    cv::Mat cv_capture = QPixmapToMat(cur_capture);
    if(cv_capture.empty()){
        qDebug() << "截图为空";
        return false;
    }

    // 设置灰度
    if(grayscale)
    {
        cv::cvtColor(cv_templat, cv_templat, cv::COLOR_BGR2GRAY);
        cv::cvtColor(cv_capture, cv_capture, cv::COLOR_BGR2GRAY);
    }

    // 设置阈值
    if(threshold)
    {
        cv::threshold(cv_templat, cv_templat, threshold_val, threshold_max, cv::THRESH_BINARY);
        cv::threshold(cv_capture, cv_capture, threshold_val, threshold_max, cv::THRESH_BINARY);
    }

    // 查看处理后的图片
    // QString path = QCoreApplication::applicationDirPath() + "/temp/opencv.png";
    // cv::imwrite(path.toStdString(), cv_capture);

    // 模板匹配
    cv::Mat cv_result;
    cv::matchTemplate(cv_capture, cv_templat, cv_result, cv::TM_SQDIFF);

    // // 获取匹配的坐标点
    double minVal;
    cv::Point matchLoc;
    cv::minMaxLoc(cv_result, &minVal, NULL, &matchLoc);
    // qDebug() <<("当前匹配系数：" + QString::number(minVal) + " 坐标:" + QString::number(matchLoc.x + x_offset) + "," + QString::number(matchLoc.y + y_offset));


    if(minVal > 1000000)
    {
        if(retry_times > 0){
            retry_times--;
            getScreenShot();
            goto begin;
        }
        // 匹配失败
        // qDebug() <<("匹配系数太低，匹配失败");
        return false;
    }

    if(iscut){
        coor.x = matchLoc.x + x_offset;
        coor.y = matchLoc.y + y_offset;
    }else{
        coor.x = matchLoc.x;
        coor.y = matchLoc.y;
    }


    qDebug() <<("当前匹配位置：" + QString::number(coor.x) + "," + QString::number(coor.y));
    return true;
}

bool MatchTemp::shapeMatch(cv::Point &coor)
{
    QString path = QCoreApplication::applicationDirPath() + "/temp/opencv.png";

    // 截图
    getScreenShot();

    if(false){
        // 输出图片
        // exportImg();
    }else{
        // 直接裁剪
        // cutPicture();
    }

    coor.x = 0;
    coor.y = 0;

    // 载入模板图片
    QPixmap tmp_file(tmp_src);

    cv::Mat cv_templat = QPixmapToMat(tmp_file);

    // 载入截图
    cv::Mat cv_capture = QPixmapToMat(cur_capture);

    // 定义目标颜色
    std::vector<cv::Vec3b> targetColors = {
        cv::Vec3b(154, 204, 229), // 注意 OpenCV 的 BGR 顺序
        cv::Vec3b(126, 159, 189),
        cv::Vec3b(86, 110, 133),
        cv::Vec3b(57, 72, 87)
    };

    // 量化图像
    quantizeImage(cv_capture, targetColors);
    cv::imwrite(path.toStdString(), cv_capture);
    return false;


    cv::Mat targetImg = cv_capture;
    cv::Mat templateImg = cv_templat;
    cv::Mat gray, edges;
    cv::cvtColor(targetImg, gray, cv::COLOR_BGR2GRAY);

    cv::threshold(gray, gray, threshold_val, threshold_max, cv::THRESH_BINARY);
    // cv::imwrite(path.toStdString(), gray);
    // return false;

    cv::Canny(gray, edges, 1, 2);

    cv::imwrite(path.toStdString(), edges);
    return false;

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edges, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    std::vector<cv::Point> arrowContour = extractArrowContour(templateImg);

    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);
        // 轮廓87
        if (area > 85 && area < 90) {
            qDebug() << "area:" << area;
            double similarity = cv::matchShapes(contour, arrowContour, cv::CONTOURS_MATCH_I1, 0);
            qDebug() << "similarity:" << similarity;
            // if (similarity < 0.1) {
                cv::drawContours(targetImg, std::vector<std::vector<cv::Point>>{contour}, -1, cv::Scalar(0, 255, 0), 2);
            // }
        }
    }


    cv::imwrite(path.toStdString(), targetImg);

    return true;
}

std::vector<cv::Point> MatchTemp::extractArrowContour(const cv::Mat& templateImg) {
    cv::Mat gray, edges;
    cv::cvtColor(templateImg, gray, cv::COLOR_BGR2GRAY);
    cv::Canny(gray, edges, 100, 200);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edges, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    if (!contours.empty()) {
        // 假设箭头模板只有一个主要轮廓
        return contours.at(0);
    } else {
        return std::vector<cv::Point>(); // 返回空轮廓
    }
}

bool MatchTemp::isTargetColor(const cv::Vec3b& color, const std::vector<cv::Vec3b>& targetColors) {
    for (const auto& targetColor : targetColors) {
        double distance = cv::norm(color - targetColor);
        if (distance < 30.0) {
            return true;
        }
    }
    return false;
}

void MatchTemp::quantizeImage(cv::Mat& image, const std::vector<cv::Vec3b>& targetColors) {
    cv::Mat output = image.clone();

    for (int y = 0; y < image.rows; ++y) {
        for (int x = 0; x < image.cols; ++x) {
            cv::Vec3b& pixel = output.at<cv::Vec3b>(y, x);
            if (!isTargetColor(pixel, targetColors)) {
                pixel = cv::Vec3b(0, 0, 0); // 替换为黑色
            }
        }
    }

    image = output;
}

int MatchTemp::tmpWidth()
{
    return tmp_width;
}

int MatchTemp::tmpHeight()
{
    return tmp_height;
}
