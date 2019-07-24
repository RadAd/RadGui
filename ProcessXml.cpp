#include "ProcessXml.h"

#include <memory>
#include <Shlwapi.h>

#include "ControlDef.h"
#include "Xml.h"

void ProcessColumns(HorizontalLayout& controls, MSXML2::IXMLDOMNode* pXMLNode, LPCTSTR sDir)
{
    MSXML2::IXMLDOMNodeListPtr pXMLChildren(pXMLNode->GetchildNodes());

    long length = pXMLChildren->Getlength();
    for (int i = 0; i < length; ++i)
    {
        MSXML2::IXMLDOMNodePtr pXMLChildNode(pXMLChildren->Getitem(i));

        MSXML2::DOMNodeType type = pXMLChildNode->GetnodeType();

        if (type == NODE_ELEMENT)
        {
            _bstr_t bstrName = pXMLChildNode->GetbaseName();

            if (bstrName == L"column")
            {
                std::unique_ptr<VerticalLayout> vs(new VerticalLayout());
                ProcessFormControls(*vs.get(), pXMLChildNode, sDir);
                controls.Add(vs.release());
            }
            else
                // TODO Unknown node bstrName
                throw std::exception("ProcessColumns");
        }
    }
}

void ProcessRadioGroup(VerticalLayout& controls, MSXML2::IXMLDOMNode* pXMLNode)
{
    MSXML2::IXMLDOMNodeListPtr pXMLChildren(pXMLNode->GetchildNodes());

    long length = pXMLChildren->Getlength();
    for (int i = 0; i < length; ++i)
    {
        MSXML2::IXMLDOMNodePtr pXMLChildNode(pXMLChildren->Getitem(i));

        MSXML2::DOMNodeType type = pXMLChildNode->GetnodeType();

        if (type == NODE_ELEMENT)
        {
            _bstr_t bstrName = pXMLChildNode->GetbaseName();

            if (bstrName == L"item" || bstrName == L"button")
            {
                _bstr_t bstrId = GetAttribute(pXMLChildNode, _T("id"));
                _bstr_t bstrCaption = GetAttribute(pXMLChildNode, _T("caption"));
                _bstr_t bstrSelected = GetAttribute(pXMLChildNode, _T("selected"));
                _bstr_t bstrCommandLine = GetAttribute(pXMLChildNode, _T("commandline"));
                controls.Add(new RadioDef(bstrId, bstrCaption, !!bstrSelected && bstrSelected == L"true", bstrCommandLine));
            }
            else
                // TODO Unknown node bstrName
                throw std::exception("ProcessRadioGroup");
        }
    }
}

void ProcessComboBox(ComboBoxDef& cbdef, MSXML2::IXMLDOMNode* pXMLNode)
{
    MSXML2::IXMLDOMNodeListPtr pXMLChildren(pXMLNode->GetchildNodes());

    long length = pXMLChildren->Getlength();
    for (int i = 0; i < length; ++i)
    {
        MSXML2::IXMLDOMNodePtr pXMLChildNode(pXMLChildren->Getitem(i));

        MSXML2::DOMNodeType type = pXMLChildNode->GetnodeType();

        if (type == NODE_ELEMENT)
        {
            _bstr_t bstrName = pXMLChildNode->GetbaseName();

            if (bstrName == L"item")
            {
                _bstr_t bstrCaption = GetAttribute(pXMLChildNode, _T("caption"));
                _bstr_t bstrSelected = GetAttribute(pXMLChildNode, _T("selected"));
                _bstr_t bstrCommandLine = GetAttribute(pXMLChildNode, _T("commandline"));
                cbdef.AddItem(bstrCaption, bstrCommandLine, !!bstrSelected && bstrSelected == L"true");
            }
            else
                // TODO Unknown node bstrName
                throw std::exception("ProcessRadioGroup");
        }
    }
}

void ProcessTabSet(TabDef& tabdef, MSXML2::IXMLDOMNode* pXMLNode, LPCTSTR sDir)
{
    MSXML2::IXMLDOMNodeListPtr pXMLChildren(pXMLNode->GetchildNodes());

    long length = pXMLChildren->Getlength();
    for (int i = 0; i < length; ++i)
    {
        MSXML2::IXMLDOMNodePtr pXMLChildNode(pXMLChildren->Getitem(i));

        MSXML2::DOMNodeType type = pXMLChildNode->GetnodeType();

        if (type == NODE_ELEMENT)
        {
            _bstr_t bstrName = pXMLChildNode->GetbaseName();

            if (bstrName == L"tabsheet")
            {
                _bstr_t bstrCaption = GetAttribute(pXMLChildNode, _T("caption"));
                _bstr_t bstrActive = GetAttribute(pXMLChildNode, _T("active"));
                ProcessFormControls(tabdef.AddPage(bstrCaption.GetBSTR()), pXMLChildNode, sDir);
            }
            else
                // TODO Unknown node bstrName
                throw std::exception("ProcessTabSet");
        }
    }
}

