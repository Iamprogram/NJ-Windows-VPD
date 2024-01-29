// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// File Name:
//
//    rasinterface.cpp
//
// Abstract:
//
//    Class to wrap rasterization-related calculations, interactions with
//    the Xps Rasterization Service, Xps Rasterization Service Callback,
//    and eventual raster output (i.e. TIFF encoding)
//

#include "precomp.h"
#include "WppTrace.h"
#include "CustomWppCommands.h"
#include "Exception.h"
#include "filtertypes.h"
#include "UnknownBase.h"
//#include "xpsrasfilter.h"
#include "RenderFilter.h"
#include "OMConvertor.h"
#include "rasinterface.h"
#include "BitmapHandler.h"

#include "rasinterface.tmh"

#include "PreEMFWICBitmap.h"


namespace NuJee_EMF_v4_Printer_Driver_Render_Filter
{

//
//Routine Name:
//
//    RasterizationInterface::CreateRasterizationInterface
//
//Routine Description:
//
//    Static factory method that creates an instance of
//    RasterizationInterface.
//
//Arguments:
//
//    pPropertyBag    - Property Bag
//    pStream         - Filter output stream (IPrintWriteStream)
//
//Return Value:
//
//    RasterizationInterface_t (smart ptr)
//    The new RasterizationInterface.
//
RasterizationInterface_t
RasterizationInterface::CreateRasterizationInterface(
    const IPrintPipelinePropertyBag_t &pPropertyBag,
    const IPrintWriteStream_t &pStream
    )
{
    Variant_t                   varRasFactory;
    IXpsRasterizationFactory_t  pXPSRasFactory;

    //
    // Get the Xps Rasterization Service factory. Since XpsRasterService.dll
    // is specified as an OptionalFilterServiceProvider in the filter pipeline 
    // configuration file, the factory may not be available in the property bag 
    // (e.g. when running on Windows Vista, Windows XP, etc). In this case,
    // this call will fail and the filter could fail, as we do here, or could 
    // default to some other behavior.
    //
    THROW_ON_FAILED_HRESULT(
        pPropertyBag->GetProperty(
            L"MS_IXpsRasterizationFactory", 
            &varRasFactory
            )
        );

    IUnknown_t pUnk(varRasFactory.punkVal);

    THROW_ON_FAILED_HRESULT(
        pUnk.QueryInterface(&pXPSRasFactory)
        );

    //
    // Prepare the rasterization/encode/stream interface
    //
    TiffStreamBitmapHandler_t pBitmapHandler(
        TiffStreamBitmapHandler::CreateTiffStreamBitmapHandler(pStream)
        );

    RasterizationInterface_t pReturnInterface(
                                new RasterizationInterface(
                                        pXPSRasFactory, 
                                        pBitmapHandler
                                        )
                                );

    return pReturnInterface;
}

//
//Routine Name:
//
//    RasterizationInterface::RasterizationInterface
//
//Routine Description:
//
//    Construct the Rasterization Interface with the
//    IXpsRasterizationFactory interface and bitmap
//    handler.
//
//Arguments:
//
//    pRasFactory     - Xps Rasterization Service object factory
//    pBitmapHandler  - Class to handle band bitmaps
//
RasterizationInterface::RasterizationInterface(
        const IXpsRasterizationFactory_t    &pRasFactory,
        TiffStreamBitmapHandler_t           pBitmapHandler
        ) : m_pXPSRasFactory(pRasFactory), 
            m_pBitmapHandler(pBitmapHandler)
{
}

//
//Routine Name:
//
//    RasterizationInterface::FinishRasterization
//
//Routine Description:
//
//    Tell the Rasterization Interface that the last
//    page has been rasterized.
//
//Arguments:
//
//    None
//
void
RasterizationInterface::FinishRasterization()
{
    m_pBitmapHandler->WriteFooter();
}


//
//Routine Name:
//
//    RasterizationInterface::RasterizeAndStreamPage
//
//Routine Description:
//
//    Given an IXpsOMPage and a set of Print Ticket
//    parameters, this method invokes the Xps Rasterization
//    Service for each band of the page, and outputs the
//    resultant raster data.
//
//Arguments:
//
//    pPage               - page to rasterize
//    printTicketparams   - raw parameters from the print ticket(s)
//
void
RasterizationInterface::RasterizePage(
    const int IdPagesCount,
    const IXpsOMPage_t                 &pPage,
    const ParametersFromPrintTicket    &printTicketParams,
    const FilterLiveness_t             &pLiveness, 
    CHAR* outputPDFFile
    )
{
    //
    // Calculate rasterization parameters
    //
    ParametersFromFixedPage fixedPageParams;
    THROW_ON_FAILED_HRESULT(
        pPage->GetPageDimensions(&fixedPageParams.fixedPageSize)
        );
    THROW_ON_FAILED_HRESULT(
        pPage->GetBleedBox(&fixedPageParams.bleedBoxRect)
        );
    THROW_ON_FAILED_HRESULT(
        pPage->GetContentBox(&fixedPageParams.contentBoxRect)
        );

    RasterizationParameters rastParams(
        printTicketParams,
        fixedPageParams
        );

    //
    // Create the Rasterizer
    //
    IXpsRasterizer_t rasterizer;
    THROW_ON_FAILED_HRESULT(
        m_pXPSRasFactory->CreateRasterizer(
                            pPage,
                            rastParams.rasterizationDPI, 
                            XPSRAS_RENDERING_MODE_ANTIALIASED, 
                            XPSRAS_RENDERING_MODE_ANTIALIASED, 
                            &rasterizer
                            )
        );

    //
    // Set the minimal line width to 1 pixel
    //
    THROW_ON_FAILED_HRESULT(
        rasterizer->SetMinimalLineWidth(1)
        );

    //
    // Loop over bands
    //
    INT bandOriginY = 0;
    INT IdCount = 0;
    m_Frame_bitmapHeight = 0;
    m_Frame_bitmapHeight = 0;
    while (pLiveness->IsAlive() && bandOriginY < rastParams.rasterHeight)
    {
        DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"Rasterizing Band");
        IWICBitmap_t bitmap;
        //
        // Calculate the height of this band
        //
        // for DeRaster, capture whole RECT
        //INT bandHeight = rastParams.bandHeight;
        INT bandHeight = (INT)rastParams.rasterHeight;
        if (bandOriginY + rastParams.bandHeight >= rastParams.rasterHeight)
        {
            bandHeight = rastParams.rasterHeight - bandOriginY;
        }
        UINT sc_bitmapWidth = rastParams.rasterWidth;
        UINT sc_bitmapHeight = bandHeight;
        DoTraceMessage(RENDERFILTER_TRACE_INFO, " rastParams Bitmap bitmapWidth== [%u], sc_bitmapHeight = [%u] ", sc_bitmapWidth, sc_bitmapHeight);

        //
        // Rasterize this band
        //
        {
            DoTraceMessage(RENDERFILTER_TRACE_INFO, " =======rastParams Bitmap rastParams.originX= [%u], rastParams.originY = [%u] ", rastParams.originX, rastParams.originY);
            DoTraceMessage(RENDERFILTER_TRACE_INFO, " rastParams Bitmap bandOriginY = [%u], rastParams.rasterWidth = [%u] ", bandOriginY, rastParams.rasterWidth);
            DoTraceMessage(RENDERFILTER_TRACE_INFO, " =======rastParams Bitmap bandHeight = [%u]", bandHeight);
            HRESULT hr = rasterizer->RasterizeRect(
                rastParams.originX,
                bandOriginY + rastParams.originY,
                rastParams.rasterWidth,
                bandHeight,
                static_cast<IXpsRasterizerNotificationCallback*>(pLiveness),
                &bitmap
            );

            //
            // Do not throw if we have cancelled rasterization
            //
            if (hr == HRESULT_FROM_WIN32(ERROR_PRINT_CANCELLED))
            {
                DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"Rasterization Cancelled");
                return;
            }

            THROW_ON_FAILED_HRESULT(hr);
        }
        bandOriginY += bandHeight;
        //
        // The resolution of the bitmap defaults to the
        // rasterization DPI, which includes the scaling factor.
        // We want the output bitmap to reflect the destination DPI.
        //
        THROW_ON_FAILED_HRESULT(
            bitmap->SetResolution(
                printTicketParams.destDPI,
                printTicketParams.destDPI
            )
        );

