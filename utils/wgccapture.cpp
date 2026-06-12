#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <opencv2/imgproc.hpp>
#include "wgccapture.h"

#include <windows.foundation.h>
#include <windows.graphics.h>
#include <windows.graphics.capture.h>
#include <windows.graphics.directx.h>
#include <windows.graphics.directx.direct3d11.h>
#include <windows.graphics.capture.interop.h>
#include <windows.graphics.directx.direct3d11.interop.h>

#include <d3d11.h>
#include <dxgi1_2.h>
#include <roapi.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>

#include <QDebug>

#include <cstring>

using Microsoft::WRL::ComPtr;
using Microsoft::WRL::Wrappers::HStringReference;
using namespace ABI::Windows::Graphics::Capture;
using namespace ABI::Windows::Graphics::DirectX;
using namespace ABI::Windows::Graphics::DirectX::Direct3D11;
using ABI::Windows::Graphics::SizeInt32;
using Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess;

struct WgcWindowCapturer::Impl {
    ComPtr<ID3D11Device> d3d_device;
    ComPtr<ID3D11DeviceContext> d3d_context;
    ComPtr<ID3D11Texture2D> staging_texture;
    ComPtr<IDirect3DDevice> winrt_device;
    ComPtr<IGraphicsCaptureItem> item;
    ComPtr<IDirect3D11CaptureFramePool> frame_pool;
    ComPtr<IGraphicsCaptureSession> session;
    int crop_x = 0;
    int crop_y = 0;
    int crop_w = 0;
    int crop_h = 0;
};

namespace {

bool EnsureWinRtRuntime()
{
    static bool initialized = false;
    if (initialized) {
        return true;
    }

    const HRESULT hr = RoInitialize(RO_INIT_SINGLETHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        qCritical() << "RoInitialize 失败, hr =" << Qt::hex << hr;
        return false;
    }

    initialized = true;
    return true;
}

bool IsCaptureSupported()
{
    ComPtr<IGraphicsCaptureSessionStatics> statics;
    const HRESULT hr = RoGetActivationFactory(
        HStringReference(RuntimeClass_Windows_Graphics_Capture_GraphicsCaptureSession).Get(),
        IID_PPV_ARGS(statics.GetAddressOf()));
    if (FAILED(hr)) {
        qCritical() << "获取 GraphicsCaptureSession 工厂失败, hr =" << Qt::hex << hr;
        return false;
    }

    boolean supported = FALSE;
    if (FAILED(statics->IsSupported(&supported))) {
        return false;
    }
    return supported == TRUE;
}

ComPtr<ID3D11Device> CreateD3D11Device()
{
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;
    const D3D_FEATURE_LEVEL feature_levels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

    const HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        feature_levels,
        ARRAYSIZE(feature_levels),
        D3D11_SDK_VERSION,
        device.GetAddressOf(),
        &feature_level,
        context.GetAddressOf());
    if (FAILED(hr)) {
        qCritical() << "D3D11CreateDevice 失败, hr =" << Qt::hex << hr;
        return nullptr;
    }
    return device;
}

ComPtr<IDirect3DDevice> CreateWinRtDevice(ID3D11Device* device)
{
    ComPtr<IDXGIDevice> dxgi_device;
    if (FAILED(device->QueryInterface(IID_PPV_ARGS(dxgi_device.GetAddressOf())))) {
        return nullptr;
    }

    ComPtr<IInspectable> inspectable;
    if (FAILED(CreateDirect3D11DeviceFromDXGIDevice(dxgi_device.Get(), inspectable.GetAddressOf()))) {
        return nullptr;
    }

    ComPtr<IDirect3DDevice> winrt_device;
    if (FAILED(inspectable.As(&winrt_device))) {
        return nullptr;
    }
    return winrt_device;
}

ComPtr<IGraphicsCaptureItem> CreateCaptureItemForWindow(HWND hwnd)
{
    ComPtr<IGraphicsCaptureItemInterop> interop;
    HRESULT hr = RoGetActivationFactory(
        HStringReference(RuntimeClass_Windows_Graphics_Capture_GraphicsCaptureItem).Get(),
        IID_PPV_ARGS(interop.GetAddressOf()));
    if (FAILED(hr)) {
        qCritical() << "获取 GraphicsCaptureItem 工厂失败, hr =" << Qt::hex << hr;
        return nullptr;
    }

    ComPtr<IGraphicsCaptureItem> item;
    hr = interop->CreateForWindow(hwnd, IID_PPV_ARGS(item.GetAddressOf()));
    if (FAILED(hr)) {
        qCritical() << "CreateForWindow 失败, hr =" << Qt::hex << hr;
        return nullptr;
    }
    return item;
}

