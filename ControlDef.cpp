#include "ControlDef.h"

#include <CommCtrl.h>
#include <Windowsx.h>
#include <Shlobj.h>
#include <Shlwapi.h>
#include <algorithm>

#include <Rad\GUI\WindowCreate.h>

#include "resource.h"

void ControlDef::CreateChild(rad::WindowProxy& Dlg, const RECT& dluRect, int pxCaptionOffset)
{
    static int id = 1000;

    HINSTANCE hInstance = (HINSTANCE) Dlg.GetWindowLongPtr(GWLP_HINSTANCE);
    HFONT hFont = Dlg.GetFont();

    rad::WindowCreate wc(hInstance);

    int pxXOffset = 0;
    if (!m_sCaption.empty())
    {
        pxXOffset = pxCaptionOffset;

        RECT pxRect(dluRect);
        pxRect.top += 3;
        pxRect.bottom = 8;
        MapDialogRect(Dlg, &pxRect);

        wc.x = pxRect.left;
        wc.y = pxRect.top;
        wc.Width = pxXOffset;
        wc.Height = pxRect.bottom;
        wc.Style = WS_TABSTOP | WS_VISIBLE | WS_CHILD;
        m_hCaption = wc.Create(m_sCaption.c_str(), nullptr, WC_STATIC, Dlg);

        m_hCaption.SetFont(hFont, FALSE);

        pxXOffset += 5; // TODO Convert 5 to px
    }

    RECT pxRect(dluRect);
    if (m_dluWidth > 0)
        pxRect.right = m_dluWidth;
    MapDialogRect(Dlg, &pxRect);

    const SIZE dluSize = GetSize(Dlg, pxRect.right);

    pxRect = dluRect;
    //pxRect.bottom = dluSize.cy;
    if (m_dluHeight > 0)
        pxRect.bottom = m_dluHeight;
    if (m_dluWidth > 0)
        pxRect.right = m_dluWidth;
    MapDialogRect(Dlg, &pxRect);

    pxRect.left += pxXOffset;
    if (m_dluWidth <= 0)
        pxRect.right -= pxXOffset;

    wc.x = pxRect.left;
    wc.y = pxRect.top;
    wc.Width = pxRect.right;
    wc.Height = pxRect.bottom;

    wc.Style = m_dwStyle | WS_CHILD;
    wc.ExStyle = m_dwExStyle;
    wc.hMenu = (HMENU) (INT_PTR) (m_id == 0 ? id++ : m_id);
    m_Ctrl = wc.Create(m_sText.c_str(), nullptr, m_sClass, Dlg);

    m_Ctrl.SetFont(hFont, FALSE);
}

ButtonDef::ButtonDef(int iIcon)
    : ControlDef(WC_BUTTON, nullptr, nullptr, WS_TABSTOP | WS_VISIBLE | BS_ICON, 0, 20, 14)
    , m_iIcon(iIcon)
{
}

ButtonDef::ButtonDef(LPCTSTR sId, LPCTSTR sText)
    : ControlDef(WC_BUTTON, sId, sText, WS_TABSTOP | WS_VISIBLE, 0, 50, 14)
    , m_iIcon(0)
{
}

ComboBoxDef::ComboBoxDef(LPCTSTR sId, LPCTSTR sCaption)
    : ControlDef(WC_COMBOBOX, sId, _T(""), WS_TABSTOP | WS_VISIBLE | CBS_DROPDOWNLIST, 0, 0, 14)
{
    if (sCaption != nullptr)
        m_sCaption = sCaption;
}

void ComboBoxDef::CreateChild(rad::WindowProxy& Dlg, const RECT& dluRect, int pxCaptionOffset)
{
    // TODO Need to move this to an earlier place
    SIZE dluSize = { m_pxWidth, 0 };
    UnMapDialogSize(Dlg, &dluSize);
    m_dluWidth = dluSize.cx;

    ControlDef::CreateChild(Dlg, dluRect, pxCaptionOffset);
    for (const std::wstring& s : m_items)
        ComboBox_AddString(m_Ctrl.GetHWND(), s.c_str());
    ComboBox_SetCurSel(m_Ctrl.GetHWND(), m_selected);
}

SIZE ComboBoxDef::GetSize(rad::WindowProxy& Dlg, int w) const
{
    // TODO Need to move this to an earlier place
    SIZE dluSize = { m_pxWidth, 0 };
    UnMapDialogSize(Dlg, &dluSize);
    const_cast<ComboBoxDef*>(this)->m_dluWidth = dluSize.cx;

    return ControlDef::GetSize(Dlg, w);
}

std::wstring ComboBoxDef::GetCommandValue() const
{
    assert(m_Ctrl.GetHWND() != NULL);
    int i = ComboBox_GetCurSel(m_Ctrl.GetHWND());
    if (i >= 0 && i < (int) m_items.size())
        return m_cl[i];
    else
        return std::wstring();
}

