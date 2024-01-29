//
// File Name:
//
//    omconvertor.cpp
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
#include "BaseConvertor.h"

#include "BaseConvertor.tmh"

namespace NuJee_EMF_v4_Printer_Driver_Render_Filter
{
    BaseConvertor::BaseConvertor() 
    {
    }
    BaseConvertor::~BaseConvertor()
    {
    }

    HRESULT BaseConvertor::GetLastErrorAsHResult(void)
    {
        DWORD dwError = GetLastError();

        return HRESULT_FROM_WIN32(dwError);
    }

    HRESULT BaseConvertor::ProcessTicket(_In_ IPartPrintTicket* pIPrintTicket)
    {
        BSTR bstr;
        HRESULT hRes = pIPrintTicket->GetUri(&bstr);
        if (SUCCEEDED(hRes))
        {
            IPrintReadStream_t   pIStream;
            hRes = pIPrintTicket->GetStream(&pIStream);
            if (SUCCEEDED(hRes))
            {
                ULONG cbRead = 0;
                BOOL  bEof = FALSE;

                DoTraceMessage(RENDERFILTER_TRACE_INFO, "ProcessTicket print ticket follows.");

                do
                {
                    BYTE  buf[200] = { 0 };

                    //
                    // Save one byte to ensure null termination
                    //
                    if (SUCCEEDED(hRes = pIStream->ReadBytes(buf, COUNTOF(buf) - 1, &cbRead, &bEof)) && cbRead)
                    {
                        //
                        // use PT data
                        //
                        DoTraceMessage(RENDERFILTER_TRACE_INFO, "%s", reinterpret_cast<PCSTR>(buf));
                    }
                } while (!bEof && cbRead && SUCCEEDED(hRes));
            }
        }

        return hRes;
    }

    HRESULT BaseConvertor::AddFontToPage(
        _In_    IPartFont* pNewFont,
        _In_    IPrintWriteStream* pFontStream,
        _In_    IFixedPage* pNewPage
    )
    {
        WCHAR      szFont[MAX_PATH];
        HRESULT    hRes = S_OK;

        if (!GetWindowsDirectory(szFont, MAX_PATH))
        {
            hRes = GetLastErrorAsHResult();
        }

        if (SUCCEEDED(hRes))
        {
            hRes = StringCchCat(szFont, MAX_PATH, L"\\fonts\\verdana.TTF");
        }

        if (SUCCEEDED(hRes))
        {
            HANDLE hFile = CreateFile(szFont,
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                0,
                NULL);

            if (hFile != INVALID_HANDLE_VALUE)
            {
                BYTE    buf[400];
                ULONG   cbBuf = 400;
                ULONG   cbRead, cbWritten;

                do
                {
                    if (ReadFile(hFile, buf, cbBuf, &cbRead, NULL))
                    {
                        hRes = pFontStream->WriteBytes(buf, cbRead, &cbWritten);
                    }
                    else
                    {
                        hRes = GetLastErrorAsHResult();
                    }
                } while (SUCCEEDED(hRes) && cbRead);

                CloseHandle(hFile);
            }
            else
            {
                hRes = GetLastErrorAsHResult();
            }

            pFontStream->Close();
        }

        if (SUCCEEDED(hRes))
        {
            hRes = pNewPage->SetPagePart(pNewFont);
        }

        return hRes;
    }

    HRESULT
        BaseConvertor::AddTextToPage(
            _In_    PCWSTR              pszReferencedFont,
            _In_    IPrintWriteStream* pPageMarkupStream
        )
    {
        HRESULT  hRes;
        ULONG    cb;
        CHAR     buf[500];

        //
        // Mark up for page generated on the fly: "hello world"
        //
        CHAR  sz[] = "<FixedPage Width=\"816\" Height=\"1056\" xmlns=\"http://schemas.microsoft.com/xps/2005/06\" xml:lang=\"en-US\">"
            "<Glyphs Fill=\"#ff000000\" FontUri=\"%ws\" FontRenderingEmSize=\"15\" StyleSimulations=\"None\" OriginX=\"172\""
            " OriginY=\"129.68\" Indices=\"75;72;79;79;82;3;90;82;85;79;71\" UnicodeString=\"hello world\" />"
            "</FixedPage>";

        //
        // Insert the URI of the font we added to the document
        //
        hRes = StringCchPrintfA(buf, COUNTOF(buf), sz, pszReferencedFont);

        //
        // Write markup for page
        //
        if (SUCCEEDED(hRes) &&
            SUCCEEDED(hRes = pPageMarkupStream->WriteBytes(buf, static_cast<ULONG>(strlen(buf)), &cb)))
        {
            pPageMarkupStream->Close();
        }

        return hRes;
    }