void ProcessFormControls(ChildrenLayout& controls, MSXML2::IXMLDOMNode* pXMLNode, LPCTSTR sDir)
{
    MSXML2::IXMLDOMNodeListPtr pXMLChildren(pXMLNode->GetchildNodes());

    long length = pXMLChildren->Getlength();
    for (int i = 0; i < length; ++i)
    {
        MSXML2::IXMLDOMNodePtr pXMLChildNode(pXMLChildren->Getitem(i));

        MSXML2::DOMNodeType type = pXMLChildNode->GetnodeType();

        if (type == NODE_ELEMENT)
        {
            //_bstr_t bstrName = pXMLChildNode->GetbaseName();

            _bstr_t bstrType = GetAttribute(pXMLChildNode, _T("type"));
            _bstr_t bstrId = GetAttribute(pXMLChildNode, _T("id"));

            if (bstrType == L"image")
            {
                _bstr_t bstrFilename = GetAttribute(pXMLChildNode, _T("filename"));
                TCHAR sFile[MAX_PATH];
                PathCombine(sFile, sDir, bstrFilename);
                controls.Add(new ImageDef(bstrId, sFile));
            }
            else if (bstrType == L"radiogroup")
            {
                _bstr_t bstrCaption = GetAttribute(pXMLChildNode, _T("caption"));
                std::unique_ptr<GroupDef> gd(new GroupDef(bstrId, bstrCaption));
                ProcessRadioGroup(gd->m_Children, pXMLChildNode);
                controls.Add(gd.release());
            }
            else if (bstrType == L"combobox")
            {
                _bstr_t bstrCaption = GetAttribute(pXMLChildNode, _T("caption"));
                _bstr_t bstrWidth = GetAttribute(pXMLChildNode, _T("width"));
                std::unique_ptr<ComboBoxDef> cbd(new ComboBoxDef(bstrId, bstrCaption));
                if (!!bstrWidth)
                    cbd->SetWidth(_wtoi(bstrWidth));
                ProcessComboBox(*cbd, pXMLChildNode);
                controls.Add(cbd.release());
            }
            else if (bstrType == L"groupbox")
            {
                _bstr_t bstrCaption = GetAttribute(pXMLChildNode, _T("caption"));
                std::unique_ptr<GroupDef> gd(new GroupDef(bstrId, bstrCaption));
                ProcessFormControls(gd->m_Children, pXMLChildNode, sDir);
                controls.Add(gd.release());
            }
            else if (bstrType == L"text")
            {
                _bstr_t bstrCaption = GetAttribute(pXMLChildNode, _T("caption"));
                controls.Add(new TextDef(bstrId, bstrCaption));
            }
            else if (bstrType == L"edit")
            {
                _bstr_t bstrCaption = GetAttribute(pXMLChildNode, _T("caption"));
                _bstr_t bstrDefault = GetAttribute(pXMLChildNode, _T("default"));
                _bstr_t bstrCommandLine = GetAttribute(pXMLChildNode, _T("commandline"));
                _bstr_t bstrQuote = GetAttribute(pXMLChildNode, _T("quote"));
                controls.Add(new EditDef(bstrId, bstrCaption, bstrDefault, bstrCommandLine, bstrQuote == _T("true")));
            }
            else if (bstrType == L"checkbox")
            {
                _bstr_t bstrCaption = GetAttribute(pXMLChildNode, _T("caption"));
                _bstr_t bstrChecked = GetAttribute(pXMLChildNode, _T("checked"));
                _bstr_t bstrCommandLine = GetAttribute(pXMLChildNode, _T("commandline"));
                controls.Add(new CheckBoxDef(bstrId, bstrCaption, !!bstrChecked && bstrChecked == L"true", bstrCommandLine));
            }
            else if (bstrType == L"tabset")
            {
                std::unique_ptr<TabDef> td(new TabDef());
                ProcessTabSet(*td.get(), pXMLChildNode, sDir);
                controls.Add(td.release());
            }
            else if (bstrType == L"spacer")
            {
                // TODO
            }
            else if (bstrType == L"columnset")
            {
                std::unique_ptr<HorizontalLayout> hs(new HorizontalLayout());
                ProcessColumns(*hs.get(), pXMLChildNode, sDir);
                controls.Add(hs.release());
            }
            else if (bstrType == L"newextdropfiles")
            {
                // TODO
            }
            else if (bstrType == L"exitcode")
            {
                // TODO
            }
            else if (bstrType == L"filefilter")
            {
                // TODO
            }
            else if (bstrType == L"dropfiles")
            {
                // TODO
            }
            else if (bstrType == L"selectdir")
            {
                std::unique_ptr<HorizontalLayout> hs(new HorizontalLayout());
                _bstr_t bstrCaption = GetAttribute(pXMLChildNode, _T("caption"));
                _bstr_t bstrDefault = GetAttribute(pXMLChildNode, _T("default"));
                _bstr_t bstrCommandLine = GetAttribute(pXMLChildNode, _T("commandline"));
                _bstr_t bstrQuote = GetAttribute(pXMLChildNode, _T("quote"));
                hs->Add(new EditDef(bstrId, bstrCaption, bstrDefault, bstrCommandLine, bstrQuote == _T("true")));
                hs->Add(new SelectDirDef());
                controls.Add(hs.release());
            }
            else if (bstrType == L"selectfile")
            {
                // Non-standard for frontend
                std::unique_ptr<HorizontalLayout> hs(new HorizontalLayout());
                _bstr_t bstrCaption = GetAttribute(pXMLChildNode, _T("caption"));
                _bstr_t bstrDefault = GetAttribute(pXMLChildNode, _T("default"));
                _bstr_t bstrCommandLine = GetAttribute(pXMLChildNode, _T("commandline"));
                _bstr_t bstrQuote = GetAttribute(pXMLChildNode, _T("quote"));
                hs->Add(new EditDef(bstrId, bstrCaption, bstrDefault, bstrCommandLine, bstrQuote == _T("true")));
                _bstr_t bstrFilter = GetAttribute(pXMLChildNode, _T("filter"));
                controls.Add(new SelectFileDef(bstrFilter));
                controls.Add(hs.release());
            }
            else
                // TODO Unknown node bstrType
                throw std::exception("ProcessFormControls");
        }
    }
}

