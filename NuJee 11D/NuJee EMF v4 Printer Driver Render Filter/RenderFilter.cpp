//
// File Name:
//
//    RenderFilter.cpp
//
// Abstract:
//
//    XPS Rendering filter implementation.
//

#include "precomp.h"
#include "WppTrace.h"
#include "CustomWppCommands.h"
#include "Exception.h"
#include "filtertypes.h"
#include "UnknownBase.h"
#include "PThandler.h"
#include "RenderFilter.h"
#include "RenderFilter.tmh"

#include "rasinterface.h"
#include "BitmapHandler.h"
#include "OMConvertor.h"
#include "EMFConvertor.h"
#include "GDIPlusHelper.h"
#include "PDFConvert.h"
#include "PDFConvertPNGImage.h"
#include "TIFFImagePDF.h"

#include "UtilHelper.h"
#include "IniHelper.h"

#include "winbase.h"
#include "wtsapi32.h" 



namespace NuJee_EMF_v4_Printer_Driver_Render_Filter
{


    long RenderFilter::ms_numObjects = 0; // Initialize static object count

    //
    //Routine Name:
    //
    //    RenderFilter::RenderFilter
    //
    //Routine Description:
    //
    //    XPS Rendering filter default constructor.
    //
    //Arguments:
    //
    //    None
    //
    //Return Value:
    //
    //    None
    //
    RenderFilter::RenderFilter() :
        m_cIdCount(0)
    {
        //
        // Take ownership with no AddRef
        //
        m_pLiveness.Attach(new FilterLiveness());

        ::InterlockedIncrement(&ms_numObjects);
    }

    //
    //Routine Name:
    //
    //    RenderFilter::~RenderFilter
    //
    //Routine Description:
    //
    //    XPS Rendering sample filter destructor.
    //
    //Arguments:
    //
    //    None
    //
    //Return Value:
    //
    //    None
    //
    RenderFilter::~RenderFilter()
    {
        ::InterlockedDecrement(&ms_numObjects);
    }

    //
    //Routine Name:
    //
    //    RenderFilter::InitializeFilter
    //
    //Routine Description:
    //
    //    Exception boundary wrapper for IPrintPipelineFilter initialization.
    //
    //Arguments:
    //
    //    pICommunicator    - interface to interfilter communicator
    //    pIPropertyBag     - interface to pipeline property bag
    //    pIPipelineControl - interface to pipeline control methods
    //
    //Return Value:
    //
    //    ULONG
    //    New reference count
    //
    _Check_return_
        HRESULT STDMETHODCALLTYPE
        RenderFilter::InitializeFilter(
            _In_ IInterFilterCommunicator* pICommunicator,
            _In_ IPrintPipelinePropertyBag* pIPropertyBag,
            _In_ IPrintPipelineManagerControl* pIPipelineControl
        )
    {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"Initializing Filter");

        if (pICommunicator == NULL ||
            pIPropertyBag == NULL ||
            pIPipelineControl == NULL)
        {
            WPP_LOG_ON_FAILED_HRESULT(E_POINTER);

            return E_POINTER;
        }

        HRESULT hr = S_OK;

        try
        {
            InitializeFilter_throws(
                pICommunicator,
                pIPropertyBag
            );
        }
        CATCH_VARIOUS(hr);

        return hr;
    }

    //
    //Routine Name:
    //
    //    RenderFilter::InitializeFilter_throws
    //
    //Routine Description:
    //
    //    Implements IPrintPipelineFilter initialization. Gets
    //    all necessary communication interfaces.
    //
    //Arguments:
    //
    //    pICommunicator    - interface to interfilter communicator
    //    pIPropertyBag     - interface to pipeline property bag
    //
    VOID
        RenderFilter::InitializeFilter_throws(
            const IInterFilterCommunicator_t& pICommunicator,
            const IPrintPipelinePropertyBag_t& pIPropertyBag
        )
    {
        //
        // Get the pipeline communication interfaces
        //
        THROW_ON_FAILED_HRESULT(
            pICommunicator->RequestReader(reinterpret_cast<void**>(&m_pReader))
        );
        THROW_ON_FAILED_HRESULT(
            pICommunicator->RequestWriter(reinterpret_cast<void**>(&m_pWriter))
        );

        {
            //
            // Check to ensure that the provided interfaces are as expected.
            // That is, that the GUIDs were correctly listed in the
            // pipeline configuration file
            //
            IXpsDocumentProvider_t pReaderCheck;
            IPrintWriteStream_t pWriterCheck;
            //IXpsDocumentConsumer_t pWriterCheck;

            THROW_ON_FAILED_HRESULT(
                m_pReader.QueryInterface(&pReaderCheck)
            );
            THROW_ON_FAILED_HRESULT(
                m_pWriter.QueryInterface(&pWriterCheck)
            );

        }

        //
        // Save a pointer to the Property Bag for further
        // initialization, later.
        //
        m_pIPropertyBag = pIPropertyBag;
    }

    //
    //Routine Name:
    //
    //    RenderFilter::ShutdownOperation
    //
    //Routine Description:
    //
    //    Called asynchronously by the pipeline manager
    //    to shutdown filter operation.
    //
    //Arguments:
    //
    //    None
    //
    //Return Value:
    //
    //    HRESULT
    //    S_OK - On success
    //
    _Check_return_
        HRESULT
        RenderFilter::ShutdownOperation()
    {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"Shutting Down Operation");
        m_pLiveness->Cancel();
        return S_OK;
    }