    //
    // This function reads the page content and writes it back. Can be extended
    // to do useful work
    //
    HRESULT
        BaseConvertor::ModifyContent(
            _In_    IPrintReadStream* pRead,
            _In_    IPrintWriteStream* pWrite
        )
    {
        CHAR buf[100] = { 0 };
        HRESULT hRes;
        ULONG cbr, cbw;
        BOOL  bEof = FALSE;

        do
        {
            hRes = pRead->ReadBytes(buf, sizeof(buf), &cbr, &bEof);

            if (SUCCEEDED(hRes) && cbr)
            {
                //
                // do something with buffer
                //
                cbw = cbr;

                hRes = pWrite->WriteBytes(buf, cbr, &cbw);
            }
        } while (!bEof && cbr && SUCCEEDED(hRes));

        return hRes;
    }


    // Write Text to an XPS OM
    HRESULT
        BaseConvertor::WriteText_AddTextToPage(
            // A pre-created object factory
            __in    IXpsOMObjectFactory* xpsFactory,
            // The font resource to use for this run
            __in    IXpsOMFontResource* xpsFont,
            // The font size
            __in    float                 fontEmSize,
            // The solid color brush to use for the font
            __in    IXpsOMSolidColorBrush* xpsBrush,
            // The starting location of this glyph run
            __in    XPS_POINT* origin,
            // The text to use for this run
            __in    LPCWSTR               unicodeString,
            // The page on which to write this glyph run
            __inout IXpsOMPage* xpsPage
        )
    {
        // The data type definitions are included in this function
        // for the convenience of this code example. In an actual
        // implementation they would probably belong in a header file.
        HRESULT                       hr = S_OK;
        XPS_POINT                     glyphsOrigin = { origin->x, origin->y };
        IXpsOMGlyphsEditor* glyphsEditor = NULL;
        IXpsOMGlyphs* xpsGlyphs = NULL;
        IXpsOMVisualCollection* pageVisuals = NULL;

        // Create a new Glyphs object and set its properties.
        hr = xpsFactory->CreateGlyphs(xpsFont, &xpsGlyphs);
        hr = xpsGlyphs->SetOrigin(&glyphsOrigin);
        hr = xpsGlyphs->SetFontRenderingEmSize(fontEmSize);
        hr = xpsGlyphs->SetFillBrushLocal(xpsBrush);

        // Some properties are inter-dependent so they
        //    must be changed by using a GlyphsEditor.
        hr = xpsGlyphs->GetGlyphsEditor(&glyphsEditor);
        hr = glyphsEditor->SetUnicodeString(unicodeString);
        hr = glyphsEditor->ApplyEdits();

        // Add the new Glyphs object to the page
        hr = xpsPage->GetVisuals(&pageVisuals);
        hr = pageVisuals->Append(xpsGlyphs);

        // Release interface pointers.
        if (NULL != xpsGlyphs) xpsGlyphs->Release();
        if (NULL != glyphsEditor) glyphsEditor->Release();
        if (NULL != pageVisuals) pageVisuals->Release();

        return hr;
    }

