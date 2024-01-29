// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// File Name:
//
//    BitmapHandler.cpp
//
// Abstract:
//
//    Abstract class that encapsulates the processing done to each 
//    individual band bitmap, as well as the concrete implemention
//    that streams the bands as Tiffs to the output stream.
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

#include "BitmapHandler.tmh"

#include "UtilHelper.h"
// header files for file I/O
#include <iostream>
#include <fstream>

namespace NuJee_EMF_v4_Printer_Driver_Render_Filter
{
    //typedef unsigned long ULONG;
    int TiffStreamBitmapHandler::PrintOutputPDFStream(const IPrintWriteStream_t& pStream, const WCHAR* outputFolder)
    {
        char basename[256];
        char srcpngfname[256];
        char fname[256];

        size_t origsize = wcslen(outputFolder) + 1;
        size_t convertedChars = 0;
        char strConcat[] = "";
        size_t strConcatsize = (strlen(strConcat) + 1) * 2;
        const size_t newsize = origsize * 2;
        char* nstring = new char[newsize + strConcatsize];
        wcstombs_s(&convertedChars, nstring, newsize, outputFolder, _TRUNCATE);
        _mbscat_s((unsigned char*)nstring, newsize + strConcatsize, (unsigned char*)strConcat);
        strcpy_s(basename, nstring);
        //pdfname
        strcpy_s(fname, basename);
        strcat_s(fname, ".pdf");
        delete[]nstring;
        DoTraceMessage(RENDERFILTER_TRACE_INFO, " PrintOutputPDFStream fname= [%ws]", outputFolder);

        //// Create an empty HGLOBAL to hold the pdf cache
        SafeHGlobal_t pHG(new SafeHGlobal(GMEM_SHARE | GMEM_MOVEABLE, 0));
        //// Create a stream to the buffer so that readin pdf
        IStream_t pIStream;
        ::CreateStreamOnHGlobal(*pHG, FALSE, &pIStream); // Do NOT Free the HGLOBAL on Release of the stream

        HRESULT hr;
        ULONG lsize = 0;
        char* data=NULL;
        std::ifstream pdffile(fname, std::ios::binary);
        // get length of file:
        pdffile.seekg(0, pdffile.end);
        std::streamsize  length = pdffile.tellg();
        pdffile.seekg(0, pdffile.beg);
        // allocate memory:
        //char* buffer = new char[length];
        // read data as a block:
        //is.read(buffer, length);
        std::streamsize size = length;
        pdffile.seekg(0, std::ios::beg);
        std::vector<char> buffer(size);
        DoTraceMessage(RENDERFILTER_TRACE_INFO, " PrintOutputPDFStream file length= [%u]", (UINT)size);
        if (pdffile.read(buffer.data(), size))
        {
            data = buffer.data();
        }

        lsize = strlen(data);
        DoTraceMessage(RENDERFILTER_TRACE_INFO, " PrintOutputPDFStream lsize length= [%u]", (UINT)lsize);
        //hr = pIStream->Write(&lsize, sizeof(int), NULL);
        hr = pIStream->Write(data, (ULONG)size, NULL);
        // Save the data
        hr = pIStream->Commit(STGC_DEFAULT | STGC_OVERWRITE);

        buffer.clear();

        //// Get the size of the TIFF from the stream position
        ULARGE_INTEGER tiffSize;
        LARGE_INTEGER zero;
        zero.QuadPart = 0;
        pIStream->Seek(zero, SEEK_CUR, &tiffSize);

        ULONG cb;
        ULongLongToULong(tiffSize.QuadPart, &cb);
        //// Get a pointer to the HGLOBAL memory
        HGlobalLock_t lock = pHG->Lock();
        BYTE* pCache = lock->GetAddress();

        //// Write the pdf buffer to the output stream
        ULONG written=0;
        DoTraceMessage(RENDERFILTER_TRACE_INFO, " PrintOutputPDFStream file length= [%u]", (UINT)written);
        pStream->WriteBytes(pCache, cb, &written);
        DoTraceMessage(RENDERFILTER_TRACE_INFO, " PrintOutputPDFStream file length= [%u]", (UINT)written);

        return 1;
    }

//
//Routine Name:
//
//    TiffStreamBitmapHandler::CreateTiffStreamBitmapHandler
//
//Routine Description:
//
//    Static factory method that creates an instance of
//    TiffStreamBitmapHandler.
//
//Arguments:
//
//    pStream         - Filter output stream (IPrintWriteStream)
//
//Return Value:
//
//    TiffStreamBitmapHandler_t (smart ptr)
//    The new TiffStreamBitmapHandler.
//
TiffStreamBitmapHandler_t
TiffStreamBitmapHandler::CreateTiffStreamBitmapHandler(
    const IPrintWriteStream_t &pStream
    )
{
    IWICImagingFactory_t        pWICFactory;

    //
    // Create an instance of a WIC Imaging Factory
    //
    THROW_ON_FAILED_HRESULT(
        ::CoCreateInstance(
            CLSID_WICImagingFactory, 
            NULL, 
            CLSCTX_INPROC_SERVER, 
            __uuidof(IWICImagingFactory), 
            reinterpret_cast<LPVOID*>(&pWICFactory)
            )
        );

    //
    // Construct the TiffStreamBitmapHandler and return it
    //
    TiffStreamBitmapHandler_t toReturn(
                                new TiffStreamBitmapHandler(
                                        pWICFactory,
                                        pStream
                                        )
                                );

    return toReturn;
}

//
//Routine Name:
//
//    TiffStreamBitmapHandler::TiffStreamBitmapHandler
//
//Routine Description:
//
//    Construct the bitmap handler with the WIC factory
//    and filter output stream.
//
//Arguments:
//
//    pWICFactory - Windows Imaging Components object factory
//    pStream     - Output stream
//    pHG         - Encoder cache HGLOBAL
//
TiffStreamBitmapHandler::TiffStreamBitmapHandler(
    const IWICImagingFactory_t        &pWICFactory,
    const IPrintWriteStream_t         &pStream
    ) : m_pWICFactory(pWICFactory),
        m_pWriter(pStream),
        m_nextTiffStart(0),
        m_numTiffs(0),
        m_tiffStarts(0)
{
}

//
//Routine Name:
//
//    TiffStreamBitmapHandler::ProcessBitmap
//
//Routine Description:
//
//    Encode the bitmap as a TIFF and stream out of the filter.
//
//Arguments:
//
//    bitmap    - bitmap of a single band, to stream
//
void TiffStreamBitmapHandler::ProcessBitmap( const IWICBitmap_t &bitmap )
{
    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"TiffStreamBitmapHandler::ProcessBitmap");

