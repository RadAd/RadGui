#pragma once

#include <Rad\GUI\Dialog.h>
#include <Rad\Rect.h>

#include <vector>

#include "MapDialog.h"

class ControlDef;

#define WM_UPDATE_CMD (WM_USER + 14)

// TODO GetSize may need pxCaptionOffset
class BaseDef
{
public:
    virtual void CreateChild(rad::WindowProxy& Dlg, const RECT& dluRect, int pxCaptionOffset) = 0;
    virtual SIZE /*dlu*/ GetSize(rad::WindowProxy& Dlg, int pxWidth) const = 0;
    virtual int /*px*/ GetLabelOffset(rad::WindowProxy& Dlg) const = 0;
    virtual void Show(bool show) = 0;
    virtual void Enable(bool enable) = 0;
    virtual ControlDef* Find(HWND hCtrl) = 0;
    virtual ControlDef* FindId(LPCWSTR id) = 0;
    virtual void Save(std::map<std::wstring, std::wstring>& properties) = 0;
    virtual bool Fill() const { return false; }
    virtual void GetCommandLine(std::wstring& cl) const = 0;
};

class ChildrenLayout : public BaseDef
{
public:
    BaseDef* Add(BaseDef* cd)
    {
        m_controls.push_back(cd);
        return cd;
    }

    virtual int GetLabelOffset(rad::WindowProxy& Dlg) const override
    {
        int pxOffset = 0;
        for (BaseDef* cd : m_controls)
        {
            int pxControlOffset = cd->GetLabelOffset(Dlg);
            if (pxControlOffset > pxOffset)
                pxOffset = pxControlOffset;
        }
        return pxOffset;
    }

    virtual void Show(bool show) override
    {
        for (BaseDef* cd : m_controls)
            cd->Show(show);
    }

    virtual void Enable(bool enable) override
    {
        for (BaseDef* cd : m_controls)
            cd->Enable(enable);
    }

    virtual ControlDef* Find(HWND hCtrl) override
    {
        for (BaseDef* bd : m_controls)
        {
            ControlDef* cd = bd->Find(hCtrl);
            if (cd != nullptr)
                return cd;
        }
        return nullptr;
    }

    virtual ControlDef* FindId(LPCWSTR id) override
    {
        for (BaseDef* bd : m_controls)
        {
            ControlDef* cd = bd->FindId(id);
            if (cd != nullptr)
                return cd;
        }
        return nullptr;
    }

    virtual void Save(std::map<std::wstring, std::wstring>& properties) override
    {
        for (BaseDef* bd : m_controls)
        {
            bd->Save(properties);
        }
    }

    virtual void GetCommandLine(std::wstring& cl) const
    {
        for (BaseDef* bd : m_controls)
        {
            bd->GetCommandLine(cl);
        }
    }

protected:
    std::vector<BaseDef*> m_controls;
};

class VerticalLayout : public ChildrenLayout
{
public:
    virtual void CreateChild(rad::WindowProxy& Dlg, const RECT& dluRect, int pxCaptionOffset) override
    {
        SIZE pxSize = { dluRect.right, dluRect.bottom };
        MapDialogSize(Dlg, &pxSize);

        // TODO Spread out better
        int dluTotalHeight = 0;
        int count = 0;
        if (m_controls.size() == 1 && m_controls.front()->Fill())
        {
            count = 1;
        }
        else for (BaseDef* cd : m_controls)
        {
            int dluHeight = cd->GetSize(Dlg, pxSize.cx).cy;
            if (dluHeight == 0)
                ++count;
            else
                dluTotalHeight += dluHeight + 5;
        }
        int dluAverageHeight = count > 0 ? (dluRect.bottom - dluTotalHeight) / count : 0;
        if (count > 0 && dluTotalHeight > 0)
            dluAverageHeight -= 5;
        //dluTotalHeight -= 5;

        RECT dluCurrent(dluRect);
        for (BaseDef* cd : m_controls)
        {
            int dluHeight = cd->GetSize(Dlg, pxSize.cx).cy;
            if (dluHeight == 0 || (m_controls.size() == 1 && cd->Fill()))
                dluHeight = dluAverageHeight;
            dluCurrent.bottom = dluHeight;
            cd->CreateChild(Dlg, dluCurrent, pxCaptionOffset);
            dluHeight += 5;
            dluCurrent.top += dluHeight;
        }
    }

