#pragma once
#include "CoreMinimal.h"
#include "SEditorViewport.h"
#include "PreviewScene.h"
#include "Containers/Ticker.h"

class FPreviewScene;
class FMyViewportClient;

class S3DViewportWidget : public SEditorViewport		// Slate 控件类 
{
public:
	SLATE_BEGIN_ARGS(S3DViewportWidget) {}
	SLATE_END_ARGS()
	void Construct(const FArguments& InArgs);
	virtual ~S3DViewportWidget();
	
	void UpdatePreviewActor(UClass* InActorClass);				// 更新窗口里的Actor实例
	void UpdatePreviewStaticMesh(UStaticMesh* StaticMesh);		// 更新窗口里的Mesh实例
	void UpdatePreviewMaterial(UMaterialInterface* Material, const FString& Shape);

protected:
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;		// 视口控制器
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override { return nullptr; }		// 视口UI

private:
	TSharedPtr<FPreviewScene> PreviewScene;
	TSharedPtr<FMyViewportClient> ViewportClient;
	AActor* PreviewActor = nullptr;

	TWeakObjectPtr<UObject> LastPreviewedAsset;			// 记录上次预览的Actor实例，避免重复假装
	FTSTicker::FDelegateHandle TickDelegateHandle;		// 用于注销 Tick 的句柄
	bool HandleTick(float DeltaTime);					// Ticker 回调
};
