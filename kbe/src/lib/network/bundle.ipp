namespace KBEngine { 
namespace Mercury
{

INLINE bool Bundle::isEmpty() const
{
	return size() == 0;
}

INLINE int Bundle::size() const
{
	return packets_.size();
}

INLINE int Bundle::sizeInPackets()
{
	return packets_.size();
}

}
}