    virtual SIZE GetSize(rad::WindowProxy& Dlg, int pxWidth) const override
    {
        SIZE dluSize = {};
        if (!m_controls.empty())
            dluSize.cy -= 5;
        for (BaseDef* cd : m_controls)
        {
            SIZE dluControlSize = cd->GetSize(Dlg, pxWidth);
            if (dluControlSize.cx > dluSize.cx)
                dluSize.cx = dluControlSize.cx;
            dluSize.cy += dluControlSize.cy + 5;
        }
        return dluSize;
    }
};

class HorizontalLayout : public ChildrenLayout
{
public:
    virtual void CreateChild(rad::WindowProxy& Dlg, const RECT& dluRect, int pxCaptionOffset) override
    {
        SIZE pxSize = { dluRect.right, dluRect.bottom };
        MapDialogSize(Dlg, &pxSize);

        // TODO Spread out better
        int dluTotalWidth = 0;
        int count = 0;
        if (m_controls.size() == 1 && m_controls.front()->Fill())
        {
            count = 1;
        }
        else for (BaseDef* cd : m_controls)
        {
            int dluWidth = cd->GetSize(Dlg, pxSize.cx).cx;
            if (dluWidth == 0 || cd->Fill())
                ++count;
            else
                dluTotalWidth += dluWidth + 5;
        }
        int dluAverageWidth = count > 0 ? (dluRect.right - dluTotalWidth) / count : 0;
        if (count > 0 && dluTotalWidth > 0)
            dluTotalWidth -= 5;
        //dluTotalWidth -= 5;

        RECT dluCurrent(dluRect);
        for (BaseDef* cd : m_controls)
        {
            int dluWidth = cd->GetSize(Dlg, pxSize.cx).cx;
            if (dluWidth == 0 || (m_controls.size() == 1 && cd->Fill()))
                dluWidth = dluAverageWidth;
            dluCurrent.right = dluWidth;
            pxCaptionOffset = cd->GetLabelOffset(Dlg);
            cd->CreateChild(Dlg, dluCurrent, pxCaptionOffset);
            dluWidth += 5;
            dluCurrent.left += dluWidth;
        }
    }

    virtual SIZE GetSize(rad::WindowProxy& Dlg, int pxWidth) const override
    {
        SIZE dluSize = {};
        if (!m_controls.empty())
            dluSize.cx -= 5;
        for (BaseDef* cd : m_controls)
        {
            SIZE dluControlSize = cd->GetSize(Dlg, pxWidth);
            if (dluControlSize.cy > dluSize.cy)
                dluSize.cy = dluControlSize.cy;
            dluSize.cx += dluControlSize.cx + 5;
        }
        return dluSize;
    }
};

class ControlDef : public BaseDef
{
public:
    ControlDef(LPCTSTR sClass, LPCTSTR sId, LPCTSTR sText, DWORD dwStyle, DWORD dwExStyle, int dluWidth, int dluHeight)
        : m_sClass(sClass)
        , m_dwStyle(dwStyle)
        , m_dwExStyle(dwExStyle)
        , m_dluWidth(dluWidth)
        , m_dluHeight(dluHeight)
    {
        if (sId)
            m_sId = sId;
        if (sText)
            m_sText = sText;
        if (m_sId.empty())
            m_sId = m_sText;
    }

    virtual void CreateChild(rad::WindowProxy& Dlg, const RECT& dluRect, int pxCaptionOffset) override;

    virtual SIZE GetSize(rad::WindowProxy& Dlg, int /*w*/) const override
    {
        SIZE dluSize = {};
        if (m_dluWidth > 0)
        {
            dluSize.cx = GetLabelOffset(Dlg);
            UnMapDialogSize(Dlg, &dluSize);
            dluSize.cx += m_dluWidth;
        }
        dluSize.cy += m_dluHeight;
        return dluSize;
    }

