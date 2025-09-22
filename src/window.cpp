#include "window.h"

#include "app_constants.h"

#include <cstdint>
#include <ctime>
#include <random>
#include <vector>

namespace halo
{
    namespace
    {
        std::mt19937 &Rng()
        {
            static std::mt19937 generator = []
            {
                std::random_device rd;
                std::vector<std::uint32_t> seedData{
                    static_cast<std::uint32_t>(rd()),
                    static_cast<std::uint32_t>(rd()),
                    static_cast<std::uint32_t>(GetTickCount()),
                    static_cast<std::uint32_t>(std::time(nullptr))};
                std::seed_seq sequence(seedData.begin(), seedData.end());
                return std::mt19937(sequence);
            }();
            return generator;
        }

        POINT RandomTopLeftForWindow(int width, int height)
        {
            const int minX = GetSystemMetrics(SM_XVIRTUALSCREEN);
            const int minY = GetSystemMetrics(SM_YVIRTUALSCREEN);
            const int screenW = GetSystemMetrics(SM_CXVIRTUALSCREEN);
            const int screenH = GetSystemMetrics(SM_CYVIRTUALSCREEN);

            int maxX = minX + screenW - width;
            int maxY = minY + screenH - height;

            if (maxX < minX)
            {
                maxX = minX;
            }
            if (maxY < minY)
            {
                maxY = minY;
            }

            std::uniform_int_distribution<int> distX(minX, maxX);
            std::uniform_int_distribution<int> distY(minY, maxY);
            return POINT{distX(Rng()), distY(Rng())};
        }
    } // namespace

    LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_CREATE:
        {
            CreateWindowExW(
                0,
                L"STATIC",
                L"Halo! :)",
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                20,
                20,
                240,
                40,
                hWnd,
                nullptr,
                GetModuleHandleW(nullptr),
                nullptr);

            CreateWindowExW(
                0,
                L"BUTTON",
                L"Cancel",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                20,
                80,
                100,
                32,
                hWnd,
                reinterpret_cast<HMENU>(kBtnCancelId),
                GetModuleHandleW(nullptr),
                nullptr);

            CreateWindowExW(
                0,
                L"BUTTON",
                L"OK",
                WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                140,
                80,
                100,
                32,
                hWnd,
                reinterpret_cast<HMENU>(kBtnOkId),
                GetModuleHandleW(nullptr),
                nullptr);
            return 0;
        }

        case WM_COMMAND:
        {
            if ((LOWORD(wParam) == kBtnOkId || LOWORD(wParam) == kBtnCancelId) && HIWORD(wParam) == BN_CLICKED)
            {
                const HINSTANCE instance = reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(hWnd, GWLP_HINSTANCE));
                CreateAppWindow(instance);
                CreateAppWindow(instance);
                DestroyWindow(hWnd);
                return 0;
            }
            break;
        }

        case WM_CLOSE:
        {
            const HINSTANCE instance = reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(hWnd, GWLP_HINSTANCE));
            CreateAppWindow(instance);
            CreateAppWindow(instance);
            DestroyWindow(hWnd);
            return 0;
        }

        case WM_SIZE:
        {
            if (wParam == SIZE_MINIMIZED)
            {
                const HINSTANCE instance = reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(hWnd, GWLP_HINSTANCE));
                CreateAppWindow(instance);
                ShowWindow(hWnd, SW_RESTORE);
                return 0;
            }
            break;
        }

        case WM_DESTROY:
            return 0;
        }

        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    HWND CreateAppWindow(HINSTANCE hInstance)
    {
        const POINT topLeft = RandomTopLeftForWindow(kWndWidth, kWndHeight);

        HWND window = CreateWindowExW(
            0,
            kClassName,
            L"Halo! :)",
            WS_OVERLAPPEDWINDOW,
            topLeft.x,
            topLeft.y,
            kWndWidth,
            kWndHeight,
            nullptr,
            nullptr,
            hInstance,
            nullptr);

        if (window)
        {
            ShowWindow(window, SW_SHOW);
            UpdateWindow(window);
        }

        return window;
    }
} // namespace halo
