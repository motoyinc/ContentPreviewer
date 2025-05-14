// Copyright Epic Games, Inc. All Rights Reserved.

#include "ContentPreviewer.h"
#include "ContentPreviewerStyle.h"
#include "ContentPreviewerCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"

static const FName ContentPreviewerTabName("ContentPreviewer");

#define LOCTEXT_NAMESPACE "FContentPreviewerModule"

void FContentPreviewerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FContentPreviewerStyle::Initialize();
	FContentPreviewerStyle::ReloadTextures();

	FContentPreviewerCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FContentPreviewerCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FContentPreviewerModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FContentPreviewerModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(ContentPreviewerTabName, FOnSpawnTab::CreateRaw(this, &FContentPreviewerModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FContentPreviewerTabTitle", "ContentPreviewer"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FContentPreviewerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FContentPreviewerStyle::Shutdown();

	FContentPreviewerCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ContentPreviewerTabName);
}

TSharedRef<SDockTab> FContentPreviewerModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FContentPreviewerModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("ContentPreviewer.cpp"))
		);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(WidgetText)
			]
		];
}

void FContentPreviewerModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(ContentPreviewerTabName);
}

void FContentPreviewerModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FContentPreviewerCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FContentPreviewerCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FContentPreviewerModule, ContentPreviewer)