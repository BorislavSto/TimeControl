#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimeRewindComponent.generated.h"

// Struct to store object state at a specific point in time
USTRUCT(BlueprintType)
struct FTimeState
{
    GENERATED_BODY()

    UPROPERTY()
    FTransform Transform;

    UPROPERTY()
    FVector Velocity;

    UPROPERTY()
    double Timestamp;
    
    UPROPERTY()
    bool bWasMoving;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRewindEvent);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ELECTIVEX_API UTimeRewindComponent : public UActorComponent
{
    GENERATED_BODY()

public:    
    UTimeRewindComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Travel")
    int32 MaxHistoryStates = 250; 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Travel")
    float RecordInterval = 0.016f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Travel")
    float RewindDuration = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Travel")
    float RewindTransitionTime = 2.0f;

    UFUNCTION(BlueprintCallable, Category = "Time Travel")
    void StartTimeRewind();

    UFUNCTION(BlueprintCallable, Category = "Time Travel")
    void StopTimeRewind();

    UPROPERTY(BlueprintAssignable, Category = "Time Rewind")
    FRewindEvent OnRewindStart;

    UPROPERTY(BlueprintAssignable, Category = "Time Rewind")
    FRewindEvent OnRewindStop;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Rewind")
    bool bIsMoving;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    FTimeState* FindStateAtTime(float TargetTime);

private:
    TArray<FTimeState> TimeHistory;
    TArray<FTimeState> OriginalHistory;

    float RecordTimer = 0.0f;
    bool bIsRewinding = false;
    float RewindProgress = 0.0f;

    void RecordState();
    void InterpolateToState(const FTimeState& TargetState);

    double RewindStartTime;
};