    virtual int GetLabelOffset(rad::WindowProxy& Dlg) const override
    {
        if (!m_sCaption.empty())
        {
            //HFONT hFont = m_Ctrl.GetFont();
            HFONT hFont = Dlg.GetFont();
            return CalcTextSize(Dlg, hFont, m_sCaption.c_str()).cx;
        }
        else
            return 0;
    }

    virtual BOOL OnNotify(LPNMHDR /*ph*/)
    {
        return FALSE;
    }

    virtual BOOL OnCommand(WORD /*NotifyCode*/)
    {
        return FALSE;
    }

    virtual void Show(bool show) override
    {
        m_Ctrl.ShowWindow(show ? SW_SHOW : SW_HIDE);
        if (m_hCaption.IsAttached())
            m_hCaption.ShowWindow(show ? SW_SHOW : SW_HIDE);
    }

    virtual void Enable(bool enable) override
    {
        m_Ctrl.EnableWindow(enable ? TRUE : FALSE);
        if (m_hCaption.IsAttached())
            m_hCaption.EnableWindow(enable ? TRUE : FALSE);
    }

    virtual ControlDef* Find(HWND hCtrl) override
    {
        return (hCtrl == GetHWnd()) ? this : nullptr;
    }

    virtual ControlDef* FindId(LPCWSTR id) override
    {
        return (m_sId == id) ? this : nullptr;
    }

    virtual void Save(std::map<std::wstring, std::wstring>& properties) override
    {
        if (!m_sId.empty())
        {
            properties[m_sId] = GetProperty();
        }
    }

    virtual std::wstring GetProperty() const
    {
        return std::wstring();
    }

    virtual void SetProperty(LPCTSTR /*sValue*/)
    {
        assert(false);
    }

    virtual bool UseCommandLine() const
    {
        return !m_CommandLine.empty();
    }

    virtual std::wstring GetCommandValue() const
    {
        return std::wstring();
    }

    virtual void GetCommandLine(std::wstring& cl) const override
    {
        if (UseCommandLine())
        {
            if (!cl.empty())
                cl += _T(" ");
            cl += m_CommandLine;
            cl += GetCommandValue();
        }
    }

    HWND GetHWnd() const
    {
        return m_Ctrl.GetHWND();
    }

protected:
    std::wstring m_CommandLine;

    rad::WindowProxy GetCtrl() const
    {
        return m_Ctrl;
    }

    rad::WindowProxy GetFrame() const
    {
        return m_Ctrl.GetAncestor(GA_ROOT);
    }

    rad::WindowProxy m_Ctrl;
    rad::WindowProxy m_hCaption;
    std::wstring m_sCaption;
    std::wstring m_sId;
    std::wstring m_sText;
    LPCTSTR m_sClass;
    DWORD m_dwStyle;
    DWORD m_dwExStyle;
    int m_dluWidth;
    int m_dluHeight;
    int m_id = 0;
};

class ControlWithChildrenDef : public ControlDef
{
public:
    using ControlDef::ControlDef;

    virtual void CreateChild(rad::WindowProxy& Dlg, const RECT& dluRect, int pxCaptionOffset) override
    {
        ControlDef::CreateChild(Dlg, dluRect, pxCaptionOffset);
        pxCaptionOffset = m_Children.GetLabelOffset(Dlg);
        m_Children.CreateChild(Dlg, GetChildrenRect(Dlg, dluRect), pxCaptionOffset);
    }

    virtual RECT /*dlu*/ GetChildrenRect(rad::WindowProxy& Dlg, const RECT& dluRect) const = 0;

    virtual void Show(bool show) override
    {
        ControlDef::Show(show);
        m_Children.Show(show);
    }

    virtual void Enable(bool enable) override
    {
        ControlDef::Enable(enable);
        m_Children.Enable(enable);
    }

    virtual ControlDef* Find(HWND hCtrl) override
    {
        ControlDef* cd = ControlDef::Find(hCtrl);
        if (cd == nullptr)
            cd = m_Children.Find(hCtrl);
        return cd;
    }

