/*-
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  Copyright (c) 1998-2017, DataCore Software Corporation. All rights reserved.
 *
 *  Details about the Windows Kernel API are based on the documentation
 *  available at https://learn.microsoft.com/en-us/windows-hardware/drivers/
 */

/*
 * DDK routines for user level testing.
 */

#include "stdddk.h"

extern "C" {
#include "tdi.h"
#include "tdikrnl.h"
#include "ws2def.h"
}
#include "netioapi.h"


DDKAPI
NTSTATUS WppAutoLogTrace(PVOID AutoLogContext, UCHAR MessageLevel,
	ULONG MessageFlags, LPGUID MessageGuid, USHORT MessageNumber, ...)
{
	UNREFERENCED_PARAMETER(AutoLogContext);
	UNREFERENCED_PARAMETER(MessageLevel);
	UNREFERENCED_PARAMETER(MessageFlags);
	UNREFERENCED_PARAMETER(MessageGuid);
	UNREFERENCED_PARAMETER(MessageNumber);
	return STATUS_SUCCESS;
}


union WPP_CB_TYPE;

DDKAPI
VOID WppAutoLogStart(WPP_CB_TYPE *WppCb, PDRIVER_OBJECT DrvObj, PCUNICODE_STRING RegPath)
{
	UNREFERENCED_PARAMETER(WppCb);
	UNREFERENCED_PARAMETER(DrvObj);
	UNREFERENCED_PARAMETER(RegPath);
}


DDKAPI
VOID WppAutoLogStop(WPP_CB_TYPE *WppCb, PDRIVER_OBJECT DrvObj)
{
	UNREFERENCED_PARAMETER(WppCb);
	UNREFERENCED_PARAMETER(DrvObj);
}


DDKAPI
NTSTATUS DbgQueryDebugFilterState(ULONG ComponentId, ULONG Level)
{
	UNREFERENCED_PARAMETER(ComponentId);
	UNREFERENCED_PARAMETER(Level);
	return FALSE;
}


DDKAPI
USHORT KeQueryNodeMaximumProcessorCount (USHORT NodeNumber)
{
	UNREFERENCED_PARAMETER(NodeNumber);
	return 64;
}


DDKAPI
PVOID MmMapLockedPagesWithReservedMapping (PVOID MappingAddress,
    ULONG PoolTag, PMDL MemoryDescriptorList, MEMORY_CACHING_TYPE CacheType)
{
	UNREFERENCED_PARAMETER(MappingAddress);
	UNREFERENCED_PARAMETER(PoolTag);
	UNREFERENCED_PARAMETER(MemoryDescriptorList);
	UNREFERENCED_PARAMETER(CacheType);
	return MappingAddress;
}


DDKAPI
VOID MmUnmapReservedMapping (PVOID BaseAddress, ULONG PoolTag, PMDL MemoryDescriptorList)
{
	UNREFERENCED_PARAMETER(BaseAddress);
	UNREFERENCED_PARAMETER(PoolTag);
	UNREFERENCED_PARAMETER(MemoryDescriptorList);

}


DDKAPI
PVOID MmAllocateMappingAddress(SIZE_T NumberOfBytes, ULONG PoolTag)
{
	return ExAllocatePoolWithTag(NonPagedPool, NumberOfBytes, PoolTag);
}


DDKAPI
VOID MmFreeMappingAddress(PVOID BaseAddress, ULONG PoolTag)
{
	ExFreePoolWithTag(BaseAddress, PoolTag);
}


DDKAPI
USHORT KeQueryActiveGroupCount()
{
	return 1;
}


DDKAPI
KAFFINITY KeQueryGroupAffinity(USHORT GroupNumber)
{
	UNREFERENCED_PARAMETER(GroupNumber);
	return 0xf;
}


DDKAPI
ULONG KeGetCurrentProcessorNumberEx(PPROCESSOR_NUMBER ProcNumber)
{
	// TODO - need a cpuid that is constant until the thread suspends
	UNREFERENCED_PARAMETER(ProcNumber);
	return 0;
}


DDKAPI
ULONG KeQueryActiveProcessorCountEx(USHORT GroupNumber)
{
	UNREFERENCED_PARAMETER(GroupNumber);
	return 16;
}


DDKAPI
VOID ExQueueWorkItem(PWORK_QUEUE_ITEM WorkItem, WORK_QUEUE_TYPE QueueType)
{
	UNREFERENCED_PARAMETER(WorkItem);
	UNREFERENCED_PARAMETER(QueueType);
}


