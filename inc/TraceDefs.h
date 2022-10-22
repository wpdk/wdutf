/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2017, DataCore Software Corporation. All rights reserved.
 */

/*
 * Trace definitions.
 */

#ifndef _DCS_TRACEDEFS_H_
#define _DCS_TRACEDEFS_H_

#include <stdarg.h>

#ifdef __cplusplus
namespace Tracing {
#endif


/*
 * Determine type of tracing
 */

#if defined(WPP) || defined(RUN_WPP)
#define _DCS_TRACE_WPP_
#endif

#ifdef _DCS_TRACE_WPP_
#undef _DCS_TRACE_DBG_
#endif

#if !defined(_DCS_TRACE_WPP_) && defined(_DCS_KERNEL_) && defined(_DEBUG)
#define _DCS_TRACE_DBG_
#endif

#if defined(_DCS_TRACE_DBG_) || (defined(_DCS_TRACE_WPP_) && defined(_DEBUG))
#define _DCS_EXPAND_WPP_
#endif


/*
 * Define name
 */

#ifndef WPPNAME
#error WPPNAME must be defined before including TraceDefs.h
#endif

#define _DCS_TRACE_QUOTE(s)		#s
#define _DCS_TRACE_STR(s)		_DCS_TRACE_QUOTE(s)

#define WPPNAMESTR				_DCS_TRACE_STR(WPPNAME)
#define WPPDBGPREFIX			WPPNAMESTR ": "


/*
 * Define WPP Trace levels
 */

#undef TRACE_LEVEL_FATAL
#undef TRACE_LEVEL_ERROR
#undef TRACE_LEVEL_WARNING
#undef TRACE_LEVEL_INFORMATION
#undef TRACE_LEVEL_VERBOSE
#undef TRACE_LEVEL_NOISY

static const int TRACE_LEVEL_FATAL			= 1;
static const int TRACE_LEVEL_ERROR			= 2;
static const int TRACE_LEVEL_WARNING		= 3;
static const int TRACE_LEVEL_INFORMATION	= 4;
static const int TRACE_LEVEL_VERBOSE		= 5;
static const int TRACE_LEVEL_NOISY			= 8;

static const int TRACE_FATAL		= TRACE_LEVEL_FATAL;
static const int TRACE_ERROR		= TRACE_LEVEL_ERROR;
static const int TRACE_WARNING		= TRACE_LEVEL_WARNING;
static const int TRACE_INFO			= TRACE_LEVEL_INFORMATION;
static const int TRACE_VERBOSE		= TRACE_LEVEL_VERBOSE;
static const int TRACE_NOISY		= TRACE_LEVEL_NOISY;


/*
 * Define DbgPrint levels
 */

#ifndef DPFLTR_ERROR_LEVEL
#define DPFLTR_ERROR_LEVEL			0
#define DPFLTR_WARNING_LEVEL		1
#define DPFLTR_TRACE_LEVEL			2
#define DPFLTR_INFO_LEVEL			3
#endif

#define DPFLTR_VERBOSE_LEVEL		4
#define DPFLTR_NOISY_LEVEL			8


/*
 * Define Level and Flag filters.
 */

#ifndef WPPLEVEL
#define WPPLEVEL	TRACE_LEVEL_INFORMATION
#endif

#ifndef WPPFLAGS
#define WPPFLAGS	0xffffffff
#endif


/*
 * Defines for WPP initialisation
 */

#ifndef _DCS_TRACE_WPP_
#undef WPP_DEFINE_BIT
#define WPP_DEFINE_BIT(x) WPP_##x,

#undef WPP_DEFINE_CONTROL_GUID
#define WPP_DEFINE_CONTROL_GUID(name,guid,...) enum { __VA_ARGS__ };

#undef WPP_INIT_TRACING
#define WPP_INIT_TRACING(...)	((void)(0, __VA_ARGS__))

#undef WPP_CLEANUP
#define WPP_CLEANUP(...)	((void)(0, __VA_ARGS__))
#endif

#undef WPP_DEFINE_DEFAULT_BITS
#define WPP_DEFINE_DEFAULT_BITS                                        \
        WPP_DEFINE_BIT(MYDRIVER_ALL_INFO)                              \
        WPP_DEFINE_BIT(TRACE_KDPRINT)                                  \
        WPP_DEFINE_BIT(DEFAULT_TRACE_LEVEL)


/*
 * Map WPP levels to DbgPrint levels
 */

__inline unsigned long TraceLevelToDbg(unsigned long v) {
	return	v <= TRACE_LEVEL_FATAL ? DPFLTR_ERROR_LEVEL :
			v == TRACE_LEVEL_ERROR ? DPFLTR_ERROR_LEVEL :
			v == TRACE_LEVEL_WARNING ? DPFLTR_WARNING_LEVEL :
			v == TRACE_LEVEL_INFORMATION ? DPFLTR_INFO_LEVEL :
			v == TRACE_LEVEL_VERBOSE ? DPFLTR_VERBOSE_LEVEL :
			v == TRACE_LEVEL_NOISY ? DPFLTR_NOISY_LEVEL : DPFLTR_TRACE_LEVEL;
}

__inline unsigned long DbgLevelToTrace(unsigned long v) {
	return	v == DPFLTR_ERROR_LEVEL ? TRACE_LEVEL_ERROR :
			v == DPFLTR_WARNING_LEVEL ? TRACE_LEVEL_WARNING :
			v == DPFLTR_TRACE_LEVEL ? TRACE_LEVEL_INFORMATION :
			v == DPFLTR_INFO_LEVEL ? TRACE_LEVEL_INFORMATION :
			v == DPFLTR_VERBOSE_LEVEL ? TRACE_LEVEL_VERBOSE :
			v == DPFLTR_NOISY_LEVEL ? TRACE_LEVEL_NOISY : TRACE_LEVEL_VERBOSE;
}


/*
 * Define trace types
 */

struct TraceData {
   const void *data;
   short len;
   TraceData(const void *d, size_t l) : data(d), len((short)l) { }
   template<class T> TraceData(const T *d) : data(d), len((short)sizeof(T)) { }
};

#define WPP_LOGDATA(data,len)	TraceData(data,len)


/*
 * User Level compatibility
 */

#ifndef _DCS_KERNEL_
#ifdef _DCS_TRACE_DBG_
#define DbgPrint(fmt,...) (printf(fmt, __VA_ARGS__), 0)
#define DbgPrintEx(id,level,fmt,...) \
	(((level) <= TraceLevelToDbg(WPPLEVEL)) ? (printf(fmt, __VA_ARGS__), 0) : 0)
#define vDbgPrintEx(id,level,fmt,ap) \
	(((level) <= TraceLevelToDbg(WPPLEVEL)) ? (vprintf(fmt, ap), 0) : 0)
#define DbgQueryDebugFilterState(id,level) ((level) <= TraceLevelToDbg(WPPLEVEL))
#else
#define DbgPrint(...) (0)
#define DbgPrintEx(...) (0)
#define vDbgPrintEx(...) (0)
#define vDbgPrintExWithPrefix(...) (0)
#define DbgQueryDebugFilterState(...) (0)
#endif
#undef KdPrint
#define KdPrint(x) DbgPrint x
#undef KdPrintEx
#define KdPrintEx(x) DbgPrintEx x
#endif


/*
 * Convert Trace to DbgPrint
 */

#ifndef _DCS_TRACE_WPP_
#ifdef _DCS_TRACE_DBG_
#define TraceEvents(level,flag,fmt,...) ((WPPFLAGS & (1 << (WPP_##flag))) ? \
	DbgPrintTraceMsg(TraceLevelToDbg(level), WPPTRACEINFO(level,flag), WPPDBGPREFIX fmt "\n", __VA_ARGS__) : (void)0)