    virtual ControlDef* FindId(LPCWSTR id) override
    {
        ControlDef* cd = ControlDef::FindId(id);
        if (cd == nullptr)
            cd = m_Children.FindId(id);
        return cd;
    }


    virtual void Save(std::map<std::wstring, std::wstring>& properties) override
    {
        ControlDef::Save(properties);
        m_Children.Save(properties);
    }

    virtual void GetCommandLine(std::wstring& cl) const
    {
        ControlDef::GetCommandLine(cl);
        m_Children.GetCommandLine(cl);
    }

    VerticalLayout m_Children;
};

class ButtonDef : public ControlDef
{
public:
    ButtonDef(int iIcon);
    ButtonDef(LPCTSTR sId, LPCTSTR sText);

    virtual void CreateChild(rad::WindowProxy& Dlg, const RECT& dluRect, int pxCaptionOffset) override
    {
        ControlDef::CreateChild(Dlg, dluRect, pxCaptionOffset);
        if (m_iIcon != 0)
        {
            HINSTANCE hInstance = (HINSTANCE) Dlg.GetWindowLongPtr(GWLP_HINSTANCE);
            //HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FOLDER_OPEN));
            HICON hIcon = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(m_iIcon), IMAGE_ICON, 16, 16, 0);;
            m_Ctrl.SendMessage(BM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);
        }
    }

    void MakeDefault()
    {
        m_dwStyle |= BS_DEFPUSHBUTTON;
        m_id = IDOK;
    }

private:
    int m_iIcon;
};

class SelectDirDef : public ButtonDef
{
public:
    SelectDirDef();

    virtual BOOL OnCommand(WORD NotifyCode) override;

private:
    static int CALLBACK BrowseCallbackProc(
        HWND   hwnd,
        UINT   uMsg,
        LPARAM /*lParam*/,
        LPARAM lpData
    );
};

class SelectFileDef : public ButtonDef
{
public:
    SelectFileDef(LPCTSTR sFilter);

    virtual BOOL OnCommand(WORD NotifyCode) override
    {
        switch (NotifyCode)
        {
        case BN_CLICKED:
            {
                rad::WindowProxy Buddy = m_Ctrl.GetWindow(GW_HWNDPREV);
                TCHAR dir[MAX_PATH];
                Buddy.GetWindowText(dir, MAX_PATH);

                OPENFILENAME ofn = { sizeof(OPENFILENAME) };
                ofn.hwndOwner = m_Ctrl.GetAncestor(GA_ROOT);
                ofn.lpstrTitle = _T("Select file");
                ofn.lpstrFilter = m_sFilter.c_str();
                ofn.lpstrFile = dir;
                ofn.nMaxFile = MAX_PATH;
                //ofn.lpstrInitialDir
                ofn.Flags = OFN_ENABLESIZING;
                if (GetOpenFileName(&ofn))
                {
                    Buddy.SetWindowText(ofn.lpstrFile);
                }
            }
            break;
        }
        return FALSE;
    }

private:
    std::wstring m_sFilter;
};

class ComboBoxDef : public ControlDef
{
public:
    ComboBoxDef(LPCTSTR sId, LPCTSTR sCaption);

    virtual void CreateChild(rad::WindowProxy& Dlg, const RECT& dluRect, int pxCaptionOffset) override;
    virtual SIZE GetSize(rad::WindowProxy& Dlg, int w) const override;

    virtual BOOL OnCommand(WORD NotifyCode) override
    {
        switch (NotifyCode)
        {
        case CBN_SELCHANGE:
            GetFrame().SendMessage(WM_UPDATE_CMD);
            break;
        }
        return FALSE;
    }

    virtual std::wstring GetProperty() const override
    {
        return m_items[m_selected];
    }

    virtual void SetProperty(LPCTSTR sValue) override
    {
        for (size_t i = 0; i < m_items.size(); ++i)
        {
            if (m_items[i] == sValue)
            {
                m_selected = i;
                break;
            }
        }
    }

