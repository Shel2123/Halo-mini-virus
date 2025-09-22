#include "win32_common.h"

#include "app_constants.h"
#include "autostart.h"
#include "window.h"

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shlwapi.lib")

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int)
{
    halo::EnsureAutostartInstalled();

    WNDCLASSEXW windowClass = {sizeof(WNDCLASSEXW)};
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = halo::WndProc;
    windowClass.hInstance = hInstance;
    windowClass.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    windowClass.lpszClassName = halo::kClassName;
    windowClass.hIconSm = windowClass.hIcon;

    if (!RegisterClassExW(&windowClass))
    {
        MessageBoxW(nullptr, L"RegisterClassEx failed", L"Error", MB_ICONERROR);
        return 1;
    }

    if (!halo::CreateAppWindow(hInstance))
    {
        MessageBoxW(nullptr, L"CreateWindowEx failed", L"Error", MB_ICONERROR);
        return 1;
    }

    MSG message;
    while (GetMessageW(&message, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return static_cast<int>(message.wParam);
}
