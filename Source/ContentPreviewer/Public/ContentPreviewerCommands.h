// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"
#include "ContentPreviewerStyle.h"

class FContentPreviewerCommands : public TCommands<FContentPreviewerCommands>
{
public:

	FContentPreviewerCommands()
		: TCommands<FContentPreviewerCommands>(TEXT("ContentPreviewer"), NSLOCTEXT("Contexts", "ContentPreviewer", "ContentPreviewer Plugin"), NAME_None, FContentPreviewerStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};