    virtual bool UseCommandLine() const override
    {
        return ControlDef::UseCommandLine() || !GetCommandValue().empty();
    }

    virtual std::wstring GetCommandValue() const override;

    void SetWidth(int pxWidth) // TODO This should be in dlu
    {
        m_pxWidth = pxWidth;
    }

    void AddItem(LPTSTR sName, LPTSTR sCommandLine, bool selected)
    {
        if (selected)
            m_selected = m_items.size();
        m_items.push_back(sName);
        m_cl.push_back(sCommandLine ? sCommandLine : _T(""));
    }

private:
    int m_pxWidth = 0;

    std::vector<std::wstring> m_items;
    std::vector<std::wstring> m_cl;
    size_t m_selected = 0;
};

class CheckBoxDef : public ControlWithChildrenDef
{
public:
    CheckBoxDef(LPCTSTR sId, LPCTSTR sText, bool bChecked, LPCTSTR sCommandLine);

    virtual void CreateChild(rad::WindowProxy& Dlg, const RECT& dluRect, int pxCaptionOffset) override
    {
        ControlWithChildrenDef::CreateChild(Dlg, dluRect, pxCaptionOffset);
        SetCheck(m_bChecked);
    }

    virtual SIZE GetSize(rad::WindowProxy& Dlg, int pxWidth) const override
    {
        SIZE s = ControlWithChildrenDef::GetSize(Dlg, pxWidth);
        SIZE c = m_Children.GetSize(Dlg, pxWidth);
        if (c.cy > 0)
            s.cy += c.cy + 5;
        if (c.cx > s.cx)
            s.cx = c.cx;
        return s;
    }

    virtual RECT GetChildrenRect(rad::WindowProxy& Dlg, const RECT& dluRect) const override
    {
        SIZE s = ControlWithChildrenDef::GetSize(Dlg, dluRect.right);
        RECT dluChildRect(dluRect);

        dluChildRect.top += s.cy + 5;
        dluChildRect.bottom -= s.cy + 5;
        dluChildRect.left += 10;
        dluChildRect.right -= 10;

        return dluChildRect;
    }

    virtual BOOL OnCommand(WORD NotifyCode) override
    {
        switch (NotifyCode)
        {
        case BN_CLICKED:
            GetFrame().SendMessage(WM_UPDATE_CMD);
            m_Children.Enable(GetCheck());
            break;
        }
        return FALSE;
    }

    virtual std::wstring GetProperty() const override
    {
        return GetCheck() ? L"checked" : L"unchecked";
    }

    virtual void SetProperty(LPCTSTR sValue) override
    {
        m_bChecked = wcscmp(L"checked", sValue) == 0;
    }

    virtual bool UseCommandLine() const override
    {
        return ControlWithChildrenDef::UseCommandLine() && GetCheck();
    }

    virtual void GetCommandLine(std::wstring& cl) const override
    {
        if (GetCheck())
            ControlWithChildrenDef::GetCommandLine(cl);
    }

private:
    void SetCheck(bool bChecked);
    bool GetCheck() const;

    bool m_bChecked;
};

class RadioDef : public ControlWithChildrenDef
{
public:
    RadioDef(LPCTSTR sId, LPCTSTR sText, bool bChecked, LPCTSTR sCommandLine);

    virtual void CreateChild(rad::WindowProxy& Dlg, const RECT& dluRect, int pxCaptionOffset) override
    {
        ControlWithChildrenDef::CreateChild(Dlg, dluRect, pxCaptionOffset);
        SetCheck(m_bChecked);
    }

    virtual SIZE GetSize(rad::WindowProxy& Dlg, int pxWidth) const override
    {
        SIZE s = ControlWithChildrenDef::GetSize(Dlg, pxWidth);
        SIZE c = m_Children.GetSize(Dlg, pxWidth);
        if (c.cy > 0)
            s.cy += c.cy + 5;
        if (c.cx > s.cx)
            s.cx = c.cx;
        return s;
    }

