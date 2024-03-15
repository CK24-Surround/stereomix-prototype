
#pragma once

#include "CoreMinimal.h"
#include "SMCharacterStat.generated.h"

USTRUCT(BlueprintType)
struct FSMCharacterStat
{
	GENERATED_BODY()

public:
	FSMCharacterStat() : MaxPostureGauge(100.0f) {}

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	float MaxPostureGauge;
};