    //
    // Create an empty HGLOBAL to hold the encode cache
    //
    SafeHGlobal_t pHG(
                    new SafeHGlobal(GMEM_SHARE | GMEM_MOVEABLE, 0)
                    );

    //
    // Create a stream to the encode buffer so that WIC can
    // encode the TIFF in-memory
    //
    IStream_t pIStream;

    THROW_ON_FAILED_HRESULT(
        ::CreateStreamOnHGlobal(
            *pHG,
            FALSE, // Do NOT Free the HGLOBAL on Release of the stream
            &pIStream
            )
        );

    //
    // Create a WIC TIFF Encoder on the stream
    //
    IWICBitmapEncoder_t pWICEncoder;
    THROW_ON_FAILED_HRESULT(
        m_pWICFactory->CreateEncoder(GUID_ContainerFormatTiff, NULL, &pWICEncoder)
        );
    THROW_ON_FAILED_HRESULT(
        pWICEncoder->Initialize(pIStream, WICBitmapEncoderNoCache)
        );

    //
    // Create a new frame for the band and configure it
    //
    IWICBitmapFrameEncode_t pWICFrame;
    IPropertyBag2_t pFramePropertyBag;

    THROW_ON_FAILED_HRESULT(
        pWICEncoder->CreateNewFrame(&pWICFrame, &pFramePropertyBag)
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
            pFramePropertyBag->Write(
                1, // number of properties being set
                &option, 
                &varValue
                )
            );
    }

    THROW_ON_FAILED_HRESULT(
        pWICFrame->Initialize(pFramePropertyBag)
        );

    //
    // Set the frame's size
    //
    UINT bitmapWidth, bitmapHeight;
    THROW_ON_FAILED_HRESULT(
        bitmap->GetSize(&bitmapWidth, &bitmapHeight)
        );
    THROW_ON_FAILED_HRESULT(
        pWICFrame->SetSize(bitmapWidth, bitmapHeight)
        );

    //
    // Set the frame's resolution
    //
    DOUBLE xDPI, yDPI;
    THROW_ON_FAILED_HRESULT(
        bitmap->GetResolution(&xDPI, &yDPI)
        );
    THROW_ON_FAILED_HRESULT(
        pWICFrame->SetResolution(xDPI, yDPI)
        );

    //
    // Set the frame's pixel format
    //
    WICPixelFormatGUID format;
    THROW_ON_FAILED_HRESULT(
        bitmap->GetPixelFormat(&format)
        );
    THROW_ON_FAILED_HRESULT(
        pWICFrame->SetPixelFormat(&format)
        );

    //
    // Write the bitmap data to the frame
    //
    WICRect rect = {0, 0, 0, 0};
    rect.Width = bitmapWidth;
    rect.Height = bitmapHeight;
    THROW_ON_FAILED_HRESULT(
        pWICFrame->WriteSource(bitmap, &rect)
        );

    //
    // Commit the frame and encoder
    //
    THROW_ON_FAILED_HRESULT(
        pWICFrame->Commit()
        );
    THROW_ON_FAILED_HRESULT(
        pWICEncoder->Commit()
        );

    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"TiffStreamBitmapHandler::ProcessBitmap where ...");
    //
    // Get the size of the TIFF from the stream position
    //
    ULARGE_INTEGER tiffSize;
    LARGE_INTEGER zero;
    zero.QuadPart = 0;

    THROW_ON_FAILED_HRESULT(
        pIStream->Seek(zero, SEEK_CUR, &tiffSize)
        );

    ULONG cb;
    
    THROW_ON_FAILED_HRESULT(
        ::ULongLongToULong(tiffSize.QuadPart, &cb)
        );

    //
    // Update the list of Tiff locations so that it can be written to
    // the end of the Tiff stream.
    //
    m_tiffStarts.push_back(m_nextTiffStart);
    m_nextTiffStart += tiffSize.QuadPart;
    m_numTiffs++;

    {
        //
        // Get a pointer to the HGLOBAL memory
        //
        HGlobalLock_t lock = pHG->Lock();
        BYTE *pCache = lock->GetAddress();

        //
        // Write the encoded Tiff to the output stream
        //
        ULONG written;

        DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"TiffStreamBitmapHandler::ProcessBitmap Write the encoded Tiff to the output stream");
        THROW_ON_FAILED_HRESULT( m_pWriter->WriteBytes(pCache, cb, &written) );
        DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"TiffStreamBitmapHandler::ProcessBitmap stream ok ...");
    }
}