    virtual RECT GetChildrenRect(rad::WindowProxy& Dlg, const RECT& dluRect) const override
    {
        SIZE s = ControlWithChildrenDef::GetSize(Dlg, dluRect.right);
        RECT dluChildRect(dluRect);

        dluChildRect.top += s.cy + 5;
        dluChildRect.bottom -= s.cy + 5;
        dluChildRect.left += 10;
        dluChildRect.right -= 10;

        return dluChildRect;
    }

    virtual BOOL OnCommand(WORD NotifyCode) override
    {
        switch (NotifyCode)
        {
        case BN_CLICKED:
            GetFrame().SendMessage(WM_UPDATE_CMD);
            //m_Children.Enable(TRUE);
            break;
        }
        return FALSE;
    }

    virtual BOOL OnNotify(LPNMHDR ph) override;

    virtual std::wstring GetProperty() const override
    {
        return GetCheck() ? L"selected" : L"unselected";
    }

    virtual void SetProperty(LPCTSTR sValue) override
    {
        m_bChecked = wcscmp(L"selected", sValue) == 0;
    }

    virtual bool UseCommandLine() const override
    {
        return ControlWithChildrenDef::UseCommandLine() && GetCheck();
    }

    virtual void GetCommandLine(std::wstring& cl) const override
    {
        if (GetCheck())
            ControlWithChildrenDef::GetCommandLine(cl);
    }

private:
    void SetCheck(bool bChecked);
    bool GetCheck() const;

    bool m_bChecked;
};

// TODO Support png
class ImageDef : public ControlDef
{
public:
    ImageDef(LPCTSTR sId, LPCTSTR sFileName);

    virtual void CreateChild(rad::WindowProxy& Dlg, const RECT& dluRect, int pxCaptionOffset) override
    {
        ControlDef::CreateChild(Dlg, dluRect, pxCaptionOffset);
        m_Ctrl.SendMessage(STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) m_hBitmap);
    }

    virtual SIZE GetSize(rad::WindowProxy& Dlg, int /*w*/) const override
    {
        if (m_hBitmap != NULL)
        {
            BITMAP bm;
            GetObject(m_hBitmap, sizeof(bm), &bm);
            SIZE dluSize = { bm.bmWidth, bm.bmHeight };
            UnMapDialogSize(Dlg, &dluSize);
            return dluSize;
        }
        else
        {
            SIZE dluSize = {};
            return dluSize;
        }
    }

private:
    HBITMAP m_hBitmap;
};

class TextDef : public ControlDef
{
public:
    TextDef(LPCTSTR sId, LPCTSTR sText);

    virtual SIZE GetSize(rad::WindowProxy& Dlg, int pxWidth) const override
    {
        SIZE dluSize = { 0, 8 };
        if (!m_sText.empty())
        {
            //HFONT hFont = m_Ctrl.GetFont();
            HFONT hFont = Dlg.GetFont();
            dluSize = CalcTextSize(Dlg, hFont, m_sText.c_str(), pxWidth);
            UnMapDialogSize(Dlg, &dluSize);
        }
        return dluSize;
    }
};

class TabDef : public ControlDef
{
public:
    TabDef();

    virtual void CreateChild(rad::WindowProxy& Dlg, const RECT& dluRect, int pxCaptionOffset) override;
    virtual SIZE GetSize(rad::WindowProxy& Dlg, int pxWidth) const override;
    virtual BOOL OnNotify(LPNMHDR ph) override;

    virtual void Show(bool show) override
    {
        ControlDef::Show(show);
        ShowPage(show);
    }

    virtual void Enable(bool enable) override
    {
        ControlDef::Enable(enable);
        // TODO Enable pages ???
    }

    virtual ControlDef* Find(HWND hCtrl) override;
    virtual void GetCommandLine(std::wstring& cl) const override;

    VerticalLayout& AddPage(const std::wstring& name);

protected:
    VerticalLayout& GetPage(int i);
    void ShowPage(bool show);

private:
    class PageDialog : public rad::Dialog
    {
        virtual BOOL OnMessage(UINT Message, WPARAM wParam, LPARAM lParam) override
        {
            switch (Message)
            {
            case WM_COMMAND:
            case WM_NOTIFY:
                rad::WindowProxy(GetAncestor(GA_ROOT)).SendMessage(Message, wParam, lParam);
                break;
            }
            return Dialog::OnMessage(Message, wParam, lParam);
        }
    };