#define Trace(level,fmt,...) \
	TraceEvents(level, MYDRIVER_ALL_INFO, fmt, __VA_ARGS__)

#define TraceEventsWithData(level,flag,data,len,fmt,...) ((WPPFLAGS & (1 << (WPP_##flag))) ? \
	DbgPrintTraceMsg(TraceLevelToDbg(level), WPPTRACEINFO(level,flag), WPPDBGPREFIX fmt " Data: %!HEXDUMP!\n", __VA_ARGS__, \
		WPP_LOGDATA(data,len)) : (void)0)

#define TraceWithData(level,data,len,fmt,...) \
	TraceEventsWithData(level, MYDRIVER_ALL_INFO, data, len, fmt, __VA_ARGS__)
#else
__inline void Trace(...) { }
__inline void TraceEvents(...) { }
__inline void TraceWithData(...) { }
__inline void TraceEventsWithData(...) { }
#endif
#endif


/*
 * SendTrace definitions
 */

#ifndef _DCS_TRACE_WPP_
#define SendTraceEvents(flags, msgid, level, msg, ...) \
	SendEvent(flags, msgid, __VA_ARGS__), \
	TraceEvents(level, flags, msg, __VA_ARGS__)

#define SendTrace(msgid, level, msg, ...) \
	SendTraceEvents(DEFAULT_TRACE_LEVEL, msgid, level, msg, __VA_ARGS__)

#define SendTraceEventsWithData(flags, msgid, level, data, len, msg, ...) \
	SendEventWithData(flags, msgid, data, len, __VA_ARGS__), \
	TraceEventsWithData(level, flags, data, len, msg, __VA_ARGS__)

#define SendTraceWithData(msgid, level, data, len, msg,...) \
	SendTraceEventsWithData(DEFAULT_TRACE_LEVEL, msgid, level, data, len, msg, __VA_ARGS__)
#endif


/*
 * DbgPrint functions
 */

__inline void DbgPrintDebugMsg(unsigned long level, int, char *fmt, ...)
{
	(void)level;
	va_list ap;
	va_start(ap, fmt);
	vDbgPrintEx(DPFLTR_DEFAULT_ID, level, fmt, ap);
	va_end(ap);
}


/*
 * DbgPrintTrace functions
 */

#if defined(_DCS_EXPAND_WPP_)
#include <TraceDbg.h>
#else
#define DbgPrintTraceMsg	DbgPrintDebugMsg
#endif


/*
 * Kernel compatibility
 */

#ifdef _DCS_KERNEL_
#define printf(fmt,...) KdPrint((fmt,__VA_ARGS__))
#endif


/*
 * Namespace compatibility
 */

#ifdef __cplusplus
}
using namespace Tracing;
#endif

#endif	/* _DCS_TRACEDEFS_H_ */

