//
// File Name:
//
//    Exception.cpp
//
// Abstract:
//
//    Exception routine definitions.
//

#include "precomp.h"
#include "WppTrace.h"
#include "CustomWppCommands.h"
#include "Exception.h"
#include "filtertypes.h"

#include "Exception.tmh"

namespace NuJee_EMF_v4_Printer_Driver_Render_Filter
{

void ThrowHRException(
    HRESULT hr,
    char const *fileName,
    int lineNum
    )
{
    DoTraceMessage(
        RENDERFILTER_TRACE_ERROR,
        L"Throwing HRESULT Exception from %s:%d (HRESULT=%!HRESULT!)", 
        fileName, 
        lineNum, 
        hr
        );

    throw hr_error(hr);
}

} // namespace NuJee_EMF_v4_Printer_Driver_Render_Filter
