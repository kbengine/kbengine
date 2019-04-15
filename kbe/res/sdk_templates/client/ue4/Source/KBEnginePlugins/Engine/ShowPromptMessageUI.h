#pragma once

#include "KBEnginePluginsPrivatePCH.h"


class KBENGINEPLUGINS_API SShowPromptMessageUI : public SCompoundWidget
{

public:
	SLATE_BEGIN_ARGS(SShowPromptMessageUI){}
	
	SLATE_END_ARGS()

	void Construct(const FArguments& args);

};