HRESULT RenderFilter::InitPrintJobEnv()
{
    HRESULT hr = S_OK;
    DoTraceMessage(RENDERFILTER_TRACE_INFO, L"Starting InitPrintJobEnv");
    // read registry

    // delete temp files


    DoTraceMessage(RENDERFILTER_TRACE_INFO, L"InitPrintJobEnv OK");
    return hr;
}
//
//Routine Name:
//
//    RenderFilter::StartOperation
//
//Routine Description:
//
//    Called by the pipeline manager to start processing
//    a document. Exception boundary for page processing.
//
//Arguments:
//
//    None
//
//Return Value:
//
//    HRESULT
//    S_OK      - On success
//    Otherwise - Failure
//
_Check_return_
HRESULT
RenderFilter::StartOperation()
{
    HRESULT hr = S_OK;

    DoTraceMessage(RENDERFILTER_TRACE_INFO, L"Starting Operation");

    InitPrintJobEnv();
    //
    // Process the Xps Package
    //
    int IdPagesCount=0;
    try {
        StartOperation_throws(&IdPagesCount);
    }
    CATCH_VARIOUS(hr);

    m_pWriter->Close();
    //m_pWriter->CloseSender();

    //MessageBox(NULL,L"NuJee Printing Job done",L"NuJee Printing Job done", MB_OK);
    DoTraceMessage(RENDERFILTER_TRACE_INFO, L"send Notify Agent NuJeeVPDAgent windows service. (%d)", IdPagesCount);
    /*
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    if (!CreateProcess(L"C:\\NuJee\\NuJeeVPDSum.exe", NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        //printf("CreateProcess failed (%d).\n", GetLastError());
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"Finishing CreateProcess NuJeeVPDSum CreateProcess o failed (%d)", GetLastError());
    }else{
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"Finishing CreateProcess NuJeeVPDSum okok");
    }
    */
    typedef DWORD (*WTSGETACTIVECONSOLESESSIONID)();
    typedef bool (*LPFN_CreateEnvironmentBlock)(LPVOID, HANDLE,bool);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    HANDLE hTokenNew = NULL, hTokenDup = NULL;
    HMODULE  hmod = LoadLibrary(L"kernel32.dll");
    WTSGETACTIVECONSOLESESSIONID lpfnWTSGetActiveConsoleSessionId = (WTSGETACTIVECONSOLESESSIONID)GetProcAddress(hmod, "WTSGetActiveConsoleSessionId");
    DWORD dwSessionId = lpfnWTSGetActiveConsoleSessionId();
    WTSQueryUserToken(dwSessionId, &hTokenNew);
    DuplicateTokenEx(hTokenNew, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &hTokenDup);
    //
    DoTraceMessage(RENDERFILTER_TRACE_INFO, L"Calling lpfnCreateEnvironmentBlock");
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.lpDesktop = NULL;// L"winsta0\\default";

    LPVOID  pEnv = NULL;
    DWORD dwCreationFlag = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;
    HMODULE hModule = LoadLibrary(L"Userenv.dll");
    if (hModule)
    {
        LPFN_CreateEnvironmentBlock lpfnCreateEnvironmentBlock = (LPFN_CreateEnvironmentBlock)GetProcAddress(hModule, "CreateEnvironmentBlock");
        if (lpfnCreateEnvironmentBlock != NULL)
        {
            if (lpfnCreateEnvironmentBlock(&pEnv, hTokenDup, FALSE))
            {
                DoTraceMessage(RENDERFILTER_TRACE_INFO, L"CreateEnvironmentBlock Ok");
                dwCreationFlag |= CREATE_UNICODE_ENVIRONMENT;
            }
            else
            {
                pEnv = NULL;
            }
        }
    }
    //
    ZeroMemory(&pi, sizeof(pi));

    //if (!CreateProcessAsUser(hTokenDup, NULL, L"C:\\NuJee\\NuJeeVPDSum.exe", NULL, NULL, FALSE, dwCreationFlag, pEnv, NULL, &si, &pi ))
    if (0)
    {
        //printf("CreateProcess failed (%d).\n", GetLastError());
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"Finishing CreateProcess NuJeeVPDSum CreateProcess o failed (%d)", GetLastError());
    }
    else {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"Finishing CreateProcess NuJeeVPDSum okok");
    }

    DoTraceMessage(RENDERFILTER_TRACE_INFO, L"OK send Notify Agent NuJeeVPDAgent windows service. (%d)", IdPagesCount);
    DoTraceMessage(RENDERFILTER_TRACE_INFO, L"Exit ");
    return hr;
}

