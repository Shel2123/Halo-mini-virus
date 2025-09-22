#pragma once

#include "win32_common.h"

namespace halo
{
    LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    HWND CreateAppWindow(HINSTANCE hInstance);
}
