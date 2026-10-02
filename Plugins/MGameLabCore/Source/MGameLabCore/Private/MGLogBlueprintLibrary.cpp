#include "MGLogBlueprintLibrary.h"

#include "Engine/Engine.h"

void UMGLogBlueprintLibrary::PrintScreenLog(const FString& Message, FLinearColor Color, float Duration)
{
    UE_LOG(LogTemp, Log, TEXT("%s"), *Message);
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, Duration, Color.ToFColor(true), Message);
    }
}
