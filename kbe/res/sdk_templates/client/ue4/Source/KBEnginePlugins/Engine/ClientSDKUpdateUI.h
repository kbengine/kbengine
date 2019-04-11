#pragma once

#include "KBEnginePluginsPrivatePCH.h"


class KBENGINEPLUGINS_API SClientSDKUpdateUI : public SCompoundWidget
{

public:
	SLATE_BEGIN_ARGS(SClientSDKUpdateUI){}
	
	SLATE_END_ARGS()

	void Construct(const FArguments& args);

	FReply UpdateSDKClicked();


};