    void BaseConvertor::CreateXpsOMTest(const IXpsOMObjectFactory_t& pOMFactory)
    {
        // Declare the variables used in this section.
        HRESULT                       hr = S_OK;
        IOpcPartUri* opcPartUri = NULL;
        IXpsOMPackage* xpsPackage = NULL;
        IXpsOMDocumentSequence* xpsFDS = NULL;
        IXpsOMDocumentCollection* fixedDocuments = NULL;
        IXpsOMDocument* xpsFD = NULL;
        IXpsOMPage* xpsPage = NULL;
        IXpsOMPageReferenceCollection* pageRefs = NULL;
        IXpsOMPageReference* xpsPageRef = NULL;
        // These values are set outside of this code example.
        XPS_SIZE pageSize = { 300, 200 };

        // Create the package.
        hr = pOMFactory->CreatePackage(&xpsPackage);

        // Create the URI for the fixed document sequence part and then  
        //  create the fixed document sequence
        hr = pOMFactory->CreatePartUri(
            L"/FixedDocumentSequence.fdseq", &opcPartUri);
        hr = pOMFactory->CreateDocumentSequence(opcPartUri, &xpsFDS);
        // Release this URI to reuse the interface pointer.
        if (NULL != opcPartUri) { opcPartUri->Release(); opcPartUri = NULL; }

        // Create the URI for the document part and then create the document.
        hr = pOMFactory->CreatePartUri(
            L"/Documents/1/FixedDocument.fdoc", &opcPartUri);
        hr = pOMFactory->CreateDocument(opcPartUri, &xpsFD);
        // Release this URI to reuse the interface pointer.
        if (NULL != opcPartUri) { opcPartUri->Release(); opcPartUri = NULL; }



        // Create a blank page.
        hr = pOMFactory->CreatePartUri(
            L"/Documents/1/Pages/1.fpage", &opcPartUri);
        hr = pOMFactory->CreatePage(
            &pageSize,                  // Page size
            L"en-US",                   // Page language
            opcPartUri,                 // Page part name
            &xpsPage);
        XPS_RECT rect = { 2,2,300, 200 };

        xpsPage->SetBleedBox(&rect);
        // Release this URI to reuse the interface pointer.
        if (NULL != opcPartUri) { opcPartUri->Release(); opcPartUri = NULL; }

        // Create a page reference for the page.
        hr = pOMFactory->CreatePageReference(&pageSize, &xpsPageRef);

        // Add the fixed document sequence to the package.
        hr = xpsPackage->SetDocumentSequence(xpsFDS);

        // Get the document collection of the fixed document sequence
        //  and then add the document to the collection.
        hr = xpsFDS->GetDocuments(&fixedDocuments);
        hr = fixedDocuments->Append(xpsFD);

        // Get the page reference collection from the document
        //  and add the page reference and blank page.
        hr = xpsFD->GetPageReferences(&pageRefs);
        hr = pageRefs->Append(xpsPageRef);
        hr = xpsPageRef->SetPage(xpsPage);

        hr = xpsPackage->WriteToFile(L"C:\\NuJee\\test1.xps", NULL, FILE_ATTRIBUTE_NORMAL, FALSE);

        // Release interface pointer
        if (NULL != xpsPage) xpsPage->Release();
        if (NULL != pageRefs) pageRefs->Release();
        if (NULL != fixedDocuments) fixedDocuments->Release();
        if (NULL != xpsPageRef) xpsPageRef->Release();
        if (NULL != xpsFD) xpsFD->Release();
        if (NULL != xpsFDS) xpsFDS->Release();
        if (NULL != xpsPackage) xpsPackage->Release();
    }


    void BaseConvertor::CreateXpsOMPackage_out(const IXpsOMObjectFactory_t& pOMFactory)
    {
        // Declare the variables used in this section.
        HRESULT  hr = S_OK;

        // Create the package.
        hr = pOMFactory->CreatePackage(&xpsPackage_out);
        // Create the URI for the fixed document sequence part and then  
        //  create the fixed document sequence
        hr = pOMFactory->CreatePartUri(L"/FixedDocumentSequence.fdseq", &opcPartUri_out);
        hr = pOMFactory->CreateDocumentSequence(opcPartUri_out, &xpsFDS_out);
        // Release this URI to reuse the interface pointer.
        if (NULL != opcPartUri_out) { opcPartUri_out->Release(); opcPartUri_out = NULL; }

        // Create the URI for the document part and then create the document.
        hr = pOMFactory->CreatePartUri(L"/Documents/1/FixedDocument.fdoc", &opcPartUri_out);
        hr = pOMFactory->CreateDocument(opcPartUri_out, &xpsFD_out);
        // Release this URI to reuse the interface pointer.
        if (NULL != opcPartUri_out) { opcPartUri_out->Release(); opcPartUri_out = NULL; }
    }

    void BaseConvertor::ReleaseXpsOMPackage_out(IXpsOMPage* xpsPage, const IXpsOMObjectFactory_t& pOMFactory)
    {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, " enter ReleaseXpsOMPackage_out ......");

        //IXpsOMPage* tempoutxpsPage = NULL;
        //tempoutxpsPage = outxpsPage;

        // Declare the variables used in this section.
        HRESULT  hr = S_OK;
        IXpsOMDocumentCollection* fixedDocuments = NULL;
        IXpsOMPageReferenceCollection* pageRefs = NULL;
        IXpsOMPageReference* xpsPageRef = NULL;
        XPS_SIZE pageSize = { 300, 200 };
        /*
        IXpsOMPage* xpsPage = NULL;
        IOpcPartUri* opcPartUri = NULL;


          // Create a blank page.
        hr = pOMFactory->CreatePartUri(
            L"/Documents/1/Pages/1.fpage", &opcPartUri);
        hr = pOMFactory->CreatePage(
            &pageSize,                  // Page size
            L"en-US",                   // Page language
            opcPartUri,                 // Page part name
            &xpsPage);
        XPS_RECT rect = { 2,2,300, 200 };

        xpsPage->SetBleedBox(&rect);
        // Release this URI to reuse the interface pointer.
        if (NULL != opcPartUri) { opcPartUri->Release(); opcPartUri = NULL; }
        */

        // Create a page reference for the page.
        hr = pOMFactory->CreatePageReference(&pageSize, &xpsPageRef);

