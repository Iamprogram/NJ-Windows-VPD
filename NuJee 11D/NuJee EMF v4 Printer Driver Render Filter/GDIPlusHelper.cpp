
#define GDIPVER     0x0110  // Use more advanced GDI+ features

#include "precomp.h"
#include "WppTrace.h"
#include "CustomWppCommands.h"
#include <windows.h>
#include <gdiplus.h>
#include <stdio.h>

using namespace Gdiplus; 
#pragma comment (lib,"Gdiplus.lib")

#include "GDIPlusHelper.h"
#include <GDIPlusHelper.tmh>

#include "UtilHelper.h"

namespace NuJee_EMF_v4_Printer_Driver_Render_Filter
{
ULONG_PTR m_gdiplusToken;

int GDIPlusHelper::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    ImageCodecInfo* pImageCodecInfo = NULL;

    GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;  // Failure

    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL)
        return -1;  // Failure

    GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }
    }

    free(pImageCodecInfo);
    return -1;  // Failure
}

INT GDIPlusHelper::GDIPlusImagePNGConvertEMF(const int IdPagesCount, _In_ PCWSTR pRasterFile, CHAR* outputFile)
{
    // Initialize GDI+.
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    DoTraceMessage(RENDERFILTER_TRACE_INFO, "StartOpen GDIPlusImagePNGConvertEMF %ws", pRasterFile);
    WCHAR tempfilenameFolder[MAX_PATH];
    WCHAR filename[MAX_PATH];
    WCHAR filenameFolder[MAX_PATH];
    if (UtilHelper::GetOutputTempFolderFileName(filenameFolder)) {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GDIPlusImagePNGConvertEMF PNG:: %ws", filenameFolder);
        wcscpy_s(tempfilenameFolder, filenameFolder);
        wcscat_s(tempfilenameFolder, L"%u.png");
        StringCchPrintf(filename, MAX_PATH, tempfilenameFolder, IdPagesCount);
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GDIPlusImagePNGConvertEMF PNG::  ***  %ws", filename);
    }
    else {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GDIPlusImagePNGConvertEMF fail PNG %ws", filename);
    }

    //Bitmap* m_pBitmap;
    DoTraceMessage(RENDERFILTER_TRACE_INFO, "GDIPlusImagePNGConvertEMF %ws", filename);
    Image* image = new Image(filename);

    WCHAR EMFfilename[MAX_PATH];
    if (UtilHelper::GetOutputTempFolderFileName(filenameFolder)) {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GDIPlusImagePNGConvertEMF EMF:: %ws", filenameFolder);
        wcscpy_s(tempfilenameFolder, filenameFolder);
        wcscat_s(tempfilenameFolder, L"%u.emf");
        StringCchPrintf(EMFfilename, MAX_PATH, tempfilenameFolder, IdPagesCount);
        StringCchPrintfA(outputFile, MAX_PATH, "%ws", EMFfilename);
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GDIPlusImagePNGConvertEMF EMF::  ***  %ws", EMFfilename);
    }
    else {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GDIPlusImagePNGConvertEMF fail EMF %ws", EMFfilename);
    }
    HDC hDC = GetDC(NULL);
    Metafile* metafile = new Metafile(EMFfilename, hDC);
    Graphics graphics(metafile);
    int wide = image->GetWidth();
    int hign = image->GetHeight();
    Gdiplus::Status st = graphics.DrawImage(image, 0, 0, wide, hign);
    DoTraceMessage(RENDERFILTER_TRACE_INFO, "GDIPlusImagePNGConvertEMF GetWidth== [%u],GetHeight = [%u] st=[%u]", image->GetWidth(), image->GetHeight(), st);

    //ReleaseDC(hWnd,hDC);
    DeleteDC(hDC);
    delete image;
    delete metafile;// I don't need to do this

    DoTraceMessage(RENDERFILTER_TRACE_INFO, "GDIPlusImagePNGConvertEMF GdiplusShutdown");
    //GdiplusShutdown(gdiplusToken);
    DoTraceMessage(RENDERFILTER_TRACE_INFO, "GDIPlusImagePNGConvertEMF GdiplusShutdown");
    return 0;
}


