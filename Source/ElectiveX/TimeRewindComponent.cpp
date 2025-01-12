#include "TimeRewindComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"

float RewindHistoryDuration;

UTimeRewindComponent::UTimeRewindComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    RewindDuration = 4.0f;
    RewindHistoryDuration = 4.0f;
    RecordInterval = 0.016f;
    MaxHistoryStates = FMath::CeilToInt(RewindHistoryDuration / RecordInterval);
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
    OriginalHistory.Reserve(MaxHistoryStates);
}

void UTimeRewindComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    AActor* Owner = GetOwner();
    if (!Owner)
        return;

    if (!bIsRewinding)
    {
        // Only record new states when not rewinding
        RecordTimer += DeltaTime;
        if (RecordTimer >= RecordInterval)
        {
            RecordState();
            RecordTimer = 0.0f;

            while (TimeHistory.Num() > MaxHistoryStates)
            {
                TimeHistory.RemoveAt(0);
            }
        }
    }
    else
    {
        RewindProgress += DeltaTime / RewindDuration;
        
        if (RewindProgress >= 1.0f)
        {
            StopTimeRewind();
            return;
        }

        float targetTime = RewindStartTime - (RewindProgress * RewindHistoryDuration);
        FTimeState* TargetState = FindStateAtTime(targetTime);
        
        if (TargetState)
        {
            InterpolateToState(*TargetState);
        }
    }
}

FTimeState* UTimeRewindComponent::FindStateAtTime(float TargetTime)
{
    if (OriginalHistory.Num() == 0)
        return nullptr;

    for (int32 i = OriginalHistory.Num() - 1; i >= 0; --i)
    {
        if (OriginalHistory[i].Timestamp <= TargetTime)
        {
            return &OriginalHistory[i];
        }
    }

    return &OriginalHistory[0];
}

void UTimeRewindComponent::RecordState()
{
    AActor* Owner = GetOwner();
    if (!Owner)
        return;

    FTimeState NewState;
    NewState.Transform = Owner->GetActorTransform();


    UCharacterMovementComponent* MovementComponent = Cast<UCharacterMovementComponent>(Owner->GetComponentByClass(UCharacterMovementComponent::StaticClass()));
    if (MovementComponent)
    {
        NewState.Velocity = MovementComponent->Velocity;
        UE_LOG(LogTemp, Log, TEXT("Velocity: %s, Size: %.2f"), *NewState.Velocity.ToString(), NewState.Velocity.Size());
        NewState.bWasMoving = NewState.Velocity.Size() > 1.0f;
        UE_LOG(LogTemp, Log, TEXT("bWasMoving: %s"), NewState.bWasMoving ? TEXT("true") : TEXT("false"));
    }
    else
    {
        NewState.bWasMoving = false;
    }
    
    NewState.Timestamp = GetWorld()->GetTimeSeconds();
    TimeHistory.Add(NewState);
}


void UTimeRewindComponent::InterpolateToState(const FTimeState& TargetState)
{
    AActor* Owner = GetOwner();
    if (!Owner)
        return;

    FTransform CurrentTransform = Owner->GetActorTransform();
    FTransform InterpolatedTransform = FTransform::Identity;
    
    float Alpha = FMath::SmoothStep(0.0f, 1.0f, RewindProgress);
    InterpolatedTransform.Blend(CurrentTransform, TargetState.Transform, Alpha);
    
    Owner->SetActorTransform(InterpolatedTransform);

    UPrimitiveComponent* PrimitiveComp = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
    if (PrimitiveComp && PrimitiveComp->IsSimulatingPhysics())
    {
        FVector CurrentVelocity = PrimitiveComp->GetPhysicsLinearVelocity();
        FVector InterpolatedVelocity = FMath::Lerp(CurrentVelocity, TargetState.Velocity, Alpha);
        
        PrimitiveComp->SetPhysicsLinearVelocity(InterpolatedVelocity);
    }

    if (TargetState.bWasMoving)
    {
        bIsMoving = true;
    }
    else
    {
        bIsMoving = false;
    }
}

void UTimeRewindComponent::StartTimeRewind()
{
    if (!bIsRewinding)
    {
        OnRewindStart.Broadcast();
        bIsRewinding = true;
        RewindProgress = 0.0f;
        RewindStartTime = GetWorld()->GetTimeSeconds();

        // Create a backup of the current history to use during rewind
        OriginalHistory = TimeHistory;
    }
}

void UTimeRewindComponent::StopTimeRewind()
{
    if (bIsRewinding)
    {
        OnRewindStop.Broadcast();
        bIsRewinding = false;
        RewindProgress = 0.0f;
        
        // Clear the history and start fresh after a rewind
        TimeHistory.Empty();
        OriginalHistory.Empty();
        
        float RewindDurationn = GetWorld()->GetTimeSeconds() - RewindStartTime;
        UE_LOG(LogTemp, Log, TEXT("Rewind finished in %.2f seconds"), RewindDurationn);
    }
}