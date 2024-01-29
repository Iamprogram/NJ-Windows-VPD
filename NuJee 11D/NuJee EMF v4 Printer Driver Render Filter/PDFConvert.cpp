/*
 * << Haru Free PDF Library 2.0.0 >> -- image_demo.c
 *
 * Copyright (c) 1999-2006 Takeshi Kanno <takeshi_kanno@est.hi-ho.ne.jp>
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 * It is provided "as is" without express or implied warranty.
 *
 */
#include "precomp.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <setjmp.h>
#include "hpdf.h"

#include "PDFConvert.h"

//#include "filtertypes.h"

#ifndef HPDF_NOPNGLIB

jmp_buf env;

#ifdef HPDF_DLL
void  __stdcall
#else
void
#endif
error_handler  (HPDF_STATUS   error_no,
                HPDF_STATUS   detail_no,
                void         *user_data)
{
    printf ("ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no,
                (HPDF_UINT)detail_no);
    longjmp(env, 1);
}

void
show_description (HPDF_Page    page,
                  float        x,
                  float        y,
                  const char  *text)
{
    char buf[255];

    HPDF_Page_MoveTo (page, x, y - 10);
    HPDF_Page_LineTo (page, x, y + 10);
    HPDF_Page_MoveTo (page, x - 10, y);
    HPDF_Page_LineTo (page, x + 10, y);
    HPDF_Page_Stroke (page);

    HPDF_Page_SetFontAndSize (page, HPDF_Page_GetCurrentFont (page), 8);
    HPDF_Page_SetRGBFill (page, 0, 0, 0);

    HPDF_Page_BeginText (page);

#ifdef __WIN32__
    _snprintf_s(buf, 255, "(x=%d,y=%d)", (int)x, (int)y);
#else
    snprintf(buf, 255, "(x=%d,y=%d)", (int)x, (int)y);
#endif /* __WIN32__ */
    HPDF_Page_MoveTextPos (page, x - HPDF_Page_TextWidth (page, buf) - 5,
            y - 10);
    HPDF_Page_ShowText (page, buf);
    HPDF_Page_EndText (page);

    HPDF_Page_BeginText (page);
    HPDF_Page_MoveTextPos (page, x - 20, y - 25);
    HPDF_Page_ShowText (page, text);
    HPDF_Page_EndText (page);
}


int GenerateDocPdfFromPNG(const WCHAR* outputFolder, const int IdPagesCount, CHAR* outputPDFFile)
{
    HPDF_Doc  pdf;
    HPDF_Font font;
    HPDF_Page page;
    char basename[256];
    char srcpngfname[256];
    char fname[256];
    HPDF_Destination dst;
    HPDF_Image image;

    double x;
    double y;
    double iw;
    double ih;

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
    strcpy_s(outputPDFFile,sizeof(fname),fname);
    delete[]nstring;

    pdf = HPDF_New(error_handler, NULL);
    if (!pdf) {
        printf("error: cannot create PdfDoc object\n");
        return 1;
    }
    /* error-handler */
    if (setjmp(env)) {
        HPDF_Free(pdf);
        return 1;
    }

    HPDF_SetCompressionMode(pdf, HPDF_COMP_ALL);
    /* create default-font */
    font = HPDF_GetFont(pdf, "Helvetica", NULL);
    for (int i = 0; i < IdPagesCount; i++) {
        // source PNGfilename
        strcpy_s(srcpngfname, basename);
        char buffer[10];
        sprintf_s(buffer, "%d", i);
        strcat_s(srcpngfname, buffer);
        strcat_s(srcpngfname, ".png");

        image = HPDF_LoadPngImageFromFile(pdf, srcpngfname);
        iw = HPDF_Image_GetWidth(image);
        ih = HPDF_Image_GetHeight(image);

        /* add a new page object. */
        page = HPDF_AddPage(pdf);
        HPDF_Page_SetWidth(page, (HPDF_REAL)(iw + 0));
        HPDF_Page_SetHeight(page, (HPDF_REAL)(ih + 0));

        dst = HPDF_Page_CreateDestination(page);
        HPDF_Destination_SetXYZ(dst, 0, HPDF_Page_GetHeight(page), 1);
        HPDF_SetOpenAction(pdf, dst);

        HPDF_Page_BeginText(page);
        HPDF_Page_SetFontAndSize(page, font, 20);
        HPDF_Page_MoveTextPos(page, 220, HPDF_Page_GetHeight(page) - 70);
        HPDF_Page_ShowText(page, "NuJee ST Demo");
        HPDF_Page_EndText(page);

        HPDF_Page_SetLineWidth(page, 0.5);
        x = 0;
        y = 0;
        //y = HPDF_Page_GetHeight(page) - 150;
        /* Draw image to the canvas. (normal-mode with actual size.)*/
        HPDF_Page_DrawImage(page, image, (HPDF_REAL)x, (HPDF_REAL)y, (HPDF_REAL)iw, (HPDF_REAL)ih);
        show_description(page, (float)x, (float)y, "Actual Size");

    }


    /* save the document to a file */
    HPDF_SaveToFile(pdf, fname);

    /* clean up */
    HPDF_Free(pdf);

    return 0;
}

