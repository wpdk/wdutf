/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 2013-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
*	DDK Basic Types.
*/

#ifndef _DDK_NTDEF_DEFS_H_
#define _DDK_NTDEF_DEFS_H_

#ifdef __cplusplus
extern "C" {
#endif


/*
 *	Basic Types (must match DDK)
 */

typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;
typedef ULONG LOGICAL, *PLOGICAL;
typedef short CSHORT;


/*
 *	Time conversion routines (must match DDK)
 */

typedef struct _TIME_FIELDS {
    CSHORT Year;
    CSHORT Month;
    CSHORT Day;
    CSHORT Hour;
    CSHORT Minute;
    CSHORT Second;
    CSHORT Milliseconds;
    CSHORT Weekday;
} TIME_FIELDS, *PTIME_FIELDS;


#ifdef __cplusplus
};
#endif

#endif /* _DDK_NTDEF_DEFS_H_ */
