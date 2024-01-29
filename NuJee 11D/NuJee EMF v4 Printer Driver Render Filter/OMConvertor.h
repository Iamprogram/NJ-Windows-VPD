//
// File Name:
//
//    omconvertor.h
//
// Abstract:
//
//    Object model conversion routines.
//

#pragma once
#include "BaseConvertor.h"

namespace NuJee_EMF_v4_Printer_Driver_Render_Filter
{
#define COUNTOF(x) (sizeof(x)/sizeof(*x))

class OMConvertor : public BaseConvertor
{

    public:
        OMConvertor();
        virtual ~OMConvertor();
    public:

        HRESULT static GetLastErrorAsHResult(void);
        HRESULT static ProcessTicket(_In_ IPartPrintTicket* pIPrintTicket);
        HRESULT static AddFontToPage(
            _In_    IPartFont* pNewFont,
            _In_    IPrintWriteStream* pFontStream,
            _In_    IFixedPage* pNewPage
        );
        HRESULT static AddTextToPage(
            _In_    PCWSTR              pszReferencedFont,
            _In_    IPrintWriteStream* pPageMarkupStream
        );
        HRESULT  static ModifyContent(
            _In_    IPrintReadStream* pRead,
            _In_    IPrintWriteStream* pWrite
        );

        void static CreateXpsOMTest(
            const IXpsOMObjectFactory_t& pOMFactory
        );

        void static ReleaseXpsOMPackage_out(IXpsOMPage* xpsPage, const IXpsOMObjectFactory_t& pOMFactory);
        void static CreateXpsOMPackage_out(const IXpsOMObjectFactory_t& pOMFactory);
        bool static CreateXpsOMPageDocFromIFixedPage(
            const IFixedPage_t& pPageIn,
            const IXpsOMObjectFactory_t& pFactory,
            const IOpcFactory_t& pOpcFactory
        );

        // Write Text to an XPS OM
        HRESULT static
            WriteText_AddTextToPage(
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
            );
        //
    // Top-level Create Page routine
    //
        IXpsOMPage_t static
            CreateXpsOMPageFromIFixedPage(
                const IFixedPage_t& pPageIn,
                const IXpsOMObjectFactory_t& pFactory,
                const IOpcFactory_t& pOpcFactory
            );

        //
        // Individual Part conversion routines
        //
        IXpsOMImageResource_t static
            CreateImageFromIPartImage(
                const IPartImage_t& pImageIn,
                const IXpsOMObjectFactory_t& pFactory,
                const IOpcFactory_t& pOpcFactory
            );

        IXpsOMColorProfileResource_t static
            CreateProfileFromIPartColorProfile(
                const IPartColorProfile_t& pProfileIn,
                const IXpsOMObjectFactory_t& pFactory,
                const IOpcFactory_t& pOpcFactory
            );

        IXpsOMRemoteDictionaryResource_t static
            CreateDictionaryFromIPartResourceDictionary(
                const IPartResourceDictionary_t& pDictionaryIn,
                const IXpsOMObjectFactory_t& pFactory,
                const IOpcFactory_t& pOpcFactory,
                const IXpsOMPartResources_t& pResources
            );

        IXpsOMFontResource_t static
            CreateFontFromIPartFont(
                const IPartFont_t& pFontIn,
                const IXpsOMObjectFactory_t& pFactory,
                const IOpcFactory_t& pOpcFactory
            );

        // 
        // Utility Routines
        //
        IStream_t static
            GetStreamFromPart(
                const IPartBase_t& pPart
            );

        IOpcPartUri_t static
            CreateOpcPartUriFromPart(
                const IPartBase_t& pPart,
                const IOpcFactory_t& pOpcFactory
            );

        IXpsOMPartResources_t static
            CollectPageResources(
                const IFixedPage_t& pPage,
                const IXpsOMObjectFactory_t& pFactory,
                const IOpcFactory_t& pOpcFactory
            );

        IStream_t static
            CreateIStreamFromIPrintReadStream(
                const IPrintReadStream_t& pReadStream
            );
};

} // namespace NuJee_EMF_v4_Printer_Driver_Render_Filter
