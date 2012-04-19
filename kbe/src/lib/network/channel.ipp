namespace KBEngine { 
namespace Mercury
{

INLINE const Address & Channel::addr() const
{
	return pEndPoint_->addr();
}

INLINE EndPoint * Channel::endpoint() const
{
	return pEndPoint_;
}

INLINE int Channel::windowSize() const
{
	return windowSize_;
}

}
}