//
//Routine Name:
//
//    RenderFilter::StartOperation_throws
//
//Routine Description:
//
//    Iterates over the 'trunk' parts of the document 
//    and calls appropriate processing methods.
//
//Arguments:
//
//    None
//
void
RenderFilter::StartOperation_throws(int* pIdPagesCount)
{
    //
    // CoInitialize/CoUninitialize RAII object.
    // COM is inititalized for the lifetime of this method.
    //
    SafeCoInit  coInit;

    IXpsOMObjectFactory_t pOMFactory;

    //
    // Create Xps Object Model Object Factory instance
    //
    THROW_ON_FAILED_HRESULT(
        ::CoCreateInstance(
            __uuidof(XpsOMObjectFactory),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(IXpsOMObjectFactory),
            reinterpret_cast<LPVOID*>(&pOMFactory)
            )
        );

    IOpcFactory_t pOpcFactory;

    //
    // Create Opc Object Factory instance
    //
    THROW_ON_FAILED_HRESULT(
        ::CoCreateInstance(
            __uuidof(OpcFactory),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(IOpcFactory),
            reinterpret_cast<LPVOID*>(&pOpcFactory)
            )
        );

    //CreateXpsOMTest(pOMFactory);
    //
    // Create the rasterization interface for EMF
    //
    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"Create the rasterization interface for EMF ......");
    RasterizationInterface_t pRasInterface = RasterizationInterface::CreateRasterizationInterface(
            m_pIPropertyBag,
            m_pWriter
        );
    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"Create the rasterization interface for EMF ok");

    //
    // Create the Print Ticket Handler
    //
    PrintTicketHandler_t pPrintTicketHandler =  PrintTicketHandler::CreatePrintTicketHandler( m_pIPropertyBag );

    IUnknown_t pUnk;

    //
    // Get first part
    //
    THROW_ON_FAILED_HRESULT(m_pReader->GetXpsPart(&pUnk));

    int IdPagesCount = 0; //pages
    while (m_pLiveness->IsAlive() && pUnk != NULL)
    {
        IXpsDocument_t               pDoc;
        IFixedDocumentSequence_t     pFDS;
        IFixedDocument_t             pFD;
        IFixedPage_t                 pFP;

        if (SUCCEEDED(pUnk.QueryInterface(&pFP)))
        {
            DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"Handling a Page ");
            pPrintTicketHandler->ProcessPart(pFP);
            DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"Handling a Page -> pPrintTicketHandler->ProcessPart");



            ParametersFromPrintTicket printTicketParams = pPrintTicketHandler-> GetMergedPrintTicketParams();
            DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"Handling a Page -> pPrintTicketHandler->GetMergedPrintTicketParams");

            CHAR outputFile[MAX_PATH];
            pRasInterface->RasterizePage(IdPagesCount,
                OMConvertor::CreateXpsOMPageFromIFixedPage(pFP, pOMFactory, pOpcFactory),
                printTicketParams,
                m_pLiveness, 
                outputFile
            );
            //[PNG] Ini
            std::string iniDoc("File");
            std::string retiniDoc = iniDoc + std::to_string(IdPagesCount);
            std::string inioutputFile(outputFile);
            m_inIni["TIFF"][retiniDoc] = inioutputFile;

            // 
            DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"After pPrintTicketHandler->ProcessPart Handling a Page = IFixedPage_t");
            
            HRESULT   hRes = E_NOINTERFACE;
            void* pvoid = NULL;
            if (SUCCEEDED(hRes = pUnk->QueryInterface(IID_IFixedPage, &pvoid)))
            {
                hRes = ProcessFixedPage(IdPagesCount, pvoid, pOMFactory, pOpcFactory, pPrintTicketHandler);
            } 
            if (SUCCEEDED(hRes)){
                //hRes = m_pReachConsumer->SendXpsUnknown(pUnk);
                DoTraceMessage(RENDERFILTER_TRACE_ERROR, L"Fail to Handling a Page ......");
            }

            IdPagesCount++;
        }
        else if (SUCCEEDED(pUnk.QueryInterface(&pFD)))
        {
            DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"Handling a FixedDocument = IFixedDocument_t");
            pPrintTicketHandler->ProcessPart(pFD);
            
            //IniPropertiesFromPrintTicket iniProperties = pPrintTicketHandler->GetIniPropertiesFromPrintTicket();

        }
        else if (SUCCEEDED(pUnk.QueryInterface(&pFDS)))
        {
            DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"Handling a FixedDocumentSequence = IFixedDocumentSequence_t");
            IniPropertiesFromPrintTicket iniProperties;
            pPrintTicketHandler->ProcessPart(pFDS, iniProperties);
            //IniPropertiesFromPrintTicket iniProperties = pPrintTicketHandler->GetIniPropertiesFromPrintTicket();
            //IniPropertiesFromPrintTicket iniProperties = pPrintTicketHandler->GetIniPropertiesFromPrintTicket(pIPrintTicket);
            // INI
            m_inIni["Document"]["HorizontalResolution"] = std::to_string(iniProperties.destDPI);
            m_inIni["Document"]["VerticalResolution"] = std::to_string(iniProperties.destDPI);
            std::string iniOrientationLandscape("Landscape");
            std::string iniOrientationPortrait("Portrait");
            if (iniProperties.PageOrientation == 1) {
                m_inIni["Document"]["Orientation"] = iniOrientationLandscape;
            }
            else {
                m_inIni["Document"]["Orientation"] = iniOrientationPortrait;
            }
            m_inIni["Document"]["PaperSize"] = std::to_string(iniProperties.DocumentPaperSize);
            std::string iniPaperSizeName(iniProperties.DocumentPaperSizeName);
            m_inIni["Document"]["PaperSizeName"] = iniPaperSizeName;
            m_inIni["Document"]["PaperWidth"] = std::to_string(iniProperties.DocumentPaperWidth);
            m_inIni["Document"]["PaperLength"] = std::to_string(iniProperties.DocumentPaperLength);
            m_inIni["Document"]["Bin"] = std::to_string(iniProperties.DocumentBin);
            std::string iniBinName(iniProperties.DocumentBinName);
            m_inIni["Document"]["BinName"] = iniBinName;
            m_inIni["Document"]["Copies"] = std::to_string(iniProperties.DocumentCopies);
            m_inIni["Document"]["Collate"] = std::to_string(iniProperties.DocumentCollate);
            std::string iniDocumentDuplex(iniProperties.DocumentDuplex);
            m_inIni["Document"]["Duplex"] = iniDocumentDuplex;
            std::string iniDocumentStaple(iniProperties.DocumentStaple);
            m_inIni["Document"]["Staple"] = iniDocumentStaple;
            std::string iniDocumentColor(iniProperties.DocumentColor);
            m_inIni["Document"]["Color"] = iniDocumentColor;
            m_inIni["Document"]["PagesPerSheet"] = std::to_string(iniProperties.DocumentPagesPerSheet);
        }
        else if (SUCCEEDED(pUnk.QueryInterface(&pDoc)))
        {
            DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L" *** Do nothing *** with a XpsDocument = IXpsDocument_t");

            //CComPtr<IPartThumbnail> testNail;
            //pDoc->GetThumbnail(&testNail);
            BSTR tempstr=nullptr;
            //testNail->GetThumbnailProperties(&tempstr);
            DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L" *** Do nothing *** with a XpsDocument = IXpsDocument_t== [%ws]", static_cast<PCWSTR>(tempstr));
            //
            // Do nothing with the XML Document part
            //
        }
        else
        {
            DoTraceMessage(RENDERFILTER_TRACE_INFO, L"Unhandled Document Part");
        }

        pUnk.Release();

        //
        // Get Next Part
        //
        THROW_ON_FAILED_HRESULT(m_pReader->GetXpsPart(&pUnk));
    }

    // Convert To PDF from PNG
    DoTraceMessage(RENDERFILTER_TRACE_INFO, L"**************** ImageNormalize ********************");
    //for (int i = 0;i< IdPagesCount; i++)
    {
        //GDIPlusHelper::ImageNormalize(i, L"");
        GDIPlusHelper::ImageAsNormalFinal(0, L"");
    }
    DoTraceMessage(RENDERFILTER_TRACE_INFO, L"**************** ImageNormalize ********************");


    WCHAR filenameFolder[MAX_PATH];
    if (UtilHelper::GetOutputTempFolderFileName(filenameFolder)) {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GenerateDocPdfFromPNG TIFF:: %ws", filenameFolder);
        WCHAR inputPDFFile[MAX_PATH];
        WCHAR inputPDFFile2[MAX_PATH];
        if (UtilHelper::OutputJobProductFile(inputPDFFile, inputPDFFile2)) {
            DoTraceMessage(RENDERFILTER_TRACE_INFO, L"OK GenerateDocPdfFromPNG OutputJobProductFile::  ***  %ws", inputPDFFile);
        }
        CHAR outputPDFFile[MAX_PATH];
        sprintf_s(outputPDFFile, "%ws", inputPDFFile);
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GenerateDocPdfFromPNG sprintf_s outputPDFFile::  ***  %s", outputPDFFile);
        //if (GenerateDocPdfFromPNG(filenameFolder, IdPagesCount, outputPDFFile) == 0)
        if (TIFFImagePDF(filenameFolder, IdPagesCount, outputPDFFile) == 0)
        {
            //[PDF] Ini
            std::string iniString(outputPDFFile);
            m_inIni["PDF"]["File0"] = iniString;
            DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GenerateDocPdfFromPNG OK::  ***  %ws", filenameFolder);
            // Finally here is the point that output final stream ---> printing Save as PDF file ......
            //if (TiffStreamBitmapHandler::PrintOutputPDFStream(m_pWriter,filenameFolder) == 1)
            {
                DoTraceMessage(RENDERFILTER_TRACE_INFO, L"PrintOutputPDFStream OK::  ***  %ws", filenameFolder);
            } //else 
            {
                DoTraceMessage(RENDERFILTER_TRACE_INFO, L"PrintOutputPDFStream Fail::  ***  %ws", filenameFolder);
            }
            //RunImageTest(filenameFolder, L"");
        }
        else {
            DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GenerateDocPdfFromPNG fail::  ***  %ws", filenameFolder);
        }
    }
    else {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GenerateDocPdfFromPNG fail %ws", filenameFolder);
    }

    DoTraceMessage(RENDERFILTER_TRACE_INFO, L"Generate Jason File ......");
    WCHAR inifilename[MAX_PATH];
    //UtilHelper::OutputJobSumFilePost(IdPagesCount, inifilename, m_inIni);
    UtilHelper::OutputJobSumJasonFilePost(IdPagesCount, inifilename, m_inIni);
    DoTraceMessage(RENDERFILTER_TRACE_INFO, L"Generate Ini File %ws", inifilename);
    //pRasInterface->FinishRasterization();
}

