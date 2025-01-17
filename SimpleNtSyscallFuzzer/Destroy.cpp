#include "stdafx.h"
#include "Header.h"

extern ulong OSVer;
extern char* NtosCalls[0x1000];
extern ulong bMode;


void CleanArguments(unsigned long SysCall,unsigned long long* Args,void** pPool,void** pSecondLevelPool)
{
	void* p = 0;

	/*
	if(SysCall == 0x104A) //NtUserEnumDisplayMonitors
	{
		if(Args[2])
		{
			DestroyDummyExecutablePage((void*)(Args[2]));
		}
	}
	*/






	/*
	else if(SysCall== 0x1321) //NtUserSfmDxSetSwapChainBindingStatus
	{
		if(Args[0])
		{
			p = *(void**)(Args[0]);
			if(p)
			{
				DestroyRandomPage(p);
			}
		}
	}
	else if(SysCall== 0x131F) //NtUserSfmDxReleaseSwapChain
	{
		if(Args[0])
		{
			p = *(void**)(Args[0]);
			if(p)
			{
				DestroyRandomPage(p);
			}
		}
	}
	else if(SysCall== 0x131E) //NtUserSfmDxQuerySwapChainBindingStatus
	{
		if(Args[0])
		{
			p = *(void**)(Args[0]);
			if(p)
			{
				DestroyRandomPage(p);
			}
		}
	}*/


	unsigned long Count = MAX_ARGS;
	if( ShouldSkipLeakChecking(SysCall) == false)
	{
		unsigned long i=0;

		bool bRet0,bRet1;

		for(i=0;i<Count;i++)
		{
			if(pPool[i])
			{
				//bRet0 = Scan((void*)(pPool[i]),RANDOM_PAGE_SIZE,OSVer);
				bRet0 = ScanCanonical((void*)(pPool[i]),RANDOM_PAGE_SIZE);
				if(bRet0)
				{
					printf("Kernel Address leak found, Syscall: %X, Arg# %X\r\n",SysCall,i);
					DumpHex_8(pPool[i],0x1000);
					printf("Press any key to continue\r\n");
					getchar();
					FillClassicRandomData((void*)(pPool[i]),RANDOM_PAGE_SIZE,bMode);
				}
			}

			if(pSecondLevelPool[i])
			{
				bRet1 = Scan((void*)(pSecondLevelPool[i]),RANDOM_PAGE_SIZE,OSVer);
				if(bRet1)
				{
					printf("Kernel Address leak found, Syscall: %X, Arg# %X (Second Level Pool)\r\n",SysCall,i);
					DumpHex_8(pPool[i],0x1000);
					printf("Press any key to continue\r\n");
					getchar();
					FillClassicRandomData((void*)(pSecondLevelPool[i]),RANDOM_PAGE_SIZE,bMode);
				}
			}
		}
	}
	else
	{
		for(ulong i=0;i<Count;i++)
		{
			if(pPool[i])
			{
				FillClassicRandomData((void*)(pPool[i]),RANDOM_PAGE_SIZE,0);
			}

			if(pSecondLevelPool[i])
			{
				FillClassicRandomData((void*)(pSecondLevelPool[i]),RANDOM_PAGE_SIZE,0);
			}
		}
	}

}