        UINT bitmapWidth, bitmapHeight;
        THROW_ON_FAILED_HRESULT(
            bitmap->GetSize(&bitmapWidth, &bitmapHeight)
        );
        m_Frame_bitmapWidth = bitmapWidth;
        m_Frame_bitmapHeight += sc_bitmapHeight;

        THROW_ON_FAILED_HRESULT(
            bitmap->GetResolution(&m_xDPI, &m_yDPI)
        );
        THROW_ON_FAILED_HRESULT(
            bitmap->GetPixelFormat(&m_format)
        );

    }

    /*
    //
    // // Create an instance of a WIC Imaging Factory
    //
    THROW_ON_FAILED_HRESULT(
        ::CoCreateInstance(
            CLSID_WICImagingFactory,
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(IWICImagingFactory),
            reinterpret_cast<LPVOID*>(&m_pWICFactory)
        )
    );

    WCHAR filename[MAX_PATH];
    StringCchPrintf(filename, MAX_PATH, L"C:\\NuJee\\outputtry.tiff", IdCount);
    THROW_ON_FAILED_HRESULT(
        m_pWICFactory->CreateStream(&m_pWICStream);
    );
    THROW_ON_FAILED_HRESULT(
        m_pWICStream->InitializeFromFilename(filename, GENERIC_WRITE);
    );
    //
    // Create a WIC TIFF Encoder on the stream
    //
    THROW_ON_FAILED_HRESULT(
        m_pWICFactory->CreateEncoder(GUID_ContainerFormatTiff, NULL, &m_pWICEncoder)
    );
    THROW_ON_FAILED_HRESULT(
        m_pWICEncoder->Initialize(m_pWICStream, WICBitmapEncoderNoCache)
    );

    //
    // Create a new frame for the band and configure it
    //
    THROW_ON_FAILED_HRESULT(
        m_pWICEncoder->CreateNewFrame(&m_pWICFrame, &m_pFramePropertyBag)
    );
    {
        //
        // Write the compression method to the frame's property bag
        //
        PROPBAG2 option = { 0 };
        option.pstrName = L"TiffCompressionMethod";

        VARIANT varValue;
        VariantInit(&varValue);
        varValue.vt = VT_UI1;
        varValue.bVal = WICTiffCompressionLZW;

        THROW_ON_FAILED_HRESULT(
            m_pFramePropertyBag->Write(
                1, // number of properties being set
                &option,
                &varValue
            )
        );
    }

    THROW_ON_FAILED_HRESULT(
        m_pWICFrame->Initialize(m_pFramePropertyBag)
    );
    //
    // // Set the frame's size
    //
    DoTraceMessage(RENDERFILTER_TRACE_INFO, " pWICFrame w = [%u], h = [%u] ", m_Frame_bitmapWidth, m_Frame_bitmapHeight);
    THROW_ON_FAILED_HRESULT(
        m_pWICFrame->SetSize(m_Frame_bitmapWidth, m_Frame_bitmapHeight)
    );
    //
    // // Set the frame's resolution
    //
    THROW_ON_FAILED_HRESULT(
        m_pWICFrame->SetResolution(m_xDPI, m_yDPI)
    );

    //
    // Set the frame's pixel format
    //
    THROW_ON_FAILED_HRESULT(
        m_pWICFrame->SetPixelFormat(&m_format)
    );
    */

    //
    // Loop over bands
    //
    bandOriginY = 0;
    IdCount = 0;
    UINT m_Frame_rectX=0, m_Frame_rectY=0;
    while (pLiveness->IsAlive() && bandOriginY < rastParams.rasterHeight)
    {
        DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"Rasterizing Band");
        IWICBitmap_t bitmap;
        //
        // Calculate the height of this band
        //
        INT bandHeight = (INT)rastParams.rasterHeight;
        if (bandOriginY + rastParams.bandHeight >= rastParams.rasterHeight)
        {
            bandHeight = rastParams.rasterHeight - bandOriginY;
        }
        UINT sc_bitmapWidth = rastParams.rasterWidth;
        UINT sc_bitmapHeight = bandHeight;
        DoTraceMessage(RENDERFILTER_TRACE_INFO, " rastParams Bitmap bitmapWidth== [%u], sc_bitmapHeight = [%u] ", sc_bitmapWidth, sc_bitmapHeight);
        //
        // Rasterize this band
        //
        {
            HRESULT hr = rasterizer->RasterizeRect(
                            rastParams.originX,
                            bandOriginY + rastParams.originY,
                            rastParams.rasterWidth,
                            bandHeight, 
                            static_cast<IXpsRasterizerNotificationCallback *>(pLiveness), 
                            &bitmap
                            );

            //
            // Do not throw if we have cancelled rasterization
            //
            if (hr == HRESULT_FROM_WIN32(ERROR_PRINT_CANCELLED))
            {
                DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"Rasterization Cancelled");
                return;
            }

            THROW_ON_FAILED_HRESULT(hr);
        }

        bandOriginY += bandHeight;

        //
        // The resolution of the bitmap defaults to the
        // rasterization DPI, which includes the scaling factor.
        // We want the output bitmap to reflect the destination DPI.
        //
        THROW_ON_FAILED_HRESULT(
            bitmap->SetResolution(
                printTicketParams.destDPI,
                printTicketParams.destDPI
                )
            );

        //
        // Encode the raster data as TIFF and stream out
        //
        DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"ProcessBitmapSaveToFileTest Rasterizing Band hop trying SaveWICBitmapToImageFile [%u] ", IdCount);
        //m_pBitmapHandler->ProcessBitmap(bitmap);
        m_Frame_rectX = 0;
        //m_Frame_rectY = m_Frame_rectY;
        WICRect rect = { 0, 0, 0, 0 };
        rect.X = 0;
        rect.Y = m_Frame_rectY;
        rect.Width = sc_bitmapWidth;
        rect.Height = sc_bitmapHeight;
        //m_pBitmapHandler->ProcessBitmapSaveToFile(bitmap, IdCount++);
        DoTraceMessage(RENDERFILTER_TRACE_INFO, "RasterizationParameters xy pWICFrame x== [%u], y = [%u] ", rect.X, rect.Y);
        DoTraceMessage(RENDERFILTER_TRACE_INFO, "RasterizationParameters wh pWICFrame bitmapWidth== [%u], bitmapHeight = [%u] ", rect.Width, rect.Height);
        //m_pBitmapHandler->ProcessBitmapSaveToOneFile(bitmap, m_pWICStream, m_pWICFrame, rect);
        if (1 == 0) {
            m_pBitmapHandler->ProcessBitmapSaveToJPGFiles(bitmap, IdPagesCount/*IdCount++*/, rect, outputPDFFile);
        }else if (1 == 0) {
            m_pBitmapHandler->ProcessBitmapSaveToPNGFiles(bitmap, IdPagesCount/*IdCount++*/, rect, outputPDFFile);
        }
        else {
            m_pBitmapHandler->ProcessBitmapSaveToTiffFiles(bitmap, IdPagesCount/*IdCount++*/, rect, outputPDFFile);
        }
        m_Frame_rectY += bandHeight;

        //PreEMFWICBitmap::SaveWICBitmapToImageFile(bitmap, sc_bitmapWidth, sc_bitmapHeight);
        DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"Rasterizing Band OK");
    }

    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"All Rasterizing Band will commit finally");
    //
    // // Commit the frame and encoder
    //
    //THROW_ON_FAILED_HRESULT(
    //    m_pWICFrame->Commit()
    //);
    //THROW_ON_FAILED_HRESULT(
    //    m_pWICEncoder->Commit()
    //);
    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"All Rasterizing commit OK finally");

}

