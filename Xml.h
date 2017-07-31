#pragma once

#import <msxml6.dll> exclude("ISequentialStream", "_FILETIME")
//#include <objbase.h>
//#include <msxml6.h>

inline MSXML2::IXMLDOMDocumentPtr LoadXml(PCWSTR wszFile)
{
    MSXML2::IXMLDOMDocumentPtr pXMLDom(__uuidof(MSXML2::DOMDocument60));
    pXMLDom->Putasync(VARIANT_FALSE);
    pXMLDom->PutvalidateOnParse(VARIANT_FALSE);
    pXMLDom->PutresolveExternals(VARIANT_FALSE);
    pXMLDom->PutpreserveWhiteSpace(VARIANT_TRUE);

    _variant_t varFileName = wszFile;
    VARIANT_BOOL varStatus = pXMLDom->load(varFileName);

    if (varStatus == VARIANT_TRUE)
    {
        return pXMLDom;
    }
    else
    {
        MSXML2::IXMLDOMParseErrorPtr pXMLErr(pXMLDom->GetparseError());
        throw pXMLErr;
    }
}

inline _bstr_t GetAttribute(MSXML2::IXMLDOMNode* pXMLNode, LPCWSTR name)
{
    MSXML2::IXMLDOMNamedNodeMapPtr pXMLAttributes(pXMLNode->Getattributes());

    MSXML2::IXMLDOMNodePtr pXMLAttr(pXMLAttributes->getNamedItem(const_cast<LPWSTR>(name)));

    _bstr_t bstrAttrValue;
    if (pXMLAttr)
        bstrAttrValue = pXMLAttr->Gettext();
        //pXMLAttr->GetnodeValue();   // TODO Use this instead???

    return bstrAttrValue;
}