HRESULT
RenderFilter::ProcessFixedPage(const int IdPagesCount, _In_ void* pVoid, const IXpsOMObjectFactory_t& pOMFactory, const IOpcFactory_t& pOpcFactory, PrintTicketHandler_t& pPrintTicketHandler)
{
    IFixedPage_t pIFixedPage;

    //
    // XpsFilter::ProcessPart already incremented the reference count for the object. This
    // function is taking ownership of the object, there is no need for another AddRef call
    // (hence the Attach)
    //
    pIFixedPage.Attach(static_cast<IFixedPage*>(pVoid));

    HRESULT  hRes;
    BSTR bstr;

    if (SUCCEEDED(pIFixedPage->GetUri(&bstr)))
    {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, "ProcessFixedPage uri %ws", static_cast<PCWSTR>(bstr));
    }

    IPartPrintTicket_t pIPrintTicket;

    hRes = pIFixedPage->GetPrintTicket(&pIPrintTicket);

    if (SUCCEEDED(hRes))
    {
        hRes = OMConvertor::ProcessTicket(pIPrintTicket);
    }
    else if (hRes == E_ELEMENT_NOT_FOUND)
    {
        //
        // No ticket. Benign.
        //
        hRes = S_OK;

        DoTraceMessage(RENDERFILTER_TRACE_INFO, "ProcessFixedPage uri %ws. Page print ticket not present.", static_cast<PCWSTR>(bstr));
    }

    pIFixedPage->SetPartCompression(Compression_Small);
    DoTraceMessage(RENDERFILTER_TRACE_INFO, "SetPartCompression() OK");

    //
    // modify content for page, optional
    //
    IPrintReadStream_t   pRead;
    IPrintWriteStream_t  pWrite;

    if (SUCCEEDED(hRes = pIFixedPage->GetStream(&pRead)) &&
        SUCCEEDED(hRes = pIFixedPage->GetWriteStream(&pWrite)))
    {
        hRes = OMConvertor::ModifyContent(pRead, pWrite);

        pWrite->Close();
        DoTraceMessage(RENDERFILTER_TRACE_INFO, " modify content for page, optional OK");
    }

    //
    // Send page to next filter
    //
    if (SUCCEEDED(hRes))
    {
        //hRes = m_pWriter->SendFixedPage(pIFixedPage);
        DoTraceMessage(RENDERFILTER_TRACE_INFO, " Send page to next filter 1. OK hRes=[%u] ", hRes);
    }

    //
    // Add a new page
    //
    if (SUCCEEDED(hRes))
    {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, "  Add a new page ...");

        IFixedPage_t         pNewPage;
        IPrintWriteStream_t  pNewPageMarkupStream;
        IPartFont_t          pNewFont;
        IPrintWriteStream_t  pNewFontStream;

        WCHAR                        szName[MAX_PATH];

        //
        // Generate unique page name
        //
        if (SUCCEEDED(hRes = StringCchPrintf(szName, MAX_PATH, L"/pages/newaddedpage%u.xaml", m_cIdCount++)))
        {
            //
            // Create new fixed page
            //
            //////hRes = m_pWriter->GetNewEmptyPart(szName, IID_IFixedPage, reinterpret_cast<void**>(&pNewPage), &pNewPageMarkupStream);

            DoTraceMessage(RENDERFILTER_TRACE_INFO, "ProcessFixedPage new page %ws", szName);
        }

        //
        // Generate unique font name, this can, of course, be optimized so we don't include
        // the same font multiple times. But this code is supposed to just to show how to
        // add a font
        //
        if (SUCCEEDED(hRes = StringCchPrintf(szName, MAX_PATH, L"/font_%u.ttf", m_cIdCount++)))
        {
            //
            // Create new font resource
            //
            //////hRes = m_pWriter->GetNewEmptyPart(szName, IID_IPartFont, reinterpret_cast<void**>(&pNewFont), &pNewFontStream);
            DoTraceMessage(RENDERFILTER_TRACE_INFO, "ProcessFixedPage new font %ws", szName);
        }

        //if (SUCCEEDED(hRes) &&
        //    SUCCEEDED(hRes = OMConvertor::AddFontToPage(pNewFont, pNewFontStream, pNewPage)) &&
         //   SUCCEEDED(hRes = OMConvertor::AddTextToPage(szName, pNewPageMarkupStream)))
        {
            //
            // Send page to next filter
            //
            //////hRes = m_pWriter->SendFixedPage(pNewPage);
            DoTraceMessage(RENDERFILTER_TRACE_INFO, " Send page to next filter hRes=[%u] ", hRes);
            DoTraceMessage(RENDERFILTER_TRACE_INFO, "SendFixedPage OK");
        }


        // All converts processing..........
        DoTraceMessage(RENDERFILTER_TRACE_INFO, " OMConvertor CreateXpsOMPageDocFromIFixedPage ......");
        if (OMConvertor::CreateXpsOMPageDocFromIFixedPage(pIFixedPage, pOMFactory, pOpcFactory)) {
            DoTraceMessage(RENDERFILTER_TRACE_INFO, " OMConvertor  CreateXpsOMPageDocFromIFixedPage ok ");
        }
        else {
            DoTraceMessage(RENDERFILTER_TRACE_INFO, " CreateXpsOMPageDocFromIFixedPage fail ......");
        }

        DoTraceMessage(RENDERFILTER_TRACE_INFO, " EMFConvertor CreateEMFOMPageDocFromIFixedPage ......");
        if (1 == 0) {
            // Convert To JPG to PNG
            WCHAR tempfilenameFolder[MAX_PATH];
            WCHAR PNGfilename[MAX_PATH];
            WCHAR filenameFolder[MAX_PATH];
            if (UtilHelper::GetOutputTempFolderFileName(filenameFolder)) {
                DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GeneratePdfFromPNG PNG:: %ws", filenameFolder);
                wcscpy_s(tempfilenameFolder, filenameFolder);
                wcscat_s(tempfilenameFolder, L"%u.png");
                StringCchPrintf(PNGfilename, MAX_PATH, tempfilenameFolder, IdPagesCount);
                DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GeneratePdfFromPNG PNG::  ***  %ws", PNGfilename);
            }
            else {
                DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GeneratePdfFromPNG fail PNG %ws", PNGfilename);
            }
            if (GDIPlusHelper::GDIPlusImageJPGConvertPNG(IdPagesCount, PNGfilename)) {
                DoTraceMessage(RENDERFILTER_TRACE_INFO, " GDIPlusImageJPGConvertPNG ok ");
                /*
                // Convert To PDF from PNG
                WCHAR tempfilenameFolder[MAX_PATH];
                WCHAR PDFfilename[MAX_PATH];
                WCHAR filenameFolder[MAX_PATH];
                if (UtilHelper::GetOutputTempFolderFileName(filenameFolder)) {
                    DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GeneratePdfFromPNG PNG:: %ws", filenameFolder);
                    wcscpy_s(tempfilenameFolder, filenameFolder);
                    wcscat_s(tempfilenameFolder, L"%u.pdf");
                    StringCchPrintf(PDFfilename, MAX_PATH, tempfilenameFolder, IdPagesCount);
                    DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GeneratePdfFromPNG PNG::  ***  %ws", PDFfilename);

                    if (GeneratePagePdfFromPNG(PNGfilename, PDFfilename)==0) {
                        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GeneratePdfFromPNG OK::  ***  %ws", PDFfilename);
                    }
                    else {
                        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GeneratePdfFromPNG fail::  ***  %ws", PDFfilename);
                    }
                }
                else {
                    DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GeneratePdfFromPNG fail PNG %ws", PDFfilename);
                }
                */
            }
            else {
                DoTraceMessage(RENDERFILTER_TRACE_INFO, " GDIPlusImageJPGConvertPNG fail ......");
            }
        }else if (1 == 0) {
            CHAR outputFile[MAX_PATH];
            if (EMFConvertor::CreateEMFOMPageDocFromPNGRaster(IdPagesCount, L"", outputFile)) {
                DoTraceMessage(RENDERFILTER_TRACE_INFO, L"EMFConvertor::CreateEMFOMPageDocFromPNGRaster::  %s", outputFile);
                //[EMF] Ini
                std::string iniDoc("File");
                std::string retiniDoc = iniDoc + std::to_string(IdPagesCount);
                std::string inioutputFile(outputFile);
                m_inIni["EMF"][retiniDoc] = inioutputFile;

                DoTraceMessage(RENDERFILTER_TRACE_INFO, " EMFConvertor PNG  CreateEMFOMPageDocFromIFixedPage ok ");
            }
            else {
                DoTraceMessage(RENDERFILTER_TRACE_INFO, "EMFConvertor PNG CreateEMFOMPageDocFromIFixedPage fail ......");
            }
        }
        else {
            if (EMFConvertor::CreateEMFOMPageDocFromTiffRaster(IdPagesCount, L"")) {
                DoTraceMessage(RENDERFILTER_TRACE_INFO, " EMFConvertor  CreateEMFOMPageDocFromIFixedPage ok ");
            }
            else {
                DoTraceMessage(RENDERFILTER_TRACE_INFO, " CreateEMFOMPageDocFromIFixedPage fail ......");
            }

            DoTraceMessage(RENDERFILTER_TRACE_INFO, " EMFConvertor CreateEMFOMPageDocFromIFixedPage ......");
            // Convert To PDF from PNG
            WCHAR tempfilenameFolder[MAX_PATH];
            WCHAR PNGfilename[MAX_PATH];
            WCHAR filenameFolder[MAX_PATH];
            if (UtilHelper::GetOutputTempFolderFileName(filenameFolder)) {
                DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GeneratePdfFromPNG PNG:: %ws", filenameFolder);
                wcscpy_s(tempfilenameFolder, filenameFolder);
                wcscat_s(tempfilenameFolder, L"%u.png");
                StringCchPrintf(PNGfilename, MAX_PATH, tempfilenameFolder, IdPagesCount);
                DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GeneratePdfFromPNG PNG::  ***  %ws", PNGfilename);
            }
            else {
                DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GeneratePdfFromPNG fail PNG %ws", PNGfilename);
            }
            if (GDIPlusHelper::GDIPlusImageTIFFConvertPNG(IdPagesCount, PNGfilename)) {
                DoTraceMessage(RENDERFILTER_TRACE_INFO, " GDIPlusImageTIFFConvertPNG ok ");
                /*
                // Convert To PDF from PNG
                WCHAR tempfilenameFolder[MAX_PATH];
                WCHAR PDFfilename[MAX_PATH];
                WCHAR filenameFolder[MAX_PATH];
                if (UtilHelper::GetOutputTempFolderFileName(filenameFolder)) {
                    DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GeneratePdfFromPNG PNG:: %ws", filenameFolder);
                    wcscpy_s(tempfilenameFolder, filenameFolder);
                    wcscat_s(tempfilenameFolder, L"%u.pdf");
                    StringCchPrintf(PDFfilename, MAX_PATH, tempfilenameFolder, IdPagesCount);
                    DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GeneratePdfFromPNG PNG::  ***  %ws", PDFfilename);

                    if (GeneratePagePdfFromPNG(PNGfilename, PDFfilename)==0) {
                        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GeneratePdfFromPNG OK::  ***  %ws", PDFfilename);
                    }
                    else {
                        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GeneratePdfFromPNG fail::  ***  %ws", PDFfilename);
                    }
                }
                else {
                    DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GeneratePdfFromPNG fail PNG %ws", PDFfilename);
                }
                */
            }
            else {
                DoTraceMessage(RENDERFILTER_TRACE_INFO, " GDIPlusImageTIFFConvertPNG fail ......");
            }
        }
        DoTraceMessage(RENDERFILTER_TRACE_INFO, "ProcessFixedPage OK , will start develping EMF ok ......");
    }
    else {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, "ProcessFixedPage Fail");
    }

    return hRes;
}