//
//Routine Name:
//
//    RasterizationParameters::RasterizationParameters
//
//Routine Description:
//
//    Given a set of fixed page parameters and a set of Print Ticket
//    parameters, this constructor calculates the parameters necessary to
//    invoke the Xps Rasterization Service.
//
//Arguments:
//
//    printTicketparams    - raw parameters from the print ticket(s)
//    fixedPageParams     - raw parameters from the fixed page
//
RasterizationParameters::RasterizationParameters(
    const ParametersFromPrintTicket &printTicketParams,
    const ParametersFromFixedPage   &fixedPageParams
    )
{
    //
    // Rasterize the entire physical page at the desired resolution. The fixed page
    // is scaled and traslated to place it correctly witin the physical page.
    //
    rasterHeight = static_cast<INT>(
                        printTicketParams.physicalPageSize.height 
                            * (printTicketParams.destDPI / xpsDPI) 
                        + 0.5
                        );
    rasterWidth  = static_cast<INT>(
                        printTicketParams.physicalPageSize.width 
                            * (printTicketParams.destDPI / xpsDPI) 
                        + 0.5
                        );

    //
    // Determine the source rectangle for the scale operation
    //
    XPS_RECT srcRect = {0,0,0,0};

    switch(printTicketParams.scaling)
    {
        case SCALE_BLEEDTOIMAGEABLE:

            srcRect = fixedPageParams.bleedBoxRect;

            break;
        default:
            DoTraceMessage(RENDERFILTER_TRACE_ERROR, L"Unknown scaling type");
            THROW_ON_FAILED_HRESULT(E_UNEXPECTED);
            break;
    }

    //
    // Determine the destination rectangle for the scale operation
    //
    XPS_RECT destRect = {0,0,0,0};

    switch(printTicketParams.scaling)
    {
        case SCALE_BLEEDTOIMAGEABLE:

            destRect = printTicketParams.imageableArea;

            break;
        default:
            DoTraceMessage(RENDERFILTER_TRACE_ERROR, L"Unknown scaling type");
            THROW_ON_FAILED_HRESULT(E_UNEXPECTED);
            break;
    }


    //
    // Because we want to fit the fixed page into a physical page of
    // potentially different aspect ratio, it is necessary to calculate
    // the scaling factor assuming either the height or width constrains
    // the operation; the rasterization dpi is then the smaller of the two.
    //
    //
    // The basic calculation for scaling factor is:
    //
    //      Scaling Factor = (Destination Dimension / Source Dimension)
    //
    // Multiplying by desired Destination DPI we get the DPI at which to 
    //    rasterize the source content in order to get the desired size
    //
    //      Rasterization DPI = Scaling Factor * Desired Destination DPI
    //
    {
        FLOAT heightScalingFactor = destRect.height / srcRect.height;
        FLOAT widthScalingFactor  = destRect.width / srcRect.width;

        FLOAT heightRastDPI   = heightScalingFactor * printTicketParams.destDPI;
        FLOAT widthRastDPI    = widthScalingFactor * printTicketParams.destDPI;

        rasterizationDPI = min(heightRastDPI, widthRastDPI);
    }

    //
    // To determine the rasterization origin, subtract the translation
    // due to the destination rectangle (e.g. imageable area) from the 
    // translation due to the choice of scale (e.g. bleed box origin)
    //
    {
        FLOAT srcOffsetX = srcRect.x * rasterizationDPI / xpsDPI;
        FLOAT srcOffsetY = srcRect.y * rasterizationDPI / xpsDPI;

        FLOAT destOffsetX = destRect.x * printTicketParams.destDPI / xpsDPI;
        FLOAT destOffsetY = destRect.y * printTicketParams.destDPI / xpsDPI;

        originX = static_cast<INT>(srcOffsetX - destOffsetX - 0.5);
        originY = static_cast<INT>(srcOffsetY - destOffsetY - 0.5);
    }

    //
    // The height of each band is determined by the maximum band size (in bytes) 
    // divided by 4 bytes per pixel to get the total pixels, and then divided by
    // the width of the raster data. This results in a band bitmap that is close 
    // to the target band size.
    //
    bandHeight = (RasterizationInterface::ms_targetBandSize / 4) / rasterWidth;

    if (bandHeight == 0)
    {
        //
        // The physical page is too wide to rasterize at this 
        // maximum band size and dpi. Throw.
        //
        THROW_ON_FAILED_HRESULT(
            E_OUTOFMEMORY
            );
    }
}

} // namespace xpsrasfilter
