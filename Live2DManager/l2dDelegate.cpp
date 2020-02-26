
#include "l2dDelegate.h"

int RenderTargetWidth = 0;
int RenderTargetHeight = 0;

constexpr const char* title = "Live2D Helper";
IDirect3D*             _direct3D = nullptr;
IDirect3DDevice*       _device = nullptr;
IDirect3DSurface9* pBackBuffer = nullptr;
IDirect3DSurface9 * p2DSurface = nullptr;
HWND _windowHandle = NULL;

LostStep                _deviceLostStep = LostStep::LostStep_None;
D3DPRESENT_PARAMETERS   _presentParameters;

D3DLOCKED_RECT lockedRect;

Live2DManager* l2dManager = nullptr;

typedef void __stdcall ReceiverFunctionType(void* pData);

ReceiverFunctionType* ReceiverFunction = nullptr;

LPD3DXFONT pFont = nullptr;
bool showFPS = true;
bool checkDelay = true;
double delay = 0.0;

void ResetParameters()
{

    memset(&_presentParameters, 0, sizeof(_presentParameters));
    _presentParameters.Windowed = TRUE;
    _presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
    _presentParameters.BackBufferFormat = D3DFMT_A8R8G8B8;
    _presentParameters.BackBufferCount = 1;
    _presentParameters.MultiSampleType = D3DMULTISAMPLE_NONE;
    _presentParameters.MultiSampleQuality = 0;
    _presentParameters.EnableAutoDepthStencil = TRUE;
    _presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
    _presentParameters.hDeviceWindow = _windowHandle;
    _presentParameters.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
    _presentParameters.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    _presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

    _presentParameters.BackBufferWidth = RenderTargetWidth;
    _presentParameters.BackBufferHeight = RenderTargetHeight;
}


void CommitFunction()
{
    if (ReceiverFunction) {
        int width = _presentParameters.BackBufferWidth;
        int height = _presentParameters.BackBufferHeight;

        if (pBackBuffer == nullptr) {
            _device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
        }
        if (p2DSurface == nullptr) {
            //_device->CreateOffscreenPlainSurface(width, height, _presentParameters.BackBufferFormat, D3DPOOL_SYSTEMMEM, &p2DSurface, NULL);
        }
        //_device->GetRenderTargetData(pBackBuffer, p2DSurface);
        //p2DSurface->LockRect(&pLockedRect, 0, 0);
        pBackBuffer->LockRect(&lockedRect, 0, 0);

        ReceiverFunction(lockedRect.pBits);
        /*
        for (int i = 0; i < width * height; i++) {
            D3DCOLOR color = reinterpret_cast<int*>(lockedRect.pBits)[i];
        }*/

        //p2DSurface->UnlockRect();
        //p2DSurface->Release();
        pBackBuffer->UnlockRect();
        //pBackBuffer->Release();
    }
}


void StartFrame()
{
    _device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

    if (SUCCEEDED(_device->BeginScene()))
    {
        _device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        _device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        _device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        _device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
    }
}

int sumFps = 0;
int GetFpsValue()
{
    static int startTimer = GetTickCount();
    //计算Fps=总帧数/时长
    int curTimer = GetTickCount();
    int delta = abs(curTimer - startTimer);
    static int curFPS = 0;
    if (delta > 1000) {
        startTimer = curTimer;
        curFPS = sumFps;
        sumFps = -1;
    }
    return curFPS;
}

void Render()
{
    if (showFPS) {
        sumFps++;
        std::wstring fps_s = L"FPS:";
        WCHAR fps[16];
        wsprintf(fps, L"%d", GetFpsValue());
        fps_s += fps;
        RECT rect{ 0, 0, 100, 100 };
        pFont->DrawText(NULL, fps_s.c_str(), fps_s.length(), &rect,
            DT_SINGLELINE | DT_NOCLIP | DT_CENTER | DT_VCENTER, D3DXCOLOR(1.0f, 0.75f, 0.0f, 1.0f));
    }

    l2dManager->OnUpdate();
}

void EndFrame()
{
    _device->EndScene();
    l2dManager->EndFrame();
}
#define EXPORT_FUNCTION(name) extern "C" __declspec(dllexport) void __cdecl name