void TiffStreamBitmapHandler::ProcessBitmapSaveToFile(const IWICBitmap_t& bitmap, INT IdCount) {
    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"TiffStreamBitmapHandler::ProcessBitmap");

    IWICStream* pWICStream = NULL;
    //static const WCHAR filename[] = L"C:\\NuJee\\outputtry%u.tiff";
    WCHAR filename[MAX_PATH];
    StringCchPrintf(filename, MAX_PATH, L"C:\\NuJee\\outputtry%u.tiff", IdCount);

    THROW_ON_FAILED_HRESULT(
        m_pWICFactory->CreateStream(&pWICStream);
    );
    THROW_ON_FAILED_HRESULT(
        pWICStream->InitializeFromFilename(filename, GENERIC_WRITE);
    );

    //
    // Create a WIC TIFF Encoder on the stream
    //
    IWICBitmapEncoder_t pWICEncoder;
    THROW_ON_FAILED_HRESULT(
        m_pWICFactory->CreateEncoder(GUID_ContainerFormatTiff , NULL, &pWICEncoder)
    );
    THROW_ON_FAILED_HRESULT(
        pWICEncoder->Initialize(pWICStream, WICBitmapEncoderNoCache)
    );

    //
    // Create a new frame for the band and configure it
    //
    IWICBitmapFrameEncode_t pWICFrame;
    IPropertyBag2_t pFramePropertyBag;

    THROW_ON_FAILED_HRESULT(
        pWICEncoder->CreateNewFrame(&pWICFrame, &pFramePropertyBag)
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
            pFramePropertyBag->Write(
                1, // number of properties being set
                &option,
                &varValue
            )
        );
    }

    THROW_ON_FAILED_HRESULT(
        pWICFrame->Initialize(pFramePropertyBag)
    );

    //
    // Set the frame's size
    //
    UINT bitmapWidth, bitmapHeight;
    THROW_ON_FAILED_HRESULT(
        bitmap->GetSize(&bitmapWidth, &bitmapHeight)
    );
    THROW_ON_FAILED_HRESULT(
        pWICFrame->SetSize(bitmapWidth, bitmapHeight)
    );

    //
    // Set the frame's resolution
    //
    DOUBLE xDPI, yDPI;
    THROW_ON_FAILED_HRESULT(
        bitmap->GetResolution(&xDPI, &yDPI)
    );
    THROW_ON_FAILED_HRESULT(
        pWICFrame->SetResolution(xDPI, yDPI)
    );

    //
    // Set the frame's pixel format
    //
    WICPixelFormatGUID format;
    THROW_ON_FAILED_HRESULT(
        bitmap->GetPixelFormat(&format)
    );
    THROW_ON_FAILED_HRESULT(
        pWICFrame->SetPixelFormat(&format)
    );

    //
    // Write the bitmap data to the frame
    //
    WICRect rect = { 0, 0, 0, 0 };
    rect.Width = bitmapWidth;
    rect.Height = bitmapHeight;
    DoTraceMessage(RENDERFILTER_TRACE_INFO, " pWICFrame bitmapWidth== [%u], bitmapHeight = [%u] ", bitmapWidth, bitmapHeight);
    THROW_ON_FAILED_HRESULT(
        pWICFrame->WriteSource(bitmap, &rect)
        //pWICFrame->WriteSource(bitmap, NULL);
    );

    //
    // Commit the frame and encoder
    //
    THROW_ON_FAILED_HRESULT(
        pWICFrame->Commit()
    );
    THROW_ON_FAILED_HRESULT(
        pWICEncoder->Commit()
    );


    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"TiffStreamBitmapHandler::ProcessBitmap where ...");
    //
    // Get the size of the TIFF from the stream position
    //
    ULARGE_INTEGER tiffSize;
    LARGE_INTEGER zero;
    zero.QuadPart = 0;

    THROW_ON_FAILED_HRESULT(
        pWICStream->Seek(zero, SEEK_CUR, &tiffSize)
    );

    //
    // Update the list of Tiff locations so that it can be written to
    // the end of the Tiff stream.
    //
    m_tiffStarts.push_back(m_nextTiffStart);
    m_nextTiffStart += tiffSize.QuadPart;
    m_numTiffs++;
    {
         DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"TiffStreamBitmapHandler::ProcessBitmap stream ok ...");
    }
}