INT GDIPlusHelper::GDIPlusImageTIFFConvertEMF(const int IdPagesCount, _In_ PCWSTR pRasterFile)
{
    // Initialize GDI+.
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    DoTraceMessage(RENDERFILTER_TRACE_INFO, "StartOpen GDIPlusImageTIFFConvertEMF %ws", pRasterFile);
    WCHAR tempfilenameFolder[MAX_PATH];
    WCHAR filename[MAX_PATH];
    WCHAR filenameFolder[MAX_PATH];
    if (UtilHelper::GetOutputTempFolderFileName(filenameFolder)) {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GDIPlusImageTIFFConvertEMF TIFF:: %ws", filenameFolder);
        wcscpy_s(tempfilenameFolder, filenameFolder);
        wcscat_s(tempfilenameFolder, L"%u.tiff");
        StringCchPrintf(filename, MAX_PATH, tempfilenameFolder, IdPagesCount);
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GDIPlusImageTIFFConvertEMF TIFF::  ***  %ws", filename);
    }
    else {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GDIPlusImageTIFFConvertEMF fail TIFF %ws", filename);
    }

    //Bitmap* m_pBitmap;
    DoTraceMessage(RENDERFILTER_TRACE_INFO, "GDIPlusImageTIFFConvertEMF %ws", filename);
    Image* image = new Image(filename);

    WCHAR EMFfilename[MAX_PATH];
    if (UtilHelper::GetOutputTempFolderFileName(filenameFolder)) {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GDIPlusImageTIFFConvertEMF EMF:: %ws", filenameFolder);
        wcscpy_s(tempfilenameFolder, filenameFolder);
        wcscat_s(tempfilenameFolder, L"%u.emf");
        StringCchPrintf(EMFfilename, MAX_PATH, tempfilenameFolder, IdPagesCount);
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GDIPlusImageTIFFConvertEMF EMF::  ***  %ws", EMFfilename);
    }
    else {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GDIPlusImageTIFFConvertEMF fail EMF %ws", EMFfilename);
    }
    HDC hDC = GetDC(NULL);
    Metafile* metafile = new Metafile(EMFfilename, hDC);
    Graphics graphics(metafile);
    int wide = 6600;//image->GetWidth();
    int hign = 5100;//image->GetHeight();
    Gdiplus::Status st = graphics.DrawImage(image, 0, 0, wide, hign);
    DoTraceMessage(RENDERFILTER_TRACE_INFO, "GDIPlusImageTIFFConvertEMF GetWidth== [%u],GetHeight = [%u] st=[%u]", image->GetWidth(), image->GetHeight(), st);

    //ReleaseDC(hWnd,hDC);
    DeleteDC(hDC);
    delete image;
    delete metafile;// I don't need to do this
   
    DoTraceMessage(RENDERFILTER_TRACE_INFO, "GDIPlusImageTIFFConvertEMF GdiplusShutdown");
    //GdiplusShutdown(gdiplusToken);
    DoTraceMessage(RENDERFILTER_TRACE_INFO, "GDIPlusImageTIFFConvertEMF GdiplusShutdown");
    return 0;
}

INT GDIPlusHelper::GDIPlusImageTIFFConvertBMP(_In_ PCWSTR pRasterFile) {
    // Initialize GDI+.
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    CLSID   encoderClsid;
    Status  stat;
    Image* image = new Image(pRasterFile);

    //Get the CLSID of the BMP encoder.
    GetEncoderClsid(L"image/bmp", &encoderClsid);
    stat = image->Save(L"C:\\NuJee\\outputtry0.bmp", &encoderClsid, NULL);

    if (stat == Ok)
        printf("outputtry0.tiff was saved successfully\n");
    else
        printf("Failure: stat = %d\n", stat);

    delete image;

    //GdiplusShutdown(gdiplusToken);
    return 0;
}
INT GDIPlusHelper::GDIPlusImageTIFFConvertGIF(_In_ PCWSTR pRasterFile) {
    // Initialize GDI+.
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    CLSID   encoderClsid;
    Status  stat;
    Image* image = new Image(pRasterFile);

    //Get the CLSID of the GIF encoder.
    GetEncoderClsid(L"image/gif", &encoderClsid);
    stat = image->Save(L"C:\\NuJee\\outputtry0.gif", &encoderClsid, NULL);

    if (stat == Ok)
        printf("outputtry0.tiff was saved successfully\n");
    else
        printf("Failure: stat = %d\n", stat);

    delete image;

    //GdiplusShutdown(gdiplusToken);
    return 0;
}
INT GDIPlusHelper::GDIPlusImageTIFFConvertJPG(_In_ PCWSTR pRasterFile) {
    // Initialize GDI+.
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    CLSID   encoderClsid;
    Status  stat;
    Image* image = new Image(pRasterFile);

    //Get the CLSID of the JPG encoder.
    GetEncoderClsid(L"image/jpeg", &encoderClsid);
    stat = image->Save(L"C:\\NuJee\\outputtry0.jpg", &encoderClsid, NULL);

    if (stat == Ok)
        printf("outputtry0.tiff was saved successfully\n");
    else
        printf("Failure: stat = %d\n", stat);

    delete image;

    //GdiplusShutdown(gdiplusToken);
    return 0;
}

