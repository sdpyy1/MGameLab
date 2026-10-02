#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "MGLogBlueprintLibrary.generated.h"

UCLASS()
class MGAMELABCORE_API UMGLogBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "MGameLab|Log")
    static void PrintScreenLog(const FString& Message, FLinearColor Color = FLinearColor::White, float Duration = 2.0f);
};