NTSTATUS TdiRegisterPnPHandlers(PTDI_CLIENT_INTERFACE_INFO ClientInterfaceInfo,
    ULONG InterfaceInfoSize, HANDLE *BindingHandle)
{
	UNREFERENCED_PARAMETER(ClientInterfaceInfo);
	UNREFERENCED_PARAMETER(InterfaceInfoSize);
	UNREFERENCED_PARAMETER(BindingHandle);
	return 0;
}


NTSTATUS TdiDeregisterPnPHandlers(HANDLE BindingHandle)
{
	UNREFERENCED_PARAMETER(BindingHandle);
	return 0;
}


struct { PVOID id; PVOID extn; } ExtensionList[10];


DDKAPI
NTSTATUS IoAllocateDriverObjectExtension(PDRIVER_OBJECT DriverObject,
    PVOID ClientIdentificationAddress, ULONG DriverObjectExtensionSize, PVOID *DriverObjectExtension)
{
	*DriverObjectExtension = ExAllocatePool(NonPagedPool, DriverObjectExtensionSize);

	if (!*DriverObjectExtension)
		return STATUS_INSUFFICIENT_RESOURCES;

	for (int i = 0; i < 10; i++)
		if (!ExtensionList[i].id || ExtensionList[i].id == ClientIdentificationAddress) {
			ExtensionList[i].id = ClientIdentificationAddress;
			ExtensionList[i].extn = *DriverObjectExtension;
			return STATUS_SUCCESS;
		}

	return STATUS_UNSUCCESSFUL;
}


DDKAPI
PVOID IoGetDriverObjectExtension(PDRIVER_OBJECT DriverObject, PVOID ClientIdentificationAddress)
{
	for (int i = 0; i < 10; i++)
		if (ExtensionList[i].id == ClientIdentificationAddress)
			return ExtensionList[i].extn;

	return 0;
}


DDKAPI
VOID IoReuseIrp(PIRP Irp, NTSTATUS Iostatus)
{
	UNREFERENCED_PARAMETER(Irp);
	UNREFERENCED_PARAMETER(Iostatus);
}


DDKAPI
NTSTATUS ZwMakeTemporaryObject(HANDLE Handle)
{
	UNREFERENCED_PARAMETER(Handle);
	return 0;
}


DDKAPI
VOID KeStallExecutionProcessor(ULONG MicroSeconds)
{
	UNREFERENCED_PARAMETER(MicroSeconds);
	return;
}


NTSTATUS PcwRegister(PPCW_REGISTRATION *Registration, PPCW_REGISTRATION_INFORMATION Info)
{
	UNREFERENCED_PARAMETER(Registration);
	UNREFERENCED_PARAMETER(Info);
	return STATUS_SUCCESS;
}


VOID PcwUnregister(PPCW_REGISTRATION Registration)
{
	UNREFERENCED_PARAMETER(Registration);
}


NTSTATUS PcwCreateInstance(PPCW_INSTANCE *Instance,
	PPCW_REGISTRATION Registration, PCUNICODE_STRING Name, ULONG Count, PPCW_DATA Data)
{
	UNREFERENCED_PARAMETER(Instance);
	UNREFERENCED_PARAMETER(Registration);
	UNREFERENCED_PARAMETER(Name);
	UNREFERENCED_PARAMETER(Count);
	UNREFERENCED_PARAMETER(Data);
	return STATUS_NOT_SUPPORTED;
}


VOID PcwCloseInstance(PPCW_INSTANCE Instance)
{
	UNREFERENCED_PARAMETER(Instance);
}


NTSTATUS PcwAddInstance(PPCW_BUFFER Buffer,
	PCUNICODE_STRING Name, ULONG Id, ULONG Count, PPCW_DATA Data)
{
	UNREFERENCED_PARAMETER(Buffer);
	UNREFERENCED_PARAMETER(Name);
	UNREFERENCED_PARAMETER(Id);
	UNREFERENCED_PARAMETER(Count);
	UNREFERENCED_PARAMETER(Data);
	return STATUS_NOT_SUPPORTED;
}


DDKAPI
VOID MmBuildMdlForNonPagedPool(PMDL MemoryDescriptorList)
{
	UNREFERENCED_PARAMETER(MemoryDescriptorList);
}


DDKAPI
NTSTATUS IoRegisterLastChanceShutdownNotification(PDEVICE_OBJECT DeviceObject)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	return STATUS_SUCCESS;
}


