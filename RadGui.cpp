#include <Rad\GUI\Dialog.h>
#include <Rad\Rect.h>

#include <CommCtrl.h>
#include <Shlwapi.h>

#include <memory>

#include "resource.h"
#include "Xml.h"
#include "Com.h"
#include "MapDialog.h"
#include "ControlDef.h"
#include "ProcessXml.h"

using namespace rad;

// TODO
// Add quotes around values
// Allow CheckBox text to be the value
// Allow Radio text to be the value
// Need a way for Group to specify the CommandLine for a set of Radio???
// Options to hide console window
// Options to not pause

// https://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing

class RadGui : public Dialog
{
public:
    RadGui(LPCTSTR sCaption, LPCTSTR sProgram, int pxWidth)
        : m_pxWidth(pxWidth)
        , m_hEdit(nullptr, nullptr, nullptr, nullptr)
        , m_hExecute(nullptr, _T("E&xecute"))
    {
        if (sCaption)
            m_sCaption = sCaption;
        if (sProgram)
            _tcscpy_s(m_sProgram, sProgram);
        if (m_sProgram[0] != _T('\"'))
            PathQuoteSpaces(m_sProgram);
        m_hEdit.m_bIgnore = true;
        m_hExecute.MakeDefault();
    }

protected:
    virtual BOOL OnInitDialog(HWND FocusControl)
    {
        if (!m_sCaption.empty())
            SetWindowText(m_sCaption.c_str());

        HINSTANCE hInstance = (HINSTANCE) GetWindowLongPtr(GWLP_HINSTANCE);
        // TODO Use PathFindOnPath
        TCHAR sIcon[MAX_PATH];
        _tcscpy_s(sIcon, m_sProgram);
        PathUnquoteSpaces(sIcon);
        WORD iIndex = 0;
        HICON hAppIcon = ExtractAssociatedIcon(hInstance, sIcon, &iIndex);
        if (hAppIcon == NULL)
            hAppIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP));
        SetIcon(ICON_SMALL, hAppIcon);

        {
            RECT pxRect;
            GetClientRect(&pxRect);
            if (m_pxWidth > 0)
                pxRect.right = m_pxWidth;
            SIZE pxSize = m_controls.GetSize(*this, pxRect.right);
            pxSize.cx += 7 + 7;
            pxSize.cy += 7 + 7;
            MapDialogSize(*this, &pxSize);
            pxRect.bottom = pxSize.cy;
            AdjustWindowRect(&pxRect, (DWORD) GetWindowLongPtr(GWL_STYLE), FALSE);

            RECT pxParent;
            WindowProxy hWndParent = GetParent();
            if (!hWndParent.IsAttached())
                hWndParent = GetDesktopWindow();
            hWndParent.GetWindowRect(&pxParent);
            int x = pxParent.left + (GetWidth(pxParent) - GetWidth(pxRect)) / 2;
            int y = pxParent.top + (GetHeight(pxParent) - GetHeight(pxRect)) / 2;

            SetWindowPos(NULL, x, y, GetWidth(pxRect), GetHeight(pxRect), SWP_NOOWNERZORDER | SWP_NOZORDER);
        }

        // TODO right is width and bottom is height
        RECT dluRect;
        GetClientRect(&dluRect);
        UnMapDialogRect(*this, &dluRect);
        dluRect.left += 7;
        dluRect.top += 7;
        dluRect.right -= 2 * 7;
        dluRect.bottom -= 2 * 7;

        int pxCaptionOffset = m_controls.GetLabelOffset(*this);
        m_controls.CreateChild(*this, dluRect, pxCaptionOffset);

        SendMessage(WM_UPDATE_CMD);

        return Dialog::OnInitDialog(FocusControl);
    }

    virtual BOOL OnCommand(WORD NotifyCode, WORD ID, HWND hWnd)
    {
        if (hWnd == m_hExecute.GetHWnd())
        {
            const std::wstring cmdorig = m_hEdit.GetCommandValue();

            // TODO Set the console window icon
            //cmd = _T("cmd /C ") + cmd + _T(" & pause");
            //cmd = _T("cmd /V:ON /C \"") + cmd + _T("\" & echo ExitCode: !ERRORLEVEL! & pause");
            //cmd = _T("\"C:\\Windows\\sysnative\\cmd.exe\" /V:ON /C \"") + cmd + _T("\" & echo ExitCode: !ERRORLEVEL! & pause");
            std::wstring cmd = m_sWrapper + std::wstring(_T(" ")) + cmdorig;

            STARTUPINFO si = { sizeof(STARTUPINFO) };
            //si.lpTitle = const_cast<LPTSTR>(cmdorig.c_str());
            si.dwFlags |= STARTF_USESTDHANDLES;
            si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
            si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
            si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
            PROCESS_INFORMATION pi = {};
            DWORD dwCreationFlags = 0;
            if (si.hStdOutput != NULL)
                dwCreationFlags |= CREATE_NO_WINDOW;
            SetEnvironmentVariable(_T("RADGUI_PAUSE"), si.hStdInput || !si.hStdOutput ? _T("true") : _T("false"));
            if (CreateProcess(nullptr, const_cast<LPTSTR>(cmd.c_str()), nullptr, nullptr, TRUE, dwCreationFlags, nullptr, nullptr, &si, &pi) == 0)
            {
                WinError we(_T("Error creating process: "));
                MessageBox(*this, we.GetString().c_str(), _T("RadGui"), MB_ICONINFORMATION | MB_OK);
            }

            // TODO Change button to Stop
            // if running kill it
            // wait for end of process on another thread

            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
            return FALSE;
        }
        else
        {
            ControlDef* cd = Find(hWnd);
            if (cd != nullptr && hWnd != m_hEdit.GetHWnd())
                return cd->OnCommand(NotifyCode);
            else
                return Dialog::OnCommand(NotifyCode, ID, hWnd);
        }
    }

    virtual BOOL OnMessage(UINT Message, WPARAM wParam, LPARAM lParam)
    {
        switch (Message)
        {
        case WM_NOTIFY:
            {
                LPNMHDR ph = (LPNMHDR) lParam;
                ControlDef* cd = Find(ph->hwndFrom);
                if (cd != nullptr)
                    return cd->OnNotify(ph);
            }
            break;

        case WM_UPDATE_CMD:
            {
                std::wstring cl = m_sProgram;
                m_controls.GetCommandLine(cl);
                m_hEdit.SetText(cl.c_str());
            }
            break;
        }
        return Dialog::OnMessage(Message, wParam, lParam);
    }

    ControlDef* Find(HWND hCtrl)
    {
        return m_controls.Find(hCtrl);
    }

