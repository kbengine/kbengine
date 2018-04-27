// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "helper/debug_helper.h"
#include "machine_infos.h"

#if KBE_PLATFORM == PLATFORM_WIN32
#include <intrin.h>
const char* szFeatures[] =
{
    "x87 FPU On Chip",
    "Virtual-8086 Mode Enhancement",
    "Debugging Extensions",
    "Page Size Extensions",
    "Time Stamp Counter",
    "RDMSR and WRMSR Support",
    "Physical Address Extensions",
    "Machine Check Exception",
    "CMPXCHG8B Instruction",
    "APIC On Chip",
    "Unknown1",
    "SYSENTER and SYSEXIT",
    "Memory Type Range Registers",
    "PTE Global Bit",
    "Machine Check Architecture",
    "Conditional Move/Compare Instruction",
    "Page Attribute Table",
    "Page Size Extension",
    "Processor Serial Number",
    "CFLUSH Extension",
    "Unknown2",
    "Debug Store",
    "Thermal Monitor and Clock Ctrl",
    "MMX Technology",
    "FXSAVE/FXRSTOR",
    "SSE Extensions",
    "SSE2 Extensions",
    "Self Snoop",
    "Hyper-threading Technology",
    "Thermal Monitor",
    "Unknown4",
    "Pend. Brk. EN."
};
#endif

