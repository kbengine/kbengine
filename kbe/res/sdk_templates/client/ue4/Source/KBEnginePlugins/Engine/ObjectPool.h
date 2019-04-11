#pragma once

namespace KBEngine
{

template<typename T>
class ObjectPool
{
public:
	typedef TDoubleLinkedList< T* > OBJECT_LIST;
	typedef typename TDoubleLinkedList< T* >::TDoubleLinkedListNode OBJECT_NODE;

	~ObjectPool()
	{
		while (objects_.Num() > 0)
		{
			OBJECT_NODE* node = objects_.GetHead();
			T* t = node->GetValue();
			delete t;
			objects_.RemoveNode(node);
		}
	}

	T* createObject()
	{
		if (objects_.Num() > 0)
		{
			OBJECT_NODE* node = objects_.GetHead();
			T* t = node->GetValue();
			objects_.RemoveNode(node);
			return t;
		} 
		else
		{
			T* t = new T();
			return t;
		}
	}

	void reclaimObject(T* obj)
	{
		objects_.AddTail(obj);
	}

private:
	OBJECT_LIST objects_;
};

}