
#include "EntityComponent.h"
#include "KBEngine.h"
#include "KBDebug.h"
#include "MemoryStream.h"

namespace KBEngine
{

EntityComponent::EntityComponent():
entityComponentPropertyID(0),
componentType(0),
ownerID(0),
pOwner(NULL)
{
}

EntityComponent::~EntityComponent()
{
}

void EntityComponent::createFromStream(MemoryStream& stream)
{
	componentType = (uint16)stream.readInt32();
	ownerID = stream.readInt32();

	//UInt16 ComponentDescrsType;
	stream.readUint16();

	uint16 count = stream.readUint16();

	if(count > 0)
		onUpdatePropertys(0, stream, count);
}

}