INT GDIPlusHelper::GDIPlusImageTIFFConvertPNG(const int IdPagesCount, _In_ PCWSTR pRasterFile) {
    // Initialize GDI+.
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    WCHAR tempfilenameFolder[MAX_PATH];
    WCHAR filename[MAX_PATH];
    WCHAR filenameFolder[MAX_PATH];
    if (UtilHelper::GetOutputTempFolderFileName(filenameFolder)) {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GDIPlusImageTIFFConvertPNG TIFF:: %ws", filenameFolder);
        wcscpy_s(tempfilenameFolder, filenameFolder);
        wcscat_s(tempfilenameFolder, L"%u.tiff");
        StringCchPrintf(filename, MAX_PATH, tempfilenameFolder, IdPagesCount);
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GDIPlusImageTIFFConvertPNG TIFF::  ***  %ws", filename);
    }
    else {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GDIPlusImageTIFFConvertPNG fail TIFF %ws", filename);
    }

    CLSID   encoderClsid;
    Status  stat;
    Image* image = new Image(filename);

    //Get the CLSID of the PNG encoder.
    WCHAR pngfilename[MAX_PATH];
    if (UtilHelper::GetOutputTempFolderFileName(filenameFolder)) {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GDIPlusImageTIFFConvertPNG PNG:: %ws", filenameFolder);
        wcscpy_s(tempfilenameFolder, filenameFolder);
        wcscat_s(tempfilenameFolder, L"%u.png");
        StringCchPrintf(pngfilename, MAX_PATH, tempfilenameFolder, IdPagesCount);
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GDIPlusImageTIFFConvertPNG PNG::  ***  %ws", pngfilename);
    }
    else {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GDIPlusImageTIFFConvertPNG fail PNG %ws", pngfilename);
    }

    GetEncoderClsid(L"image/png", &encoderClsid);
    stat = image->Save(pngfilename, &encoderClsid, NULL);

    if (stat == Ok)
        printf("outputtry0.tiff was saved successfully\n");
    else
        printf("Failure: stat = %d\n", stat);

    delete image;

    GdiplusShutdown(gdiplusToken);
    return 1;
}


INT GDIPlusHelper::GDIPlusImageJPGConvertPNG(const int IdPagesCount, _In_ PCWSTR pRasterFile) {
    // Initialize GDI+.
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    WCHAR tempfilenameFolder[MAX_PATH];
    WCHAR filename[MAX_PATH];
    WCHAR filenameFolder[MAX_PATH];
    if (UtilHelper::GetOutputTempFolderFileName(filenameFolder)) {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GDIPlusImageJPGConvertPNG JPG:: %ws", filenameFolder);
        wcscpy_s(tempfilenameFolder, filenameFolder);
        wcscat_s(tempfilenameFolder, L"%u.jpg");
        StringCchPrintf(filename, MAX_PATH, tempfilenameFolder, IdPagesCount);
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GDIPlusImageJPGConvertPNG JPG::  ***  %ws", filename);
    }
    else {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GDIPlusImageJPGConvertPNG fail JPG %ws", filename);
    }

    CLSID   encoderClsid;
    Status  stat;
    Image* image = new Image(filename);

    //Get the CLSID of the PNG encoder.
    WCHAR pngfilename[MAX_PATH];
    if (UtilHelper::GetOutputTempFolderFileName(filenameFolder)) {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GDIPlusImageJPGConvertPNG PNG:: %ws", filenameFolder);
        wcscpy_s(tempfilenameFolder, filenameFolder);
        wcscat_s(tempfilenameFolder, L"%u.png");
        StringCchPrintf(pngfilename, MAX_PATH, tempfilenameFolder, IdPagesCount);
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GDIPlusImageJPGConvertPNG PNG::  ***  %ws", pngfilename);
    }
    else {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"GDIPlusImageJPGConvertPNG fail PNG %ws", pngfilename);
    }

    GetEncoderClsid(L"image/png", &encoderClsid);
    stat = image->Save(pngfilename, &encoderClsid, NULL);

    if (stat == Ok)
        printf("outputtry0.png was saved successfully\n");
    else
        printf("Failure: stat = %d\n", stat);

    delete image;

    GdiplusShutdown(gdiplusToken);
    return 1;
}