        // Add the fixed document sequence to the package.
        hr = xpsPackage_out->SetDocumentSequence(xpsFDS_out);

        // Get the document collection of the fixed document sequence
        //  and then add the document to the collection.
        hr = xpsFDS_out->GetDocuments(&fixedDocuments);
        hr = fixedDocuments->Append(xpsFD_out);

        // Get the page reference collection from the document
        //  and add the page reference and blank page.
        hr = xpsFD_out->GetPageReferences(&pageRefs);
        hr = pageRefs->Append(xpsPageRef);

        DoTraceMessage(RENDERFILTER_TRACE_INFO, " SetPage  ......");
        hr = xpsPageRef->SetPage(xpsPage);
        DoTraceMessage(RENDERFILTER_TRACE_INFO, " SetPage ooo ......");

        DoTraceMessage(RENDERFILTER_TRACE_INFO, " ReleaseXpsOMPackage_out save ......");
        hr = xpsPackage_out->WriteToFile(L"C:\\NuJee\\test1.xps", NULL, FILE_ATTRIBUTE_NORMAL, FALSE);
        DoTraceMessage(RENDERFILTER_TRACE_INFO, " ReleaseXpsOMPackage_out save ok......");

        // Release interface pointer
        //if (NULL != xpsPage) xpsPage->Release();
        if (NULL != pageRefs) pageRefs->Release();
        if (NULL != fixedDocuments) fixedDocuments->Release();
        if (NULL != xpsPageRef) xpsPageRef->Release();

        if (NULL != xpsFD_out) xpsFD_out->Release();
        if (NULL != xpsFDS_out) xpsFDS_out->Release();
        if (NULL != xpsPackage_out) xpsPackage_out->Release();
    }

    bool BaseConvertor::CreateXpsOMPageDocFromIFixedPage(
            const IFixedPage_t& pPageIn,
            const IXpsOMObjectFactory_t& pOMFactory,
            const IOpcFactory_t& pOpcFactory
        )
    {

        CreateXpsOMPackage_out(pOMFactory);

        //
        // Get additional page parameters (uri and stream)
        //
        IOpcPartUri_t pPartUri;
        IStream_t pPartStream;

        pPartStream = GetStreamFromPart(
            static_cast<IPartBase_t>(pPageIn)
        );
        pPartUri = CreateOpcPartUriFromPart(
            static_cast<IPartBase_t>(pPageIn),
            pOpcFactory
        );

        //
        // Call Xps Object Model to create the page resource
        //

        IXpsOMPage_t pPageOut;
        IXpsOMPage* xpsOMPage=NULL;
        DoTraceMessage(RENDERFILTER_TRACE_INFO, " CreatePageFromStream  ......");
        THROW_ON_FAILED_HRESULT(
            pOMFactory->CreatePageFromStream(
                pPartStream,
                pPartUri,
                CollectPageResources(pPageIn, pOMFactory, pOpcFactory),
                FALSE, // Do not reuse objects
                &pPageOut
            )
        );
        xpsOMPage = pPageOut.Detach();
        DoTraceMessage(RENDERFILTER_TRACE_INFO, " CreatePageFromStream ok ok ok......");

        ReleaseXpsOMPackage_out(xpsOMPage, pOMFactory);

        return true;
    }

//
//Routine Name:
//
//    CreateXpsOMPageFromIFixedPage
//
//Routine Description:
//
//    This is the main method called by the filter. It
//    proceeds to call the remaining Create* methods
//    to convert from the print pipeline Object Model 
//    to the Xps Object Model.
//    
//    Takes an IFixedPage (print pipeline Object Model) 
//    and converts it to an IXpsOMPage (Xps Object Model).
//
//Arguments:
//
//    pPageIn     - IFixedPage to convert
//    pOMFactory  - Xps Object Model Object Factory
//    pOpcFactory - Opc Object Factory
//
//Return Value:
//
//    IXpsOMPage_t (smart pointer)
//    Result IXpsOMPage
//
IXpsOMPage_t
BaseConvertor::CreateXpsOMPageFromIFixedPage(
   const IFixedPage_t          &pPageIn,
   const IXpsOMObjectFactory_t &pOMFactory,
   const IOpcFactory_t         &pOpcFactory
   )
{
    //
    // Get additional page parameters (uri and stream)
    //
    IOpcPartUri_t pPartUri;
    IStream_t pPartStream;

    pPartStream = GetStreamFromPart(
        static_cast<IPartBase_t>(pPageIn)
        );
    pPartUri = CreateOpcPartUriFromPart(
        static_cast<IPartBase_t>(pPageIn), 
        pOpcFactory
        );

    //
    // Call Xps Object Model to create the page resource
    //
    IXpsOMPage_t pPageOut;

    THROW_ON_FAILED_HRESULT(
        pOMFactory->CreatePageFromStream(
            pPartStream,
            pPartUri,
            CollectPageResources(pPageIn, pOMFactory, pOpcFactory),
            FALSE, // Do not reuse objects
            &pPageOut
            )
        );

    return pPageOut;
}

