#pragma once


int GeneratePagePdfFromPNG(const WCHAR* pngFileName, const WCHAR* pdfFileName);
int GenerateDocPdfFromPNG(const WCHAR* outputFolder, const int IdPagesCount, CHAR* outputPDFFile);