DDKAPI
POWER_STATE PoSetPowerState(PDEVICE_OBJECT DeviceObject, POWER_STATE_TYPE Type, POWER_STATE State)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Type);
	UNREFERENCED_PARAMETER(State);

	POWER_STATE state;
	state.SystemState = PowerSystemUnspecified;
	return state;
}


DDKAPI
VOID ExAcquireFastMutex(PFAST_MUTEX FastMutex)
{
	UNREFERENCED_PARAMETER(FastMutex);
}


DDKAPI
VOID ExReleaseFastMutex(PFAST_MUTEX FastMutex)
{
	UNREFERENCED_PARAMETER(FastMutex);
}


DDKAPI
BOOLEAN ExTryToAcquireFastMutex(PFAST_MUTEX FastMutex)
{
	UNREFERENCED_PARAMETER(FastMutex);
	return FALSE;
}


DDKAPI
BOOLEAN IoIs32bitProcess(PIRP Irp)
{
	UNREFERENCED_PARAMETER(Irp);
	return FALSE;
}


DDKAPI
ULONG IoWMIDeviceObjectToProviderId(PDEVICE_OBJECT DeviceObject)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	return 0;
}


DDKAPI
ULONG ExSetTimerResolution(ULONG DesiredTime, BOOLEAN SetResolution)
{
	UNREFERENCED_PARAMETER(DesiredTime);
	UNREFERENCED_PARAMETER(SetResolution);
	return 0;
}


DDKAPI
VOID IoAcquireCancelSpinLock(PKIRQL Irql)
{
	UNREFERENCED_PARAMETER(Irql);
}


DDKAPI
VOID IoReleaseCancelSpinLock(KIRQL Irql)
{
	UNREFERENCED_PARAMETER(Irql);
}


DDKAPI
LARGE_INTEGER KeQueryPerformanceCounter(PLARGE_INTEGER PerformanceFrequency)
{
	UNREFERENCED_PARAMETER(PerformanceFrequency);

	LARGE_INTEGER v;
	v.QuadPart = 0;
	return v;
}


DDKAPI
NTSTATUS ZwCreateSection(PHANDLE SectionHandle, ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes, PLARGE_INTEGER MaximumSize, ULONG SectionPageProtection,
    ULONG AllocationAttributes, HANDLE FileHandle)
{
	UNREFERENCED_PARAMETER(SectionHandle);
	UNREFERENCED_PARAMETER(DesiredAccess);
	UNREFERENCED_PARAMETER(ObjectAttributes);
	UNREFERENCED_PARAMETER(MaximumSize);
	UNREFERENCED_PARAMETER(SectionPageProtection);
	UNREFERENCED_PARAMETER(AllocationAttributes);
	UNREFERENCED_PARAMETER(FileHandle);
	return STATUS_OBJECT_NAME_NOT_FOUND;
}


DDKAPI
NTSTATUS ZwOpenSection(PHANDLE SectionHandle,
	ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes)
{
	UNREFERENCED_PARAMETER(SectionHandle);
	UNREFERENCED_PARAMETER(DesiredAccess);
	UNREFERENCED_PARAMETER(ObjectAttributes);
	return STATUS_OBJECT_NAME_NOT_FOUND;
}


DDKAPI
NTSTATUS ZwMapViewOfSection(HANDLE SectionHandle, HANDLE ProcessHandle,
    PVOID *BaseAddress, ULONG_PTR ZeroBits, SIZE_T CommitSize, PLARGE_INTEGER SectionOffset,
    PSIZE_T ViewSize, SECTION_INHERIT InheritDisposition, ULONG AllocationType, ULONG Win32Protect)
{
	UNREFERENCED_PARAMETER(SectionHandle);
	UNREFERENCED_PARAMETER(ProcessHandle);
	UNREFERENCED_PARAMETER(BaseAddress);
	UNREFERENCED_PARAMETER(ZeroBits);
	UNREFERENCED_PARAMETER(CommitSize);
	UNREFERENCED_PARAMETER(SectionOffset);
	UNREFERENCED_PARAMETER(ViewSize);
	UNREFERENCED_PARAMETER(InheritDisposition);
	UNREFERENCED_PARAMETER(AllocationType);
	UNREFERENCED_PARAMETER(Win32Protect);
	return STATUS_OBJECT_NAME_NOT_FOUND;
}


