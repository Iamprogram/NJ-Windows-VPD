
#include "precomp.h"
#include "WppTrace.h"
#include "CustomWppCommands.h"

#include "PDFWriter.h"
#include "PDFFormXObject.h"
#include "PDFPage.h"
#include "PageContentContext.h"
#include "BoxingBase.h"

#include <iostream>
#include <mbstring.h>
//#include "testing/TestIO.h"
#include "TIFFImagePDF.h"
#include "TIFFImagePDF.tmh"

using namespace std;
using namespace PDFHummus;

static EStatusCode AddPageForTIFF(PDFWriter& inpdfWriter, const string& inTiffFilePath)
{
	EStatusCode status = PDFHummus::eSuccess;

	do {
		PDFPage* page = new PDFPage();
		page->SetMediaBox(PDFRectangle(0,0,595,842));

		PageContentContext* pageContentContext = inpdfWriter.StartPageContentContext(page);
		if(NULL == pageContentContext)
		{
			status = PDFHummus::eFailure;
			cout<<"failed to create content context for page, for file"<<inTiffFilePath.c_str()<<"\n";
		}

		PDFFormXObject* imageFormXObject = inpdfWriter.CreateFormXObjectFromTIFFFile(inTiffFilePath.c_str());
		if(!imageFormXObject)
		{
			cout<<"failed to create image form XObject from file, for file"<<inTiffFilePath.c_str()<<"\n";
			status = PDFHummus::eFailure;
			break;
		}

		string imageXObjectName = page->GetResourcesDictionary().AddFormXObjectMapping(imageFormXObject->GetObjectID());

		// continue page drawing, place the image in 0,0 (playing...could avoid CM at all)
		pageContentContext->q();
		pageContentContext->cm(1,0,0,1,0,0);
		pageContentContext->Do(imageXObjectName);
		pageContentContext->Q();

		delete imageFormXObject;

		status = inpdfWriter.EndPageContentContext(pageContentContext);
		if(status != PDFHummus::eSuccess)
		{
			cout<<"failed to end page content context, for file"<<inTiffFilePath.c_str()<<"\n";
			break;
		}

		status = inpdfWriter.WritePageAndRelease(page);
		if(status != PDFHummus::eSuccess)
		{
			cout<<"failed to write page, for file"<<inTiffFilePath.c_str()<<"\n";
			break;
		}
	}while(false);

	return status;
}

int TIFFImagePDF(const WCHAR* outputFolder, const int IdPagesCount, const CHAR* inPDFName)  
{
	DoTraceMessage(RENDERFILTER_TRACE_INFO, L"TIFFImagePDF ***   %ws ", outputFolder);
	DoTraceMessage(RENDERFILTER_TRACE_INFO, L"TIFFImagePDF ***  %s", inPDFName);
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
	strcat_s(fname, "\\outputtry.pdf");
	strcpy_s(fnamelog, fname);
	strcat_s(fnamelog, ".log");
	//strcpy_s(inImageName, sizeof(fname), fname);
	delete[]nstring;

	if (inPDFName==NULL) {
		return 1;
	}
	status = pdfWriter.StartPDF(inPDFName, ePDFVersion13, LogConfiguration(true, true, fnamelog));
		if(status != PDFHummus::eSuccess){
			cout<<"failed to start PDF\n";
		}	

		for (int i = 0; i < IdPagesCount; i++) {
			strcpy_s(srcpngfname, basename);
			strcat_s(srcpngfname, "\\outputtry");
			char buffer[10];
			sprintf_s(buffer, "%d", i);
			strcat_s(srcpngfname, buffer);
			strcat_s(srcpngfname, ".tiff");

			status = AddPageForTIFF(pdfWriter, srcpngfname);
			if (status != PDFHummus::eSuccess) {
				;
			}
		}

		status = pdfWriter.EndPDF();
		if (status != PDFHummus::eSuccess)
		{
			cout << "failed in end PDF\n";
		}
	return 0;	
}

