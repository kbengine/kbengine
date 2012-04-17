namespace KBEngine { 
namespace Mercury
{

INLINE const Address & Channel::addr() const
{
	return socket_->addr();
}

INLINE const Socket * Channel::socket() const
{
	return socket_;
}

INLINE int Channel::windowSize() const
{
	return windowSize_;
}

}
}