int GeneratePagePdfFromPNG(const WCHAR* pngFileName, const WCHAR* pdfFileName)
{
    HPDF_Doc  pdf;
    HPDF_Font font;
    HPDF_Page page;
    char srcpngfname[256];
    char fname[256];
    HPDF_Destination dst;
    HPDF_Image image;

    double x;
    double y;
    double iw;
    double ih;

    size_t origsize = wcslen(pngFileName) + 1;
    size_t convertedChars = 0;
    char strConcat[] = "";
    size_t strConcatsize = (strlen(strConcat) + 1) * 2;
    const size_t newsize = origsize * 2;
    char* nstring = new char[newsize + strConcatsize];
    wcstombs_s(&convertedChars, nstring, newsize, pngFileName, _TRUNCATE);
    _mbscat_s((unsigned char*)nstring, newsize + strConcatsize, (unsigned char*)strConcat);
    strcpy_s(srcpngfname, nstring);
    //strcat_s(srcpngfname, ".png");
    delete[]nstring;

    origsize = wcslen(pdfFileName) + 1;
    convertedChars = 0;
    strConcatsize = (strlen(strConcat) + 1) * 2;
    const size_t newsize2 = origsize * 2;
    char* nstring2 = new char[newsize2 + strConcatsize];
    wcstombs_s(&convertedChars, nstring2, newsize2, pdfFileName, _TRUNCATE);
    _mbscat_s((unsigned char*)nstring2, newsize2 + strConcatsize, (unsigned char*)strConcat);
    strcpy_s(fname, nstring2);
    //strcat_s(fname, ".pdf");
    delete[]nstring2;


    pdf = HPDF_New(error_handler, NULL);
    if (!pdf) {
        printf("error: cannot create PdfDoc object\n");
        return 1;
    }
    /* error-handler */
    if (setjmp(env)) {
        HPDF_Free(pdf);
        return 1;
    }

    HPDF_SetCompressionMode(pdf, HPDF_COMP_ALL);
    /* create default-font */
    font = HPDF_GetFont(pdf, "Helvetica", NULL);
    for (int i = 0; i < 1; i++) {
        image = HPDF_LoadPngImageFromFile(pdf, srcpngfname);
        iw = HPDF_Image_GetWidth(image);
        ih = HPDF_Image_GetHeight(image);

        /* add a new page object. */
        page = HPDF_AddPage(pdf);
        HPDF_Page_SetWidth(page, (HPDF_REAL)(iw + 0));
        HPDF_Page_SetHeight(page, (HPDF_REAL)(ih + 0));

        dst = HPDF_Page_CreateDestination(page);
        HPDF_Destination_SetXYZ(dst, 0, HPDF_Page_GetHeight(page), 1);
        HPDF_SetOpenAction(pdf, dst);

        HPDF_Page_BeginText(page);
        HPDF_Page_SetFontAndSize(page, font, 20);
        HPDF_Page_MoveTextPos(page, 220, HPDF_Page_GetHeight(page) - 70);
        HPDF_Page_ShowText(page, "NuJee ST Demo");
        HPDF_Page_EndText(page);

        HPDF_Page_SetLineWidth(page, 0.5);
        x = 0;
        y = 0;
        //y = HPDF_Page_GetHeight(page) - 150;
        /* Draw image to the canvas. (normal-mode with actual size.)*/
        HPDF_Page_DrawImage(page, image, (HPDF_REAL)x, (HPDF_REAL)y, (HPDF_REAL)iw, (HPDF_REAL)ih);
        show_description(page, (float)x, (float)y, "Actual Size");

    }


    /* save the document to a file */
    HPDF_SaveToFile(pdf, fname);

    /* clean up */
    HPDF_Free(pdf);

    return 0;
}

#else

int main()
{
    printf("WARNING: if you want to run this demo, \n"
           "make libhpdf with HPDF_USE_PNGLIB option.\n");
    return 0;
}

#endif /* HPDF_NOPNGLIB */