DDKAPI
NTSTATUS ZwUnmapViewOfSection(HANDLE ProcessHandle, PVOID BaseAddress)
{
	UNREFERENCED_PARAMETER(ProcessHandle);
	UNREFERENCED_PARAMETER(BaseAddress);
	return STATUS_OBJECT_NAME_NOT_FOUND;
}


DDKAPI
NTSTATUS ZwCreateDirectoryObject(PHANDLE DirectoryHandle,
	ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes)
{
	UNREFERENCED_PARAMETER(DirectoryHandle);
	UNREFERENCED_PARAMETER(DesiredAccess);
	UNREFERENCED_PARAMETER(ObjectAttributes);
	return STATUS_OBJECT_NAME_NOT_FOUND;
}


DDKAPI
VOID IoInvalidateDeviceState(PDEVICE_OBJECT PhysicalDeviceObject)
{
	UNREFERENCED_PARAMETER(PhysicalDeviceObject);
}


DDKAPI
VOID IoInvalidateDeviceRelations(PDEVICE_OBJECT DeviceObject, DEVICE_RELATION_TYPE Type)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Type);
}


DDKAPI
VOID IoDisconnectInterrupt(PKINTERRUPT InterruptObject)
{
	UNREFERENCED_PARAMETER(InterruptObject);
}


DDKAPI
PIRP IoBuildSynchronousFsdRequest(ULONG MajorFunction,
    PDEVICE_OBJECT DeviceObject, PVOID Buffer, ULONG Length,
	PLARGE_INTEGER StartingOffset, PKEVENT Event, PIO_STATUS_BLOCK IoStatusBlock)
{
	UNREFERENCED_PARAMETER(MajorFunction);
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Buffer);
	UNREFERENCED_PARAMETER(Length);
	UNREFERENCED_PARAMETER(StartingOffset);
	UNREFERENCED_PARAMETER(Event);
	UNREFERENCED_PARAMETER(IoStatusBlock);
	return 0;
}


DDKAPI
NTSTATUS PoCallDriver(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Irp);
	return STATUS_SUCCESS;
}


DDKAPI
NTSTATUS IoRegisterShutdownNotification(PDEVICE_OBJECT DeviceObject)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	return STATUS_SUCCESS;
}


DDKAPI
VOID IoUnregisterShutdownNotification(PDEVICE_OBJECT DeviceObject)
{
	UNREFERENCED_PARAMETER(DeviceObject);
}


DDKAPI
struct _DMA_ADAPTER *IoGetDmaAdapter(PDEVICE_OBJECT PhysicalDeviceObject,
    struct _DEVICE_DESCRIPTION *DeviceDescription,PULONG NumberOfMapRegisters)
{
	UNREFERENCED_PARAMETER(PhysicalDeviceObject);
	UNREFERENCED_PARAMETER(DeviceDescription);
	UNREFERENCED_PARAMETER(NumberOfMapRegisters);
	return 0;
}


DDKAPI
NTSTATUS IoWMIRegistrationControl(PDEVICE_OBJECT DeviceObject, ULONG Action)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Action);
	return STATUS_SUCCESS;
}


DDKAPI
NTSTATUS IoWMIWriteEvent(PVOID WnodeEventItem)
{
	UNREFERENCED_PARAMETER(WnodeEventItem);
	return STATUS_SUCCESS;
}


DDKAPI
VOID PoStartNextPowerIrp(PIRP Irp)
{
	UNREFERENCED_PARAMETER(Irp);
}


DDKAPI
VOID RtlAssert(PVOID VoidFailedAssertion, PVOID VoidFileName, ULONG LineNumber, PSTR MutableMessage)
{
	UNREFERENCED_PARAMETER(VoidFailedAssertion);
	UNREFERENCED_PARAMETER(VoidFileName);
	UNREFERENCED_PARAMETER(LineNumber);
	UNREFERENCED_PARAMETER(MutableMessage);
}


DDKAPI
NTSTATUS IoRegisterPlugPlayNotification(IO_NOTIFICATION_EVENT_CATEGORY EventCategory,
    ULONG EventCategoryFlags, PVOID EventCategoryData, PDRIVER_OBJECT DriverObject,
    PDRIVER_NOTIFICATION_CALLBACK_ROUTINE CallbackRoutine, PVOID Context, PVOID *NotificationEntry)
{
	UNREFERENCED_PARAMETER(EventCategory);
	UNREFERENCED_PARAMETER(EventCategoryFlags);
	UNREFERENCED_PARAMETER(EventCategoryData);
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(CallbackRoutine);
	UNREFERENCED_PARAMETER(Context);
	UNREFERENCED_PARAMETER(NotificationEntry);
	return STATUS_SUCCESS;
}