//
//Routine Name:
//
//    CreateImageFromIPartImage
//
//Routine Description:
//
//    Takes an IPartImage (print pipeline Object Model) 
//    and converts it to an IXpsOMImageResource (Xps Object Model).
//
//Arguments:
//
//    pImageIn    - IPartImage to convert
//    pOMFactory  - Xps Object Model Object Factory
//    pOpcFactory - Opc Object Factory
//
//Return Value:
//
//    IXpsOMImageResource_t (smart pointer)
//    Result IXpsOMImageResource
//
IXpsOMImageResource_t
BaseConvertor::CreateImageFromIPartImage(
    const IPartImage_t             &pImageIn,
    const IXpsOMObjectFactory_t    &pOMFactory,
    const IOpcFactory_t            &pOpcFactory
    )
{
    //
    // Get IPartBase parameters (stream and uri)
    //
    IStream_t            pPartStream;
    IOpcPartUri_t        pPartUri;

    pPartStream = GetStreamFromPart(
        static_cast<IPartBase_t>(pImageIn)
        );
    pPartUri = CreateOpcPartUriFromPart(
        static_cast<IPartBase_t>(pImageIn),
        pOpcFactory
        );

    //
    // Get the image type and convert it to the corresponding enum
    //
    BSTR_t strContentType;
    XPS_IMAGE_TYPE type = XPS_IMAGE_TYPE_WDP;
    
    THROW_ON_FAILED_HRESULT(
        pImageIn->GetImageProperties(&strContentType)
        );

    if (0 == _wcsicmp(strContentType, L"image/jpeg"))
    {
        type = XPS_IMAGE_TYPE_JPEG;
    }
    else if (0 == _wcsicmp(strContentType, L"image/png"))
    {
        type = XPS_IMAGE_TYPE_PNG;
    }
    else if (0 == _wcsicmp(strContentType, L"image/tiff"))
    {
        type = XPS_IMAGE_TYPE_TIFF;
    }
    else if (0 == _wcsicmp(strContentType, L"image/vnd.ms-photo"))
    {
        type = XPS_IMAGE_TYPE_WDP;
    }
    else
    {
        //
        // unknown content type
        //
        THROW_ON_FAILED_HRESULT(E_INVALIDARG);
    }

    //
    // Call Xps Object Model to create the image resource
    //
    IXpsOMImageResource_t   pImageOut;

    THROW_ON_FAILED_HRESULT(
        pOMFactory->CreateImageResource(
            pPartStream, 
            type, 
            pPartUri, 
            &pImageOut
            )
        );

    return pImageOut;
}

//
//Routine Name:
//
//    CreateProfileFromIPartColorProfile
//
//Routine Description:
//
//    Takes an IPartColorProfile (print pipeline Object Model) 
//    and converts it to an IXpsOMColorProfileResource (Xps Object Model).
//
//Arguments:
//
//    pProfileIn      - IPartColorProfile to convert
//    pOMFactory      - Xps Object Model Object Factory
//    pOpcFactory     - Opc Object Factory
//
//Return Value:
//
//    IXpsOMColorProfileResource_t (smart pointer)
//    Result IXpsOMColorProfileResource
//
IXpsOMColorProfileResource_t
BaseConvertor::CreateProfileFromIPartColorProfile(
    const IPartColorProfile_t      &pProfileIn,
    const IXpsOMObjectFactory_t    &pOMFactory,
    const IOpcFactory_t            &pOpcFactory
    )
{
    //
    // Get IPartBase parameters (stream and uri)
    //
    IStream_t            pPartStream;
    IOpcPartUri_t        pPartUri;

    pPartStream = GetStreamFromPart(
        static_cast<IPartBase_t>(pProfileIn)
        );
    pPartUri = CreateOpcPartUriFromPart(
        static_cast<IPartBase_t>(pProfileIn),
        pOpcFactory
        );

    //
    // Call Xps Object Model to create the color profile resource
    //
    IXpsOMColorProfileResource_t pProfileOut;

    THROW_ON_FAILED_HRESULT(
        pOMFactory->CreateColorProfileResource(
            pPartStream, 
            pPartUri, 
            &pProfileOut
            )
        );

    return pProfileOut;
}