EXPORT_FUNCTION(L2DManager_SetWindowHandle)(HWND hWnd)
{
    _windowHandle = hWnd;
}

EXPORT_FUNCTION(L2DManager_InitManager)()
{
    if (RenderTargetWidth == 0 || RenderTargetHeight == 0) {
        MessageBoxA(_windowHandle, "L2DManager_InitManager::RenderSize Invalid", title, MB_OK);
        return;
    }

    if ((_direct3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
    {
        MessageBoxA(_windowHandle, "Direct3DCreate::return nullptr", title, MB_OK);
        return;
    }

    ResetParameters();

    HRESULT createResult = _direct3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, _windowHandle,
        D3DCREATE_HARDWARE_VERTEXPROCESSING, &_presentParameters,
        &_device);

    if (FAILED(createResult))
    {
        MessageBoxA(_windowHandle, "CreateDevice::return failed", title, MB_OK);
        return;
    }

    if (showFPS) {
        createResult = D3DXCreateFont(_device, 15, 0, FW_BOLD, 1,
            FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE,
            TEXT("Arial"), &pFont);

        if (FAILED(createResult))
        {
            MessageBoxA(_windowHandle, "D3DXCreateFont::return failed", title, MB_OK);
            return;
        }
    }

    l2dManager = Live2DManager::GetInstance();
    RECT rect{ 0, 0, RenderTargetWidth, RenderTargetHeight };
    l2dManager->Initialize(rect, _direct3D, _device, CommitFunction, (int*)&_deviceLostStep);
}

EXPORT_FUNCTION(L2DManager_Render)()
{
    if (Delay()) {
        Live2DPal::UpdateTime();
        StartFrame();
        Render();
        EndFrame();
    }
}

bool Delay()
{
    if (checkDelay) {
        //constexpr double freq = 0.02;
        if (Live2DPal::MarkTimeIfElapsed(delay) == false) {
            double time = Live2DPal::GetDeltaMarkedTime();
            return false;
            //Sleep((delay - time) * 1000u);
            //bool flag = Live2DPal::MarkTimeIfElapsed(freq);
            //time = Live2DPal::GetDeltaMarkedTime();
        }
    }
    return true;
}

EXPORT_FUNCTION(L2DManager_OnTouchesBegan)(float px, float py)
{
    l2dManager->TouchesBegan(px, py);
}

EXPORT_FUNCTION(L2DManager_OnTouchesMoved)(float px, float py)
{
    l2dManager->TouchesMoved(px, py);
}

EXPORT_FUNCTION(L2DManager_OnTouchesEnded)(float px, float py)
{
    l2dManager->TouchesEnded(px, py);
}

EXPORT_FUNCTION(L2DManager_SetRenderSize)(int width, int height)
{
    RenderTargetWidth = width;
    RenderTargetHeight = height;
    if (l2dManager) {
        l2dManager->SetClientSize(width, height);
    }
}

EXPORT_FUNCTION(L2DManager_SetRenderScale)(float x, float y)
{
    l2dManager->SetRenderScale(x, y);
}

EXPORT_FUNCTION(L2DManager_OpenScene)(const char* dir, const char* name)
{
    l2dManager->OpenScene(dir, name);
}

EXPORT_FUNCTION(L2DManager_Release)()
{
    Live2DManager::ReleaseInstance();

    _device->Release();
    _device = nullptr;

    _direct3D->Release();
    _direct3D = nullptr;

    if (pBackBuffer) {
        pBackBuffer->Release();
        pBackBuffer = nullptr;
    }

    if (p2DSurface) {
        p2DSurface->Release();
        p2DSurface = nullptr;
    }
}

EXPORT_FUNCTION(L2DManager_SetReceiverFunction)(ReceiverFunctionType* function)
{
    ReceiverFunction = function;
}

EXPORT_FUNCTION(L2DManager_SetRenderDelay)(double _delay)
{
    delay = _delay;
}

EXPORT_FUNCTION(L2DManager_StartRandomMotion)(const char* group, int priority)
{
    l2dManager->GetModel(0)->StartRandomMotion(group, priority);
}

EXPORT_FUNCTION(L2DManager_SetExpression)(const char* expressionID)
{
    l2dManager->GetModel(0)->SetExpression(expressionID);
}