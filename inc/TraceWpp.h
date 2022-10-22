/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2017, DataCore Software Corporation. All rights reserved.
 */

/*
 * Trace definitions for WPP.
 */

#ifdef __cplusplus
namespace Tracing {
#endif

#ifndef _DCS_TRACEWPP_H_
#define _DCS_TRACEWPP_H_


#define WPPNULLFUNC(...)	(0)

#ifndef WPPTRACEINFO
#define WPPTRACEINFO(level,flag)	(0)
#endif

#define WPPNULLDATA(...)
#define WPPTRACEDATA(data,len)		,WPP_LOGDATA(data,len)


/*
 * Trace() definitions
 */

#define WPP_EX_FLAG_LEVEL(flag, level) \
	TraceLevelToDbg(level), WPPNULLFUNC, 0, 0, DbgPrintTraceMsg, \
	WPPTRACEINFO(level,flag), WPPNULLDATA, 0, 0, WPPDBGPREFIX, "\n"

#define WPP_FLAG_LEVEL_LOGGER(flag, level) \
    WPP_LEVEL_LOGGER(flag)

#define WPP_FLAG_LEVEL_ENABLED(flag, lvl) \
    (WPP_LEVEL_ENABLED(flag) && WPP_CONTROL(WPP_BIT_ ## flag).Level >= lvl)

#define WPP_RECORDER_FLAG_LEVEL_ARGS(flag, lvl) \
	WPP_CONTROL(WPP_BIT_ ## flag).AutoLogContext, lvl, WPP_BIT_ ## flag

#define WPP_RECORDER_FLAG_LEVEL_FILTER(flag, lvl) \
	(lvl < TRACE_LEVEL_VERBOSE || WPP_CONTROL(WPP_BIT_ ## flag).AutoLogVerboseEnabled)


/*
 * TraceEvents() definitions
 */

#define WPP_EX_LEVEL_FLAGS(level, flags) \
	TraceLevelToDbg(level), WPPNULLFUNC, 0, 0, DbgPrintTraceMsg, \
	WPPTRACEINFO(level,flags), WPPNULLDATA, 0, 0, WPPDBGPREFIX, "\n"

#define WPP_LEVEL_FLAGS_LOGGER(lvl, flags) \
    WPP_LEVEL_LOGGER(flags)
               
#define WPP_LEVEL_FLAGS_ENABLED(lvl, flags) \
    (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level >= lvl)

#define WPP_RECORDER_LEVEL_FLAGS_ARGS(lvl, flags) \
	WPP_CONTROL(WPP_BIT_ ## flags).AutoLogContext, lvl, WPP_BIT_ ## flags

#define WPP_RECORDER_LEVEL_FLAGS_FILTER(lvl, flags) \
	(lvl < TRACE_LEVEL_VERBOSE || WPP_CONTROL(WPP_BIT_ ## flags).AutoLogVerboseEnabled)


/*
 * TraceWithData() definitions
 */

#define WPP_EX_FLAG_LEVEL_DATA_LEN(flag, level, data, len) \
	TraceLevelToDbg(level), WPPNULLFUNC, 0, 0, DbgPrintTraceMsg, \
	WPPTRACEINFO(level,flag), WPPTRACEDATA, data, len, WPPDBGPREFIX, " Data: %!HEXDUMP!\n"

#define WPP_FLAG_LEVEL_DATA_LEN_LOGGER(flag, level, data, len) \
    WPP_LEVEL_LOGGER(flag)

#define WPP_FLAG_LEVEL_DATA_LEN_ENABLED(flag, lvl, data, len) \
    (WPP_LEVEL_ENABLED(flag) && WPP_CONTROL(WPP_BIT_ ## flag).Level >= lvl)

#define WPP_RECORDER_FLAG_LEVEL_DATA_LEN_ARGS(flag, lvl, data, len) \
	WPP_CONTROL(WPP_BIT_ ## flag).AutoLogContext, lvl, WPP_BIT_ ## flag

#define WPP_RECORDER_FLAG_LEVEL_DATA_LEN_FILTER(flag, lvl, data, len) \
	(lvl < TRACE_LEVEL_VERBOSE || WPP_CONTROL(WPP_BIT_ ## flag).AutoLogVerboseEnabled)


/*
 * TraceEventsWithData() definitions
 */

#define WPP_EX_LEVEL_FLAGS_DATA_LEN(level, flags, data, len) \
	TraceLevelToDbg(level), WPPNULLFUNC, 0, 0, DbgPrintTraceMsg, \
	WPPTRACEINFO(level,flags), WPPTRACEDATA, data, len, WPPDBGPREFIX, " Data: %!HEXDUMP!\n"

#define WPP_LEVEL_FLAGS_DATA_LEN_LOGGER(lvl, flags, data, len) \
    WPP_LEVEL_LOGGER(flags)
               
#define WPP_LEVEL_FLAGS_DATA_LEN_ENABLED(lvl, flags, data, len) \
    (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level >= lvl)

#define WPP_RECORDER_LEVEL_FLAGS_DATA_LEN_ARGS(lvl, flags, data, len) \
	WPP_CONTROL(WPP_BIT_ ## flags).AutoLogContext, lvl, WPP_BIT_ ## flags

#define WPP_RECORDER_LEVEL_FLAGS_DATA_LEN_FILTER(lvl, flags, data, len) \
	(lvl < TRACE_LEVEL_VERBOSE || WPP_CONTROL(WPP_BIT_ ## flags).AutoLogVerboseEnabled)


/*
 * SendTrace() definitions
 */

#define WPP_EX_FLAG_MSGID_LEVEL(flag, msgid, level) \
	TraceLevelToDbg(level), SendEventWithData, flag, msgid, DbgPrintTraceMsg, \
	WPPTRACEINFO(level,flag), WPPNULLDATA, 0, 0, WPPDBGPREFIX, "\n"

#define WPP_FLAG_MSGID_LEVEL_LOGGER(flag, msgid, level) \
    WPP_LEVEL_LOGGER(flag)
               
#define WPP_FLAG_MSGID_LEVEL_ENABLED(flag, msgid, lvl) \
    (WPP_LEVEL_ENABLED(flag) && WPP_CONTROL(WPP_BIT_ ## flag).Level >= lvl)

#define WPP_RECORDER_FLAG_MSGID_LEVEL_ARGS(flag, msgid, lvl) \
	WPP_CONTROL(WPP_BIT_ ## flag).AutoLogContext, lvl, WPP_BIT_ ## flag

#define WPP_RECORDER_FLAG_MSGID_LEVEL_FILTER(flag, msgid, lvl) \
	(lvl < TRACE_LEVEL_VERBOSE || WPP_CONTROL(WPP_BIT_ ## flag).AutoLogVerboseEnabled)


/*
 * SendTraceEvents() definitions
 */

#define WPP_EX_FLAGS_MSGID_LEVEL(flags, msgid, level) \
	TraceLevelToDbg(level), SendEventWithData, flags, msgid, DbgPrintTraceMsg, \
	WPPTRACEINFO(level,flags), WPPNULLDATA, 0, 0, WPPDBGPREFIX, "\n"

#define WPP_FLAGS_MSGID_LEVEL_LOGGER(flags, msgid, level) \
    WPP_LEVEL_LOGGER(flags)

#define WPP_FLAGS_MSGID_LEVEL_ENABLED(flags, msgid, lvl) \
    (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level >= lvl)

#define WPP_RECORDER_FLAGS_MSGID_LEVEL_ARGS(lvl, flags, data, len) \
	WPP_CONTROL(WPP_BIT_ ## flags).AutoLogContext, lvl, WPP_BIT_ ## flags

#define WPP_RECORDER_FLAGS_MSGID_LEVEL_FILTER(lvl, flags, data, len) \
	(lvl < TRACE_LEVEL_VERBOSE || WPP_CONTROL(WPP_BIT_ ## flags).AutoLogVerboseEnabled)


/*
 * SendTraceWithData() definitions
 */

#define WPP_EX_FLAG_MSGID_LEVEL_DATA_LEN(flag, msgid, level, data, len) \
	TraceLevelToDbg(level), SendEventWithData, flag, msgid, DbgPrintTraceMsg, \
	WPPTRACEINFO(level,flag), WPPTRACEDATA, data, len, WPPDBGPREFIX, " Data: %!HEXDUMP!\n"

#define WPP_FLAG_MSGID_LEVEL_DATA_LEN_LOGGER(flag, msgid, level, data, len) \
    WPP_LEVEL_LOGGER(flag)

#define WPP_FLAG_MSGID_LEVEL_DATA_LEN_ENABLED(flag, msgid, lvl, data, len) \
    (WPP_LEVEL_ENABLED(flag) && WPP_CONTROL(WPP_BIT_ ## flag).Level >= lvl)

#define WPP_RECORDER_FLAG_MSGID_LEVEL_DATA_LEN_ARGS(flag, msgid, lvl, data, len) \
	WPP_CONTROL(WPP_BIT_ ## flag).AutoLogContext, lvl, WPP_BIT_ ## flag

#define WPP_RECORDER_FLAG_MSGID_LEVEL_DATA_LEN_FILTER(flag, msgid, lvl, data, len) \
	(lvl < TRACE_LEVEL_VERBOSE || WPP_CONTROL(WPP_BIT_ ## flag).AutoLogVerboseEnabled)


/*
 * SendTraceEventsWithData() definitions
 */

#define WPP_EX_FLAGS_MSGID_LEVEL_DATA_LEN(flags, msgid, level, data, len) \
	TraceLevelToDbg(level), SendEventWithData, flags, msgid, DbgPrintTraceMsg, \
	WPPTRACEINFO(level,flags), WPPTRACEDATA, data, len, WPPDBGPREFIX, " Data: %!HEXDUMP!\n"

#define WPP_FLAGS_MSGID_LEVEL_DATA_LEN_LOGGER(flags, msgid, level, data, len) \
    WPP_LEVEL_LOGGER(flags)
               
#define WPP_FLAGS_MSGID_LEVEL_DATA_LEN_ENABLED(flags, msgid, lvl, data, len) \
    (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level >= lvl)

#define WPP_RECORDER_FLAGS_MSGID_LEVEL_DATA_LEN_ARGS(flags, msgid, lvl, data, len) \
	WPP_CONTROL(WPP_BIT_ ## flags).AutoLogContext, lvl, WPP_BIT_ ## flags

#define WPP_RECORDER_FLAGS_MSGID_LEVEL_DATA_LEN_FILTER(flags, msgid, lvl, data, len) \
	(lvl < TRACE_LEVEL_VERBOSE || WPP_CONTROL(WPP_BIT_ ## flags).AutoLogVerboseEnabled)


/*
 * DbgPrint() definitions
 */

#define WPP_EX_DBGFLAG_ID_LEVEL(flag, id, level) \
	level, WPPNULLFUNC, 0, 0, DbgPrintDebugMsg, 0, WPPNULLDATA, 0, 0, "", ""

#define WPP_DBGFLAG_ID_LEVEL_LOGGER(flag, id, level) \
    WPP_LEVEL_LOGGER(flag)

#define WPP_DBGFLAG_ID_LEVEL_ENABLED(flag, id, level) \
    (WPP_LEVEL_ENABLED(flag) && \
     WPP_CONTROL(WPP_BIT_ ## flag).Level >= DbgLevelToTrace(level))

#define WPP_RECORDER_DBGFLAG_ID_LEVEL_ARGS(flag, id, level) \
	WPP_CONTROL(WPP_BIT_ ## flag).AutoLogContext, (UCHAR)DbgLevelToTrace(level), WPP_BIT_ ## flag

#define WPP_RECORDER_DBGFLAG_ID_LEVEL_FILTER(flag, id, level) \
	(DbgLevelToTrace(level) < TRACE_LEVEL_VERBOSE || WPP_CONTROL(WPP_BIT_ ## flag).AutoLogVerboseEnabled)


/*
 * Log Always definitions
 */

#if defined(_DEBUG)
#define WPP_LOG_ALWAYS_TRACE_CALL(level, ProxyMsg, flags, msgid, \
		PrintMsg, info, DataArg, data, len, prefix, tail, fmt, ...) \
	ProxyMsg(flags, msgid, (PVOID)data, (ULONG)len, __VA_ARGS__), \
	PrintMsg(level, info, prefix fmt tail, __VA_ARGS__ DataArg(data, len))
#else
#define WPP_LOG_ALWAYS_TRACE_CALL(level, ProxyMsg, flags, msgid, \
		PrintMsg, info, PrintData, data, len, prefix, tail, fmt, ...) \
	ProxyMsg(flags, msgid, data, len, __VA_ARGS__)
#endif

#define WPP_LOG_ALWAYS_TRACE(x)	WPP_LOG_ALWAYS_TRACE_CALL x
#define WPP_LOG_ALWAYS(EX, ...)	WPP_LOG_ALWAYS_TRACE((EX, __VA_ARGS__)),


/*
 * DbgPrint definitions
 */

#ifndef _DCS_TRACE_WPP_
WPP_CONTROL_GUIDS
#endif


/*
 * WPP custom types
 */

//
// begin_wpp config
// DEFINE_FLAVOR(VDISKID, GUID,,);
// DEFINE_FLAVOR(VDISKU, USTR,,);
// DEFINE_FLAVOR(VDISKW, WSTR,,);
// DEFINE_FLAVOR(VDISK, ASTR,,);
// DEFINE_FLAVOR(LDISKID, GUID,,);
// DEFINE_FLAVOR(LDISKU, USTR,,);
// DEFINE_FLAVOR(LDISKW, WSTR,,);
// DEFINE_FLAVOR(LDISK, ASTR,,);
// DEFINE_FLAVOR(VBLOCK, XINT64,,);
// DEFINE_FLAVOR(VBLOCKLEN, XINT,,);
// DEFINE_FLAVOR(VBLOCKEXT, XINT64,,);
// end_wpp
//

#if defined(WPP_DEFINE_TYPE)

__inline char *CDbgPrintTrace::ExpandCustom(const char *arg, va_list &ap)
{
#if defined(WPP_EXPAND_PROJECT)
	if (ExpandProject(arg, ap))
		return out;
#endif

	switch ((arg[0] << 8) | arg[1]) {
	case 'LD':
		WPP_DEFINE_ALIAS(LDISKID, GUID)
		WPP_DEFINE_ALIAS(LDISKU, USTR)
		WPP_DEFINE_ALIAS(LDISKW, WSTR)
		WPP_DEFINE_ALIAS(LDISK, ASTR)
		break;

	case 'VB':
		WPP_DEFINE_ALIAS(VBLOCK, XINT64)
		WPP_DEFINE_ALIAS(VBLOCKLEN, XINT)
		WPP_DEFINE_ALIAS(VBLOCKEXT, XINT64)
		break;

	case 'VD':
		WPP_DEFINE_ALIAS(VDISKID, GUID)
		WPP_DEFINE_ALIAS(VDISKU, USTR)
		WPP_DEFINE_ALIAS(VDISKW, WSTR)
		WPP_DEFINE_ALIAS(VDISK, ASTR)
		break;
	}

	return NULL;
}

#endif	/* WPP_DEFINE_TYPE */


/*
 * WPP complex types
 */

//
// begin_wpp config
// DEFINE_CPLX_TYPE(HEXDUMP, WPP_LOGHEXDUMP, const TraceData&, ItemHEXDump, "s", _HEX_, 0, 2);
// WPP_FLAGS(-DWPP_LOGHEXDUMP(x) WPP_LOGPAIR(2,&(x).len) WPP_LOGPAIR((x).len, (x).data));
// end_wpp
//


/*
 * Define trace functions
 */

//
// begin_wpp config
// FUNC Trace{FLAG=MYDRIVER_ALL_INFO}(LEVEL, MSG, ...);
// FUNC TraceEvents(LEVEL, FLAGS, MSG, ...);
// FUNC TraceWithData{FLAG=MYDRIVER_ALL_INFO}(LEVEL, DATA, LEN, MSG, ...);
// FUNC TraceEventsWithData(LEVEL, FLAGS, DATA, LEN, MSG, ...);
// FUNC SendTrace{FLAG=DEFAULT_TRACE_LEVEL}(MSGID, LEVEL, MSG, ...);
// FUNC SendTraceEvents(FLAGS, MSGID, LEVEL, MSG, ...);
// FUNC SendTraceWithData{FLAG=DEFAULT_TRACE_LEVEL}(MSGID, LEVEL, DATA, LEN, MSG, ...);
// FUNC SendTraceEventsWithData(FLAGS, MSGID, LEVEL, DATA, LEN, MSG, ...);
// WPP_FLAGS(-public:SendTrace);
// WPP_FLAGS(-public:SendTraceEvents);
// WPP_FLAGS(-public:SendTraceWithData);
// WPP_FLAGS(-public:SendTraceEventsWithData);
// USEPREFIX(SendTrace, "[MsgId 0x%08x]\040 ", (unsigned long)MSGID);
// USEPREFIX(SendTraceEvents, "[MsgId 0x%08x]\040", (unsigned long)MSGID);
// USEPREFIX(SendTraceWithData, "[MsgId 0x%08x]\040", (unsigned long)MSGID);
// USEPREFIX(SendTraceEventsWithData, "[MsgId 0x%08x]\040", (unsigned long)MSGID);
// USESUFFIX(TraceWithData, " Data: %!HEXDUMP!", WPP_LOGDATA(DATA,LEN));
// USESUFFIX(TraceEventsWithData, " Data: %!HEXDUMP!", WPP_LOGDATA(DATA,LEN));
// USESUFFIX(SendTraceWithData, " Data: %!HEXDUMP!", WPP_LOGDATA(DATA,LEN));
// USESUFFIX(SendTraceEventsWithData, " Data: %!HEXDUMP!", WPP_LOGDATA(DATA,LEN));
// FUNC DbgPrint{DBGFLAG=TRACE_KDPRINT, ID=DPFLTR_DEFAULT_ID, LEVEL=DPFLTR_INFO_LEVEL}(MSG, ...);
// FUNC DbgPrintEx{DBGFLAG=TRACE_KDPRINT}(ID, LEVEL, MSG,...);
// FUNC KdPrint{DBGFLAG=TRACE_KDPRINT, ID=DPFLTR_DEFAULT_ID, LEVEL=DPFLTR_INFO_LEVEL}((MSG, ...));
// FUNC KdPrintEx{DBGFLAG=TRACE_KDPRINT}((ID, LEVEL, MSG,...));
// end_wpp
//

#endif /* _DCS_TRACEWPP_H_ */


/*
 * Include WPP pre-processed file
 */

#if defined(_DCS_TRACE_WPP_) && defined(WPPFILE)
#undef KdPrint
#define KdPrint(x)
#undef PAGED_CODE
#define PAGED_CODE()
#undef ALLOC_PRAGMA
extern "C" {
#include WPPFILE
}
#endif

#undef WPPFILE

#ifdef __cplusplus
}
#endif
