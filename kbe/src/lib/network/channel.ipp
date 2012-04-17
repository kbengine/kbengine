namespace KBEngine { 
namespace Mercury
{

INLINE const Address & Channel::addr() const
{
	return pSocket_->addr();
}

INLINE const Socket * Channel::socket() const
{
	return pSocket_;
}

INLINE int Channel::windowSize() const
{
	return windowSize_;
}

}
}
