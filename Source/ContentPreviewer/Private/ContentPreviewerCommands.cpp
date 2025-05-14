// Copyright Epic Games, Inc. All Rights Reserved.

#include "ContentPreviewerCommands.h"

#define LOCTEXT_NAMESPACE "FContentPreviewerModule"

void FContentPreviewerCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "ContentPreviewer", "Bring up ContentPreviewer window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