    class Page
    {
    public:
        Page(const std::wstring& sName)
            : m_sName(sName)
        {
        }

        std::wstring m_sName;
        PageDialog* m_Dlg = nullptr;
        VerticalLayout m_Children;
    };

    std::vector<Page> m_Pages;
};

class GroupDef : public ControlWithChildrenDef
{
public:
    GroupDef(LPCTSTR sId, LPCTSTR sText);

    virtual SIZE GetSize(rad::WindowProxy& Dlg, int pxWidth) const override
    {
        SIZE pxOffset = { 6, 0 };
        MapDialogSize(Dlg, &pxOffset);
        SIZE dluSize = m_Children.GetSize(Dlg, pxWidth - 2 * pxOffset.cx);
#if 0
        if (dluSize.cx > 0)
            dluSize.cx += 6 + 6;
#else
        dluSize.cx = 0; // Fill width ???
#endif
#if 1
        if (dluSize.cy > 0)
            dluSize.cy += 11 + 7;
#else
        dluSize.cy = 0; // Fill height ???
#endif
        return dluSize;
    }

    virtual RECT GetChildrenRect(rad::WindowProxy& /*Dlg*/, const RECT& dluRect) const override
    {
        RECT dluChildRect(dluRect);

        dluChildRect.left += 6;
        dluChildRect.right -= 2 * 6;
        dluChildRect.top += 11;
        dluChildRect.bottom -= 11 + 7;

        return dluChildRect;
    }

    virtual bool Fill() const { return true; }
};

class EditDef : public ControlDef
{
public:
    EditDef(LPCTSTR sId, LPCTSTR sCaption, LPCTSTR sValue, LPCTSTR sCommandLine, bool bAcceptFiles, bool bQuote);

    void SetText(LPCTSTR sValue)
    {
        m_Ctrl.SetWindowText(sValue);
    }

    virtual void CreateChild(rad::WindowProxy& Dlg, const RECT& dluRect, int pxCaptionOffset) override;

    virtual BOOL OnCommand(WORD NotifyCode) override
    {
        switch (NotifyCode)
        {
        case EN_CHANGE:
            if (!m_bIgnore)
                GetFrame().SendMessage(WM_UPDATE_CMD);
            break;
        }
        return FALSE;
    }

    virtual bool UseCommandLine() const override
    {
        return !m_bIgnore && m_Ctrl.GetWindowTextLength() > 0;
    }

    virtual std::wstring GetCommandValue() const override;

    virtual std::wstring GetProperty() const override { return GetCommandValue(); }

    virtual void SetProperty(LPCTSTR sValue) override
    {
        m_sText = sValue;
    }

    bool m_bQuote;
    bool m_bIgnore = false;
};

class ListBoxDef : public ControlDef
{
public:
    ListBoxDef(LPCTSTR sId, LPCTSTR sCommandLine, bool bMultiSelect, TCHAR cSeparator, int dluWidth, int dluHeight);

    void LoadDir(LPCTSTR sPath)
    {
        m_sLoadDir = sPath;
    }

    virtual void CreateChild(rad::WindowProxy& Dlg, const RECT& dluRect, int pxCaptionOffset) override;

    virtual BOOL OnCommand(WORD NotifyCode) override
    {
        switch (NotifyCode)
        {
        case LBN_SELCHANGE:
            GetFrame().SendMessage(WM_UPDATE_CMD);
            break;
        }
        return FALSE;
    }

    virtual std::wstring GetProperty() const override
    {
        return GetCommandValue();
    }

    virtual void SetProperty(LPCTSTR sValue) override
    {
        m_sInit = sValue;
    }

    virtual bool UseCommandLine() const override;
    virtual std::wstring GetCommandValue() const override;

private:
    TCHAR m_sSep;
    std::wstring m_sLoadDir;
    std::wstring m_sInit;
};