public:
    VerticalLayout m_controls;
    std::wstring m_sCaption;
    TCHAR m_sWrapper[MAX_PATH] = _T("");
    TCHAR m_sProgram[MAX_PATH] = _T("");
    int m_pxWidth;

    EditDef m_hEdit;
    ButtonDef m_hExecute;
};

int CALLBACK WinMain(
    _In_ HINSTANCE hInstance,
    _In_ HINSTANCE /*hPrevInstance*/,
    _In_ LPSTR     /*lpCmdLine*/,
    _In_ int       /*nCmdShow*/
)
{
    CoInit coinit;

    INITCOMMONCONTROLSEX iccex = { sizeof(INITCOMMONCONTROLSEX) };
    iccex.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&iccex);

    try
    {
        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLine(), &argc);
        // TODO LocalFree(argv);

        LPCWSTR file = nullptr;
        std::map<std::wstring, std::wstring> properties;

        for (int i = 1; i < argc; ++i)
        {
            LPCWSTR arg = argv[i];
            if (_wcsnicmp(arg, L"/p:", 3) == 0)
            {
                LPCWSTR prop = arg + 3;
                LPCWSTR eq = wcschr(prop, L'=');
                LPCWSTR value = eq + 1;
                properties[std::wstring(prop, eq)] = value;
            }
            else if (file == nullptr)
                file = arg;
            else
                throw std::exception("Too many paramters");
        }

