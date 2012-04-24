#include "helper/debug_helper.hpp"
#include "serverinfos.hpp"

namespace KBEngine{
KBE_SINGLETON_INIT(ServerInfos);

//-------------------------------------------------------------------------------------
ServerInfos::ServerInfos()
{
#if KBE_PLATFORM == PLATFORM_WIN32

	serverName_ = "Not yet implemented";

	cpuInfo_ = "Not yet implemented";
	memUsed_ = 0;
	memTotal_ = 0;

	cpuSpeeds_.push_back( 1.0 );

#else 

	char hostName[ 128 ];
	gethostname(hostName, 128);
	hostName[ 128 - 1 ] = '\0';

	char* firstDot = strchr(hostName, '.');
	if ( firstDot != NULL )
	{
		*firstDot = '\0';
	}
	serverName_ = hostName;

	fetchLinuxCpuInfo();
	fetchLinuxMemInfo();

#endif

	std::stringstream memInfoBuf;
	memInfoBuf << ( memTotal_ >> 10 ) << "kB";
	memInfo_ = memInfoBuf.str();
}

//-------------------------------------------------------------------------------------
void ServerInfos::updateMem()
{
#if KBE_PLATFORM != PLATFORM_WIN32
	fetchLinuxMemInfo();
#endif 
}

//-------------------------------------------------------------------------------------
#if KBE_PLATFORM != PLATFORM_WIN32
void ServerInfos::fetchLinuxCpuInfo()
{
	// /proc/cpuinfo format assumption
	// Each line is either blank, or is 'label\t+: Value'

	FILE* pProcCpu;
	char procLine[BUFSIZ];
	char* pInterest;
	float mhz;
	typedef std::tr1::unordered_map<std::string, unsigned int> modelNameMap;
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
void ServerInfos::fetchLinuxMemInfo()
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


#endif 
}
