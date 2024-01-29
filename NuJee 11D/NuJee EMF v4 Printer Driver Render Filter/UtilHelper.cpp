//
// File Name:
//
//    UtilHelper.cpp
//
// Abstract:
//
//    Object model conversion routines. This class provides routines
//      to convert from filter pipeline objects into Xps Object Model 
//      objects.
//

#include "precomp.h"
#include "WppTrace.h"
#include "CustomWppCommands.h"
#include "Exception.h"
#include "filtertypes.h"
#include "UnknownBase.h"

#include "RenderFilter.h"
#include "OMConvertor.h"
#include "rasinterface.h"

#include "UtilHelper.h"
#include "UtilHelper.tmh"

// header files for file I/O
//#include <iostream>
//#include <fstream>
#include "IniHelper.h"
using namespace mINI;

#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <time.h>
//#include <Cfgmgr32.h>
#include "Registry.h"
using namespace m4x1m1l14n;

namespace NuJee_EMF_v4_Printer_Driver_Render_Filter
{
    bool  ReadRegistryKey(_Out_ WCHAR* pSpoolerPrinterWorkingPath, _Out_ WCHAR* pPendingFolderPath) {
        
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"ReadRegistryKey::  ..............................................");

        WCHAR SpoolerPrinterWorking[MAX_PATH];
        WCHAR PendingFolderPath[MAX_PATH];
        wcscpy_s(SpoolerPrinterWorking, L"C:\\Program Files\\NuJeePrint\\NuJeePrint for Windows\\spooler");
        wcscpy_s(PendingFolderPath, L"C:\\Program Files\\NuJeePrint\\NuJeePrint for Windows\\spooler\\pending");

        //WCHAR wcSource[MAX_PATH];
        //wcscpy_s(wcSource, L"C:\\NuJee\\output");
        //wcscat_s(wcSource, L"\\outputtry.ini");
 
        //wcscpy_s(pStringPath, iSize / (sizeof(wchar_t)), wcSource);
        //std::wstring wsSource(wcSource);
        //std::string inifile(wsSource.begin(), wsSource.end());
        // first, create a file instance
        //mINI::INIFile file(inifile);