//
//Routine Name:
//
//    CreateDictionaryFromIPartResourceDictionary
//
//Routine Description:
//
//    Takes an IPartResourceDictionary (print pipeline Object Model) 
//    and converts it to an IXpsOMRemoteDictionaryResource (Xps Object Model).
//
//Arguments:
//
//    pDictionaryIn     - IPartResourceDictionary to convert
//    pOMFactory        - Xps Object Model Object Factory
//    pOpcFactory       - Opc Object Factory
//    pResources        - The resources of the fixed page
//
//Return Value:
//
//    IXpsOMRemoteDictionaryResource_t (smart pointer)
//    Result IXpsOMRemoteDictionaryResource
//
IXpsOMRemoteDictionaryResource_t
BaseConvertor::CreateDictionaryFromIPartResourceDictionary(
    const IPartResourceDictionary_t    &pDictionaryIn,
    const IXpsOMObjectFactory_t        &pOMFactory,
    const IOpcFactory_t                &pOpcFactory,
    const IXpsOMPartResources_t        &pResources
    )
{
    //
    // Get IPartBase parameters (stream and uri)
    //
    IStream_t            pPartStream;
    IOpcPartUri_t        pPartUri;
    
    pPartStream = GetStreamFromPart(
        static_cast<IPartBase_t>(pDictionaryIn)
        );
    pPartUri = CreateOpcPartUriFromPart(
        static_cast<IPartBase_t>(pDictionaryIn),
        pOpcFactory
        );

    //
    // Call Xps Object Model to create the remote dictionary resource
    //
    IXpsOMRemoteDictionaryResource_t  pDictionaryOut;

    THROW_ON_FAILED_HRESULT(
        pOMFactory->CreateRemoteDictionaryResourceFromStream(
            pPartStream,
            pPartUri,
            pResources,
            &pDictionaryOut)
        );
    
    return pDictionaryOut;
}

//
//Routine Name:
//
//    CreateFontFromIPartFont
//
//Routine Description:
//
//    Takes an IPartFont (print pipeline Object Model) 
//    and converts it to an IXpsOMFontResource (Xps Object Model).
//
//Arguments:
//
//    pFontIn     - IPartFont to convert
//    pFactory    - Xps Object Model Object Factory
//    pOpcFactory - Opc Object Factory
//
//Return Value:
//
//    IXpsOMFontResource_t (smart pointer)
//    Result IXpsOMFontResource
//
IXpsOMFontResource_t
BaseConvertor::CreateFontFromIPartFont(
    const IPartFont_t              &pFontIn,
    const IXpsOMObjectFactory_t    &pOMFactory,
    const IOpcFactory_t            &pOpcFactory
    )
{
    //
    // Get IPartBase parameters (stream and uri)
    //
    IStream_t            pPartStream;
    IOpcPartUri_t        pPartUri;

    pPartStream = GetStreamFromPart(
        static_cast<IPartBase_t>(pFontIn)
        );
    pPartUri = CreateOpcPartUriFromPart(
        static_cast<IPartBase_t>(pFontIn),
        pOpcFactory
        );

    //
    // Get the font restriction
    //
    EXpsFontRestriction eFontRestriction = Xps_Restricted_Font_Installable;

    {
        IPartFont2_t            pFont2In;

        if (SUCCEEDED(pFontIn->QueryInterface(__uuidof(IPartFont2), reinterpret_cast<void **>(&pFont2In))))
        {
            pFont2In->GetFontRestriction(&eFontRestriction);
        }
    }

    //
    // Get the font obfuscation
    //
    EXpsFontOptions eFontOptions;

    {
        BSTR_t                  contentType;

        THROW_ON_FAILED_HRESULT(
            pFontIn->GetFontProperties(&contentType, &eFontOptions)
            );
    }

    //
    // It is necessary to combine the obfuscation and restriction 
    // attributes from the print pipeline into the one parameter that
    // the Xps Object Model consumes.
    //
    
    XPS_FONT_EMBEDDING omEmbedding;

    if (eFontOptions == Font_Normal)
    {
        omEmbedding = XPS_FONT_EMBEDDING_NORMAL;
    }
    else if (eFontOptions == Font_Obfusticate &&
             (eFontRestriction &
                (Xps_Restricted_Font_PreviewPrint | 
                 Xps_Restricted_Font_NoEmbedding)))
    {
        //
        // If the font is obfuscated, and either the PreviewPrint or 
        // NoEmbedding restriction flags are set, then create a
        // Restricted font
        //
        omEmbedding = XPS_FONT_EMBEDDING_RESTRICTED;
    }
    else
    {
        omEmbedding = XPS_FONT_EMBEDDING_OBFUSCATED;
    }

    //
    // Call Xps Object Model to create the font resource
    //
    IXpsOMFontResource_t pFontOut;

    THROW_ON_FAILED_HRESULT(
        pOMFactory->CreateFontResource(
            pPartStream, 
            omEmbedding, 
            pPartUri,
            FALSE,  // fonts received from the pipeline are already de-obfuscated
            &pFontOut
            )
        );

    return pFontOut;
}