CheckBoxDef::CheckBoxDef(LPCTSTR sId, LPCTSTR sText, bool bChecked, LPCTSTR sCommandLine)
    : ControlWithChildrenDef(WC_BUTTON, sId, sText, WS_TABSTOP | WS_VISIBLE | BS_AUTOCHECKBOX, 0, 0, 10)
    , m_bChecked(bChecked)
{
    if (sCommandLine)
        m_CommandLine = sCommandLine;
}

void CheckBoxDef::SetCheck(bool bChecked)
{
    assert(m_Ctrl.GetHWND() != NULL);
    Button_SetCheck(m_Ctrl.GetHWND(), bChecked ? BST_CHECKED : BST_UNCHECKED);
}

bool CheckBoxDef::GetCheck() const
{
    assert(m_Ctrl.GetHWND() != NULL);
    return Button_GetCheck(m_Ctrl.GetHWND()) == BST_CHECKED;
}

RadioDef::RadioDef(LPCTSTR sId, LPCTSTR sText, bool bChecked, LPCTSTR sCommandLine)
    : ControlWithChildrenDef(WC_BUTTON, sId, sText, WS_TABSTOP | WS_VISIBLE | BS_AUTORADIOBUTTON, 0, 0, 10)
    , m_bChecked(bChecked)
{
    if (sCommandLine)
        m_CommandLine = sCommandLine;
}

BOOL RadioDef::OnNotify(LPNMHDR ph)
{
    switch (ph->code)
    {
    case NM_CUSTOMDRAW: // Using this message to be notified when unclicked
        m_Children.Enable(GetCheck());
        break;
    }
    return FALSE;
}

void RadioDef::SetCheck(bool bChecked)
{
    assert(m_Ctrl.GetHWND() != NULL);
    Button_SetCheck(m_Ctrl.GetHWND(), bChecked ? BST_CHECKED : BST_UNCHECKED);
}

bool RadioDef::GetCheck() const
{
    assert(m_Ctrl.GetHWND() != NULL);
    return Button_GetCheck(m_Ctrl.GetHWND()) == BST_CHECKED;
}

ImageDef::ImageDef(LPCTSTR sId, LPCTSTR sFileName)
    : ControlDef(WC_STATIC, sId, _T(""), WS_TABSTOP | WS_VISIBLE | SS_BITMAP, 0, 0, 0)
    , m_hBitmap((HBITMAP) LoadImage(NULL, sFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE))
{
}

TextDef::TextDef(LPCTSTR sId, LPCTSTR sText)
    : ControlDef(WC_STATIC, sId, sText, WS_TABSTOP | WS_VISIBLE, 0, 0, 0)
{
}

TabDef::TabDef()
    : ControlDef(WC_TABCONTROL, nullptr, _T(""), WS_TABSTOP | WS_VISIBLE | TCS_TABS, 0, 0, 0)
{
}

void TabDef::CreateChild(rad::WindowProxy& Dlg, const RECT& dluRect, int pxCaptionOffset)
{
    HINSTANCE hInstance = (HINSTANCE) Dlg.GetWindowLongPtr(GWLP_HINSTANCE);
    HFONT hFont = Dlg.GetFont();

    ControlDef::CreateChild(Dlg, dluRect, pxCaptionOffset);

    for (const Page& p : m_Pages)
    {
        TCITEM i = {};
        i.mask = TCIF_TEXT;
        i.pszText = const_cast<LPTSTR>(p.m_sName.c_str());
        TabCtrl_InsertItem(m_Ctrl.GetHWND(), 100, &i);
    }

    RECT pxChildRect = {};
    m_Ctrl.GetClientRect(&pxChildRect);
    TabCtrl_AdjustRect(m_Ctrl.GetHWND(), FALSE, &pxChildRect);

    RECT dluChildRect;
    dluChildRect.left = 0;
    dluChildRect.top = 0;
    dluChildRect.right = GetWidth(pxChildRect);
    dluChildRect.bottom = GetHeight(pxChildRect);
    UnMapDialogRect(Dlg, &dluChildRect);
    dluChildRect.left += 6;
    dluChildRect.top += 6;
    dluChildRect.right -= 6 + 6;
    dluChildRect.bottom -= 6 + 6;

    for (Page& p : m_Pages)
    {
        p.m_Dlg = new PageDialog;
        p.m_Dlg->CreateDlg(hInstance, IDD_DIALOG2, m_Ctrl);
        p.m_Dlg->SetWindowPos(NULL, pxChildRect.left - 2, pxChildRect.top, GetWidth(pxChildRect) + 2, GetHeight(pxChildRect) + 1, SWP_NOOWNERZORDER | SWP_NOZORDER);

        p.m_Dlg->SetFont(hFont, FALSE);

        pxCaptionOffset = p.m_Children.GetLabelOffset(Dlg);
        p.m_Children.CreateChild(*p.m_Dlg, dluChildRect, pxCaptionOffset);
    }

    ShowPage(true);
}

