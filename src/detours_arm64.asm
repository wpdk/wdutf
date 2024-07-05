;;-
;;  SPDX-License-Identifier: BSD-3-Clause
;;
;;  Copyright (c) 2022, DataCore Software Corporation. All rights reserved.
;;
;;  Details about the Windows Kernel API are based on the documentation
;;  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
;;

;; 
;;  Position-independent template wrapper that preserves the arguments and
;;  'this' pointer, calls DdkGetIntercept to identify the appropriate detour
;;  and then executes it.
;;
;;  Derived from:
;;
;;  typedef void *(*fn_t)(void *, void *, void *, void *);
;;  extern fn_t DdkGetIntercept(void *);
;;  fn_t (*get)(void *) = &DdkGetIntercept;
;;
;;  void _DdkWrapper(void *a, void *b, void *c, void *d) {
;;    (*(*get)((void *)0))(a,b,c,d);
;;  }
;;

	AREA	|.drectve|,DRECTVE

	IMPORT	|DdkGetIntercept|
	EXPORT	|_DdkWrapper|
	EXPORT	|_DdkWrapperEnd|
	EXPORT	|_DdkWrapperMain|

	AREA	|.text|,CODE

|_DdkWrapper| PROC
	mov			x9,lr
	adr			x10,_GetMain
	ldr			x10,[x10]
	blr			x10
|_ReturnAddr|

	ALIGN	8
|_GetMain| DCQ _DdkWrapperMain
|_DdkWrapperEnd|

|_GetIntercept|	DCQ	DdkGetIntercept

|_DdkWrapperMain|
	stp			x0,x1,[sp,#-0x60]!
	stp			x2,x3,[sp,#0x10]
	stp			x4,x5,[sp,#0x20]
	stp			x6,x7,[sp,#0x30]
	stp         x8,x16,[sp,#0x40]
	stp			x17,x9,[sp,#0x50]

;;  Call DdkGetIntercept

	sub			x0,lr,_ReturnAddr-_DdkWrapper
	adr			x9,_GetIntercept
	ldr			x9,[x9]
	blr			x9
	mov			x9,x0

	ldp			x17,lr,[sp,#0x50]
	ldp			x8,x16,[sp,#0x40]
	ldp			x6,x7,[sp,#0x30]
	ldp			x4,x5,[sp,#0x20]
	ldp			x2,x3,[sp,#0x10]
	ldp			x0,x1,[sp],#0x60
	br			x9	

	ENDP	; |_DdkWrapper|
	END
 