DDKAPI
NTSTATUS IoReportTargetDeviceChange(PDEVICE_OBJECT PhysicalDeviceObject, PVOID NotificationStructure)
{
	UNREFERENCED_PARAMETER(PhysicalDeviceObject);
	UNREFERENCED_PARAMETER(NotificationStructure);
	return STATUS_SUCCESS;
}


DDKAPI
NTSTATUS IoUnregisterPlugPlayNotification(PVOID NotificationEntry)
{
	UNREFERENCED_PARAMETER(NotificationEntry);
	return STATUS_SUCCESS;
}


DDKAPI
NTSTATUS IoUnregisterPlugPlayNotificationEx(PVOID NotificationEntry)
{
	UNREFERENCED_PARAMETER(NotificationEntry);
	return STATUS_SUCCESS;
}


DDKAPI
NTSTATUS ZwOpenSymbolicLinkObject(PHANDLE LinkHandle,
	ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes)
{
	UNREFERENCED_PARAMETER(LinkHandle);
	UNREFERENCED_PARAMETER(DesiredAccess);
	UNREFERENCED_PARAMETER(ObjectAttributes);
	return STATUS_OBJECT_NAME_NOT_FOUND;
}


DDKAPI
NTSTATUS ZwQuerySymbolicLinkObject(HANDLE LinkHandle,
    PUNICODE_STRING LinkTarget, PULONG ReturnedLength)
{
	UNREFERENCED_PARAMETER(LinkHandle);
	UNREFERENCED_PARAMETER(LinkTarget);
	UNREFERENCED_PARAMETER(ReturnedLength);
	return STATUS_OBJECT_NAME_NOT_FOUND;
}


DDKAPI
VOID KeSetSystemAffinityThread(KAFFINITY Affinity)
{
	UNREFERENCED_PARAMETER(Affinity);
}

DDKAPI
KAFFINITY KeSetSystemAffinityThreadEx(KAFFINITY Affinity)
{
	UNREFERENCED_PARAMETER(Affinity);
	return ULONGLONG_MAX;
}

DDKAPI
VOID KeRevertToUserAffinityThreadEx(KAFFINITY Affinity)
{
	UNREFERENCED_PARAMETER(Affinity);
}

DDKAPI
VOID KeRevertToUserAffinityThread(VOID)
{
}

DDKAPI
VOID
KeSetSystemGroupAffinityThread(PGROUP_AFFINITY Affinity, PGROUP_AFFINITY PreviousAffinity)
{
	UNREFERENCED_PARAMETER(Affinity);
	UNREFERENCED_PARAMETER(PreviousAffinity);
}

DDKAPI
VOID
KeRevertToUserGroupAffinityThread(PGROUP_AFFINITY PreviousAffinity)
{
	UNREFERENCED_PARAMETER(PreviousAffinity);
}

DDKAPI
VOID
KeQueryNodeActiveAffinity(USHORT NodeNumber, PGROUP_AFFINITY Affinity, PUSHORT Count)
{
	UNREFERENCED_PARAMETER(NodeNumber);
	UNREFERENCED_PARAMETER(Affinity);
	UNREFERENCED_PARAMETER(Count);
}

DDKAPI
USHORT
KeQueryHighestNodeNumber()
{
	return 0;
}

NTSTATUS
KeGetProcessorNumberFromIndex(ULONG ProcIndex, PPROCESSOR_NUMBER ProcNumber)
{
	UNREFERENCED_PARAMETER(ProcIndex);
	UNREFERENCED_PARAMETER(ProcNumber);
	return STATUS_SUCCESS;
}

ULONG
KeGetProcessorIndexFromNumber(PPROCESSOR_NUMBER ProcNumber)
{
	UNREFERENCED_PARAMETER(ProcNumber);
	return 0;
}

DDKAPI
NTSTATUS
KeQueryLogicalProcessorRelationship(PPROCESSOR_NUMBER ProcessorNumber,
	LOGICAL_PROCESSOR_RELATIONSHIP RelationshipType, PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX Information, PULONG Length)
{
	UNREFERENCED_PARAMETER(ProcessorNumber);
	UNREFERENCED_PARAMETER(RelationshipType);
	UNREFERENCED_PARAMETER(Information);
	UNREFERENCED_PARAMETER(Length);
	return STATUS_NOT_IMPLEMENTED;
}


