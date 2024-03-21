

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SMPlayerControllerAssetData.generated.h"

/**
 * 
 */
UCLASS()
class STEREOMIXPROTOTYPE_API USMPlayerControllerAssetData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> BattleHUDClass;
};
