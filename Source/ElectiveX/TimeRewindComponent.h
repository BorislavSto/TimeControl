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
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ELECTIVEX_API UTimeRewindComponent : public UActorComponent
{
    GENERATED_BODY()

public:    
    UTimeRewindComponent();

    // Maximum number of states to store
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Travel")
    int32 MaxHistoryStates = 250; // Stores up to 4 seconds at 60 FPS

    // How often to record states (in seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Travel")
    float RecordInterval = 0.016f; // Approximately 60 FPS

    // Rewind duration in seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Travel")
    float RewindDuration = 4.0f;

    // Transition time for rewinding
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Travel")
    float RewindTransitionTime = 2.0f;

    UFUNCTION(BlueprintCallable, Category = "Time Travel")
    void StartTimeRewind();

    UFUNCTION(BlueprintCallable, Category = "Time Travel")
    void StopTimeRewind();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // Circular buffer to store object states
    TArray<FTimeState> TimeHistory;

    // Timer to track state recording
    float RecordTimer = 0.0f;

    // Is currently rewinding?
    bool bIsRewinding = false;

    // Current rewind progress
    float RewindProgress = 0.0f;

    // Record the current state of the object
    void RecordState();

    // Interpolate to a previous state
    void InterpolateToState(const FTimeState& TargetState);
};