/*++
Routine Name:
    CWatermarkMarkup::CreateXPSStream
Routine Description:
    Method for writing out the vector image to the container
Arguments:
    None
Return Value:
    HRESULT
    S_OK - On success
    E_*  - On error
--*/
//
// Module's Instance handle from DLLEntry of process.
//
HINSTANCE g_hInstance = NULL;
UINT m_resourceID = 1;
//
// Macros for checking pointers and handles.
//
#define CHECK_POINTER(p, hr) ((p) == NULL ? hr : S_OK)

HRESULT RenderFilter::CreateXPSStream( VOID )
{
    HRESULT hr = S_OK;

    if (m_pXPSStream == NULL)
    {
        HRSRC hrSrc = FindResourceEx(g_hInstance,
            RT_RCDATA,
            MAKEINTRESOURCE(m_resourceID),
            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
        if (hrSrc != NULL)
        {
            //
            // Load the resource
            //
            HGLOBAL hXPSRes = LoadResource(g_hInstance, hrSrc);

            if (hXPSRes != NULL)
            {
                if (SUCCEEDED(hr = CreateStreamOnHGlobal(NULL, TRUE, &m_pXPSStream)))
                {
                    //
                    // Retrieve the XPS data and the size of the data
                    //
                    PVOID pXPSData = LockResource(hXPSRes);
                    DWORD cbXPSData = SizeofResource(g_hInstance, hrSrc);

                    if (SUCCEEDED(hr = CHECK_POINTER(pXPSData, E_FAIL)) &&
                        cbXPSData > 0)
                    {
                        ULONG cbTotalWritten = 0;

                        //
                        // Write the size of the data to the stream.
                        // This is required as the stream will be read back using the CComBSTR ReadFromStream() method
                        //
                        DWORD cbXPSDataWithTerm = cbXPSData + sizeof(OLECHAR);

                        if (SUCCEEDED(hr = m_pXPSStream->Write(&cbXPSDataWithTerm, sizeof(cbXPSDataWithTerm), &cbTotalWritten)))
                        {
                            //
                            // Write the data to the stream
                            // Note the XPS data content must be in Unicode format to be compatible with
                            // the CComBSTR ReadFromStream() method.
                            //
                            ULONG cbWritten = 0;

                            if (SUCCEEDED(hr = m_pXPSStream->Write(pXPSData, cbXPSData, &cbWritten)))
                            {
                                cbTotalWritten += cbWritten;

                                hr = m_pXPSStream->Write(OLESTR("\0"), sizeof(OLECHAR), &cbWritten);
                                cbTotalWritten += cbWritten;
                            }
                        }
                    }
                }

                FreeResource(hXPSRes);
            }
            else
            {
                hr = OMConvertor::GetLastErrorAsHResult();
            }
        }
        else
        {
            hr = OMConvertor::GetLastErrorAsHResult();
        }
    }

    //
    // Make sure the stream is pointing back at the start of the data
    //
    if (SUCCEEDED(hr))
    {
        LARGE_INTEGER cbMoveFromStart = { 0 };

        hr = m_pXPSStream->Seek(cbMoveFromStart,
            STREAM_SEEK_SET,
            NULL);
    }

    if (SUCCEEDED(hr)) {
        DoTraceMessage(RENDERFILTER_TRACE_ERROR, "CreateXPSStream OK");
    }
    else {
        DoTraceMessage(RENDERFILTER_TRACE_ERROR, "CreateXPSStream fail");
        //ERR_ON_HR(hr);
    }
    return hr;
}

} // namespace NuJee_EMF_v4_Printer_Driver_Render_Filter
