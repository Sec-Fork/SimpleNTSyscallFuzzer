#include "stdafx.h"
#include "Header.h"
#include "Evntrace.h"
#include "ktmw32.h"

extern ulong OSVer;

//------------Global GDI Objects------------

#pragma comment(linker,"/section:ntos,rws")
#pragma data_seg("ntos")

unsigned long Magic1 = 0xCCCCDDDD;

ulonglong InitProcessId;
ulonglong InitThreadId;
//hAdapter
HANDLE hLpc;
//hCallback
//Controller
HANDLE hDebugObject;
//hDevice
HANDLE hDirectory;
//hDriver
//hEtwConsumer
HANDLE hEtwRegistration;
HANDLE hEvent;
HANDLE hEventPair;
HANDLE hFile;
//hFilterCommunicationPort
//hFilterConnectionPort
HANDLE hIoCompletion;
HANDLE hIoCompletionReserve;
HANDLE hJob;
HKEY hKey;
HANDLE hKeyedEvent;
HANDLE hMutex;
//hPcwObject ==> IOCTL on pcw.sys
HANDLE hPowerRequest;
HANDLE hProcess;
HANDLE hProfile;
HANDLE hSection;
HANDLE hSemaphore;
HANDLE hSession;
HANDLE hSymLink;
HANDLE hThread;
HANDLE hTimer;

HANDLE hTmRm;
HANDLE hTmTm;
HANDLE hTmTx;
HANDLE hToken;
HANDLE hTmEn;
HANDLE hTpWorkerFactory;
//Type
HANDLE hUserApcReserve;
HWINSTA hWinsta = 0;
//hWmiGuid





HANDLE AllFiles[0x1000]={0};
HANDLE AllProcesses[0x1000]={0};


ulonglong AllKernelObject[0x5000]={0};
ulong AllKernelObjectsUsed = 1;	//fix for div by zero, == side effec==> first object is always zero
ulong AllFilesUsed = 1;			//fix for div by zero, == side effec==> first object is always zero
ulong AllProcessesUsed = 1;		//fix for div by zero, == side effec==> first object is always zero



unsigned long Magic2 = 0xCCCCCCCC;
#pragma data_seg()
//-------------------------------



ulong cached_AllKernelObjectsUsed = 0;
ulong cached_AllFilesUsed = 0;
ulong cached_AllProcessesUsed = 0;

//------------------------------------------





//---------------------------
ULONG WINAPI ControlCallback(
  _In_ WMIDPREQUESTCODE RequestCode,
  _In_ PVOID            Context,
  _In_ ULONG            *Reserved,
  _In_ PVOID            Buffer
)
{
	return ERROR_SUCCESS;
}


void TpRoutine(void* pContext)
{
	return;
}

HANDLE CreateLpcPort(wchar_t* BaseName,ulong Rx,bool bPrint)
{
	wchar_t ObjectName_All[MAX_PATH+1]={0};

	wcscpy(ObjectName_All,BaseName);
	unsigned long  LenX = wcslen(ObjectName_All);
	_ultow(Rx,&ObjectName_All[LenX],0x10);

	if(bPrint)	wprintf(L"LPC: %s\r\n",ObjectName_All);
	
	_UNICODE_STRING UniS = {0};
	UniS.Length = wcslen(ObjectName_All)*2;
	UniS.MaxLength = (UniS.Length)+2;
	UniS.Buffer = ObjectName_All;

	_OBJECT_ATTRIBUTES ObjAttr = {sizeof(ObjAttr)};
	ObjAttr.Attributes=OBJ_CASE_INSENSITIVE;
	ObjAttr.ObjectName=&UniS;
	
	HANDLE hLpc_l = 0;
	int retValue = ZwCreatePort(&hLpc_l,&ObjAttr,0x0,0x200,0);
	if(retValue < 0)
	{
		if(bPrint)	printf ("Error creating ALPC port, ret: %X\r\n",retValue);
		return 0;
	}
	if(bPrint) printf("LPC: %X\r\n",hLpc_l);
	return hLpc_l;
}

HANDLE CreateDebug(wchar_t* BaseName,ulong Rx,bool bPrint)
{
	wchar_t ObjectName_All[MAX_PATH+1]={0};

	wcscpy(ObjectName_All,BaseName);
	ulong LenX = wcslen(ObjectName_All);
	_ultow(Rx,&ObjectName_All[LenX],0x10);
	LenX = wcslen(ObjectName_All);
	ulong Rxx = Rand();
	_ultow(Rxx,&ObjectName_All[LenX],0x10);
	if(bPrint)	wprintf(L"DebugObject: %s\r\n",ObjectName_All);

	_UNICODE_STRING UniSec = {0};
	UniSec.Length = wcslen(ObjectName_All)*2;
	UniSec.MaxLength = UniSec.Length + 2;
	UniSec.Buffer = ObjectName_All;


	_OBJECT_ATTRIBUTES ObjAttrDebug={0};
	ObjAttrDebug.Length = sizeof(ObjAttrDebug);
	ObjAttrDebug.Attributes= OBJ_CASE_INSENSITIVE;
	ObjAttrDebug.ObjectName = & UniSec;

	HANDLE hDebug_l = 0;
	int retValue = ZwCreateDebugObject(&hDebug_l,GENERIC_ALL,&ObjAttrDebug,0);
	if(retValue < 0)
	{
		if(bPrint)	printf ("Error creating Debug Object ret: %X\r\n",retValue);
		return 0;
	}
	if(bPrint) printf("hDebugObject: %X\r\n",hDebug_l);
	return hDebug_l;
}

