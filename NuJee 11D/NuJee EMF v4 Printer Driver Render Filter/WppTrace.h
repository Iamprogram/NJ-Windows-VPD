//
// File Name:
//
//    WppTrace.h
//
// Abstract:
//
//    WPP tracing definitions.
//

#pragma once


#define WPP_CONTROL_GUIDS WPP_DEFINE_CONTROL_GUID(                                       \
                                NuJeeEMFv4PrinterDriverRenderFilter,                              \
                                (098fec04,4275,45dc,81ae,26777e629d27),                  \
                                WPP_DEFINE_BIT(RENDERFILTER_TRACE_ERROR)                 \
                                WPP_DEFINE_BIT(RENDERFILTER_TRACE_WARNING)               \
                                WPP_DEFINE_BIT(RENDERFILTER_TRACE_INFO)                  \
                                WPP_DEFINE_BIT(RENDERFILTER_TRACE_VERBOSE)               \
                                )


