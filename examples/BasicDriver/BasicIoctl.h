/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 2024, rtegrity ltd. All rights reserved.
 */

 /*
  * Basic Driver definitions.
  */

#ifndef _BASIC_IOCTL_H_
#define _BASIC_IOCTL_H_

/*
 * Define IOCTL constants
 */

#define BASIC_DEVICE_TYPE	0x9f4f
#define BASIC_MAKE_IOCTL(c) \
        (ULONG)CTL_CODE(BASIC_DEVICE_TYPE, 0x800+(c), METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_BASIC_TEST_FUNCTION	    BASIC_MAKE_IOCTL(10)
#define IOCTL_BASIC_INVALID     	    BASIC_MAKE_IOCTL(100)

#endif /* _BASIC_IOCTL_H_ */