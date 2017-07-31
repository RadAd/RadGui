#pragma once

#define CHK_HR(stmt)        do { hr=(stmt); if (FAILED(hr)) throw _com_error(hr); } while(0)

class CoInit
{
public:
    CoInit()
    {
        HRESULT hr = S_OK;
        CHK_HR(CoInitialize(NULL));
    }

    ~CoInit()
    {
        CoUninitialize();
    }
};
