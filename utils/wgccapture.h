#ifndef WGCCAPTURE_H
#define WGCCAPTURE_H

#include <opencv2/core.hpp>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

class WgcWindowCapturer {
public:
    WgcWindowCapturer();
    ~WgcWindowCapturer();

    bool init(HWND hwnd, int cropX, int cropY, int cropW, int cropH);
    bool captureBgr(cv::Mat& outBgr);
    void shutdown();

    bool isReady() const { return ready_; }

private:
    struct Impl;
    Impl* impl_ = nullptr;
    bool ready_ = false;
};

#endif // WGCCAPTURE_H