void TiffStreamBitmapHandler::ProcessBitmapSaveToJPGFiles(const IWICBitmap_t& bitmap, INT IdCount, WICRect& rect, CHAR* outputFile)
{
    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"Raster TiffStreamBitmapHandler::ProcessBitmap [%u] to JPG", IdCount);

    WICRect i = rect;
    IWICStream* pWICStream = NULL;
    WCHAR tempfilenameFolder[MAX_PATH];
    WCHAR filename[MAX_PATH];
    //static const WCHAR filename[] = L"C:\\NuJee\\outputtry%u.png";
    WCHAR filenameFolder[MAX_PATH];
    if (UtilHelper::GetOutputTempFolderFileName(filenameFolder)) {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"TiffStreamBitmapHandler::ProcessBitmap *** ::GetOutputTempFolderFileName   %ws", filenameFolder);
        wcscpy_s(tempfilenameFolder, filenameFolder);
        wcscat_s(tempfilenameFolder, L"%u.jpg");
        StringCchPrintf(filename, MAX_PATH, tempfilenameFolder, IdCount);
        StringCchPrintfA(outputFile, MAX_PATH, "%ws", filename);
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"TiffStreamBitmapHandler::ProcessBitmap *** ::GetOutputTempFolderFileName   %ws", filename);
    }
    else {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"TiffStreamBitmapHandler::ProcessBitmap fixed *** ::GetOutputTempFolderFileName   %ws", filename);
        return;
    }

    THROW_ON_FAILED_HRESULT(
        m_pWICFactory->CreateStream(&pWICStream);
    );
    THROW_ON_FAILED_HRESULT(
        pWICStream->InitializeFromFilename(filename, GENERIC_WRITE);
    );

    //
    // Create a WIC PNG Encoder on the stream
    //
    IWICBitmapEncoder_t pWICEncoder;
    THROW_ON_FAILED_HRESULT(
        m_pWICFactory->CreateEncoder(GUID_ContainerFormatJpeg, NULL, &pWICEncoder)
    );
    THROW_ON_FAILED_HRESULT(
        pWICEncoder->Initialize(pWICStream, WICBitmapEncoderNoCache)
    );

    //
    // Create a new frame for the band and configure it
    //
    IWICBitmapFrameEncode_t pWICFrame;
    IPropertyBag2_t pFramePropertyBag;

    THROW_ON_FAILED_HRESULT(
        pWICEncoder->CreateNewFrame(&pWICFrame, &pFramePropertyBag)
    );
    /*
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
            pFramePropertyBag->Write(
                //1, // number of properties being set
                1, // number of properties being set
                &option,
                &varValue
            )
        );
    }
    */
    THROW_ON_FAILED_HRESULT(
        pWICFrame->Initialize(pFramePropertyBag)
    );

    //
    // Set the frame's size
    //
    UINT bitmapWidth, bitmapHeight;
    THROW_ON_FAILED_HRESULT(
        bitmap->GetSize(&bitmapWidth, &bitmapHeight)
    );
    THROW_ON_FAILED_HRESULT(
        pWICFrame->SetSize(bitmapWidth, bitmapHeight)
    );

    //
    // Set the frame's resolution
    //
    DOUBLE xDPI, yDPI;
    THROW_ON_FAILED_HRESULT(
        bitmap->GetResolution(&xDPI, &yDPI)
    );
    THROW_ON_FAILED_HRESULT(
        pWICFrame->SetResolution(xDPI, yDPI)
    );

    //
    // Set the frame's pixel format
    //
    WICPixelFormatGUID format;
    THROW_ON_FAILED_HRESULT(
        bitmap->GetPixelFormat(&format)
    );
    THROW_ON_FAILED_HRESULT(
        pWICFrame->SetPixelFormat(&format)
    );

    //
    // Write the bitmap data to the frame
    //
    WICRect rect1 = { 0, 0, 0, 0 };
    rect1.Width = bitmapWidth;
    rect1.Height = bitmapHeight;
    DoTraceMessage(RENDERFILTER_TRACE_INFO, "xy pWICFrame x== [%u], y = [%u] ", rect1.X, rect1.Y);
    DoTraceMessage(RENDERFILTER_TRACE_INFO, "wh pWICFrame bitmapWidth== [%u], bitmapHeight = [%u] ", rect1.Width, rect1.Height);
    THROW_ON_FAILED_HRESULT(
        pWICFrame->WriteSource(bitmap, &rect1)
        //pWICFrame->WriteSource(bitmap, NULL);
    );

    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"pWICFrame->Commit() where ...");
    //
    // Commit the frame and encoder
    //
    THROW_ON_FAILED_HRESULT(
        pWICFrame->Commit()
    );

    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"pWICEncoder->Commit() where ...");
    THROW_ON_FAILED_HRESULT(
        pWICEncoder->Commit()
    );

    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"TiffStreamBitmapHandler::ProcessBitmap where ...");
    //
    // Get the size of the TIFF from the stream position
    //
    ULARGE_INTEGER tiffSize;
    LARGE_INTEGER zero;
    zero.QuadPart = 0;

    THROW_ON_FAILED_HRESULT(
        pWICStream->Seek(zero, SEEK_CUR, &tiffSize)
    );

    pWICStream->Commit(tagSTGC::STGC_OVERWRITE);
    pWICStream->Release();
    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"TiffStreamBitmapHandler::ProcessBitmap stream Release()  ...");

    //
    // Update the list of Tiff locations so that it can be written to
    // the end of the Tiff stream.
    //
    m_tiffStarts.push_back(m_nextTiffStart);
    m_nextTiffStart += tiffSize.QuadPart;
    m_numTiffs++;
    {
        DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"TiffStreamBitmapHandler::ProcessBitmap stream ok ...");
    }
}