DDKAPI
BOOLEAN MmIsAddressValid(PVOID VirtualAddress)
{
	UNREFERENCED_PARAMETER(VirtualAddress);
	return true;
}


typedef PVOID PWSK_REGISTRATION;
typedef PVOID PWSK_PROVIDER_NPI;
typedef PVOID PWSK_CLIENT_NPI;

CONST GUID NPI_WSK_INTERFACE_ID = {};

DDKAPI
NTSTATUS WskRegister(PWSK_CLIENT_NPI WskClientNpi, PWSK_REGISTRATION WskRegistration)
{
	UNREFERENCED_PARAMETER(WskClientNpi);
	UNREFERENCED_PARAMETER(WskRegistration);
	return STATUS_NOT_IMPLEMENTED;
}

DDKAPI
VOID WskDeregister(PWSK_REGISTRATION WskRegistration)
{
	UNREFERENCED_PARAMETER(WskRegistration);
}


DDKAPI
NTSTATUS WskCaptureProviderNPI(PWSK_REGISTRATION  WskRegistration, ULONG WaitTimeout, PWSK_PROVIDER_NPI WskProviderNpi)
{
	UNREFERENCED_PARAMETER(WskRegistration);
	UNREFERENCED_PARAMETER(WaitTimeout);
	UNREFERENCED_PARAMETER(WskProviderNpi);
	return STATUS_NOT_IMPLEMENTED;
}

DDKAPI
VOID WskReleaseProviderNPI(PWSK_REGISTRATION WskRegistration)
{
	UNREFERENCED_PARAMETER(WskRegistration);
}

NTSTATUS 
NETIOAPI_API_
GetUnicastIpAddressTable(ADDRESS_FAMILY Family, PMIB_UNICASTIPADDRESS_TABLE *Table)
{
	UNREFERENCED_PARAMETER(Family);
	UNREFERENCED_PARAMETER(Table);
	return STATUS_NOT_IMPLEMENTED;
}


NTSTATUS 
NETIOAPI_API_
NotifyUnicastIpAddressChange(
	ADDRESS_FAMILY Family, PUNICAST_IPADDRESS_CHANGE_CALLBACK Callback, 
	PVOID CallerContext, BOOLEAN InitialNotification, HANDLE *NotificationHandle)
{
	UNREFERENCED_PARAMETER(Family);
	UNREFERENCED_PARAMETER(Callback);
	UNREFERENCED_PARAMETER(CallerContext);
	UNREFERENCED_PARAMETER(InitialNotification);
	UNREFERENCED_PARAMETER(NotificationHandle);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
NETIOAPI_API_ GetUnicastIpAddressEntry(PMIB_UNICASTIPADDRESS_ROW row)
{
	UNREFERENCED_PARAMETER(row);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
NETIOAPI_API_
CancelMibChangeNotify2(HANDLE NotificationHandle)
{
	UNREFERENCED_PARAMETER(NotificationHandle);
	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
NETIOAPI_API_
GetIfEntry2(PMIB_IF_ROW2 Row)
{
	UNREFERENCED_PARAMETER(Row);
	return STATUS_NOT_IMPLEMENTED;
}


VOID 
NETIOAPI_API_
FreeMibTable(PVOID Memory)
{
	UNREFERENCED_PARAMETER(Memory);
}

DDKAPI
PWSTR RtlIpv6AddressToStringW(const struct in6_addr *Addr, PWSTR S)
{
	UNREFERENCED_PARAMETER(Addr);
	return S;
}

DDKAPI
PWSTR RtlIpv4AddressToStringW(const struct in_addr *Addr, PWSTR S)
{
	UNREFERENCED_PARAMETER(Addr);
	return S;
}

DDKAPI
LONG RtlIpv4StringToAddressW(PCWSTR S, BOOLEAN Strict, LPCWSTR *Terminator, struct in_addr *Addr)
{
	UNREFERENCED_PARAMETER(S);
	UNREFERENCED_PARAMETER(Strict);
	UNREFERENCED_PARAMETER(Terminator);
	UNREFERENCED_PARAMETER(Addr);
	return STATUS_NOT_IMPLEMENTED;
}

DDKAPI
LONG RtlIpv6StringToAddressW(PCWSTR S, PCWSTR *Terminator, struct in6_addr *Addr)
{
	UNREFERENCED_PARAMETER(S);
	UNREFERENCED_PARAMETER(Terminator);
	UNREFERENCED_PARAMETER(Addr);
	return STATUS_NOT_IMPLEMENTED;
}

