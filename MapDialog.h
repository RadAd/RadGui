#pragma once

#include <Windows.h>

namespace rad
{
    class WindowProxy;
}

void MapDialogRect(rad::WindowProxy& Dlg, LPRECT lppxRect);
void UnMapDialogRect(rad::WindowProxy& Dlg, LPRECT lppxRect);
void UnMapDialogSize(rad::WindowProxy& Dlg, LPSIZE lppxSize);
void MapDialogSize(rad::WindowProxy& Dlg, LPSIZE lpdluSize);

SIZE /*px*/ CalcTextSize(rad::WindowProxy& Wnd, HFONT hFont, LPCWSTR pString, int pxWidth = 0);