void TiffStreamBitmapHandler::ProcessBitmapSaveToPNGFiles(const IWICBitmap_t& bitmap, INT IdCount, WICRect& rect, CHAR* outputFile)
{
    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"Raster TiffStreamBitmapHandler::ProcessBitmap [%u] to PNG", IdCount);

    WICRect i = rect;
    IWICStream* pWICStream = NULL;
    WCHAR tempfilenameFolder[MAX_PATH];
    WCHAR filename[MAX_PATH];
    //static const WCHAR filename[] = L"C:\\NuJee\\outputtry%u.png";
    WCHAR filenameFolder[MAX_PATH];
    if (UtilHelper::GetOutputTempFolderFileName(filenameFolder)) {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"TiffStreamBitmapHandler::ProcessBitmap *** ::GetOutputTempFolderFileName   %ws", filenameFolder);
        wcscpy_s(tempfilenameFolder, filenameFolder);
        wcscat_s(tempfilenameFolder, L"%u.png");
        StringCchPrintf(filename, MAX_PATH, tempfilenameFolder, IdCount);
        StringCchPrintfA(outputFile, MAX_PATH, "%ws", filename);
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"TiffStreamBitmapHandler::ProcessBitmap *** ::GetOutputTempFolderFileName   %ws", filename);
    }
    else {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"TiffStreamBitmapHandler::ProcessBitmap fixed *** ::GetOutputTempFolderFileName   %ws", filename);
        return;
    }

    THROW_ON_FAILED_HRESULT(
        m_pWICFactory->CreateStream(&pWICStream);
    );
    THROW_ON_FAILED_HRESULT(
        pWICStream->InitializeFromFilename(filename, GENERIC_WRITE);
    );

    //
    // Create a WIC PNG Encoder on the stream
    //
    IWICBitmapEncoder_t pWICEncoder;
    THROW_ON_FAILED_HRESULT(
        m_pWICFactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &pWICEncoder)
    );
    THROW_ON_FAILED_HRESULT(
        pWICEncoder->Initialize(pWICStream, WICBitmapEncoderNoCache)
    );

    //
    // Create a new frame for the band and configure it
    //
    IWICBitmapFrameEncode_t pWICFrame;
    IPropertyBag2_t pFramePropertyBag;

    THROW_ON_FAILED_HRESULT(
        pWICEncoder->CreateNewFrame(&pWICFrame, &pFramePropertyBag)
    );
    /*
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
            pFramePropertyBag->Write(
                //1, // number of properties being set
                1, // number of properties being set
                &option,
                &varValue
            )
        );
    }
    */
    THROW_ON_FAILED_HRESULT(
        pWICFrame->Initialize(pFramePropertyBag)
    );

    //
    // Set the frame's size
    //
    UINT bitmapWidth, bitmapHeight;
    THROW_ON_FAILED_HRESULT(
        bitmap->GetSize(&bitmapWidth, &bitmapHeight)
    );
    THROW_ON_FAILED_HRESULT(
        pWICFrame->SetSize(bitmapWidth, bitmapHeight)
    );

    //
    // Set the frame's resolution
    //
    DOUBLE xDPI, yDPI;
    THROW_ON_FAILED_HRESULT(
        bitmap->GetResolution(&xDPI, &yDPI)
    );
    THROW_ON_FAILED_HRESULT(
        pWICFrame->SetResolution(xDPI, yDPI)
    );

    //
    // Set the frame's pixel format
    //
    WICPixelFormatGUID format;
    THROW_ON_FAILED_HRESULT(
        bitmap->GetPixelFormat(&format)
    );
    THROW_ON_FAILED_HRESULT(
        pWICFrame->SetPixelFormat(&format)
    );

    //
    // Write the bitmap data to the frame
    //
    WICRect rect1 = { 0, 0, 0, 0 };
    rect1.Width = bitmapWidth;
    rect1.Height = bitmapHeight;
    DoTraceMessage(RENDERFILTER_TRACE_INFO, "xy pWICFrame x== [%u], y = [%u] ", rect1.X, rect1.Y);
    DoTraceMessage(RENDERFILTER_TRACE_INFO, "wh pWICFrame bitmapWidth== [%u], bitmapHeight = [%u] ", rect1.Width, rect1.Height);
    THROW_ON_FAILED_HRESULT(
        pWICFrame->WriteSource(bitmap, &rect1)
        //pWICFrame->WriteSource(bitmap, NULL);
    );

    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"pWICFrame->Commit() where ...");
    //
    // Commit the frame and encoder
    //
    THROW_ON_FAILED_HRESULT(
        pWICFrame->Commit()
    );

    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"pWICEncoder->Commit() where ...");
    THROW_ON_FAILED_HRESULT(
        pWICEncoder->Commit()
    );

    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"TiffStreamBitmapHandler::ProcessBitmap where ...");
    //
    // Get the size of the TIFF from the stream position
    //
    ULARGE_INTEGER tiffSize;
    LARGE_INTEGER zero;
    zero.QuadPart = 0;

    THROW_ON_FAILED_HRESULT(
        pWICStream->Seek(zero, SEEK_CUR, &tiffSize)
    );

    pWICStream->Commit(tagSTGC::STGC_OVERWRITE);
    pWICStream->Release();
    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"TiffStreamBitmapHandler::ProcessBitmap stream Release()  ...");

    //
    // Update the list of Tiff locations so that it can be written to
    // the end of the Tiff stream.
    //
    m_tiffStarts.push_back(m_nextTiffStart);
    m_nextTiffStart += tiffSize.QuadPart;
    m_numTiffs++;
    {
        DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"TiffStreamBitmapHandler::ProcessBitmap stream ok ...");
    }
}