HANDLE CreateObjDir(wchar_t* BaseName,ulong Rx,bool bPrint)
{
	wchar_t ObjectName_All[MAX_PATH+1]={0};
	wcscpy(ObjectName_All,BaseName);
	ulong LenX = wcslen(ObjectName_All);
	_ultow(Rx,&ObjectName_All[LenX],0x10);
	LenX = wcslen(ObjectName_All);
	ulong Rxx = Rand();
	_ultow(Rxx,&ObjectName_All[LenX],0x10);
	if(bPrint) wprintf(L"Directory: %s\r\n",ObjectName_All);

	_UNICODE_STRING UniSD = {0};
	UniSD.Length = wcslen(ObjectName_All)*2;
	UniSD.MaxLength = UniSD.Length + 2;
	UniSD.Buffer = ObjectName_All;


	_OBJECT_ATTRIBUTES ObjAttrDir={0};
	ObjAttrDir.Length = sizeof(ObjAttrDir);
	ObjAttrDir.Attributes= OBJ_CASE_INSENSITIVE;
	ObjAttrDir.ObjectName = & UniSD;

	HANDLE hDir_l = 0;
	int retValue = ZwCreateDirectoryObject(&hDir_l,GENERIC_ALL,&ObjAttrDir);
	if(retValue < 0)
	{
		if(bPrint)	printf ("Error creating Directory Object ret: %X\r\n",retValue);
		return 0;
	}
	if(bPrint)	printf("hDirectory: %X\r\n",hDir_l);
	return hDir_l;
}

//Major Code change in latest Win10, please update
//Requires an admin
HANDLE CreateEtwConsumer(bool bPrint)
{
	//printf("sizeof: %X\r\n",sizeof(_ETW_CONSUMER_INPUT_OUTPUT));
	ulonglong ResultLength = 0;
	
	
	_ETW_CONSUMER_INPUT_OUTPUT EtwConsumer = {0};
	EtwConsumer.LoggerId = 2;
	EtwConsumer.NumberOfBlocks = 4;

	ulong X0 = 0;
	EtwConsumer.pAddr0 = &X0;


	EtwConsumer.pAddr1 = VirtualAlloc(0,(0x4 * 0x80f8),MEM_COMMIT,PAGE_READWRITE);
	printf("Addr1: %I64X\r\n",EtwConsumer.pAddr1 );

	ulong X3 = 0;
	EtwConsumer.pAddr3 = &X3;

	ulonglong X2 = 0;
	EtwConsumer.pAddr2 = &X2;




	EtwConsumer.hEvent0 = CreateEvent(0,TRUE,TRUE,0);
	printf("hEvent0: %I64X\r\n",EtwConsumer.hEvent0);

	EtwConsumer.hEvent1 = CreateEvent(0,TRUE,TRUE,0);
	printf("hEvent1: %I64X\r\n",EtwConsumer.hEvent1);


	int retValue = ZwTraceControl(0xB,
								&EtwConsumer,
								sizeof(_ETW_CONSUMER_INPUT_OUTPUT),
								&EtwConsumer,
								sizeof(_ETW_CONSUMER_INPUT_OUTPUT),
								&ResultLength);
	printf("ZwTraceControl, ret: %X, ResultLength: %X\r\n",retValue,ResultLength);
	if(retValue < 0)
	{
		VirtualFree(EtwConsumer.pAddr1,0,MEM_RELEASE);
		return 0;
	}

	printf("hEtwConsumer: %I64X\r\n",EtwConsumer.hEtwConsumer);

	VirtualFree(EtwConsumer.pAddr1,0,MEM_RELEASE);
	return EtwConsumer.hEtwConsumer;
}

HANDLE CreateEtwRegistration(bool bPrint)
{
	//printf("sizeof: %X\r\n",sizeof(_ETW_REGISTRATION_INPUT_OUTPUT));
	unsigned char In[0x11]="\xE8\x2A\xA9\xBD\x11\x9F\x49\x4D\xBA\x1D\xA4\xC2\xAB\xCA\x69\x2E";//copied as is


	_ETW_REGISTRATION_INPUT_OUTPUT EtwReg = {0};
	memcpy(&EtwReg.guid,In,0x10);

	ulonglong ResultLength = 0;
	int retValue = 
		ZwTraceControl(0xF,
		&EtwReg,
		sizeof(_ETW_REGISTRATION_INPUT_OUTPUT),
		&EtwReg,
		sizeof(_ETW_REGISTRATION_INPUT_OUTPUT),
		&ResultLength);

	if(bPrint) printf("ZwTraceControl, ret: %X\r\n",retValue);
	if(retValue < 0)
	{
		if(bPrint)	printf("Can't create EtwRegistration kernel object\r\n");
		return 0;
	}
	if(bPrint)	printf("hEtwRegistration: %I64X\r\n",EtwReg.hEtwReg);
	return EtwReg.hEtwReg;
}