namespace KBEngine{
KBE_SINGLETON_INIT(MachineInfos);

//-------------------------------------------------------------------------------------
MachineInfos::MachineInfos()
{
#if KBE_PLATFORM == PLATFORM_WIN32
	machineName_ = getUsername();
	fetchWindowsCpuInfo();
	fetchWindowsMemInfo();
#else 

	char hostName[ 128 ];
	gethostname(hostName, 128);
	hostName[ 128 - 1 ] = '\0';

	char* firstDot = strchr(hostName, '.');
	if ( firstDot != NULL )
	{
		*firstDot = '\0';
	}

	machineName_ = hostName;

	fetchLinuxCpuInfo();
	fetchLinuxMemInfo();

#endif

	std::stringstream memInfoBuf;
	memInfoBuf << ( memTotal_ >> 10 ) << "kB";
	memInfo_ = memInfoBuf.str();
}

//-------------------------------------------------------------------------------------
void MachineInfos::updateMem()
{
#if KBE_PLATFORM != PLATFORM_WIN32
	fetchLinuxMemInfo();
#endif 
}

//-------------------------------------------------------------------------------------
#if KBE_PLATFORM != PLATFORM_WIN32
void MachineInfos::fetchLinuxCpuInfo()
{
	// /proc/cpuinfo format assumption
	// Each line is either blank, or is 'label\t+: Value'

	FILE* pProcCpu;
	char procLine[BUFSIZ];
	char* pInterest;
	float mhz;
	typedef KBEUnordered_map<std::string, unsigned int> modelNameMap;
	modelNameMap modelNames;

	pProcCpu = fopen( "/proc/cpuinfo", "r" );
	if ( pProcCpu == NULL )
	{
		return;
	}

	while ( fgets( procLine, BUFSIZ, pProcCpu ) != NULL )
	{
		pInterest = strchr( procLine, '\n' );
		if ( pInterest != NULL )
		{
			*pInterest = '\0';
		}
		pInterest = strchr( procLine, '\r' );
		if ( pInterest != NULL )
		{
			*pInterest = '\0';
		}

		if ( procLine[ 0 ] != '\0' )
		{
			pInterest = strchr( procLine, '\t' );
			if ( pInterest == NULL )
			{
				continue;
			}
			*pInterest = '\0';
			pInterest++;
			pInterest = strchr( pInterest, ':' );
			if ( pInterest == NULL )
			{
				continue;
			}
			pInterest += 2;

			if (!strcmp( procLine, "cpu MHz" ) )
			{
				sscanf( pInterest, "%f", &mhz );
				cpuSpeeds_.push_back( mhz );
			}
			else
			if ( !strcmp( procLine, "model name" ) )
			{
				modelNames[ pInterest ] += 1;
			}
		}
	}

	fclose( pProcCpu );

	std::stringstream cpuInfoBuf;

	modelNameMap::const_iterator modelNamesIt;
	modelNameMap::const_iterator modelNamesEnd = modelNames.end();

	bool first = true;

	for ( modelNamesIt = modelNames.begin(); modelNamesIt != modelNamesEnd;
		++modelNamesIt )
	{
		if ( first )
		{
			first = false;
		}
		else
		{
			cpuInfoBuf << ", ";
		}

		if ( modelNamesIt->second == 1 )
		{
			cpuInfoBuf << modelNamesIt->first;
		}
		else
		{
			cpuInfoBuf << modelNamesIt->first << " x" << modelNamesIt->second;
		}
	}

	cpuInfo_ = cpuInfoBuf.str();

}

//-------------------------------------------------------------------------------------
void MachineInfos::fetchLinuxMemInfo()
{
	unsigned long memTotal, memFree, buffers, cached, slab;
	FILE* pProcMem;
	char procLine[BUFSIZ];

	pProcMem = fopen( "/proc/meminfo", "r" );
	if ( pProcMem == NULL )
	{
		return;
	}

#define INTERESTING_VALUES 5
	int i = 0;
	while ( fgets( procLine, BUFSIZ, pProcMem ) != NULL )
	{
		if (sscanf( procLine, "MemTotal: %lu kB", &memTotal ) == 1 )
		{
			i++;
		}
		else if (sscanf( procLine, "MemFree: %lu kB", &memFree ) == 1 )
		{
			i++;
		}
		else if (sscanf( procLine, "Buffers: %lu kB", &buffers ) == 1 )
		{
			i++;
		}
		else if (sscanf( procLine, "Cached: %lu kB", &cached ) == 1 )
		{
			i++;
		}
		else if (sscanf( procLine, "Slab: %lu kB", &slab ) == 1 )
		{
			i++;
		}

		if ( i >= INTERESTING_VALUES )
		{
			break;
		}
	}

	fclose( pProcMem );

	memTotal_ = memTotal << 10;
	memUsed_ = ( memTotal - memFree - buffers - cached - slab ) << 10;
}
#else
//-------------------------------------------------------------------------------------
void MachineInfos::fetchWindowsCpuInfo()
{
	cpuInfo_ = "Not yet implemented";
	cpuSpeeds_.push_back( 1.0 );

	char CPUString[0x20];
    char CPUBrandString[0x40];
    int CPUInfo[4] = {-1};
    int nSteppingID = 0;
    int nModel = 0;
    int nFamily = 0;
    int nProcessorType = 0;
    int nExtendedmodel = 0;
    int nExtendedfamily = 0;
    int nBrandIndex = 0;
    int nCLFLUSHcachelinesize = 0;
    int nAPICPhysicalID = 0;
    int nFeatureInfo = 0;
    int nCacheLineSize = 0;
    int nL2Associativity = 0;
    int nCacheSizeK = 0;
    int nRet = 0;
    unsigned    nIds, nExIds, i;
    bool    bSSE3NewInstructions = false;
    bool    bMONITOR_MWAIT = false;
    bool    bCPLQualifiedDebugStore = false;
    bool    bThermalMonitor2 = false;


    // __cpuid with an InfoType argument of 0 returns the number of
    // valid Ids in CPUInfo[0] and the CPU identification string in
    // the other three array elements. The CPU identification string is
    // not in linear order. The code below arranges the information 
    // in a human readable form.
    __cpuid(CPUInfo, 0);
    nIds = CPUInfo[0];
    memset(CPUString, 0, sizeof(CPUString));
    *((int*)CPUString) = CPUInfo[1];
    *((int*)(CPUString+4)) = CPUInfo[3];
    *((int*)(CPUString+8)) = CPUInfo[2];
	
    // Get the information associated with each valid Id
    for (i=0; i<=nIds; ++i)
    {
        __cpuid(CPUInfo, i);
        //printf_s("\nFor InfoType %d\n", i); 
        //printf_s("CPUInfo[0] = 0x%x\n", CPUInfo[0]);
        //printf_s("CPUInfo[1] = 0x%x\n", CPUInfo[1]);
        //printf_s("CPUInfo[2] = 0x%x\n", CPUInfo[2]);
        //printf_s("CPUInfo[3] = 0x%x\n", CPUInfo[3]);

        // Interpret CPU feature information.
        if  (i == 1)
        {
            nSteppingID = CPUInfo[0] & 0xf;
            nModel = (CPUInfo[0] >> 4) & 0xf;
            nFamily = (CPUInfo[0] >> 8) & 0xf;
            nProcessorType = (CPUInfo[0] >> 12) & 0x3;
            nExtendedmodel = (CPUInfo[0] >> 16) & 0xf;
            nExtendedfamily = (CPUInfo[0] >> 20) & 0xff;
            nBrandIndex = CPUInfo[1] & 0xff;
            nCLFLUSHcachelinesize = ((CPUInfo[1] >> 8) & 0xff) * 8;
            nAPICPhysicalID = (CPUInfo[1] >> 24) & 0xff;
            bSSE3NewInstructions = (CPUInfo[2] & 0x1) || false;
            bMONITOR_MWAIT = (CPUInfo[2] & 0x8) || false;
            bCPLQualifiedDebugStore = (CPUInfo[2] & 0x10) || false;
            bThermalMonitor2 = (CPUInfo[2] & 0x100) || false;
            nFeatureInfo = CPUInfo[3];
        }
    }

    // Calling __cpuid with 0x80000000 as the InfoType argument
    // gets the number of valid extended IDs.
    __cpuid(CPUInfo, 0x80000000);
    nExIds = CPUInfo[0];
    memset(CPUBrandString, 0, sizeof(CPUBrandString));

    // Get the information associated with each extended ID.
    for (i=0x80000000; i<=nExIds; ++i)
    {
        __cpuid(CPUInfo, i);
        //printf_s("\nFor InfoType %x\n", i); 
        //printf_s("CPUInfo[0] = 0x%x\n", CPUInfo[0]);
        //printf_s("CPUInfo[1] = 0x%x\n", CPUInfo[1]);
        //printf_s("CPUInfo[2] = 0x%x\n", CPUInfo[2]);
        //printf_s("CPUInfo[3] = 0x%x\n", CPUInfo[3]);

        // Interpret CPU brand string and cache information.
        if  (i == 0x80000002)
            memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
        else if  (i == 0x80000003)
            memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
        else if  (i == 0x80000004)
            memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
        else if  (i == 0x80000006)
        {
            nCacheLineSize = CPUInfo[2] & 0xff;
            nL2Associativity = (CPUInfo[2] >> 12) & 0xf;
            nCacheSizeK = (CPUInfo[2] >> 16) & 0xffff;
        }
    }

    // Display all the information in user-friendly format.
	/*
    printf_s("\n\nCPU String: %s\n", CPUString);

    if  (nIds >= 1)
    {
        if  (nSteppingID)
            printf_s("Stepping ID = %d\n", nSteppingID);
        if  (nModel)
            printf_s("Model = %d\n", nModel);
        if  (nFamily)
            printf_s("Family = %d\n", nFamily);
        if  (nProcessorType)
            printf_s("Processor Type = %d\n", nProcessorType);
        if  (nExtendedmodel)
            printf_s("Extended model = %d\n", nExtendedmodel);
        if  (nExtendedfamily)
            printf_s("Extended family = %d\n", nExtendedfamily);
        if  (nBrandIndex)
            printf_s("Brand Index = %d\n", nBrandIndex);
        if  (nCLFLUSHcachelinesize)
            printf_s("CLFLUSH cache line size = %d\n",
                     nCLFLUSHcachelinesize);
        if  (nAPICPhysicalID)
            printf_s("APIC Physical ID = %d\n", nAPICPhysicalID);

		if  (nFeatureInfo || bSSE3NewInstructions ||
             bMONITOR_MWAIT || bCPLQualifiedDebugStore ||
             bThermalMonitor2)
		{
            printf_s("\nThe following features are supported:\n");

			if  (bSSE3NewInstructions)
			printf_s("\tSSE3 New Instructions\n");
			if  (bMONITOR_MWAIT)
			printf_s("\tMONITOR/MWAIT\n");
			if  (bCPLQualifiedDebugStore)
			printf_s("\tCPL Qualified Debug Store\n");
			if  (bThermalMonitor2)
			printf_s("\tThermal Monitor 2\n");

            i = 0;
            nIds = 1;
            while (i < (sizeof(szFeatures)/sizeof(const char*)))
            {
                if  (nFeatureInfo & nIds)
                {
                    printf_s("\t");
                    printf_s(szFeatures[i]);
                    printf_s("\n");
                }

                nIds <<= 1;
                ++i;
            }
        }
    }

    if  (nExIds >= 0x80000004)
        printf_s("\nCPU Brand String: %s\n", CPUBrandString);

    if  (nExIds >= 0x80000006)
    {
        printf_s("Cache Line Size = %d\n", nCacheLineSize);
        printf_s("L2 Associativity = %d\n", nL2Associativity);
        printf_s("Cache Size = %dK\n", nCacheSizeK);
    }
	*/

	cpuInfo_ = CPUBrandString;
}

//-------------------------------------------------------------------------------------
void MachineInfos::fetchWindowsMemInfo()
{
	memUsed_ = 0;
	memTotal_ = 0;

	MEMORYSTATUS memstatus;
	GlobalMemoryStatus(&memstatus);

	memTotal_ = memstatus.dwTotalPhys;
	memUsed_ = memstatus.dwMemoryLoad;
}

#endif 

}
