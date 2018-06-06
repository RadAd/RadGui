#include "MapDialog.h"

#include <Rad\GUI\DevContext.h>
#include <Rad\GUI\GdiObject.h>
#include <Rad\Rect.h>

using namespace rad;

// Reverse MapDialogRect https://www.google.com.au/url?sa=t&rct=j&q=&esrc=s&source=web&cd=1&ved=0ahUKEwjGhMGU6tzSAhXEQpQKHbUsD4MQFggbMAA&url=https%3A%2F%2Fgroups.google.com%2Fd%2Ftopic%2Fcomp.os.ms-windows.programmer.win32%2FfT3gXGbwISc&usg=AFQjCNGMPqIQoxrBP7Nkt0NAnvAGZrbzLg&sig2=etRcY1jDvxceYdEYWquq1g

void MapDialogRect(WindowProxy& Dlg, LPRECT lppxRect)
{
    MapDialogRect(Dlg.GetHWND(), lppxRect);
}

void UnMapDialogRect(WindowProxy& Dlg, LPRECT lppxRect)
{
    RECT rect = { 0, 0, 4, 8 };
    MapDialogRect(Dlg, &rect);
    UINT baseunitX = rect.right;
    UINT baseunitY = rect.bottom;

    lppxRect->left = MulDiv(lppxRect->left, 4, baseunitX);
    lppxRect->right = MulDiv(lppxRect->right, 4, baseunitX);
    lppxRect->top = MulDiv(lppxRect->top, 8, baseunitY);
    lppxRect->bottom = MulDiv(lppxRect->bottom, 8, baseunitY);
}

void UnMapDialogSize(WindowProxy& Dlg, LPSIZE lppxSize)
{
    RECT pxRect = { 0, 0, lppxSize->cx, lppxSize->cy };
    UnMapDialogRect(Dlg, &pxRect);
    lppxSize->cx = pxRect.right;
    lppxSize->cy = pxRect.bottom;
}

void MapDialogSize(WindowProxy& Dlg, LPSIZE lpdluSize)
{
    RECT dluRect = { 0, 0, lpdluSize->cx, lpdluSize->cy };
    MapDialogRect(Dlg, &dluRect);
    lpdluSize->cx = dluRect.right;
    lpdluSize->cy = dluRect.bottom;
}

SIZE /*px*/ CalcTextSize(WindowProxy& Wnd, HFONT hFont, LPCWSTR pString, int pxWidth)
{
    WindowDC DC(Wnd);
    FontRef font(hFont);  // TODO Pass in GDIObject
    TempSelectObject oldfont(DC, font);
    RECT pxRect = { 0, 0, pxWidth, 0 };
    UINT Format = DT_CALCRECT | DT_EXPANDTABS;
    if (pxWidth > 0)
        Format |= DT_WORDBREAK;
    DC.DrawText(pString, -1, &pxRect, Format);
    font.Release();
    return GetSize(pxRect);
}
