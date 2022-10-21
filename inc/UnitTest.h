/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2015, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 *	CPP Unit Test Definitions
 */

#ifndef _UNITTEST_H_
#define _UNITTEST_H_

#include <stdarg.h>
#include <winnt.h>

#include <CppUnitTest.h>
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


/*
 *	TEST_MODULE_START(cleanupMethod)
 *		Start of module tests. If Visual Studio starts a new set
 *		of tests on the same thread, cleanup will be called first.
 *
 *	TEST_MODULE_END(cleanupMethod)
 *		End of module tests. If cleanup has already been called
 *		the macro will return.
 */

#define TEST_MODULE_START(cleanupMethod) \
	extern void CALLING_CONVENTION cleanupMethod(); \
	DdkModuleStart(#cleanupMethod, cleanupMethod)

#define TEST_MODULE_END() \
	if (!DdkModuleEnd(__FUNCTION__)) return


/*
 *	TEST_METHOD_ASYNC(methodName)
 *		Define a method that can run asychronously during a test.
 *		The method can use TEST_IS_ASYNC to determine it has been
 *		invoked asynchronously.
 *
 *	TEST_ASYNC_START(instanceId, methodName)
 *		Start the asynchronous method and wait until it is running.
 *
 *	TEST_ASYNC_INIT(instanceId, methodName)
 *		Start the asynchronous method and wait until it is ready.
 *		TEST_ASYNC_RUN must be called to allow it to execute.
 *
 *	TEST_ASYNC_RUN(instanceId)
 *		Allow the asynchronous instance to start executing.
 *
 *	TEST_ASYNC_TIMEOUT(instanceId, secs)
 *		Change the timeout value to use when waiting for the asynchronous
 *		instance to complete.
 *
 *	TEST_ASYNC_WAIT(instanceId)
 *		Wait for the asynchronous method to complete and report any
 *		errors. A wait will automatically occur as soon as instanceId
 *		goes out of scope.
 */

#define TEST_METHOD_ASYNC(methodName) \
	static void _##methodName(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work) { \
		((CUnitTestAsync *)Context)->Invoke(&__##methodName); } \
	void __##methodName(bool async) { CrtHandlersSetter setter; methodName(async); } \
	void methodName(bool TEST_IS_ASYNC = false)

#define TEST_ASYNC_START(instanceId, methodName) \
	CUnitTestAsync instanceId(this, _##methodName, true)

#define TEST_ASYNC_INIT(instanceId, methodName) \
	CUnitTestAsync instanceId(this, _##methodName, false)

#define TEST_ASYNC_RUN(instanceId) instanceId.Run()
#define TEST_ASYNC_TIMEOUT(instanceId, secs) instanceId.Timeout(secs)
#define TEST_ASYNC_WAIT(instanceId) instanceId.Wait()


/*
 *	TEST_ASYNC_INIT_VEC(instanceIdVec, methodName, instanceCount)
 *		Prepare for a set of asynchronous methods. Each individual
 *		method is identified with instanceIdVec[i].
 *
 *	TEST_ASYNC_START_VEC(instanceIdVec, methodName, instanceCount)
 *		Prepare for a set of asynchronous methods. Each individual
 *		method is identified with instanceIdVec[i].
 *
 *	TEST_ASYNC_TIMEOUT_VEC(instanceId, secs)
 *		Change the timeout value to use when waiting for the set of
 *		asynchronous methods to complete.
 *
 *	TEST_ASYNC_WAIT_VEC(instanceId)
 *		Wait for the set of asynchronous methods to complete.
 */

#define TEST_ASYNC_START_VEC(instanceIdVec, methodName, instanceCount) \
	CUnitTestAsync instanceIdVec[(instanceCount)]; \
	for (int i = 0; i < (instanceCount); i++) instanceIdVec[i].InitAsync(this, _##methodName, false); \
	for (int i = 0; i < (instanceCount); i++) instanceIdVec[i].Run()

#define TEST_ASYNC_INIT_VEC(instanceIdVec, methodName, instanceCount) \
	CUnitTestAsync instanceIdVec[(instanceCount)]; \
	for (int i = 0; i < (instanceCount); i++) instanceIdVec[i].InitAsync(this, _##methodName, false)

#define TEST_ASYNC_TIMEOUT_VEC(instanceIdVec, secs) \
	for (int i = 0; i < (sizeof(instanceIdVec) / sizeof(CUnitTestAsync)); i++) instanceIdVec[i].Timeout(secs)

#define TEST_ASYNC_WAIT_VEC(instanceIdVec) \
	for (int i = 0; i < (sizeof(instanceIdVec) / sizeof(CUnitTestAsync)); i++) instanceIdVec[i].Wait()


/*
 *	TEST_METHOD_CALLBACK(methodName, ...)
 *		Define a method to be used as a callback function.
 *		There must be a Context argument which is the instanceId
 *		of the current callback. The remaining arguments are not
 *		passed to the callback method.
 *
 *	TEST_METHOD_CMD_CALLBACK(methodName, CStorageCmd *pCmd)
 *		Define a method to be used as a CStorageCmd callback.
 *
 *	TEST_METHOD_IRP_CALLBACK(methodName, IRP *pIrp)
 *		Define a method to be used as an IRP callback.
 *
 *	TEST_CALLBACK_INIT(instanceId)
 *		Prepare for an asynchronous callback.
 *
 *	TEST_CALLBACK_STARTED(instanceId)
 *		Notification that the callback has been started and should be
 *		waited for. Notification is optional if TEST_CALLBACK_WAIT is
 *		the next call.
 *
 *	TEST_CALLBACK_CANCELLED(instanceId)
 *		Notification that the callback has been cancelled. It is
 *		important to ensure that the callback cannot subsequently occur.
 *
 *	TEST_CALLBACK_TIMEOUT(instanceId, secs)
 *		Change the timeout value to use when waiting for the callback
 *		to complete.
 *
 *	TEST_CALLBACK_WAIT(instanceId)
 *		Wait for the callback to complete. Any errors will be reported.
 *		A wait will automatically occur as soon as instanceId goes
 *		out of scope. For a recurring callback, multiple calls to
 *		TEST_ASYNC_WAIT can be made. However, it is important to ensure
 *		that all callbacks have completed before instanceId goes out of
 *		scope.
 */

#define TEST_METHOD_CALLBACK(methodName, ...) \
	static void methodName(__VA_ARGS__) { \
		((CUnitTestAsync *)Context)->Invoke(&_##methodName); } \
	void _##methodName(bool async) { CrtHandlersSetter setter; __##methodName(async); } \
	void __##methodName(bool)

#define TEST_METHOD_CMD_CALLBACK(methodName, ...) \
	static void methodName(CStorageCmd *pCmd) { \
		((CUnitTestAsync *)(pCmd->getExtension()))->Invoke(&_##methodName, pCmd); } \
	void _##methodName(CStorageCmd *pCmd) { CrtHandlersSetter setter; __##methodName(pCmd); } \
	void __##methodName(__VA_ARGS__)

#define TEST_METHOD_IRP_CALLBACK(methodName, ...) \
	static NTSTATUS methodName(DEVICE_OBJECT *, IRP *pIrp, PVOID Context) { \
		((CUnitTestAsync *)Context)->Invoke(&_##methodName, pIrp); \
		return STATUS_MORE_PROCESSING_REQUIRED; } \
	void _##methodName(IRP *pIrp) { CrtHandlersSetter setter; __##methodName(pIrp); } \
	void __##methodName(__VA_ARGS__)

#define TEST_CALLBACK_INIT(instanceId) CUnitTestAsync instanceId(this)
#define TEST_CALLBACK_STARTED(instanceId) instanceId.Started()
#define TEST_CALLBACK_CANCELLED(instanceId) instanceId.Cancelled()
#define TEST_CALLBACK_TIMEOUT(instanceId, secs) instanceId.Timeout(secs)
#define TEST_CALLBACK_WAIT(instanceId) (instanceId.Started(), instanceId.Wait())


/*
 *	TEST_CALLBACK_INIT_VEC(instanceIdVec, instanceCount)
 *		Prepare for a set of asynchronous callbacks. Each individual
 *		callback is identified with instanceIdVec[i]. Note that
 *		TEST_CALLBACK_STARTED must be called individually for each
 *		instanceId once it has been started.
 *
 *	TEST_CALLBACK_TIMEOUT_VEC(instanceId, secs)
 *		Change the timeout value to use when waiting for the set of
 *		callbacks to complete.
 *
 *	TEST_CALLBACK_WAIT_VEC(instanceId, instanceCount)
 *		Wait for the set of callbacks to complete.
 */

#define TEST_CALLBACK_INIT_VEC(instanceIdVec, instanceCount) \
	CUnitTestAsync instanceIdVec[(instanceCount)]; \
	for (int i = 0; i < (instanceCount); i++) instanceIdVec[i].SetTest(this)

#define TEST_CALLBACK_TIMEOUT_VEC(instanceIdVec, secs) \
	for (int i = 0; i < (sizeof(instanceIdVec) / sizeof(CUnitTestAsync)); i++) instanceIdVec[i].Timeout(secs)

#define TEST_CALLBACK_WAIT_VEC(instanceIdVec) \
	for (int i = 0; i < (sizeof(instanceIdVec) / sizeof(CUnitTestAsync)); i++) instanceIdVec[i].Wait()


/*
 *	CUnitTestAsync
 */

class CUnitTestAsync {
	PVOID			pTest;					// Test object
	TP_WORK			*pWork;					// Work Item
	HANDLE			ready;					// Ready
	HANDLE			run;					// Run
	HANDLE			done;					// Done
	DWORD			started;				// Start time
	DWORD			timeout;				// Timeout in msecs
	volatile LONG	failed;					// Failed
	volatile LONG	waitdone;				// Wait done
	wchar_t			*pMessage;				// Message
	__LineInfo		*pLine;					// Line info
	__LineInfo		line;					// LineInfo Copy

	void Init(PVOID test, bool runflag) {
		pTest = test;
		pWork = 0;
		pLine = 0;
		started = 0;
		timeout = 10 * 1000;
		ready = run = done;
		failed = waitdone = 0;
		pMessage = L"Failure in Async Test or Callback";
		ready = CreateEvent(NULL, TRUE, FALSE, NULL);
		run = CreateEvent(NULL, TRUE, (runflag) ? TRUE : FALSE, NULL);
		done = CreateEvent(NULL, FALSE, FALSE, NULL);
		Assert::IsTrue(ready && run && done);
	}

	bool StartInvoke(volatile int &once) {
		if (once++) return false;
		DdkThreadInit();
		SetEvent(ready);
		InterlockedExchange(&waitdone, 0);
		WaitForSingleObject(run, INFINITE);
		return true;
	}

public:
	void InitAsync(PVOID test, PTP_WORK_CALLBACK func, bool runflag) {
		Init(test, runflag);
		pWork = CreateThreadpoolWork(func, this, NULL);
		Assert::IsNotNull(pWork);
		started = GetTickCount();
		SubmitThreadpoolWork(pWork);
		WaitForSingleObject(ready, INFINITE);
	}

	CUnitTestAsync() : line(0,0,0) { Init(0, true); }
	CUnitTestAsync(PVOID test) : line(0,0,0) { Init(test, true); }

	CUnitTestAsync(PVOID test, PTP_WORK_CALLBACK func, bool runflag)
		: line(0,0,0) { InitAsync(test, func, runflag); }
	
	~CUnitTestAsync() {
		WaitForCompletion();
		if (pWork) CloseThreadpoolWork(pWork);
		if (ready) CloseHandle(ready);
		if (run) CloseHandle(run);
		if (done) CloseHandle(done);
	}

	template <class T>
	void Invoke(void (T::*fn)(bool)) {
		volatile int once = 0;
		CONTEXT ctx;
		RtlCaptureContext(&ctx);
		if (StartInvoke(once)) {
			__try { ((T *)pTest->*fn)(true); }
			__except (Exception(GetExceptionInformation())) { RtlRestoreContext(&ctx, NULL); }
		}
		SetEvent(done);
	}

	template <class T, class C>
	void Invoke(void (T::*fn)(C *), C *c) {
		volatile int once = 0;
		CONTEXT ctx;
		RtlCaptureContext(&ctx);
		if (StartInvoke(once)) {
			__try { ((T *)pTest->*fn)(c); }
			__except (Exception(GetExceptionInformation())) { RtlRestoreContext(&ctx, NULL); }
		}
		SetEvent(done);
	}

	void SetTest(PVOID test) { pTest = test; }

	template <class T>
	T *GetTest(void (T::*fn)(bool)) { return (T *)pTest; }

	void Run() { SetEvent(run); }
	void Started() { started = GetTickCount(); }
	void Cancelled() { SetEvent(done); }
	void Timeout(int secs) { timeout = secs * 1000; }

	void Wait() {
		InterlockedExchange(&waitdone, 0);
		WaitForCompletion();
	}

	void WaitForCompletion() {
		if (started && !waitdone && done) {
			DWORD running = GetTickCount() - started;
			DWORD delay = (running < timeout) ? (timeout - running) : 0;
			DWORD rc = WaitForSingleObject(done, (IsDebuggerPresent() ? INFINITE : delay));
			InterlockedExchange(&waitdone, 1);
			if (rc == WAIT_TIMEOUT) {
				pWork = NULL;
				if (!__uncaught_exception())
					Assert::Fail(L"Timeout waiting for Async Test or Callback");
			}
		}
		if (pWork) WaitForThreadpoolWorkCallbacks(pWork, FALSE);
		if (InterlockedExchange(&failed, 0) && !__uncaught_exception())
			Assert::Fail(pMessage, pLine);
	}

	operator PVOID() { return this; }
	operator ULONG_PTR() { return (ULONG_PTR)this; }

private:
	int Exception(EXCEPTION_POINTERS *xp) {
		if (xp->ExceptionRecord->ExceptionCode == ERROR_ASSERT_FAILED
				&& xp->ExceptionRecord->NumberParameters == 2) {
			if (xp->ExceptionRecord->ExceptionInformation[0])
				pMessage = (wchar_t *)xp->ExceptionRecord->ExceptionInformation[0];
			if (xp->ExceptionRecord->ExceptionInformation[1]) {
				line = *(__LineInfo *)xp->ExceptionRecord->ExceptionInformation[1];
				pLine = &line;
			}
			InterlockedExchange(&failed, 1);
			return EXCEPTION_EXECUTE_HANDLER;
		}

		SetEvent(done);
		return EXCEPTION_CONTINUE_SEARCH;
	}
};


/*
 *	TEST_EXPECT_ASSERT(exp)
 *		Evaluate the expression and expect an assertion to occur.
 */

#define TEST_EXPECT_ASSERT(exp) { \
	bool _ok_ = false; __try { exp; } \
	__except(GetExceptionCode() == 0xe3530001 ? EXCEPTION_EXECUTE_HANDLER : \
		EXCEPTION_CONTINUE_SEARCH) { _ok_ = true; } \
	Assert::IsTrue(_ok_, L"Expected failure did not occur"); \
}


/*
 *	Logger::PrintMessage
 *		Write a printf formatted string to the test logger.
 */

class CUnitTestLogger : public Logger {
	static void Print(char *fmt, va_list ap) {
		char msg[1024];
		vsnprintf(msg, sizeof(msg)-1, fmt, ap);
		msg[sizeof(msg)-1] = 0;
		Logger::WriteMessage(msg);
	}

public:
	static void PrintMessage(char *fmt, ...) {
		va_list ap;
		va_start(ap, fmt);
		Print(fmt, ap);
		va_end(ap);
	}

	static void PrintVerbose(bool verbose, char *fmt, ...) {
		if (verbose) {
			va_list ap;
			va_start(ap, fmt);
			Print(fmt, ap);
			va_end(ap);
		}
	}
};

#define Logger CUnitTestLogger


#if _MSC_VER < 1920
/*
 *	Add specializations of basic types for equality asserts in test code.
 */

namespace Microsoft { namespace VisualStudio { namespace CppUnitTestFramework 
{
	template<> static std::wstring ToString<long long>    (const long long& t)    { RETURN_WIDE_STRING(t); }
	template<> static std::wstring ToString<long long>    (const long long* t)    { RETURN_WIDE_STRING(t); }
	template<> static std::wstring ToString<long long>    (long long* t)          { RETURN_WIDE_STRING(t); }
}}}
#endif


#endif /* _UNITTEST_H_ */
