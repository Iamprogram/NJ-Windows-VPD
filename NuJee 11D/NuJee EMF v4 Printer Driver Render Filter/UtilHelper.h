//
// File Name:
//
//    UtilHelper.h
//
// Abstract:
//
//    helper routines.
//

#pragma once
#include "IniHelper.h"
//using namespace mINI;

namespace NuJee_EMF_v4_Printer_Driver_Render_Filter
{
    class UtilHelper 
    {
    public:

        static bool  GetOutputTempFolderFileName(_Out_ WCHAR* pRasterFile);

        static bool  OutputJobSumFilePost(const int IdPagesCount, _Out_ WCHAR* pFile, mINI::INIStructure& inIni);

        static bool  OutputJobSumJasonFilePost(const int IdPagesCount, _In_ WCHAR* pFile, mINI::INIStructure& inIni);
        static bool  OutputJobWorkFolder(_Out_ WCHAR* pPrinterWorkingFolder, _Out_ WCHAR* pPrinterJobUNIXTIME);

        static bool  OutputJobProductFile(_Out_ WCHAR* pPrinterPDFFileName, _Out_ WCHAR* pPrinterIniFileName);
    };
} // namespace NuJee_EMF_v4_Printer_Driver_Render_Filter
