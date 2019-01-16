#pragma once

template<typename T>
class ObjectPool
{
public:
	~ObjectPool()
	{
		while (objects_.Num() > 0)
		{
			TDoubleLinkedList< T* >::TDoubleLinkedListNode* node = objects_.GetHead();
			T* t = node->GetValue();
			delete t;
			objects_.RemoveNode(node);
		}
	}

	T* createObject()
	{
		if (objects_.Num() > 0)
		{
			TDoubleLinkedList< T* >::TDoubleLinkedListNode* node = objects_.GetHead();
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
	TDoubleLinkedList< T* > objects_;
};