void TiffStreamBitmapHandler::ProcessBitmapSaveToTiffFiles(const IWICBitmap_t& bitmap, INT IdCount, WICRect& rect, CHAR* outputFile) {
    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"TiffStreamBitmapHandler::ProcessBitmap [%u] ", IdCount);

    WICRect i = rect;
    IWICStream* pWICStream = NULL;
    WCHAR tempfilenameFolder[MAX_PATH];
    WCHAR filename[MAX_PATH];
    //static const WCHAR filename[] = L"C:\\NuJee\\outputtry%u.tiff";
    WCHAR filenameFolder[MAX_PATH];
    if (UtilHelper::GetOutputTempFolderFileName(filenameFolder)) {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"TiffStreamBitmapHandler::ProcessBitmap *** ::GetOutputTempFolderFileName   %ws", filenameFolder);
        wcscpy_s(tempfilenameFolder, filenameFolder);
        wcscat_s(tempfilenameFolder, L"\\outputtry%u.tiff");
        StringCchPrintf(filename, MAX_PATH, tempfilenameFolder, IdCount);
        StringCchPrintfA(outputFile, MAX_PATH, "%ws", filename);
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"TiffStreamBitmapHandler::ProcessBitmap *** ::GetOutputTempFolderFileName   %ws", filename);
    }
    else {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"TiffStreamBitmapHandler::ProcessBitmap fixed *** ::GetOutputTempFolderFileName   %ws", filename);
        return;
    }

    THROW_ON_FAILED_HRESULT(
        m_pWICFactory->CreateStream(&pWICStream);
    );
    THROW_ON_FAILED_HRESULT(
        pWICStream->InitializeFromFilename(filename, GENERIC_WRITE);
    );

    //
    // Create a WIC TIFF Encoder on the stream
    //
    IWICBitmapEncoder_t pWICEncoder;
    THROW_ON_FAILED_HRESULT(
        m_pWICFactory->CreateEncoder(GUID_ContainerFormatTiff, NULL, &pWICEncoder)
    );
    THROW_ON_FAILED_HRESULT(
        pWICEncoder->Initialize(pWICStream, WICBitmapEncoderNoCache)
    );

    //
    // Create a new frame for the band and configure it
    //
    IWICBitmapFrameEncode_t pWICFrame;
    IPropertyBag2_t pFramePropertyBag;

    THROW_ON_FAILED_HRESULT(
        pWICEncoder->CreateNewFrame(&pWICFrame, &pFramePropertyBag)
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
            pFramePropertyBag->Write(
                //1, // number of properties being set
                1, // number of properties being set
                &option,
                &varValue
            )
        );
    }

    THROW_ON_FAILED_HRESULT(
        pWICFrame->Initialize(pFramePropertyBag)
    );

    //
    // Set the frame's size
    //
    UINT bitmapWidth, bitmapHeight;
    THROW_ON_FAILED_HRESULT(
        bitmap->GetSize(&bitmapWidth, &bitmapHeight)
    );
    THROW_ON_FAILED_HRESULT(
        pWICFrame->SetSize(bitmapWidth, bitmapHeight)
    );

    //
    // Set the frame's resolution
    //
    DOUBLE xDPI, yDPI;
    THROW_ON_FAILED_HRESULT(
        bitmap->GetResolution(&xDPI, &yDPI)
    );
    THROW_ON_FAILED_HRESULT(
        pWICFrame->SetResolution(xDPI, yDPI)
    );

    //
    // Set the frame's pixel format
    //
    WICPixelFormatGUID format;
    THROW_ON_FAILED_HRESULT(
        bitmap->GetPixelFormat(&format)
    );
    THROW_ON_FAILED_HRESULT(
        pWICFrame->SetPixelFormat(&format)
    );

    //
    // Write the bitmap data to the frame
    //
    WICRect rect1 = { 0, 0, 0, 0 };
    rect1.Width = bitmapWidth;
    rect1.Height = bitmapHeight;
    DoTraceMessage(RENDERFILTER_TRACE_INFO, "xy pWICFrame x== [%u], y = [%u] ", rect1.X, rect1.Y);
    DoTraceMessage(RENDERFILTER_TRACE_INFO, "wh pWICFrame bitmapWidth== [%u], bitmapHeight = [%u] ", rect1.Width, rect1.Height);
    THROW_ON_FAILED_HRESULT(
        pWICFrame->WriteSource(bitmap, &rect1)
        //pWICFrame->WriteSource(bitmap, NULL);
    );

    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"pWICFrame->Commit() where ...");
    //
    // Commit the frame and encoder
    //
    THROW_ON_FAILED_HRESULT(
        pWICFrame->Commit()
    );

    /*
    //
    // Create a new frame for the band and configure it
    //
    IWICBitmapFrameEncode_t pWICFrame2;
    IPropertyBag2_t pFramePropertyBag2;

    THROW_ON_FAILED_HRESULT(
        pWICEncoder->CreateNewFrame(&pWICFrame2, &pFramePropertyBag2)
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
            pFramePropertyBag2->Write(
                //1, // number of properties being set
                1, // number of properties being set
                &option,
                &varValue
            )
        );
    }

    THROW_ON_FAILED_HRESULT(
        pWICFrame2->Initialize(pFramePropertyBag2)
    );

    //
    // Set the frame's size
    //
    //UINT bitmapWidth, bitmapHeight;
    THROW_ON_FAILED_HRESULT(
        bitmap->GetSize(&bitmapWidth, &bitmapHeight)
    );
    THROW_ON_FAILED_HRESULT(
        pWICFrame2->SetSize(bitmapWidth, bitmapHeight)
    );

    //
    // Set the frame's resolution
    //
    //DOUBLE xDPI, yDPI;
    THROW_ON_FAILED_HRESULT(
        bitmap->GetResolution(&xDPI, &yDPI)
    );
    THROW_ON_FAILED_HRESULT(
        pWICFrame2->SetResolution(xDPI, yDPI)
    );

    //
    // Set the frame's pixel format
    //
    //WICPixelFormatGUID format;
    THROW_ON_FAILED_HRESULT(
        bitmap->GetPixelFormat(&format)
    );
    THROW_ON_FAILED_HRESULT(
        pWICFrame2->SetPixelFormat(&format)
    );

    //
    // Write the bitmap data to the frame
    //
    WICRect rect2 = { 0, 0, 0, 0 };
    rect2.Width = bitmapWidth;
    rect2.Height = bitmapHeight;
    DoTraceMessage(RENDERFILTER_TRACE_INFO, "xy pWICFrame x== [%u], y = [%u] ", rect2.X, rect2.Y);
    DoTraceMessage(RENDERFILTER_TRACE_INFO, "okokokok wh pWICFrame bitmapWidth== [%u], bitmapHeight = [%u] ", rect2.Width, rect2.Height);
    THROW_ON_FAILED_HRESULT(
        pWICFrame2->WriteSource(bitmap, &rect2)
        //pWICFrame->WriteSource(bitmap, NULL);
    );

    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"pWICFrame2->Commit() where ...");
    //
    // Commit the frame and encoder
    //
    THROW_ON_FAILED_HRESULT(
        pWICFrame2->Commit()
    );
    */
    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"pWICEncoder->Commit() where ...");
    THROW_ON_FAILED_HRESULT(
        pWICEncoder->Commit()
    );


    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"TiffStreamBitmapHandler::ProcessBitmap where ...");
    //
    // Get the size of the TIFF from the stream position
    //
    ULARGE_INTEGER tiffSize;
    LARGE_INTEGER zero;
    zero.QuadPart = 0;

    THROW_ON_FAILED_HRESULT(
        pWICStream->Seek(zero, SEEK_CUR, &tiffSize)
    );

    pWICStream->Commit(tagSTGC::STGC_OVERWRITE);
    pWICStream->Release();
    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"TiffStreamBitmapHandler::ProcessBitmap stream Release()  ...");

    //
    // Update the list of Tiff locations so that it can be written to
    // the end of the Tiff stream.
    //
    m_tiffStarts.push_back(m_nextTiffStart);
    m_nextTiffStart += tiffSize.QuadPart;
    m_numTiffs++;
    {
        DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"TiffStreamBitmapHandler::ProcessBitmap stream ok ...");
    }
}

