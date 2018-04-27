// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


namespace KBEngine {
namespace script{

INLINE MemoryStream& PyMemoryStream::stream()
{ 
	return stream_;
}

INLINE PyObject* PyMemoryStream::pyBytes()
{
	if(stream_.size() == 0)
	{
		return PyBytes_FromString("");
	}
	
	return PyBytes_FromStringAndSize((char*)stream_.data(), stream_.size());
}

INLINE int PyMemoryStream::length(void) const
{ 
	return (int)stream_.size();
}

INLINE bool PyMemoryStream::readonly() const
{
	return readonly_; 
}

}
}

