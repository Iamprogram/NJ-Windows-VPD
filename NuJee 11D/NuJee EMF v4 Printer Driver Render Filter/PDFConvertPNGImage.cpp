/*
   Source File : JPGImageTest.cpp


   Copyright 2011 Gal Kahana PDFWriter

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   
*/

#include "precomp.h"
#include "WppTrace.h"
#include "CustomWppCommands.h"

#include "PDFWriter.h"
#include "PDFPage.h"
#include "PageContentContext.h"
#include "PDFFormXObject.h"

#include "PDFConvertPNGImage.tmh"

#include <iostream>

#include "TestIO.h"
#include "PDFConvertPNGImage.h"

using namespace std;
using namespace PDFHummus;


bool RunImageTest(const WCHAR* outputFolder, const WCHAR* inImageName) {
	PDFWriter pdfWriter;
	EStatusCode status;

	char basename[256];
	char srcpngfname[256];
	char fname[256];
	char fnamelog[256];
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
	strcat_s(fname, "111.pdf");
	strcpy_s(fnamelog, fname);
	strcat_s(fnamelog, ".log");
	//strcpy_s(outputPDFFile, sizeof(fname), fname);
	delete[]nstring;

	strcpy_s(srcpngfname, basename);
	strcat_s(srcpngfname, "0.png");

	char srcpngfname2[256];
	strcpy_s(srcpngfname2, basename);
	strcat_s(srcpngfname2, "original.png");

	do
	{
		status = pdfWriter.StartPDF(fname, ePDFVersion14, LogConfiguration(true, true, fnamelog));
		if (status != PDFHummus::eSuccess)
		{
			cout << "failed to start PDF\n";
			break;
		}

		PDFPage* page = new PDFPage();
		page->SetMediaBox(PDFRectangle(0, 0, 595, 842));

		PageContentContext* pageContentContext = pdfWriter.StartPageContentContext(page);
		if (NULL == pageContentContext)
		{
			status = PDFHummus::eFailure;
			cout << "failed to create content context for page\n";
		}

		// place a large red rectangle all over the page
		AbstractContentContext::GraphicOptions pathFillOptions(AbstractContentContext::eFill,
			AbstractContentContext::eRGB,
			0xFF0000);
		/*
		pageContentContext->DrawRectangle(0, 0, 595, 842, pathFillOptions);

		DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"PDFConvertPNGImage 2::......[%s]", srcpngfname2);
		// place the image on top...hopefully we can see soem transparency
		AbstractContentContext::ImageOptions imageOptions;
		imageOptions.transformationMethod = AbstractContentContext::eMatrix;
		imageOptions.matrix[0] = imageOptions.matrix[3] = 0.5;
		pageContentContext->DrawImage(150, 160, srcpngfname2, imageOptions);
		*/

		DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"PDFConvertPNGImage::......[%s]", srcpngfname);
		// place the image on top...hopefully we can see soem transparency
		AbstractContentContext::ImageOptions imageOptions2;
		imageOptions2.transformationMethod = AbstractContentContext::eFit;
		//imageOptions2.matrix[0] = imageOptions2.matrix[3] = 1;
		//imageOptions2.fitProportional = true;
		pageContentContext->DrawImage(0, 0, srcpngfname, imageOptions2);

		status = pdfWriter.EndPageContentContext(pageContentContext);
		if (status != PDFHummus::eSuccess)
		{
			DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"PDFConvertPNGImage::failed to end page content context...");
				cout << "failed to end page content context\n";
			break;
		}

		status = pdfWriter.WritePageAndRelease(page);
		if (status != PDFHummus::eSuccess)
		{
			DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"PDFConvertPNGImage::failed to write page...");
			cout << "failed to write page\n";
			break;
		}


		status = pdfWriter.EndPDF();
		if (status != PDFHummus::eSuccess)
		{
			DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"PDFConvertPNGImage::failed in end PDF ...");
			cout << "failed in end PDF\n";
			break;
		}

		DoTraceMessage(RENDERFILTER_TRACE_VERBOSE, L"PDFConvertPNGImage::okok...");
	} while (false);

	return 0;
}

int PNGImageTest(int argc, char* argv[])
{
	EStatusCode status;
	/*
	do
	{
		status = RunImageTest(argv, "original");
		if (status != eSuccess) {
			cout << "failed in original.png test" << "\n";
		}
		

		status = RunImageTest(argv, "original_transparent");
		if (status != eSuccess) {
			cout << "failed in original.png test" << "\n";
		}

		status = RunImageTest(argv, "gray-alpha-8-linear");
		if (status != eSuccess) {
			cout << "failed in original.png test" << "\n";
		}

		status = RunImageTest(argv, "gray-16-linear");
		if (status != eSuccess) {
			cout << "failed in original.png test" << "\n";
		}

		status = RunImageTest(argv, "pnglogo-grr");
		if (status != eSuccess) {
			cout << "failed in original.png test" << "\n";
		}

	} while (false);

	*/
	return 1;
}