#if 0
        if (argc <= 1)
        {
            //file = L"frog.frontend";
            //file = L"lame.frontend";
            file = L"msbuild.frontend";
            //file = L"test.frontend";
        }
#endif

        if (file == nullptr)
            throw std::exception("Must specify file");

        TCHAR sExeDir[MAX_PATH];
        GetModuleFileName(hInstance, sExeDir, ARRAYSIZE(sExeDir));
        PathRemoveFileSpec(sExeDir);

        // TODO Load from resource

        MSXML2::IXMLDOMDocumentPtr pXMLDom = LoadXml(file);

        MSXML2::IXMLDOMElementPtr pXMLRoot(pXMLDom->GetdocumentElement());

        _bstr_t bstrCaption = GetAttribute(pXMLRoot, _T("caption"));
        _bstr_t bstrWidth = GetAttribute(pXMLRoot, _T("width"));
        _bstr_t bstrProgram = GetAttribute(pXMLRoot, _T("program"));

        // TODO Support icon
        // TODO Add help page
        // TODO Add path of bstrProgram to PATH so we can just use the filename in the text
        // TODO Use PathQuoteSpaces on bstrProgram
        // TODO width is in DLU if bstrName is dialog

        RadGui dlg(bstrCaption, bstrProgram, !bstrWidth ? 200 : _wtoi(bstrWidth));
        _tcscpy_s(dlg.m_sWrapper, sExeDir);
        PathAppend(dlg.m_sWrapper, _T("RadGuiRun.bat"));
        PathQuoteSpaces(dlg.m_sWrapper);
        // TODO Get wrapper from xml file

        TCHAR sDir[MAX_PATH];
        _tcscpy_s(sDir, file);
        PathRemoveFileSpec(sDir);

        _bstr_t bstrName = pXMLRoot->GetbaseName();
        if (bstrName == L"form")
            ProcessFormControls(dlg.m_controls, pXMLRoot, sDir);
        else if (bstrName == L"dialog")
            ProcessDialogControls(dlg.m_controls, pXMLRoot, sDir);
        else
            // TODO Unknown node bstrName
            throw std::exception("RootNode");

        for (auto it = properties.begin(); it != properties.end(); ++it)
        {
            ControlDef* cd = dlg.m_controls.FindId(it->first.c_str());
            if (cd != nullptr)
            {
                EditDef* ed = dynamic_cast<EditDef*>(cd);
                if (ed != nullptr)
                    ed->SetValue(it->second.c_str());
            }
        }

        {
            std::unique_ptr<HorizontalLayout> hs(new HorizontalLayout());
            hs->Add(&dlg.m_hEdit);
            hs->Add(&dlg.m_hExecute);
            dlg.m_controls.Add(hs.release());
        }

        // TODO Set control values from command line

        return (int) dlg.DoModal(hInstance, IDD_DIALOG1, NULL);
    }
    catch (const std::exception& e)
    {
        MessageBoxA(NULL, e.what(), "RadGui", MB_ICONERROR | MB_OK);
        return -1;
    }
    catch (const _com_error& e)
    {
        MessageBoxW(NULL, e.Description(), L"RadGui", MB_ICONERROR | MB_OK);
        return e.Error();
    }
    catch (const MSXML2::IXMLDOMParseErrorPtr& pXMLErr)
    {
        _bstr_t bstrErr = pXMLErr->Getreason();
        MessageBox(NULL, bstrErr, _T("RadGui"), MB_ICONERROR | MB_OK);
        return pXMLErr->GeterrorCode() + 1000;
    }
}
