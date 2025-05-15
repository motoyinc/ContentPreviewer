
#include "S3DViewportWidget.h"
#include "EditorViewportClient.h"
#include "Engine/World.h"
#include "Engine/Blueprint.h"
#include "Editor.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Containers/Ticker.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInstanceConstant.h"

/// ----------------------CLASS: FMyViewportClient------------------------------------------------------------------ ///
class FMyViewportClient : public FEditorViewportClient
{
public:
	FMyViewportClient(FPreviewScene* InPreviewScene, const TSharedRef<SEditorViewport>& InViewport)
		: FEditorViewportClient(nullptr, InPreviewScene, InViewport)
	{
		SetViewLocation(FVector(0, -300, 150));
		SetViewRotation(FRotator(-10, 0, 0));
		ViewportType = LVT_Perspective;
		SetViewModes(VMI_Lit, VMI_Lit);
		bSetListenerPosition = false;
	}
};



/// ----------------------CLASS: S3DViewportWidget------------------------------------------------------------------ ///

// 初始化控件
void S3DViewportWidget::Construct(const FArguments& InArgs)
{
	PreviewScene = MakeShared<FPreviewScene>(FPreviewScene::ConstructionValues());
	SEditorViewport::Construct(SEditorViewport::FArguments());

	// 注册 FTSTicker 监听蓝图资源变化
	TickDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateRaw(this, &S3DViewportWidget::HandleTick),		// 将 HandleTick() 方法注册为Ticker回调函数
		0.25f																				// 每 0.25s 执行一次回调函数
	);
}

// 析构
S3DViewportWidget::~S3DViewportWidget()
{
	if (PreviewActor && PreviewActor->IsValidLowLevel())
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}

	if (TickDelegateHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);					// 清理 Ticker 委托
	}
}

// 初始化Viewport窗口
TSharedRef<FEditorViewportClient> S3DViewportWidget::MakeEditorViewportClient()
{
	ViewportClient = MakeShared<FMyViewportClient>(PreviewScene.Get(), SharedThis(this));
	ViewportClient->SetViewLocation(FVector(0, -300, 150));
	ViewportClient->SetViewRotation(FRotator(-15, 0, 0));
	return ViewportClient.ToSharedRef();
}


// 控件主函数
bool S3DViewportWidget::HandleTick(float DeltaTime)
{
	TArray<FAssetData> SelectedAssets;
	FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser")
		.Get().GetSelectedAssets(SelectedAssets);

	if (SelectedAssets.Num() == 1)
	{
		UObject* Asset = SelectedAssets[0].GetAsset();

		if (UBlueprint* BP = Cast<UBlueprint>(Asset))
		{
			UClass* Class = BP->GeneratedClass;
			if (Class && Class->IsChildOf(AActor::StaticClass()) && Class != LastPreviewedAsset.Get())
			{
				LastPreviewedAsset = Class;
				UpdatePreviewActor(Class);
			}
		}
		else if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(Asset))
		{
			if (LastPreviewedAsset.Get() != StaticMesh)
			{
				LastPreviewedAsset = StaticMesh;
				UpdatePreviewStaticMesh(StaticMesh);
			}
		}
		else if (USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(Asset))
		{
			// TODO: 实现 SkeletalMesh 预览（创建 Actor 并附上 SkeletalMeshComponent）
		}
		
		else if (UMaterialInterface* Mat = Cast<UMaterialInterface>(Asset))
		{
			if (LastPreviewedAsset.Get() != Mat)
			{
				LastPreviewedAsset = Mat;
				if (UMaterial* BaseMat = Cast<UMaterial>(Mat))
				{
					UMaterialInstanceConstant* TempInstance = NewObject<UMaterialInstanceConstant>(GetTransientPackage(), NAME_None, RF_Transient);
					TempInstance->SetParentEditorOnly(BaseMat);
					Mat = TempInstance;
				}
				UpdatePreviewMaterial(Mat, TEXT("Sphere"));
			}
		}

		else if (UTexture2D* Texture = Cast<UTexture2D>(Asset))
		{
			// TODO: 实现贴图预览（用一个平面或 UI 显示）
		}
	}

	return true;
}


// 更新视口
void S3DViewportWidget::UpdatePreviewActor(UClass* InActorClass)
{
	if (!PreviewScene || !InActorClass || !InActorClass->IsChildOf(AActor::StaticClass()))
	{
		return;
	}

	// 清除旧的 Actor
	if (PreviewActor && PreviewActor->IsValidLowLevel())
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}

	// 生成新的预览 Actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.bHideFromSceneOutliner = true;
	SpawnParams.bTemporaryEditorActor = true;
	UWorld* World = PreviewScene->GetWorld();
	PreviewActor = World->SpawnActor<AActor>(InActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	// 自动聚焦视口
	if (PreviewActor)
	{
		FBox Bounds = PreviewActor->GetComponentsBoundingBox();
		ViewportClient->FocusViewportOnBox(Bounds);
	}
}

void S3DViewportWidget::UpdatePreviewStaticMesh(UStaticMesh* StaticMesh)
{
	if (!StaticMesh || !PreviewScene)
	{
		return;
	}

	if (PreviewActor && PreviewActor->IsValidLowLevel())
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}

	UWorld* World = PreviewScene->GetWorld();
	AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass());
	MeshActor->SetActorLocation(FVector::ZeroVector);
	MeshActor->GetStaticMeshComponent()->SetStaticMesh(StaticMesh);
	MeshActor->SetActorScale3D(FVector(1));

	PreviewActor = MeshActor;

	FBox Bounds = MeshActor->GetComponentsBoundingBox();
	ViewportClient->FocusViewportOnBox(Bounds);
}

void S3DViewportWidget::UpdatePreviewMaterial(UMaterialInterface* Material, const FString& Shape)
{
	if (!Material || !PreviewScene) return;
	if (PreviewActor && PreviewActor->IsValidLowLevel())
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}

	FString ShapePath;
	if (Shape == "Sphere")
	{
		ShapePath = TEXT("/Engine/BasicShapes/Sphere.Sphere");
	}
	else if (Shape == "Plane")
	{
		ShapePath = TEXT("/Engine/BasicShapes/Plane.Plane");
	}
	else if (Shape == "Cube")
	{
		ShapePath = TEXT("/Engine/BasicShapes/Cube.Cube");
	}
	else
	{
		ShapePath = TEXT("/Engine/BasicShapes/Sphere.Sphere");
	}

	UStaticMesh* PreviewMesh = LoadObject<UStaticMesh>(nullptr, *ShapePath);
	if (!PreviewMesh) return;

	UWorld* World = PreviewScene->GetWorld();
	PreviewActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass());
	AStaticMeshActor* StaticActor = Cast<AStaticMeshActor>(PreviewActor);
	StaticActor->GetStaticMeshComponent()->SetStaticMesh(PreviewMesh);
	StaticActor->GetStaticMeshComponent()->SetMaterial(0, Material);
	StaticActor->SetActorScale3D(FVector(1));


	ViewportClient->FocusViewportOnBox(StaticActor->GetComponentsBoundingBox());

	FBox Bounds = PreviewActor->GetComponentsBoundingBox();
	FVector Center = Bounds.GetCenter();
	ViewportClient->bUsingOrbitCamera = true;
	ViewportClient->SetViewLocationForOrbiting(Center + FVector(0, -300, 150));
	ViewportClient->SetLookAtLocation(Center);
}