ComPtr<ID3D11Texture2D> GetFrameTexture(IDirect3D11CaptureFrame* frame)
{
    ComPtr<IDirect3DSurface> surface;
    if (FAILED(frame->get_Surface(surface.GetAddressOf()))) {
        return nullptr;
    }

    ComPtr<IDirect3DDxgiInterfaceAccess> access;
    if (FAILED(surface->QueryInterface(IID_PPV_ARGS(access.GetAddressOf())))) {
        return nullptr;
    }

    ComPtr<ID3D11Texture2D> texture;
    if (FAILED(access->GetInterface(IID_PPV_ARGS(texture.GetAddressOf())))) {
        return nullptr;
    }
    return texture;
}

bool CreateStagingTexture(ID3D11Device* device, int width, int height, ComPtr<ID3D11Texture2D>& out)
{
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = static_cast<UINT>(width);
    desc.Height = static_cast<UINT>(height);
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    return SUCCEEDED(device->CreateTexture2D(&desc, nullptr, out.GetAddressOf()));
}

void CloseInspectable(IUnknown* unknown)
{
    if (!unknown) {
        return;
    }

    ComPtr<ABI::Windows::Foundation::IClosable> closable;
    if (SUCCEEDED(unknown->QueryInterface(IID_PPV_ARGS(closable.GetAddressOf())))) {
        closable->Close();
    }
}

} // namespace

WgcWindowCapturer::WgcWindowCapturer() = default;

WgcWindowCapturer::~WgcWindowCapturer()
{
    shutdown();
}

bool WgcWindowCapturer::init(HWND hwnd, int cropX, int cropY, int cropW, int cropH)
{
    shutdown();

    if (!hwnd || cropW <= 0 || cropH <= 0) {
        qCritical() << "WGC 参数无效";
        return false;
    }
    if (!EnsureWinRtRuntime()) {
        return false;
    }
    if (!IsCaptureSupported()) {
        qCritical() << "当前系统不支持 Windows Graphics Capture";
        return false;
    }

    impl_ = new Impl();
    impl_->crop_x = cropX;
    impl_->crop_y = cropY;
    impl_->crop_w = cropW;
    impl_->crop_h = cropH;

    impl_->d3d_device = CreateD3D11Device();
    if (!impl_->d3d_device) {
        shutdown();
        return false;
    }
    impl_->d3d_device->GetImmediateContext(impl_->d3d_context.GetAddressOf());

    impl_->winrt_device = CreateWinRtDevice(impl_->d3d_device.Get());
    if (!impl_->winrt_device) {
        qCritical() << "创建 WinRT D3D 设备失败";
        shutdown();
        return false;
    }

    impl_->item = CreateCaptureItemForWindow(hwnd);
    if (!impl_->item) {
        shutdown();
        return false;
    }

    SizeInt32 item_size{};
    if (FAILED(impl_->item->get_Size(&item_size))) {
        qCritical() << "读取 WGC 窗口尺寸失败";
        shutdown();
        return false;
    }
    if (item_size.Width <= cropX + cropW || item_size.Height <= cropY + cropH) {
        qCritical() << "WGC 窗口尺寸不足, 窗口:"
                    << item_size.Width << "x" << item_size.Height
                    << " ROI:" << cropW << "x" << cropH
                    << " offset:" << cropX << cropY;
        shutdown();
        return false;
    }

    if (!CreateStagingTexture(impl_->d3d_device.Get(), cropW, cropH, impl_->staging_texture)) {
        qCritical() << "创建 WGC staging texture 失败";
        shutdown();
        return false;
    }

    ComPtr<IDirect3D11CaptureFramePoolStatics> pool_statics;
    HRESULT hr = RoGetActivationFactory(
        HStringReference(RuntimeClass_Windows_Graphics_Capture_Direct3D11CaptureFramePool).Get(),
        IID_PPV_ARGS(pool_statics.GetAddressOf()));
    if (FAILED(hr)) {
        qCritical() << "获取 Direct3D11CaptureFramePool 工厂失败, hr =" << Qt::hex << hr;
        shutdown();
        return false;
    }

    hr = pool_statics->Create(
        impl_->winrt_device.Get(),
        DirectXPixelFormat_B8G8R8A8UIntNormalized,
        2,
        item_size,
        impl_->frame_pool.GetAddressOf());
    if (FAILED(hr)) {
        qCritical() << "创建 WGC frame pool 失败, hr =" << Qt::hex << hr;
        shutdown();
        return false;
    }

    hr = impl_->frame_pool->CreateCaptureSession(impl_->item.Get(), impl_->session.GetAddressOf());
    if (FAILED(hr)) {
        qCritical() << "创建 WGC capture session 失败, hr =" << Qt::hex << hr;
        shutdown();
        return false;
    }

    ComPtr<IGraphicsCaptureSession2> session2;
    if (SUCCEEDED(impl_->session.As(&session2))) {
        session2->put_IsCursorCaptureEnabled(FALSE);
    }

    hr = impl_->session->StartCapture();
    if (FAILED(hr)) {
        qCritical() << "StartCapture 失败, hr =" << Qt::hex << hr;
        shutdown();
        return false;
    }

    ready_ = true;
    qDebug() << "WGC 初始化成功, 窗口尺寸:"
             << item_size.Width << "x" << item_size.Height
             << " ROI:" << cropW << "x" << cropH
             << " offset:" << cropX << cropY;
    return true;
}