void ProcessDialogControls(ChildrenLayout& controls, MSXML2::IXMLDOMNode* pXMLNode, LPCTSTR sDir)
{
    MSXML2::IXMLDOMNodeListPtr pXMLChildren(pXMLNode->GetchildNodes());

    long length = pXMLChildren->Getlength();
    for (int i = 0; i < length; ++i)
    {
        MSXML2::IXMLDOMNodePtr pXMLChildNode(pXMLChildren->Getitem(i));

        MSXML2::DOMNodeType type = pXMLChildNode->GetnodeType();

        if (type == NODE_ELEMENT)
        {
            _bstr_t bstrName = pXMLChildNode->GetbaseName();
            _bstr_t bstrId = GetAttribute(pXMLChildNode, _T("id"));

            if (bstrName == L"groupbox")
            {
                _bstr_t bstrCaption = GetAttribute(pXMLChildNode, _T("caption"));
                std::unique_ptr<GroupDef> gd(new GroupDef(bstrId, bstrCaption));
                ProcessDialogControls(gd->m_Children, pXMLChildNode, sDir);
                controls.Add(gd.release());
            }
            else if (bstrName == L"radio")
            {
                _bstr_t bstrCaption = GetAttribute(pXMLChildNode, _T("caption"));
                _bstr_t bstrSelected = GetAttribute(pXMLChildNode, _T("selected"));
                _bstr_t bstrCommandLine = GetAttribute(pXMLChildNode, _T("commandline"));
                std::unique_ptr<RadioDef> rd(new RadioDef(bstrId, bstrCaption, !!bstrSelected && bstrSelected == L"true", bstrCommandLine));
                ProcessDialogControls(rd->m_Children, pXMLChildNode, sDir);
                controls.Add(rd.release());
            }
            else if (bstrName == L"checkbox")
            {
                _bstr_t bstrCaption = GetAttribute(pXMLChildNode, _T("caption"));
                _bstr_t bstrSelected = GetAttribute(pXMLChildNode, _T("selected"));
                _bstr_t bstrCommandLine = GetAttribute(pXMLChildNode, _T("commandline"));
                std::unique_ptr<CheckBoxDef> cbd(new CheckBoxDef(bstrId, bstrCaption, !!bstrSelected && bstrSelected == L"true", bstrCommandLine));
                ProcessDialogControls(cbd->m_Children, pXMLChildNode, sDir);
                controls.Add(cbd.release());
            }
            else if (bstrName == L"edit")
            {
                _bstr_t bstrCaption = GetAttribute(pXMLChildNode, _T("caption"));
                _bstr_t bstrDefault = GetAttribute(pXMLChildNode, _T("default"));
                _bstr_t bstrCommandLine = GetAttribute(pXMLChildNode, _T("commandline"));
                _bstr_t bstrQuote = GetAttribute(pXMLChildNode, _T("quote"));
                controls.Add(new EditDef(bstrId, bstrCaption, bstrDefault, bstrCommandLine, bstrQuote == _T("true")));
            }
            else if (bstrName == L"selectfile")
            {
                _bstr_t bstrFilter = GetAttribute(pXMLChildNode, _T("filter"));
                controls.Add(new SelectFileDef(bstrFilter));
            }
            else if (bstrName == L"selectdir")
            {
                controls.Add(new SelectDirDef());
            }
            else if (bstrName == L"horzlayout")
            {
                std::unique_ptr<HorizontalLayout> hs(new HorizontalLayout());
                ProcessDialogControls(*hs.get(), pXMLChildNode, sDir);
                controls.Add(hs.release());
            }
            else if (bstrName == L"vertlayout")
            {
                std::unique_ptr<VerticalLayout> hs(new VerticalLayout());
                ProcessDialogControls(*hs.get(), pXMLChildNode, sDir);
                controls.Add(hs.release());
            }
            else
                // TODO Unknown node bstrName
                throw std::exception("ProcessDialogControls");
        }
    }
}
