namespace KBEngine { 
namespace Mercury
{

INLINE bool Bundle::isEmpty() const
{
	return totalSize() == 0;
}

INLINE int Bundle::totalSize() const
{
	return packets_.size();
}

INLINE int Bundle::sizeInPackets()
{
	return packets_.size();
}

}
}
