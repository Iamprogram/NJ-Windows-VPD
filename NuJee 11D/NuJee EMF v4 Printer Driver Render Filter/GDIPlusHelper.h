#pragma once
namespace NuJee_EMF_v4_Printer_Driver_Render_Filter
{
    class GDIPlusHelper
    {
    public:
        INT static GetEncoderClsid(const WCHAR* format, CLSID* pClsid);  // helper function

        INT static GDIPlusImagePNGConvertEMF(const int IdPagesCount, _In_ PCWSTR pRasterFile, CHAR* outputPDFFile);
        INT static GDIPlusImageTIFFConvertEMF(const int IdPagesCount, _In_ PCWSTR pRasterFile);
        INT static GDIPlusImageTIFFConvertBMP(_In_ PCWSTR pRasterFile);
        INT static GDIPlusImageTIFFConvertGIF(_In_ PCWSTR pRasterFile);
        INT static GDIPlusImageTIFFConvertJPG(_In_ PCWSTR pRasterFile);
        INT static GDIPlusImageTIFFConvertPNG(const int IdPagesCount, _In_ PCWSTR pRasterFile);

        INT static GDIPlusImageJPGConvertPNG(const int IdPagesCount, _In_ PCWSTR pRasterFile);
        BOOL static ImageNormalize(const int IdPagesCount, _In_ PCWSTR pRasterFile);
        BOOL static ImageAsNormalFinal(const int IdPagesCount, _In_ PCWSTR pRasterFile);
    };
} // namespace NuJee_EMF_v4_Printer_Driver_Render_Filter

