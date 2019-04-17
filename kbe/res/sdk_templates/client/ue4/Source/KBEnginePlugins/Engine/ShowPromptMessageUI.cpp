#include "ShowPromptMessageUI.h"
#include "Engine.h"
#include "KBEvent.h"
#include "KBEngine.h"
#include "KBEventTypes.h"
#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShowPromptMessageUI::Construct(const FArguments& args)
{
	FTextBlockStyle textStyle = FCoreStyle::Get().GetWidgetStyle< FTextBlockStyle >("NormalText");
	textStyle.SetFont(FSlateFontInfo("Veranda", 26));
	textStyle.SetColorAndOpacity(FLinearColor::White);

	ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.ColorAndOpacity(FLinearColor::Red)
		.ShadowColorAndOpacity(FLinearColor::Black)
		.ShadowOffset(FIntPoint(-1, 1))
		.Font(FSlateFontInfo("Arial", 30))
		.Text(FText::FromString("Please close the editor and recompile!"))
		]
		];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