SIZE TabDef::GetSize(rad::WindowProxy& Dlg, int pxWidth) const
{
    SIZE pxOffset = { 6, 0 };
    MapDialogSize(Dlg, &pxOffset);
    SIZE dluSize = {};
    for (const Page& p : m_Pages)
    {
        SIZE dluControlSize = p.m_Children.GetSize(Dlg, pxWidth - 2 * pxOffset.cx);
        if (dluControlSize.cx > dluSize.cx)
            dluSize.cx = dluControlSize.cx;
        if (dluControlSize.cy > dluSize.cy)
            dluSize.cy = dluControlSize.cy;
    }
    dluSize.cx += 6 + 6;
    dluSize.cy += 6 + 6;
    dluSize.cy += 15;   // TODO This is a guess at the tab header
    return dluSize;
}

BOOL TabDef::OnNotify(LPNMHDR ph)
{
    switch (ph->code)
    {
    case TCN_SELCHANGE:
        ShowPage(true);
        break;

    case TCN_SELCHANGING:
        ShowPage(false);
        break;
    }
    return FALSE;
}

VerticalLayout& TabDef::GetPage(int i)
{
    return m_Pages[i].m_Children;
}

void TabDef::ShowPage(bool show)
{
    assert(m_Ctrl.GetHWND() != NULL);
    int i = TabCtrl_GetCurSel(m_Ctrl.GetHWND());
    m_Pages[i].m_Dlg->ShowWindow(show ? SW_SHOW : SW_HIDE);
}

ControlDef* TabDef::Find(HWND hCtrl)
{
    ControlDef* cd = ControlDef::Find(hCtrl);
    if (cd == nullptr)
    {
        for (Page& p : m_Pages)
        {
            cd = p.m_Children.Find(hCtrl);
            if (cd != nullptr)
                break;
        }
    }
    return cd;
}

VerticalLayout& TabDef::AddPage(const std::wstring& name)
{
    for (Page& p : m_Pages)
    {
        if (p.m_sName == name)
            return p.m_Children;
    }
    m_Pages.push_back(name);
    return m_Pages.back().m_Children;
}

void TabDef::GetCommandLine(std::wstring& cl) const
{
    for (const Page& p : m_Pages)
        p.m_Children.GetCommandLine(cl);
}

GroupDef::GroupDef(LPCTSTR sId, LPCTSTR sText)
    : ControlWithChildrenDef(WC_BUTTON, sId, sText, WS_TABSTOP | WS_VISIBLE | WS_GROUP | BS_GROUPBOX, 0, 0, 0)
{
}

EditDef::EditDef(LPCTSTR sId, LPCTSTR sCaption, LPCTSTR sValue, LPCTSTR sCommandLine, bool bQuote)
    : ControlDef(WC_EDIT, sId, sValue, WS_TABSTOP | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 0, 0, 14)
    , m_bQuote(bQuote)
{
    if (sCaption != nullptr)
        m_sCaption = sCaption;
    if (sCommandLine)
        m_CommandLine = sCommandLine;
}

std::wstring EditDef::GetCommandValue() const
{
    TCHAR text[1024] = _T("");
    m_Ctrl.GetWindowText(text);
    if (m_bQuote)
        PathQuoteSpaces(text);
    return text;
}

SelectDirDef::SelectDirDef()
    : ButtonDef(IDI_FOLDER_OPEN)
{
}

BOOL SelectDirDef::OnCommand(WORD NotifyCode)
{
    switch (NotifyCode)
    {
    case BN_CLICKED:
        {
            rad::WindowProxy Buddy = m_Ctrl.GetWindow(GW_HWNDPREV);
            TCHAR dir[MAX_PATH];
            Buddy.GetWindowText(dir, MAX_PATH);

            BROWSEINFO bi = {};
            bi.hwndOwner = m_Ctrl.GetAncestor(GA_ROOT);
            bi.lpszTitle = _T("Select directory");
            bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
            bi.lpfn = BrowseCallbackProc;
            bi.lParam = (LPARAM) dir;
            PIDLIST_ABSOLUTE pidlFolder = SHBrowseForFolder(&bi);
            if (pidlFolder != NULL)
            {
                SHGetPathFromIDList(pidlFolder, dir);
                Buddy.SetWindowText(dir);
                CoTaskMemFree(pidlFolder);
            }
        }
        break;
    }
    return FALSE;
}

int CALLBACK SelectDirDef::BrowseCallbackProc(
    HWND   hwnd,
    UINT   uMsg,
    LPARAM /*lParam*/,
    LPARAM lpData
)
{
    switch (uMsg)
    {
    case BFFM_INITIALIZED:
        ::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
        break;
    }
    return 0;
}

SelectFileDef::SelectFileDef(LPCTSTR sFilter)
    : ButtonDef(IDI_FOLDER_OPEN)
{
    if (sFilter != nullptr)
    {
        m_sFilter = sFilter;
        std::replace(m_sFilter.begin(), m_sFilter.end(), L'|', L'\0');
        m_sFilter += L'\0';
    }
}