        bool bSpoolerPrinterWorking = false;
        bool bPendingFolderPath = false;
        std::wstring stringSpoolerPrinterWorking;
        std::wstring stringPendingFolderPath;
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"ReadRegistryKey:: start.............");
        try {
            auto key = Registry::LocalMachine->Open(L"SYSTEM\\CurrentControlSet\\Control\\Print\\Printers\\NuJee EMF v4\\WorkSpace");
            if (key->HasValue(L"SpoolerWorkSpace")){
                stringSpoolerPrinterWorking = key->GetString(L"SpoolerWorkSpace");
                bSpoolerPrinterWorking = true;
            }
            if (key->HasValue(L"PendingFolderPath"))
            {
                stringPendingFolderPath = key->GetString(L"PendingFolderPath");
                bool bPendingFolderPath = true;
            }
        }
        catch (const std::exception&)
        {
            // handle thrown exception
            DoTraceMessage(RENDERFILTER_TRACE_INFO, L"ReadRegistryKey:: exception");
        }
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"ReadRegistryKey:: pass reading");
        if (pSpoolerPrinterWorkingPath != NULL) {
            if (bSpoolerPrinterWorking && stringSpoolerPrinterWorking.length() > 5) {
                DoTraceMessage(RENDERFILTER_TRACE_INFO, L"ReadRegistryKey:: bSpoolerPrinterWorking= %ws", stringSpoolerPrinterWorking.c_str());
                UINT iSize = sizeof(SpoolerPrinterWorking);
                wcscpy_s(pSpoolerPrinterWorkingPath, stringSpoolerPrinterWorking.length(), stringSpoolerPrinterWorking.c_str());
                DoTraceMessage(RENDERFILTER_TRACE_INFO, L"ReadRegistryKey:: pSpoolerPrinterWorkingPath  %ws", SpoolerPrinterWorking);
            }
            else {
                DoTraceMessage(RENDERFILTER_TRACE_INFO, L"ReadRegistryKey:: no bSpoolerPrinterWorking= %ws", stringSpoolerPrinterWorking.c_str());
                UINT iSize = sizeof(SpoolerPrinterWorking);
                wcscpy_s(pSpoolerPrinterWorkingPath, iSize / (sizeof(wchar_t)), SpoolerPrinterWorking);
            }
        }
        if (pPendingFolderPath != NULL) {
            if (bPendingFolderPath && stringPendingFolderPath.length() > 5) {
                DoTraceMessage(RENDERFILTER_TRACE_INFO, L"ReadRegistryKey:: bPendingFolderPath= %ws", stringPendingFolderPath.c_str());
                wcscpy_s(pPendingFolderPath, stringPendingFolderPath.length(), stringPendingFolderPath.c_str());
                DoTraceMessage(RENDERFILTER_TRACE_INFO, L"ReadRegistryKey:: copy ok ");
            }
            else {
                DoTraceMessage(RENDERFILTER_TRACE_INFO, L"ReadRegistryKey:: no bPendingFolderPath= %ws", stringPendingFolderPath.c_str());
                UINT iSize = sizeof(PendingFolderPath);
                wcscpy_s(pPendingFolderPath, iSize / (sizeof(wchar_t)), PendingFolderPath);
            }
        }
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"ReadRegistryKey:: pass");
        return true;
    }

    bool UtilHelper::GetOutputTempFolderFileName(_Out_ WCHAR* pPrinterOutputFileTempFolder)
    {
        WCHAR wcSource[MAX_PATH];
        //wcscpy_s(wcSource, L"C:\\NuJee\\output");
        //wcscat_s(wcSource, L"\\outputtry");
        bool ret=OutputJobWorkFolder(NULL, wcSource);
        if (ret) {
            UINT iSize = sizeof(wcSource);
            wcscpy_s(pPrinterOutputFileTempFolder, iSize / (sizeof(wchar_t)), wcSource);
        }

        return ret;
    }

    bool  OutputJobSumFilePre(_Out_ WCHAR* pFile) {
        WCHAR wcSource[MAX_PATH];
        wcscpy_s(wcSource, L"C:\\NuJee\\output");
        wcscat_s(wcSource, L"\\outputtry.ini");
        UINT iSize = sizeof(wcSource);
        wcscpy_s(pFile, iSize / (sizeof(wchar_t)), wcSource);
        //std::wstring wsSource(wcSource);
        //std::string inifile(wsSource.begin(), wsSource.end());
        // first, create a file instance
        //mINI::INIFile file(inifile);

        return true;
    }

    bool  OutputJobSumFile(_In_ WCHAR* pFile) {
        WCHAR wcSource[MAX_PATH];
        wcscpy_s(wcSource, L"C:\\NuJee\\output");
        wcscat_s(wcSource, L"\\outputtry.Ini");

        return true;
    }

    bool  UtilHelper::OutputJobSumFilePost(const int IdPagesCount, _In_ WCHAR* pFile, mINI::INIStructure& inIni) {
        WCHAR wcSource[MAX_PATH];
        wcscpy_s(wcSource, L"C:\\NuJee\\output");
        wcscat_s(wcSource, L"\\outputtry.ini");

        //std::wstring wsSource(wcSource);
        //std::string inifile2(wsSource.begin(), wsSource.end());
        std::string inifile("C:\\NuJee\\output\\outputtry.ini");
        mINI::INIFile file(inifile);

        // next, create a structure that will hold data
        mINI::INIStructure ini;
        // add a new entry ["Device"]["DeviceName"]
        std::string iniDeviceName("NuJeePrint Virtual Driver");
        ini["Device"]["DeviceName"] = iniDeviceName;
        ini["Document"]["Name"] = inIni["Document"]["Name"];
        // JobID=3
        // MachineName = \\UShallNotPassDe
        // UserName = Alang
        // NotifyName = Alang
        // SessionID = 2
        // SecurityID = S - 1 - 5 - 21 - 2292555829 - 420465608 - 1375873136 - 500
        ini["Document"]["HorizontalResolution"] = inIni["Document"]["HorizontalResolution"];
        ini["Document"]["VerticalResolution"] = inIni["Document"]["VerticalResolution"];
        ini["Document"]["Orientation"] = inIni["Document"]["Orientation"];
        ini["Document"]["PaperSize"] = inIni["Document"]["PaperSize"];
        ini["Document"]["PaperSizeName"] = inIni["Document"]["PaperSizeName"];
        ini["Document"]["PaperWidth"] = inIni["Document"]["PaperWidth"];
        ini["Document"]["PaperLength"] = inIni["Document"]["PaperLength"];
        ini["Document"]["Bin"] = inIni["Document"]["Bin"];
        ini["Document"]["BinName"] = inIni["Document"]["BinName"];
        ini["Document"]["Copies"] = inIni["Document"]["Copies"];
        ini["Document"]["Collate"] = inIni["Document"]["Collate"];
        ini["Document"]["Duplex"] = inIni["Document"]["Duplex"];
        ini["Document"]["Staple"] = inIni["Document"]["Staple"];
        ini["Document"]["Color"] = inIni["Document"]["Color"];
        std::string iniStatus("Printed");
        ini["Document"]["Status"] = iniStatus;
        ini["Document"]["Pages"] = std::to_string(IdPagesCount);
        ini["Document"]["PagesPerSheet"] = inIni["Document"]["PagesPerSheet"];
        //[Spool]
        ini["Spool"]["Type"] = "XPS";
        //[PNG]
        ini["PNG"]["Count"] = std::to_string(IdPagesCount); 
        for (int i = 0; i < IdPagesCount; i++) {
            std::string iniDoc("File");
            std::string retiniDoc = iniDoc + std::to_string(i);
            ini["PNG"][retiniDoc] = inIni["PNG"][retiniDoc];
        }
        //[EMF]
        ini["EMF"]["Count"] = std::to_string(IdPagesCount);
        for (int i = 0; i < IdPagesCount; i++) {
            std::string iniDoc("File");
            std::string retiniDoc = iniDoc + std::to_string(i);
            ini["EMF"][retiniDoc] = inIni["EMF"][retiniDoc];
        }
        //[PDF] OK
        ini["PDF"]["Count"] = "1";
        ini["PDF"]["File0"] = inIni["PDF"]["File0"];
        //[INI] OK
        ini["INI"]["Count"] = "1";
        ini["INI"]["File0"] = inifile;

        file.write(ini,true);
        return true;
    }

    bool  UtilHelper::OutputJobWorkFolder(_Out_ WCHAR* pPrinterWorkingFolder, _Out_ WCHAR* pPrinterPendingFolder) {
        if (pPrinterWorkingFolder == NULL && pPrinterPendingFolder == NULL) {
            return true;
        }
        if (pPrinterWorkingFolder != NULL) {
            WCHAR PrinterWorkingFolder[MAX_PATH];
            //wcscpy_s(PrinterWorkingFolder, L"C:\\Program Files\\NuJeePrint\\NuJeePrint for Windows\\spooler");
            ReadRegistryKey(PrinterWorkingFolder, NULL);
            UINT iSize = sizeof(PrinterWorkingFolder);
            wcscpy_s(pPrinterWorkingFolder, iSize / (sizeof(wchar_t)), PrinterWorkingFolder);
        }

        if (pPrinterPendingFolder != NULL) {
            WCHAR PrinterWorkPendingFolder[MAX_PATH];
            //wcscpy_s(PrinterWorkPendingFolder, L"C:\\Program Files\\NuJeePrint\\NuJeePrint for Windows\\spooler\\pending");
            ReadRegistryKey(NULL, PrinterWorkPendingFolder);
            UINT iSize = sizeof(PrinterWorkPendingFolder);
            wcscpy_s(pPrinterPendingFolder, iSize / (sizeof(wchar_t)), PrinterWorkPendingFolder);
        }

        return true;
    }


    bool  UtilHelper::OutputJobProductFile(_Out_ WCHAR* pPrinterPDFFileName, _Out_ WCHAR* pPrinterIniFileName) {
        if (pPrinterPDFFileName == NULL && pPrinterIniFileName == NULL) {
            return true;
        }

        WCHAR PrinterWorkingPDFFileName[MAX_PATH];
        WCHAR PrinterWorkingIniFileName[MAX_PATH];
        //wcscpy_s(PrinterWorkingPDFFileName, L"C:\\Program Files\\NuJeePrint\\NuJeePrint for Windows\\spooler");
        //wcscpy_s(PrinterWorkingIniFileName, L"C:\\Program Files\\NuJeePrint\\NuJeePrint for Windows\\spooler");
        ReadRegistryKey(PrinterWorkingPDFFileName, NULL);
        ReadRegistryKey(PrinterWorkingIniFileName, NULL);

        struct tm newtime;
        __int64 ltime;
        _time64(&ltime);
        WCHAR wcTempSource[26];
        errno_t err = localtime_s(&newtime, &ltime);
        if (err) {
            return false;
        }
        wcsftime(wcTempSource, 26, L"%Y%m%d%H%M%S", &newtime);

        if (pPrinterPDFFileName != NULL) {
            wcscat_s(PrinterWorkingPDFFileName, L"\\");
            wcscat_s(PrinterWorkingPDFFileName, wcTempSource);
            wcscat_s(PrinterWorkingPDFFileName, L"_NuJeePrint.apspool");
            UINT iSize = sizeof(PrinterWorkingPDFFileName);
            wcscpy_s(pPrinterPDFFileName, iSize / (sizeof(wchar_t)), PrinterWorkingPDFFileName);
        }
        if (pPrinterIniFileName!=NULL) {
            wcscat_s(PrinterWorkingIniFileName, L"\\");
            wcscat_s(PrinterWorkingIniFileName, wcTempSource);
            wcscat_s(PrinterWorkingIniFileName, L"_NuJeePrint-job.json");
            UINT iSize = sizeof(PrinterWorkingIniFileName);
            wcscpy_s(pPrinterIniFileName, iSize / (sizeof(wchar_t)), PrinterWorkingIniFileName);
        }

        return true;
    }


    bool  UtilHelper::OutputJobSumJasonFilePost(const int IdPagesCount, _In_ WCHAR* pFile, mINI::INIStructure& inIni) {
        //WCHAR wcSource[MAX_PATH];
        //wcscpy_s(wcSource, L"C:\\NuJee\\output");
        //wcscat_s(wcSource, L"\\outputtry.ini");

        //std::wstring wsSource(wcSource);
        //std::string inifile2(wsSource.begin(), wsSource.end());
        WCHAR wcjasonFileName[MAX_PATH];
        UtilHelper::OutputJobProductFile(NULL, wcjasonFileName);
        //std::string  jasonFileName("C:\\NuJee\\output\\outputtry.json");
        std::wstring jasonFileName(wcjasonFileName);
        //mINI::INIFile file(inifile);

        // create an empty structure (null)
        json jasonFile;
        // next, create a structure that will hold data
        //mINI::INIStructure ini;


        // add a new entry ["Device"]["DeviceName"]
        std::string iniDeviceName("NuJeePrint Virtual Driver");
        //ini["Device"]["DeviceName"] = iniDeviceName;
        jasonFile["Device"]["DeviceName"] = iniDeviceName;

        jasonFile["Document"]["Name"] = inIni["Document"]["Name"];
        // JobID=3
        // MachineName = \\UShallNotPassDe
        // UserName = Alang
        // NotifyName = Alang
        // SessionID = 2
        // SecurityID = S - 1 - 5 - 21 - 2292555829 - 420465608 - 1375873136 - 500
        jasonFile["Document"]["HorizontalResolution"] = inIni["Document"]["HorizontalResolution"];
        jasonFile["Document"]["VerticalResolution"] = inIni["Document"]["VerticalResolution"];
        jasonFile["Document"]["Orientation"] = inIni["Document"]["Orientation"];
        jasonFile["Document"]["PaperSize"] = inIni["Document"]["PaperSize"];
        jasonFile["Document"]["PaperSizeName"] = inIni["Document"]["PaperSizeName"];
        jasonFile["Document"]["PaperWidth"] = inIni["Document"]["PaperWidth"];
        jasonFile["Document"]["PaperLength"] = inIni["Document"]["PaperLength"];
        jasonFile["Document"]["Bin"] = inIni["Document"]["Bin"];
        jasonFile["Document"]["BinName"] = inIni["Document"]["BinName"];
        jasonFile["Document"]["Copies"] = inIni["Document"]["Copies"];
        jasonFile["Document"]["Collate"] = inIni["Document"]["Collate"];
        jasonFile["Document"]["Duplex"] = inIni["Document"]["Duplex"];
        jasonFile["Document"]["Staple"] = inIni["Document"]["Staple"];
        jasonFile["Document"]["Color"] = inIni["Document"]["Color"];
        std::string iniStatus("Printed");
        jasonFile["Document"]["Status"] = iniStatus;
        jasonFile["Document"]["Pages"] = std::to_string(IdPagesCount);
        jasonFile["Document"]["PagesPerSheet"] = inIni["Document"]["PagesPerSheet"];
        //[Spool]
        jasonFile["Spool"]["Type"] = "XPS";
        //[PNG]
        jasonFile["PNG"]["Count"] = std::to_string(IdPagesCount);
        for (int i = 0; i < IdPagesCount; i++) {
            std::string iniDoc("File");
            std::string retiniDoc = iniDoc + std::to_string(i);
            jasonFile["PNG"][retiniDoc] = inIni["PNG"][retiniDoc];
        }
        //[EMF]
        jasonFile["EMF"]["Count"] = std::to_string(IdPagesCount);
        for (int i = 0; i < IdPagesCount; i++) {
            std::string iniDoc("File");
            std::string retiniDoc = iniDoc + std::to_string(i);
            jasonFile["EMF"][retiniDoc] = inIni["EMF"][retiniDoc];
        }
        //[PDF] OK
        jasonFile["PDF"]["Count"] = "1";
        jasonFile["PDF"]["File0"] = inIni["PDF"]["File0"];
        //[INI] OK
        jasonFile["INI"]["Count"] = "1";
        jasonFile["INI"]["File0"] = jasonFileName;

        //file.write(ini, true);
        // write prettified JSON to another file
        std::ofstream finalJasonFile(jasonFileName);
        finalJasonFile << std::setw(4) << jasonFile << std::endl;
        return true;
    }

} // namespace NuJee_EMF_v4_Printer_Driver_Render_Filter
