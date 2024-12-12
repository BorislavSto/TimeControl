#include "TimeRewindComponent.h"
#include "GameFramework/Actor.h"

UTimeRewindComponent::UTimeRewindComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UTimeRewindComponent::BeginPlay()
{
    Super::BeginPlay();
    
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("TimeRewindComponent: No owner specified"));
        return;
    }

    TimeHistory.Reserve(MaxHistoryStates);
}

void UTimeRewindComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    AActor* Owner = GetOwner();
    if (!Owner)
        return;

    // Record states
    if (!bIsRewinding)
    {
        RecordTimer += DeltaTime;
        if (RecordTimer >= RecordInterval)
        {
            RecordState();
            RecordTimer = 0.0f;

            // Maintain fixed history size
            if (TimeHistory.Num() > MaxHistoryStates)
            {
                TimeHistory.RemoveAt(0);
            }
        }
    }
    else
    {
        // Rewind logic
        RewindProgress += DeltaTime;
        float NormalizedProgress = FMath::Clamp(RewindProgress / RewindTransitionTime, 0.0f, 1.0f);

        // Find the target state to rewind to
        int32 TargetIndex = FMath::FloorToInt(TimeHistory.Num() * (1.0f - (RewindDuration / (MaxHistoryStates * RecordInterval))));
        
        if (TimeHistory.IsValidIndex(TargetIndex))
        {
            InterpolateToState(TimeHistory[TargetIndex]);
            TimeHistory.Pop(); // Remove the used state
        }

        // Stop rewinding when transition is complete
        if (NormalizedProgress >= 1.0f)
        {
            StopTimeRewind();
        }
    }
}

void UTimeRewindComponent::RecordState()
{
    AActor* Owner = GetOwner();
    if (!Owner)
        return;

    FTimeState NewState;
    NewState.Transform = Owner->GetActorTransform();
    
    // Try to get velocity if the owner is a movable object
    UPrimitiveComponent* PrimitiveComp = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
    if (PrimitiveComp && PrimitiveComp->IsSimulatingPhysics())
    {
        NewState.Velocity = PrimitiveComp->GetPhysicsLinearVelocity();
    }
    
    NewState.Timestamp = GetWorld()->GetTimeSeconds();
    
    TimeHistory.Add(NewState);
}

void UTimeRewindComponent::InterpolateToState(const FTimeState& TargetState)
{
    AActor* Owner = GetOwner();
    if (!Owner)
        return;

    // Interpolate transform
    FTransform CurrentTransform = Owner->GetActorTransform();
    FTransform InterpolatedTransform = FTransform::Identity;
    InterpolatedTransform.Blend(CurrentTransform, TargetState.Transform, RewindProgress / RewindTransitionTime);
    
    Owner->SetActorTransform(InterpolatedTransform);

    // Interpolate velocity for physics objects
    UPrimitiveComponent* PrimitiveComp = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
    if (PrimitiveComp && PrimitiveComp->IsSimulatingPhysics())
    {
        FVector CurrentVelocity = PrimitiveComp->GetPhysicsLinearVelocity();
        FVector InterpolatedVelocity = FMath::Lerp(CurrentVelocity, TargetState.Velocity, RewindProgress / RewindTransitionTime);
        
        PrimitiveComp->SetPhysicsLinearVelocity(InterpolatedVelocity);
    }
}

void UTimeRewindComponent::StartTimeRewind()
{
    if (!bIsRewinding)
    {
        bIsRewinding = true;
        RewindProgress = 0.0f;
    }
}

void UTimeRewindComponent::StopTimeRewind()
{
    bIsRewinding = false;
    RewindProgress = 0.0f;
}