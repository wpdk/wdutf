/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 */

/*
 * Trace definitions.
 */

#ifndef _BASIC_TRACE_H_
#define _BASIC_TRACE_H_

/*
 * Define Trace constants
 */

#define WPPNAME		DcsBasic
#define WPPGUID		4DC7A0CC,3C78,45A6,91B4,99690CB74954

#include <TraceDefs.h>

/*
 * Define Trace Flags
 */

#define WPP_CONTROL_GUIDS                                              \
    WPP_DEFINE_CONTROL_GUID(                                           \
        WPPNAME, (WPPGUID),		                                       \
        WPP_DEFINE_DEFAULT_BITS                                        \
        )

// WPP is always initialised.
#define WPP_CHECK_INIT

#endif /* _BASIC_TRACE_H_ */

#include <TraceWpp.h>