bool WgcWindowCapturer::captureBgr(cv::Mat& outBgr)
{
    if (!ready_ || !impl_) {
        return false;
    }

    ComPtr<IDirect3D11CaptureFrame> frame;
    HRESULT hr = impl_->frame_pool->TryGetNextFrame(frame.GetAddressOf());
    if (FAILED(hr) || !frame) {
        return false;
    }

    (void)outBgr;

    // 截图效率测试：以下 CPU 读回与格式转换暂不需要
    const ComPtr<ID3D11Texture2D> src_texture = GetFrameTexture(frame.Get());
    if (!src_texture) {
        return false;
    }

    D3D11_BOX src_box{};
    src_box.left = static_cast<UINT>(impl_->crop_x);
    src_box.top = static_cast<UINT>(impl_->crop_y);
    src_box.right = static_cast<UINT>(impl_->crop_x + impl_->crop_w);
    src_box.bottom = static_cast<UINT>(impl_->crop_y + impl_->crop_h);
    src_box.front = 0;
    src_box.back = 1;

    impl_->d3d_context->CopySubresourceRegion(
        impl_->staging_texture.Get(),
        0,
        0,
        0,
        0,
        src_texture.Get(),
        0,
        &src_box);

    D3D11_MAPPED_SUBRESOURCE mapped{};
    hr = impl_->d3d_context->Map(
        impl_->staging_texture.Get(),
        0,
        D3D11_MAP_READ,
        0,
        &mapped);
    if (FAILED(hr)) {
        return false;
    }

    cv::Mat bgra(impl_->crop_h, impl_->crop_w, CV_8UC4);
    const int row_bytes = impl_->crop_w * 4;
    for (int y = 0; y < impl_->crop_h; ++y) {
        std::memcpy(
            bgra.ptr(y),
            static_cast<const uint8_t*>(mapped.pData) + static_cast<size_t>(y) * mapped.RowPitch,
            row_bytes);
    }

    impl_->d3d_context->Unmap(impl_->staging_texture.Get(), 0);
    cv::cvtColor(bgra, outBgr, cv::COLOR_BGRA2BGR);
    return true;
}

void WgcWindowCapturer::shutdown()
{
    ready_ = false;
    if (!impl_) {
        return;
    }

    CloseInspectable(impl_->session.Get());
    CloseInspectable(impl_->frame_pool.Get());

    impl_->session.Reset();
    impl_->frame_pool.Reset();
    impl_->item.Reset();
    impl_->winrt_device.Reset();
    impl_->staging_texture.Reset();
    impl_->d3d_context.Reset();
    impl_->d3d_device.Reset();

    delete impl_;
    impl_ = nullptr;
}
