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

PUBLIC _DdkWrapper
PUBLIC _DdkWrapperEnd
PUBLIC _DdkWrapperMain

EXTRN DdkGetIntercept:PROC

_TEXT SEGMENT

a$ = 48
b$ = 56
c$ = 64
d$ = 72

_DdkWrapper PROC
        call    QWORD PTR _GetMain
_ReturnAddr::

        ALIGN   8
_GetMain::
        DQ      _DdkWrapperMain
_DdkWrapperEnd::
_GetIntercept::
        DQ      DdkGetIntercept

_DdkWrapperMain::
        pop     rax
        mov     QWORD PTR [rsp+32], r9
        mov     QWORD PTR [rsp+24], r8
        mov     QWORD PTR [rsp+16], rdx
        mov     QWORD PTR [rsp+8], rcx
        sub     rsp, 40                                 ; 00000028H

;;  Call DdkGetIntercept

        mov     rcx, rax
        sub     rcx, _ReturnAddr - _DdkWrapper
        call    QWORD PTR _GetIntercept

        mov     r9, QWORD PTR d$[rsp]
        mov     r8, QWORD PTR c$[rsp]
        mov     rdx, QWORD PTR b$[rsp]
        mov     rcx, QWORD PTR a$[rsp]
        add     rsp, 40                                 ; 00000028H
        jmp     rax  

_DdkWrapper ENDP

_TEXT ENDS

END
 