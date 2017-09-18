#pragma once

#include <comutil.h>

#include "Xml.h"

class ChildrenLayout;

inline bool operator==(const _bstr_t& s1, LPCWSTR s2)
{
    if (!s1)
        return false;
    else
        return wcscmp(s1, s2) == 0;
}

void ProcessFormControls(ChildrenLayout& controls, MSXML2::IXMLDOMNode* pXMLNode, LPCTSTR sDir);
void ProcessDialogControls(ChildrenLayout& controls, MSXML2::IXMLDOMNode* pXMLNode, LPCTSTR sDir);