//
//Routine Name:
//
//    CollectPageResources
//
//Routine Description:
//
//    Iterates over all of the resources related to
//    a fixed page and adds them to a resource 
//    collection.
//
//Arguments:
//
//    pPage       - The page to query for resources
//    pOMFactory  - Xps Object Model Object Factory
//    pOpcFactory - Opc Object Factory
//
//Return Value:
//
//    IXpsOMPartResources_t (smart pointer)
//    The resource collection of all of the resources of the page
//
IXpsOMPartResources_t
BaseConvertor::CollectPageResources(
    const IFixedPage_t             &pPage,
    const IXpsOMObjectFactory_t    &pOMFactory,
    const IOpcFactory_t            &pOpcFactory
    )
{
    IXpsOMPartResources_t pResources;

    IXpsOMFontResourceCollection_t              pFonts;
    IXpsOMImageResourceCollection_t             pImages;
    IXpsOMColorProfileResourceCollection_t      pProfiles;
    IXpsOMRemoteDictionaryResourceCollection_t  pDictionaries;

    //
    // collection of resource dictionaries saved for later processing.
    //
    ResourceDictionaryList_t dictionaryList;

    //
    // Create the resource collection and get all of the
    // resource-specific sub-collections.
    //
    THROW_ON_FAILED_HRESULT(
        pOMFactory->CreatePartResources(&pResources)
        );
    THROW_ON_FAILED_HRESULT(
        pResources->GetFontResources(&pFonts)
        );
    THROW_ON_FAILED_HRESULT(
        pResources->GetImageResources(&pImages)
        );
    THROW_ON_FAILED_HRESULT(
        pResources->GetColorProfileResources(&pProfiles)
        );
    THROW_ON_FAILED_HRESULT(
        pResources->GetRemoteDictionaryResources(&pDictionaries)
        );
    //
    // Get the XpsPartIterator and iterate through all of the parts
    // related to this fixed page.
    //
    IXpsPartIterator_t itPart;

    THROW_ON_FAILED_HRESULT(
        pPage->GetXpsPartIterator(&itPart)
        );

    for (;  !itPart->IsDone(); itPart->Next())
    {
        BSTR_t uri;
        IUnknown_t pUnkPart;

        THROW_ON_FAILED_HRESULT(
            itPart->Current(&uri, &pUnkPart)
            );

        IPartFont_t               pFontPart;
        IPartImage_t              pImagePart;
        IPartColorProfile_t       pProfilePart;
        IPartResourceDictionary_t pDictionaryPart;

        if (SUCCEEDED(pUnkPart.QueryInterface(&pFontPart)))
        {
            //
            // Convert the font part to Xps Object Model and add it to the 
            // font resource collection
            //
            THROW_ON_FAILED_HRESULT(
                pFonts->Append(
                    CreateFontFromIPartFont(
                        pFontPart,
                        pOMFactory,
                        pOpcFactory
                        )
                    )
                );
        }
        else if (SUCCEEDED(pUnkPart.QueryInterface(&pImagePart)))
        {
            //
            // Convert the image part to Xps Object Model and add it to the 
            // image resource collection
            //
            THROW_ON_FAILED_HRESULT(
                pImages->Append(
                    CreateImageFromIPartImage(
                        pImagePart, 
                        pOMFactory,
                        pOpcFactory
                        )
                    )
                );
        }
        else if (SUCCEEDED(pUnkPart.QueryInterface(&pProfilePart)))
        {
            //
            // Convert the color profile part to Xps Object Model and add it 
            // to the color profile resource collection
            //
            THROW_ON_FAILED_HRESULT(
                pProfiles->Append(
                    CreateProfileFromIPartColorProfile(
                        pProfilePart, 
                        pOMFactory,
                        pOpcFactory
                        )
                    )
                );
        }
        else if (SUCCEEDED(pUnkPart.QueryInterface(&pDictionaryPart)))
        {
            //
            // In order to process the remote resource dictionary, all of 
            // its linked resources must be present in pResources. To ensure 
            // this, we delay the conversion of the remote resource
            // dictionaries until all of the other resources have been converted.
            //
            dictionaryList.push_back(pDictionaryPart);
        }
        else
        {
            DoTraceMessage(RENDERFILTER_TRACE_INFO, L"Unhandled Page Resource");
        }
    }

    for (ResourceDictionaryList_t::const_iterator it = dictionaryList.begin();
            it != dictionaryList.end();
            ++it)
    {
        //
        // Convert the remote dictionary to Xps Object Model and add it 
        // to the remote dictionary collection
        //
        THROW_ON_FAILED_HRESULT(
                pDictionaries->Append(
                    CreateDictionaryFromIPartResourceDictionary(
                        *it, 
                        pOMFactory,
                        pOpcFactory,
                        pResources
                        )
                    )
                );
    }
    
    return pResources;
}