void TiffStreamBitmapHandler::ProcessBitmapSaveToOneFile(const IWICBitmap_t& bitmap, IWICStream* pWICStream, IWICBitmapFrameEncode_t& pWICFrame, WICRect& rect) {
    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"TiffStreamBitmapHandler::ProcessBitmap");

    //
    // Write the bitmap data to the frame
    //
    WICRect rectIn = { 0, 0, 0, 0 };
    rectIn.X = rect.X;
    rectIn.Y = rect.Y;
    rectIn.Width = rect.Width;
    rectIn.Height = rectIn.Height;
    DoTraceMessage(RENDERFILTER_TRACE_INFO, " diff pWICFrame rectIn == [%u], rectIn rectY = [%u] ", rectIn.X, rectIn.Y);
    DoTraceMessage(RENDERFILTER_TRACE_INFO, " pWICFrame bitmapWidth== [%u], bitmapHeight = [%u] ", rect.Width, rect.Height);
    THROW_ON_FAILED_HRESULT(
        pWICFrame->WriteSource(bitmap, &rectIn)
        //pWICFrame->WriteSource(bitmap, NULL);
    );
    //
    // // Commit the frame and encoder
    //
    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"pWICFrame->Commit...");
    THROW_ON_FAILED_HRESULT(
        pWICFrame->Commit()
    );

    DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"TiffStreamBitmapHandler::ProcessBitmap where ...");
    //
    // Get the size of the TIFF from the stream position
    //
    ULARGE_INTEGER tiffSize;
    LARGE_INTEGER zero;
    zero.QuadPart = 0;

    THROW_ON_FAILED_HRESULT(
        pWICStream->Seek(zero, SEEK_CUR, &tiffSize)
    );

    //
    // Update the list of Tiff locations so that it can be written to
    // the end of the Tiff stream.
    //
    m_tiffStarts.push_back(m_nextTiffStart);
    m_nextTiffStart += tiffSize.QuadPart;
    m_numTiffs++;
    {
        DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"TiffStreamBitmapHandler::ProcessBitmap stream ok ...");
    }
}

