
#include "KBEvent.h"
#include "KBDebug.h"

TMap<FString, TArray<KBEvent::EventObj>> KBEvent::events_;

KBEvent::KBEvent()
{
}

KBEvent::~KBEvent()
{
}

void KBEvent::clear()
{
	clearFiredEvents();
}

void KBEvent::clearFiredEvents()
{

}

bool KBEvent::registerEvent(const FString& eventName, const FString& funcName, TFunction<void(const UKBEventData*)> func, void* objPtr)
{
	TArray<EventObj>* eo_array = NULL;
	TArray<EventObj>* eo_array_find = events_.Find(eventName);

	if (!eo_array_find)
	{
		events_.Add(eventName, TArray<EventObj>());
		eo_array = &(*events_.Find(eventName));
	}
	else
	{
		eo_array = &(*eo_array_find);
	}

	EventObj eo;
	eo.funcName = funcName;
	eo.method = func;
	eo.objPtr = objPtr;
	eo_array->Add(eo);
	return true;
}

bool KBEvent::deregister(void* objPtr, const FString& eventName, const FString& funcName)
{
	TArray<EventObj>* eo_array_find = events_.Find(eventName);
	if (!eo_array_find || (*eo_array_find).Num() == 0)
	{
		return false;
	}

	// 从后往前遍历，以避免中途删除的问题
	for (int i = (*eo_array_find).Num() - 1; i >= 0; --i)
	{
		EventObj& item = (*eo_array_find)[i];
		if (objPtr == item.objPtr && (funcName.Len() == 0 || funcName == item.funcName))
		{
			(*eo_array_find).RemoveAt(i, 1);
		}
	}

	return true;
}

bool KBEvent::deregister(void* objPtr)
{
	for (auto& item : events_)
	{
		deregister(objPtr, item.Key, TEXT(""));
	}

	return true;
}

void KBEvent::fire(const FString& eventName, UKBEventData* pEventData)
{
	TArray<EventObj>* eo_array_find = events_.Find(eventName);
	if (!eo_array_find || (*eo_array_find).Num() == 0)
	{
		//SCREEN_WARNING_MSG("KBEvent::fire(): event(%s) not found!", *eventName);
		return;
	}

	pEventData->eventName = eventName;

	for (auto& item : (*eo_array_find))
	{
		item.method(pEventData);
	}

	pEventData->ConditionalBeginDestroy();
	//GetWorld()->ForceGarbageCollection(true);
}