int InitKernelObjects(bool bPrint)
{
	AllKernelObjectsUsed = 1;
	AllFilesUsed = 1;
	AllProcessesUsed = 1;

	InitProcessId = GetCurrentProcessId();
	InitThreadId = GetCurrentThreadId();

	wchar_t ObjectName_All[MAX_PATH+1]={0};


	//------------ Static Name Parts -------------------------------------
	wchar_t* Name = L"walied";
	wchar_t* FullName = L"\\BaseNamedObjects\\Local\\walied";
	//-------------- Random Name Parts ------------------------------------
	unsigned long Rx = Rand();
	
	unsigned long Rxx;
	//---------------- ALPC ----------------------------------------------
	HANDLE hLpc_l= CreateLpcPort(FullName,Rx,bPrint);
	//if(bPrint) printf("LPC: %X\r\n",hLpc_l);
	hLpc = hLpc_l;

	
	AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hLpc_l;
	AllKernelObjectsUsed++;
	ObjectName_All[0]=0;
	//--------------------------- Debug -----------------------------------
	HANDLE hDebug_l = CreateDebug(FullName,Rx,bPrint);
	//if(bPrint) printf("hDebugObject: %X\r\n",hDebug_l);
	hDebugObject = hDebug_l;
	AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hDebug_l;
	AllKernelObjectsUsed++;
	ObjectName_All[0] = 0;
	
	//------------------------ Directory ---------------------------------
	HANDLE hDir_l = CreateObjDir(FullName,Rx,bPrint);
	//if(bPrint)	printf("hDirectory: %X\r\n",hDir_l);
	hDirectory = hDir_l;
	AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hDir_l;
	AllKernelObjectsUsed++;
	ObjectName_All[0] = 0;
	
	//------------------------ ETW --------------------------------------------------
	//----------------------EtwConsumer -------------- Requires admin----------------
	//CreateEtwConsumer(bPrint);
	//-------------------------- hEtwRegistration ------------------------
	HANDLE hEtwRegistration_l = CreateEtwRegistration(bPrint);
	//if(bPrint)	printf("hEtwRegistration: %I64X\r\n",hEtwRegistration_l);
	hEtwRegistration = hEtwRegistration_l;

	AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hEtwRegistration_l;
	AllKernelObjectsUsed++;
	ObjectName_All[0]=0;
	//ExitProcess(0);
	//-------------------------Event--------------------------------------------
	wcscpy(ObjectName_All,Name);
	ulong LenX = wcslen(ObjectName_All);
	_ultow(Rx,&ObjectName_All[LenX],0x10);
	if(bPrint) wprintf(L"Event: %s\r\n",ObjectName_All);
	HANDLE hEvent_l = CreateEvent(0,TRUE,TRUE,ObjectName_All);
	if(hEvent_l == INVALID_HANDLE_VALUE)
	{
		if(bPrint)	printf("Error creating kernel event object\r\n");
		return -5;
	}
	hEvent = hEvent_l;
	if(bPrint)	printf("Event: %X\r\n",hEvent);
	ObjectName_All[0]=0;

	AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hEvent_l;
	AllKernelObjectsUsed++;
	//-------------------- Event Pair -------------------------------------
	if(OSVer == 0) //Event pair not implemented in windows 10
	{
		wcscpy(ObjectName_All,FullName);
		LenX = wcslen(ObjectName_All);
		_ultow(Rx,&ObjectName_All[LenX],0x10);
		LenX = wcslen(ObjectName_All);
		Rxx = Rand();
		_ultow(Rxx,&ObjectName_All[LenX],0x10);
	


		if(bPrint)	wprintf(L"EventPair: %s\r\n",ObjectName_All);

		_UNICODE_STRING UniSS_ep = {0};
		UniSS_ep.Length = wcslen(ObjectName_All)*2;
		UniSS_ep.MaxLength = UniSS_ep.Length + 2;
		UniSS_ep.Buffer = ObjectName_All;


		_OBJECT_ATTRIBUTES ObjAttrEP={0};
		ObjAttrEP.Length = sizeof(ObjAttrEP);
		ObjAttrEP.Attributes= OBJ_CASE_INSENSITIVE;
		ObjAttrEP.ObjectName = & UniSS_ep;
		HANDLE hEventPair_l = 0;
		int retValue = ZwCreateEventPair(&hEventPair_l,GENERIC_ALL,&ObjAttrEP);
		if(bPrint) printf("ZwCreateEventPair, ret: %X, hEventPair_l: %I64X\r\n",retValue,hEventPair_l);
		if(retValue < 0)
		{
			if(bPrint)	printf("Error creating kernel event pair\r\n");
			return -9;
		}
		hEventPair = hEventPair_l;

		if(bPrint)	printf("hEventPair: %X\r\n",hEventPair);
		ObjectName_All[0] = 0;

		AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hEventPair_l;
		AllKernelObjectsUsed++;
	}
	//------------------- File ---------------------------
	wcscpy(ObjectName_All,Name);
	LenX = wcslen(ObjectName_All);
	_ultow(Rx,&ObjectName_All[LenX],0x10);

	if(bPrint)	wprintf(L"File: %s\r\n",ObjectName_All);
	
	HANDLE hFile_l = CreateFile(ObjectName_All,GENERIC_READ|GENERIC_WRITE,0,0,CREATE_ALWAYS,0,0);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		if(bPrint)	printf("Error creating kernel file object\r\n");
		return -5;
	}
	hFile = hFile_l;
	if(bPrint)	printf("File: %X\r\n",hFile);
	ObjectName_All[0]=0;
	AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hFile_l;
	AllKernelObjectsUsed++;
	//----------------------- IO Completion ------------------------------
	wcscpy(ObjectName_All,FullName);
	LenX = wcslen(ObjectName_All);
	_ultow(Rx,&ObjectName_All[LenX],0x10);
	LenX = wcslen(ObjectName_All);
	Rxx = Rand();
	_ultow(Rxx,&ObjectName_All[LenX],0x10);


	if(bPrint)	wprintf(L"IOCompletion: %s\r\n",ObjectName_All);

	_UNICODE_STRING UniSS_iocomp = {0};
	UniSS_iocomp.Length = wcslen(ObjectName_All)*2;
	UniSS_iocomp.MaxLength = UniSS_iocomp.Length + 2;
	UniSS_iocomp.Buffer = ObjectName_All;


	_OBJECT_ATTRIBUTES ObjAttrIOCOMP={0};
	ObjAttrIOCOMP.Length = sizeof(ObjAttrIOCOMP);
	ObjAttrIOCOMP.Attributes= OBJ_CASE_INSENSITIVE;
	ObjAttrIOCOMP.ObjectName = & UniSS_iocomp;
	HANDLE hIoCompletion_l = 0;
	int retValue = ZwCreateIoCompletion(&hIoCompletion_l,GENERIC_ALL,&ObjAttrIOCOMP,2);
	if(bPrint)	printf("ZwCreateIoCompletion, ret: %X, hIoCompletion_l: %I64X\r\n",retValue,hIoCompletion_l);
	if(retValue < 0)
	{
		if(bPrint)	printf("Error creating kernel IOCompletion\r\n");
		return -10;
	}
	hIoCompletion = hIoCompletion_l;

	if(bPrint)	printf("hIOCompletion: %X\r\n",hIoCompletion);
	ObjectName_All[0] = 0;

	AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hIoCompletion_l;
	AllKernelObjectsUsed++;
	//------------------------ IO Completion Reserve ----------------------

	/*
	wcscpy(ObjectName_All,FullName);
	LenX = wcslen(ObjectName_All);
	_ultow(Rx,&ObjectName_All[LenX],0x10);
	LenX = wcslen(ObjectName_All);
	Rxx = Rand();
	_ultow(Rxx,&ObjectName_All[LenX],0x10);


	if(bPrint)	wprintf(L"IOCompletionReserve: %s\r\n",ObjectName_All);

	_UNICODE_STRING UniSS_iocomprsv = {0};
	UniSS_iocomprsv.Length = wcslen(ObjectName_All)*2;
	UniSS_iocomprsv.MaxLength = UniSS_iocomprsv.Length + 2;
	UniSS_iocomprsv.Buffer = ObjectName_All;


	_OBJECT_ATTRIBUTES ObjAttrIOCOMPrsv={0};
	ObjAttrIOCOMPrsv.Length = sizeof(ObjAttrIOCOMPrsv);
	ObjAttrIOCOMPrsv.Attributes= OBJ_CASE_INSENSITIVE;
	ObjAttrIOCOMPrsv.ObjectName = & UniSS_iocomprsv;
	*/
	HANDLE hIoCompletionReserve_l = 0;
	
	retValue = ZwAllocateReserveObject(&hIoCompletionReserve_l,0,1);
	if(bPrint)	printf("ZwAllocateReserveObject, ret: %X, hIoCompletionReserve_l: %I64X\r\n",retValue,hIoCompletionReserve_l);
	if(retValue < 0)
	{
		if(bPrint)	printf("Error creating kernel IOCompletionReserve\r\n");
		return -11;
	}
	hIoCompletionReserve = hIoCompletionReserve_l;

	if(bPrint)	printf("hIoCompletionReserve: %X\r\n",hIoCompletionReserve);
	ObjectName_All[0] = 0;

	AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hIoCompletionReserve_l;
	AllKernelObjectsUsed++;
	//---------------------- Job ------------------------------------------
	wcscpy(ObjectName_All,Name);
	LenX = wcslen(ObjectName_All);
	_ultow(Rx,&ObjectName_All[LenX],0x10);
	LenX = wcslen(ObjectName_All);
	Rxx = Rand();
	_ultow(Rxx,&ObjectName_All[LenX],0x10);



	if(bPrint)	wprintf(L"Job: %s\r\n",ObjectName_All);
	HANDLE hJob_l = CreateJobObject(0,ObjectName_All);
	if(!hJob_l)
	{
		if(bPrint)	printf("Error creating job object, Err: %X\r\n",GetLastError());
		return -5;
	}
	hJob = hJob_l;
	if(bPrint)	printf("Job: %X\r\n",hJob);
	ObjectName_All[0] = 0;

	AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hJob_l;
	AllKernelObjectsUsed++;
	
	
	
	//Visual studio has its own job while testing, uncomment later
	if(!AssignProcessToJobObject(hJob,GetCurrentProcess()))
	{
		if(bPrint)	printf("Error assigning to  job object, process already a job object\r\n");
		return -5;
	}
	//---------------------- Key ---------------------------------------------
	wcscpy(ObjectName_All,Name);
	LenX = wcslen(ObjectName_All);
	_ultow(Rx,&ObjectName_All[LenX],0x10);
	LenX = wcslen(ObjectName_All);
	Rxx = Rand();
	_ultow(Rxx,&ObjectName_All[LenX],0x10);

	HKEY hKey_l = 0;

	
	if( RegCreateKey(HKEY_CURRENT_USER,ObjectName_All,&hKey_l) != ERROR_SUCCESS)
	{
		if(bPrint)	printf("Error creating key object\r\n");
		return -12;
	}
	hKey = hKey_l;
	if(bPrint)	printf("hKey: %X\r\n",hKey);
	ObjectName_All[0] = 0;

	AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hKey_l;
	AllKernelObjectsUsed++;
	//-------------------------- Keyed Event --------------------------------
	wcscpy(ObjectName_All,FullName);
	LenX = wcslen(ObjectName_All);
	_ultow(Rx,&ObjectName_All[LenX],0x10);
	LenX = wcslen(ObjectName_All);
	Rxx = Rand();
	_ultow(Rxx,&ObjectName_All[LenX],0x10);


	if(bPrint)	wprintf(L"hKeyedEvent: %s\r\n",ObjectName_All);

	_UNICODE_STRING UniSS_keyed = {0};
	UniSS_keyed.Length = wcslen(ObjectName_All)*2;
	UniSS_keyed.MaxLength = UniSS_keyed.Length + 2;
	UniSS_keyed.Buffer = ObjectName_All;


	_OBJECT_ATTRIBUTES ObjAttr_keyed={0};
	ObjAttr_keyed.Length = sizeof(ObjAttr_keyed);
	ObjAttr_keyed.Attributes= OBJ_CASE_INSENSITIVE;
	ObjAttr_keyed.ObjectName = & UniSS_keyed;

	HANDLE hKeyedEvent_l = 0;
	
	retValue = ZwCreateKeyedEvent(&hKeyedEvent_l,GENERIC_ALL,&ObjAttr_keyed,0);
	if(bPrint)	printf("ZwCreateKeyedEvent, ret: %X, hKeyedEvent_l: %I64X\r\n",retValue,hKeyedEvent_l);
	if(retValue < 0)
	{
		if(bPrint)	printf("Error creating kernel KeyedEvent\r\n");
		return -11;
	}
	hKeyedEvent = hKeyedEvent_l;

	if(bPrint)	printf("hKeyedEvent: %X\r\n",hKeyedEvent);
	ObjectName_All[0] = 0;

	AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hKeyedEvent;
	AllKernelObjectsUsed++;
	//---------------- Mutex / Mutant ---------------------------
	wcscpy(ObjectName_All,Name);
	wcscat(ObjectName_All,L"-");//mutex and event can't have the same exact name?!!!
	LenX = wcslen(ObjectName_All);
	
	_ultow(Rx,&ObjectName_All[LenX],0x10);

	
	if(bPrint)	wprintf(L"Mutex: %s\r\n",ObjectName_All);
	HANDLE hMutex_l = CreateMutex(0,0,ObjectName_All);
	if(!hMutex_l)
	{
		if(GetLastError()==ERROR_ALREADY_EXISTS)
		{
			if(bPrint)	printf("A mutex with the same name already exists\r\n");
		}
		else
		{
			if(bPrint)	printf("Error creating kernel mutex object, Err: %X\r\n",GetLastError());
		}
		return -5;
	}
	hMutex = hMutex_l;
	if(bPrint)	printf("hMutext: %I64X\r\n",hMutex);
	ObjectName_All[0]=0;
	AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hMutex_l;
	AllKernelObjectsUsed++;
	//--------------------- Power Request ---------------------------

	pAddr Addr = {0};
	Addr.X = 0;
	Addr.Flags = 0x1;


	HANDLE hPowerRequest_l = 0;
	retValue = ZwPowerInformation(0x2B,&Addr,0x28,&hPowerRequest_l,0x8);
	if(bPrint)	printf("ZwPowerInformation: ret: %X, hPowerRequest: %I64X\r\n",retValue,hPowerRequest_l);
	if(retValue < 0)
	{
		if(bPrint)	printf("Error creating kernel power request\r\n");
		return -13;
	}
	hPowerRequest = hPowerRequest_l;
	if(bPrint)	printf("hPowerRequest: %I64X\r\n",hPowerRequest_l);
	ObjectName_All[0]=0;
	AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hPowerRequest_l;
	AllKernelObjectsUsed++;
	//------------------------- Process ---------------------------------
	PROCESS_INFORMATION PI;
	STARTUPINFO SI = {sizeof(SI)};
	wchar_t Path_All[MAX_PATH+1] = {0};
	GetSystemDirectory(Path_All,MAX_PATH);
	wcscat(Path_All,L"\\calc.exe");

	if(!CreateProcess(Path_All,0,0,0,FALSE,CREATE_SUSPENDED,0,0,&SI,&PI))
	{
		if(bPrint)	printf("Error creating process\r\n");
		return -14;
	}
	HANDLE hProcess_l = PI.hProcess;
	hProcess = hProcess_l;

	if(bPrint)	printf("hProcess: %I64X\r\n",hProcess_l);
	ObjectName_All[0]=0;
	AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hProcess_l;
	AllKernelObjectsUsed++;
	//------------------- Profile ----------------------------
	
	unsigned char BufferProf[0x8]={0};
	HANDLE hProfile_l = 0;

	retValue = ZwCreateProfile(&hProfile_l,GetCurrentProcess(),(void*)GetModuleHandle(0),0x8 /* ProfileSize */,
				0x8 /* BucketSize */,
				BufferProf /* Buffer */, 8 /* BufferSize */,
				2 /* Source */,
				-1 /* Affinity*/);

	if(bPrint)	printf("ZwCreateProfile, ret: %X\r\n",retValue);
	//Profile objects get out of service in some windows builds
	if(retValue < 0)
	{
		if(bPrint)	printf("Error creating kernel profile object\r\n");
		//Profile objects get out of service in some windows builds
		//return -15;
	}
	else
	{
		hProfile = hProfile_l;
		if(bPrint)	printf("hProfile: %I64X\r\n",hProfile_l);
		ObjectName_All[0]=0;
		AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hProfile_l;
		AllKernelObjectsUsed++;
	}

	//-------------------- Section ------------------ Implement Named Section Later ---
	//wcscpy(ObjectName_All,FullName);
	//LenX = wcslen(ObjectName_All);
	//_ultow(Rx,&ObjectName_All[LenX],0x10);
	//LenX = wcslen(ObjectName_All);
	//Rxx = Rand();
	//_ultow(Rxx,&ObjectName_All[LenX],0x10);
	


	//wprintf(L"Section: %s\r\n",ObjectName_All);

	//_UNICODE_STRING UniSS = {0};
	//UniSS.Length = wcslen(ObjectName_All)*2;
	//UniSS.MaxLength = UniSS.Length + 2;
	//UniSS.Buffer = ObjectName_All;


	//_OBJECT_ATTRIBUTES ObjAttrSection={0};
	//ObjAttrSection.Length = sizeof(ObjAttrSection);
	//ObjAttrSection.Attributes= OBJ_CASE_INSENSITIVE;
	//ObjAttrSection.ObjectName = & UniSS;

	HANDLE hSection_l = 0;

	ulonglong MaxSize=0x400000;


	retValue = ZwCreateSection(&hSection,
		SECTION_MAP_READ|SECTION_MAP_WRITE,
		0,
		(_LARGE_INTEGER*)(&MaxSize),
		PAGE_READWRITE,
		SEC_COMMIT,
		0);
	if(bPrint)	printf("ZwCreateSection, ret: %X\r\n",retValue);
	if( retValue < 0)
	{
		if(bPrint)	printf("Error creating kernel section Object\r\n");
		return -6;
	}
	hSection = hSection_l;
	if(bPrint)	printf("hSection: %I64X\r\n",hSection);
	ObjectName_All[0]=0;

	AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hSection_l;
	AllKernelObjectsUsed++;
	//------------------------- Semaphore ---------------------
	wcscpy(ObjectName_All,Name);
	LenX = wcslen(ObjectName_All);
	_ultow(Rx,&ObjectName_All[LenX],0x10);
	LenX = wcslen(ObjectName_All);
	Rxx = Rand();
	_ultow(Rxx,&ObjectName_All[LenX],0x10);
	HANDLE hSemaphore_l = CreateSemaphore(0,0x10,0x100,ObjectName_All);
	if(!hSemaphore_l)
	{
		if(bPrint)	printf("Error creating kernel semaphore object\r\n");
		return -16;
	}
	hSemaphore = hSemaphore_l;
	if(bPrint)	printf("hSemaphore: %I64X\r\n",hSemaphore_l);
	ObjectName_All[0]=0;
	AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hSemaphore_l;
	AllKernelObjectsUsed++;
	//---------------------- Session -----------Requires priv, try NtCreateUserProcess

	/*

	wcscpy(ObjectName_All,L"\\KernelObjects\\session");
	LenX = wcslen(ObjectName_All);
	for(unsigned long ss = 0; ss < 10;ss++)
	{
		_ultow(ss,&ObjectName_All[LenX],0x10);
		wprintf(L"Trying: %s\r\n",ObjectName_All);

		_UNICODE_STRING UniSS_session = {0};
		UniSS_session.Length = wcslen(ObjectName_All)*2;
		UniSS_session.MaxLength = UniSS_session.Length + 2;
		UniSS_session.Buffer = ObjectName_All;

		_OBJECT_ATTRIBUTES ObjAttrSession={0};
		ObjAttrSession.Length = sizeof(ObjAttrSession);
		ObjAttrSession.Attributes= OBJ_CASE_INSENSITIVE;
		ObjAttrSession.ObjectName = & UniSS_session;


		HANDLE hSession_l = 0;
		retValue = ZwOpenSession(&hSession,GENERIC_ALL,&ObjAttrSession);
		if(bPrint)	printf("ZwOpenSession, ret: %X\r\n",retValue);

	}
	
	*/
	//-----------------------  SymLink ---------------ZwCreateSymbolicLink ==> Priv required


	wchar_t* pXXX = L"\\BaseNamedObjects\\local";
	_UNICODE_STRING UniSS_sl = {0};
	UniSS_sl.Length = wcslen(pXXX)*2;
	UniSS_sl.MaxLength = UniSS_sl.Length + 2;
	UniSS_sl.Buffer = pXXX;


	_OBJECT_ATTRIBUTES ObjAttr_sl={0};
	ObjAttr_sl.Length = sizeof(ObjAttr_sl);
	ObjAttr_sl.Attributes= OBJ_CASE_INSENSITIVE;
	ObjAttr_sl.ObjectName = & UniSS_sl;



	HANDLE hSymLink_l = 0;
	retValue = ZwOpenSymbolicLinkObject(&hSymLink_l,GENERIC_READ,&ObjAttr_sl);
	if(bPrint)	printf("ZwOpenSymbolicLinkObject, ret: %X\r\n",retValue);
	if(retValue < 0)
	{
		if(bPrint)	printf("Error opening symbolic link\r\n");
		return -15;
	}
	hSymLink = hSymLink_l;

	if(bPrint)	printf("hSymLink: %I64X\r\n",hSymLink_l);
	ObjectName_All[0]=0;
	AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hSymLink_l;
	AllKernelObjectsUsed++;
	//----------------------- Thread --------------------------
	HANDLE hThread_l = PI.hThread;
	if(bPrint)	printf("hThread: %I64X\r\n",hThread_l);
	hThread = hThread_l;
	AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hThread_l;
	AllKernelObjectsUsed++;
	//--------------------- Timer ----------------------
	wcscpy(ObjectName_All,FullName);
	LenX = wcslen(ObjectName_All);
	_ultow(Rx,&ObjectName_All[LenX],0x10);
	LenX = wcslen(ObjectName_All);
	Rxx = Rand();
	_ultow(Rxx,&ObjectName_All[LenX],0x10);
	


	if(bPrint)	wprintf(L"Timer: %s\r\n",ObjectName_All);

	_UNICODE_STRING UniSS_ti = {0};
	UniSS_ti.Length = wcslen(ObjectName_All)*2;
	UniSS_ti.MaxLength = UniSS_ti.Length + 2;
	UniSS_ti.Buffer = ObjectName_All;


	_OBJECT_ATTRIBUTES ObjAttrTi={0};
	ObjAttrTi.Length = sizeof(ObjAttrTi);
	ObjAttrTi.Attributes= OBJ_CASE_INSENSITIVE;
	ObjAttrTi.ObjectName = & UniSS_ti;

	HANDLE hTimer_l = 0;
	retValue = ZwCreateTimer(&hTimer_l,GENERIC_ALL,&ObjAttrTi,1);
	if(bPrint)	printf("ZwCreateTimer, ret: %X\r\n",retValue);
	if(retValue < 0)
	{
		if(bPrint)	printf("Error creating kernel timer\r\n");
		return -16;
	}
	hTimer = hTimer_l;

	if(bPrint)	printf("hTimer: %I64X\r\n",hTimer_l);
	ObjectName_All[0]=0;
	AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hTimer_l;
	AllKernelObjectsUsed++;
	//------------------------------- TmTm Transaction Manager ------------------------
	HANDLE hTmTm_l = 0;

	wchar_t* pTmLog = L"tmlog.txt";

	/*
	_UNICODE_STRING UniTmLog = {0};
	UniTmLog.Length = wcslen(pTmLog)*2;
	UniTmLog.MaxLength = UniTmLog.Length + 2;
	UniTmLog.Buffer = pTmLog;

	retValue = ZwCreateTransactionManager(&hTmTm_l,GENERIC_ALL,0,0,1,0);

	
	if(bPrint)	printf("ZwCreateTransactionManager, ret: %X\r\n",retValue);
	if(retValue < 0)
	{
		if(bPrint)	printf("Error creating TmTm Transaction Manager\r\n");
		return -17;
	}
	*/

	wchar_t TmLogName[MAX_PATH]={0};
	wcscpy(TmLogName,L"walied");

	ulong LogX = Rand();
	_ultow(LogX,&TmLogName[6],0x10);

	wcscat(TmLogName,L".log");

	hTmTm_l = CreateTransactionManager(0,TmLogName,0,0);
	if(hTmTm_l == INVALID_HANDLE_VALUE)
	{
		if(bPrint)	printf("Error creating TmTm Transaction Manager\r\n");
		return -17;
	}

	hTmTm = hTmTm_l;

	if(bPrint)	printf("hTmTm: %I64X\r\n",hTmTm_l);

	ObjectName_All[0]=0;
	AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hTmTm_l;
	AllKernelObjectsUsed++;
	//-------------------------- TmRm  Resource Manager ------------------------------
	
	HANDLE hTmRm_l = 0;

	GUID Guid;
	FillRandomData(&Guid,sizeof(GUID));

	retValue = ZwCreateResourceManager(&hTmRm_l,GENERIC_ALL,hTmTm_l,&Guid,0,0,0);

	if(bPrint)	printf("ZwCreateResourceManager, ret: %X\r\n",retValue);

	if(retValue < 0)
	{
		if(bPrint)	printf("Error creating TmRm Resource Manager\r\n");
		return -18;
	}

	hTmRm = hTmRm_l;

	if(bPrint)	printf("hTmRm: %I64X\r\n",hTmRm_l);

	ObjectName_All[0]=0;
	AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hTmRm_l;
	AllKernelObjectsUsed++;
	//----------------------- TmTx ------------------------
	HANDLE hTmTx_l = CreateTransaction(0,0,0,0,0,0,0);
	if(hTmTx_l == INVALID_HANDLE_VALUE)
	{
		if(bPrint)	printf("Error creating TmTx Transaction\r\n");
		return -19;
	}
	hTmTx = hTmTx_l;

	if(bPrint)	printf("hTmTx: %I64X\r\n",hTmTx_l);

	
	ObjectName_All[0]=0;
	AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hTmTx_l;
	AllKernelObjectsUsed++;
	//---------------------- TmEn Enlistment --------------------------
	//unsigned char Buff[0x200]={0};
	//HANDLE hTmEn_l = CreateEnlistment(0,hTmRm_l,hTmTx_l,TRANSACTION_NOTIFY_PREPARE,0,Buff);
	
	
	//HANDLE hTmEn_l = 0;
	//retValue = ZwCreateEnlistment(&hTmEn_l,GENERIC_ALL,hTmRm_l,hTmTx_l,0,1 /* CreateOptions 0 or 1 */,0 /* Notification Mask*/,0);
	//printf("ZwCreateEnlistment: %X\r\n",retValue);
	//printf("hTmEn: %I64X\r\n",hTmEn_l);
	//ExitProcess(0);
	
	//---------------------- Token ------------------------------------
	HANDLE hToken_l = 0;
	if(!	OpenProcessToken(hProcess_l,TOKEN_ALL_ACCESS,&hToken_l) )
	{
		if(bPrint)	printf("Error OpenProcessToken, %X\r\n",GetLastError());
		return -18;
	}
	hToken = hToken_l;
	
	if(bPrint)	printf("hToken: %I64X\r\n",hToken_l);
	ObjectName_All[0]=0;
	AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hToken_l;
	AllKernelObjectsUsed++;
	//---------------------- Thread Pool ----------------------
	wcscpy(ObjectName_All,FullName);
	LenX = wcslen(ObjectName_All);
	_ultow(Rx,&ObjectName_All[LenX],0x10);
	LenX = wcslen(ObjectName_All);
	Rxx = Rand();
	_ultow(Rxx,&ObjectName_All[LenX],0x10);
	


	if(bPrint)	wprintf(L"ThreadPool: %s\r\n",ObjectName_All);

	_UNICODE_STRING UniSS_tp = {0};
	UniSS_tp.Length = wcslen(ObjectName_All)*2;
	UniSS_tp.MaxLength = UniSS_tp.Length + 2;
	UniSS_tp.Buffer = ObjectName_All;


	_OBJECT_ATTRIBUTES ObjAttrTP={0};
	ObjAttrTP.Length = sizeof(ObjAttrTP);
	ObjAttrTP.Attributes= OBJ_CASE_INSENSITIVE;
	ObjAttrTP.ObjectName = & UniSS_tp;




	HANDLE hTpWorkerFactory_l = 0;

	retValue = ZwCreateWorkerFactory(&hTpWorkerFactory_l,GENERIC_ALL,&ObjAttrTP,hIoCompletion_l,GetCurrentProcess(),TpRoutine,0,1,0x10000,0x1000);
	if(bPrint)	printf("ZwCreateWorkerFactory, ret: %X, hTpWorkerFactory_l: %I64X\r\n",retValue,hTpWorkerFactory_l);
	if(retValue < 0)
	{
		if(bPrint)	printf("Error creating kernel TP Worker Factory\r\n");
		return -20;
	}
	hTpWorkerFactory = hTpWorkerFactory_l;

	if(bPrint)	printf("hTpWorkerFactory: %X\r\n",hTpWorkerFactory_l);
	ObjectName_All[0] = 0;

	AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hTpWorkerFactory_l;
	AllKernelObjectsUsed++;
	//---------------------- UserApcReserve --------------------
	HANDLE hUserApcReserve_l = 0;
	
	retValue = ZwAllocateReserveObject(&hUserApcReserve_l,0,0);
	if(bPrint)	printf("ZwAllocateReserveObject, ret: %X, hUserApcReserve_l: %I64X\r\n",retValue,hUserApcReserve_l);
	if(retValue < 0)
	{
		if(bPrint)	printf("Error creating kernel UserApcReserve\r\n");
		return -19;
	}
	hUserApcReserve = hUserApcReserve_l;

	if(bPrint)	printf("hUserApcReserve: %X\r\n",hUserApcReserve_l);
	ObjectName_All[0] = 0;

	AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hUserApcReserve_l;
	AllKernelObjectsUsed++;
	//--------------------- Window Station ------------- Named WindowStation requires privilege
	//replace with Native call later
	hWinsta = CreateWindowStation(0,0,WINSTA_ALL_ACCESS,0);
	if(!hWinsta)
	{
		if(bPrint)	printf("Error creating WindowStation object\r\n");
		//return -1; //For fuzzing under "NT Authority\SYSTEM"
	}
	else
	{
		if(bPrint)	printf("hWInsta: %I64X\r\n",hWinsta);
		AllKernelObject[AllKernelObjectsUsed] = (ulonglong)hWinsta;
		AllKernelObjectsUsed++;
	}

	
	return 0;
}


void ObjCreatorThread()
{

	while(1)
	{
		Sleep(5000);
		InitKernelObjects(false);
		cached_AllKernelObjectsUsed = AllKernelObjectsUsed;
		cached_AllFilesUsed = AllFilesUsed;
		cached_AllProcessesUsed = AllProcessesUsed;
	}
}

void ObjDestroyerThread()
{
	while(1)
	{
		Sleep(20000);
		ulong var_AllKernelObjectsUsed = AllKernelObjectsUsed;

		ulong i = 0;
		while(i < var_AllKernelObjectsUsed)
		{
			ZwClose( (HANDLE) (AllKernelObject[i]) );
			i++;
		}
	}
}