//
//Routine Name:
//
//    TiffStreamBitmapHandler::WriteFooter
//
//Routine Description:
//
//    Write the footer to the stream, making it easier
//    to decode the individual TIFFs later. The footer
//    looks like this:
//
//       |<--8 bytes--->|
//
//       +--------------+
//       | Tiff 1 start |
//       +--------------+
//       | Tiff 2 start |
//       +--------------+
//       |     ...      |
//       +--------------+
//       | Tiff N start |
//       +--------------+
//       |      N       | 
//       +--------------+
//
void
TiffStreamBitmapHandler::WriteFooter()
{
    ULONG written;

    //
    // Write the vector of Tiff starts to the stream, if any Tiffs
    // have been written to the stream.
    //
    if (m_numTiffs > 0)
    {
        ULONG toWrite;

        THROW_ON_FAILED_HRESULT(
            SizeTToULong(
                sizeof(ULONGLONG) * m_tiffStarts.size(),
                &toWrite
                )
            );

        THROW_ON_FAILED_HRESULT(
            m_pWriter->WriteBytes(
                            reinterpret_cast<BYTE *>(&m_tiffStarts[0]),
                            toWrite, 
                            &written
                            )
            );
    }

    //
    // Write the number of Tiffs to the stream
    //
    THROW_ON_FAILED_HRESULT(
        m_pWriter->WriteBytes(
            reinterpret_cast<BYTE *>(&m_numTiffs),
            sizeof(m_numTiffs),
            &written
            )
        );
}

} // namespace xpsrasfilter