//
//Routine Name:
//
//    GetStreamFromPart
//
//Routine Description:
//
//    Gets the IStream from this part.
//
//Arguments:
//
//    pPart           - An Xps Part
//
//Return Value:
//    
//    IStream_t (smart pointer)
//    The stream of the part's content
//
IStream_t
BaseConvertor::GetStreamFromPart(
    const IPartBase_t  &pPart
    )
{
    //
    // Get the IPrintReadStream for the part from the pipeline Object Model
    //
    IPrintReadStream_t   pStream;
    THROW_ON_FAILED_HRESULT(
        pPart->GetStream(&pStream)
        );

    return CreateIStreamFromIPrintReadStream(pStream);
}

//
//Routine Name:
//
//    CreateIStreamFromIPrintReadStream
//
//Routine Description:
//
//    Creates an IStream from an IPrintReadStream.
//
//Arguments:
//
//    pReadStream - A Print Pipeline IPrintReadStream
//
//Return Value:
//    
//    IStream_t (smart pointer)
//    A stream with the same content as the argument stream.
//
IStream_t
BaseConvertor::CreateIStreamFromIPrintReadStream(
    const IPrintReadStream_t   &pReadStream
    )
{
    //
    // Get the size of the stream
    //
    ULONGLONG tmpPos;
    size_t partSize;

    THROW_ON_FAILED_HRESULT(
        pReadStream->Seek(0, SEEK_END, &tmpPos)
        );

    //
    // GlobalAlloc can only allocate size_t bytes, so
    // throw if the part is larger than that
    //
    THROW_ON_FAILED_HRESULT(
        ULongLongToSizeT(tmpPos, &partSize)
        );

    THROW_ON_FAILED_HRESULT(
        pReadStream->Seek(0, SEEK_SET, &tmpPos)
        );

    //
    // Allocate an HGLOBAL for the part cache
    //
    SafeHGlobal_t pHBuf(
                    new SafeHGlobal(GMEM_FIXED, partSize)
                    );

    //
    // Read the part into the cache
    //
    {
        //
        // Lock the HGLOBAL and get the address of the buffer
        // from the RAII lock object
        //
        HGlobalLock_t lock = pHBuf->Lock();
        BYTE *pBuffer = lock->GetAddress();

        //
        // Allow the number of bytes to read to be clipped to max ULONG
        // and then spin on fEOF until the stream is exhausted
        //
        ULONG numToRead;

        if (FAILED(SizeTToULong(partSize, &numToRead)))
        {
            numToRead = MAXUINT;
        }

        BOOL fEOF;
        ULONG numRead;
        size_t pos = 0;

        //
        // Iterate until all bytes from the stream 
        // have been read into the buffer
        //
        do
        {
            THROW_ON_FAILED_HRESULT(
                pReadStream->ReadBytes(pBuffer + pos, numToRead, &numRead, &fEOF)
                );

            pos += numRead;
        } while (!fEOF && numRead);
    }

    //
    // Create an IStream from the part cache
    //
    IStream_t pIStream = pHBuf->ConvertToIStream();

    LARGE_INTEGER zero = {0};
    THROW_ON_FAILED_HRESULT(
        pIStream->Seek(zero, SEEK_SET, NULL)
        );

    return pIStream;
}

//
//Routine Name:
//
//    CreateOpcPartUriFromPart
//
//Routine Description:
//
//    Gets the Opc Uri from the Xps Part.
//
//Arguments:
//
//    pPart       - An Xps Part
//    pFactory    - Opc Factory
//
//Return Value:
//
//    IOpcPartUri_t (smart pointer)
//    The Uri of the part
//
IOpcPartUri_t
BaseConvertor::CreateOpcPartUriFromPart(
    const IPartBase_t      &pPart,
    const IOpcFactory_t    &pFactory
    )
{
    BSTR_t strPartUri;
    
    THROW_ON_FAILED_HRESULT(
        pPart->GetUri(&strPartUri)
        );
   
    IOpcPartUri_t   pPartUri;
    THROW_ON_FAILED_HRESULT(
        pFactory->CreatePartUri(strPartUri, &pPartUri)
        );
    return pPartUri;
}

} // namespace NuJee_EMF_v4_Printer_Driver_Render_Filter