BOOL GDIPlusHelper::ImageNormalize(const int IdPagesCount, _In_ PCWSTR pRasterFile)
{
    // Initialize GDI+.
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    WCHAR tempfilenameFolder[MAX_PATH];
    WCHAR filename[MAX_PATH];
    WCHAR filenameFolder[MAX_PATH];
    WCHAR tempfilename[MAX_PATH];
    if (UtilHelper::GetOutputTempFolderFileName(filenameFolder)) {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"ImageNormalize TIFF:: %ws", filenameFolder);
        wcscpy_s(tempfilenameFolder, filenameFolder);
        wcscat_s(tempfilenameFolder, L"%u.png");
        wcscpy_s(tempfilename, filenameFolder);
        wcscat_s(tempfilename, L".png");
        StringCchPrintf(filename, MAX_PATH, tempfilenameFolder, IdPagesCount);
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"ImageNormalize TIFF::  %ws ***  %ws", filename, tempfilename);
    }
    else {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"ImageNormalize fail TIFF %ws", filename);
    }

    //Gdiplus::Image* image = new Image(L"C:\\NuJee\\outputtry1.tiff");

    Gdiplus::Bitmap* pBmpWithoutAlpha = new Gdiplus::Bitmap(filename, 1);
    Gdiplus::ColorPalette* originalColorPalette;
    UINT PaletteSize;
    PaletteSize = pBmpWithoutAlpha->GetPaletteSize();
    originalColorPalette = (ColorPalette*)malloc(PaletteSize);
    if (originalColorPalette == NULL) {
        ;//DebugText("error: palette not created!!!!");
    }
    Status stImage;
    stImage = pBmpWithoutAlpha->GetPalette(originalColorPalette, PaletteSize);
    if (stImage != Ok) {
        ;//DebugText("error: palette not geted!!!!");
    }

    Status status = pBmpWithoutAlpha->ConvertFormat(PixelFormat32bppRGB,
        Gdiplus::DitherTypeNone,
        Gdiplus::PaletteTypeCustom,
        originalColorPalette,
        0);
    Gdiplus::Bitmap* pBmpWithAlpha = new Gdiplus::Bitmap(filename, PixelFormat32bppPARGB);
    Gdiplus::Color pixelColor;

    int widthWithoutAlpha = pBmpWithAlpha->GetWidth();
    int heightWithoutAlpha = pBmpWithAlpha->GetHeight();
    int widthAlpha = pBmpWithoutAlpha->GetWidth();
    int heightAlpha = pBmpWithoutAlpha->GetHeight();

    //Lock the whole bitmap so we can read pixel data easily.
    Gdiplus::Rect rect(0, 0, widthWithoutAlpha, heightWithoutAlpha);
    //auto* bitmapData = new Gdiplus::BitmapData;
    BitmapData bitmapData;
    ZeroMemory(&bitmapData, sizeof(bitmapData));

    //bitmap.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, bitmapData);
    pBmpWithoutAlpha->LockBits(&rect, ImageLockModeWrite, PixelFormat32bppRGB, &bitmapData);

    //Get the individual pixels from the locked area.
    auto* pixels = static_cast<unsigned*>(bitmapData.Scan0);

    const int stride = abs(bitmapData.Stride);
    if (widthAlpha == widthWithoutAlpha && heightAlpha == heightWithoutAlpha) {
        for (int x = 0; x < widthAlpha; x++) {
            for (int y = 0; y < heightAlpha; y++) {
                pBmpWithAlpha->GetPixel(x, y, &pixelColor);
                if (pixelColor.GetValue() > 0) {
                }
                else {
                    //Set the pixel colour from the pixels array which we got earlier.
                    //pBmpWithoutAlpha->SetPixel(x, y, Gdiplus::Color::White);
                    //const unsigned pxColor = pixels[y * stride / 4 + x];
                    //Get each individual colour component. Bitmap colours are in reverse order.
                    //const unsigned red = (pxColor & 0xFF0000) >> 16;
                    //const unsigned green = (pxColor & 0xFF00) >> 8;
                    //const unsigned blue = pxColor & 0xFF;

                    //Combine the values in a more typical RGB format (as opposed to the bitmap way).
                    //const int rgbValue = RGB(red, green, blue);
                    pixels[y * stride / 4 + x] = 0xFFFFFF;

                    //Assign this RGB value to the pixel location in the vector o' vectors.
                    //resultPixels[x][y] = rgbValue;
                }
            }
        }
    }
    /*
    if (widthAlpha== widthWithoutAlpha && heightAlpha == heightWithoutAlpha) {
        for (int x = widthAlpha - 1; x >= 0; --x)
        {
            for (int y = 0; y < heightAlpha; ++y)
            {
                pBmpWithAlpha->GetPixel(x, y, &pixelColor);
                if (pixelColor.GetValue() > 0) {
                    if (pixelColor.GetAlpha() > 0)
                    {
                    }
                    else {
                      //pBmpWithoutAlpha->SetPixel(x, y, Gdiplus::Color::White);
                    }
                }
                else {
                   pBmpWithoutAlpha->SetPixel(x, y, Gdiplus::Color::White);
                }
            }
        }
    }
    */
    //Unlock the bits that we locked before.
    //bitmap.UnlockBits(bitmapData);
    pBmpWithoutAlpha->UnlockBits(&bitmapData);

    CLSID   encoderClsid;
    Status  stat;
    //Get the CLSID of the PNG encoder.
    GetEncoderClsid(L"image/png", &encoderClsid);

    // Save the image

    BOOL bRes = (pBmpWithoutAlpha->Save(tempfilename, &encoderClsid, NULL) == Ok);

    // Clean up GDI+ image
    delete pBmpWithoutAlpha;
    delete pBmpWithAlpha;

    bool fSuccess = DeleteFile(filename);
    if (fSuccess)
    {
        _wrename(tempfilename, filename);
    }

    GdiplusShutdown(gdiplusToken);
    return bRes;
}

