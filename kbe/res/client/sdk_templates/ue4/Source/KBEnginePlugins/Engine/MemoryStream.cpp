
#include "MemoryStream.h"
#include "KBDebug.h"

MemoryStream* MemoryStream::createObject()
{
	return new MemoryStream();
}

void MemoryStream::reclaimObject(MemoryStream* obj)
{
	delete obj;
}

void MemoryStream::print_storage()
{
	FString fbuffer;
	uint32 trpos = rpos_;

	DEBUG_MSG("MemoryStream::print_storage(): STORAGE_SIZE: %lu, rpos=%lu.", (unsigned long)wpos(), (unsigned long)rpos());

	for (uint32 i = rpos(); i < wpos(); ++i)
	{
		fbuffer.AppendInt((int)read<uint8>(i));
		fbuffer += TEXT(" ");
	}

	DEBUG_MSG("%s", *fbuffer);

	rpos_ = trpos;
}