BOOL GDIPlusHelper::ImageAsNormalFinal(const int IdPagesCount, _In_ PCWSTR pRasterFile)
{
    // Initialize GDI+.
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    WCHAR tempfilenameFolder[MAX_PATH];
    WCHAR filenameOriginal[MAX_PATH];
    WCHAR filename[MAX_PATH];
    WCHAR filenameFolder[MAX_PATH];
    WCHAR tempfilename[MAX_PATH];
    if (UtilHelper::GetOutputTempFolderFileName(filenameFolder)) {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"ImageNormalize TIFF:: %ws", filenameFolder);
        wcscpy_s(tempfilenameFolder, filenameFolder);
        wcscat_s(tempfilenameFolder, L"\\outputtry%u.tiff");
        wcscpy_s(tempfilename, filenameFolder);
        wcscat_s(tempfilename, L"\\outputtry.tiff");
        StringCchPrintf(filename, MAX_PATH, tempfilenameFolder, IdPagesCount);
        wcscpy_s(filenameOriginal, filenameFolder);
        wcscat_s(filenameOriginal, L"\\outputtry%u%u.tiff");
        StringCchPrintf(filenameOriginal, MAX_PATH, filenameOriginal, IdPagesCount, IdPagesCount);
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"ImageNormalize TIFF::  %ws ***  %ws ***  %ws", filename, tempfilename, filenameOriginal);
    }
    else {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"ImageNormalize fail TIFF %ws", filename);
    }

    Gdiplus::Bitmap* pBmpWithAlpha = new Gdiplus::Bitmap(filename, PixelFormat32bppPARGB);
    Gdiplus::Color pixelColorWithAlpha;
    int  widthAlpha = pBmpWithAlpha->GetWidth();
    int  heightAlpha = pBmpWithAlpha->GetHeight();
    if (widthAlpha > 0 && heightAlpha > 0) {
        for (int x = widthAlpha - 1; x >= 0; --x)
        {
            for (int y = 0; y < heightAlpha; ++y)
            {
                pBmpWithAlpha->GetPixel(x, y, &pixelColorWithAlpha);
                DWORD pixelColorWithAlphaValue = pixelColorWithAlpha.GetValue();
                byte pixelColorWithAlphaAlpha = pixelColorWithAlpha.GetAlpha();

                if (pixelColorWithAlphaValue > 0 && pixelColorWithAlphaAlpha > 0 && pixelColorWithAlphaAlpha != 255) {
                    //pBmpWithoutAlpha->SetPixel(x, y, Gdiplus::Color::White);
                    pBmpWithAlpha->SetPixel(x, y, Gdiplus::Color(255, 255, 255));
                }
            }
        }
    }

    CLSID   encoderClsid;
    Status  stat;
    //Get the CLSID of the PNG encoder.
    GetEncoderClsid(L"image/tiff", &encoderClsid);

    // Save the image
    BOOL bRes = (pBmpWithAlpha->Save(tempfilename, &encoderClsid, NULL) == Ok);

    // Clean up GDI+ image
    delete pBmpWithAlpha;
    //_wrename(filename, filenameOriginal);
    bool fSuccess = DeleteFile(filename);
    if (fSuccess)
    {
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"ImageNormalize _wrename TIFF %ws", tempfilename);
        _wrename(tempfilename, filename);
        DoTraceMessage(RENDERFILTER_TRACE_INFO, L"ImageNormalize _wrename TIFF %ws", filename);
    }
    GdiplusShutdown(gdiplusToken);
    return bRes;
}

}