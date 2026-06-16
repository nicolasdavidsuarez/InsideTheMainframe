// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "InsideTheMainframeGameMode.generated.h"


class AInsideTheMainframeGameState;
class AInsideTheMainframePlayerState;

UCLASS()
class INSIDETHEMAINFRAME_API AInsideTheMainframeGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AInsideTheMainframeGameMode();

   
    virtual void PostLogin(APlayerController* NewPlayer) override;
    
    virtual void Logout(AController* Exiting) override;

protected:
    virtual void BeginPlay() override;

    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match Config")
    int32 MinPlayersToStart;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match Config")
    float MatchDuration;

    
    bool bMatchStarted;

    FTimerHandle MatchTimerHandle;

    public:
    void TryStartMatch();
    void StartMatchDelayed();
    void UpdatePlayerCounts();

    void SelectInitialVirus();

    void OnMatchTick();

    void CheckVictoryCondition();

    
    void EndMatch(bool bVirusWon);

private:
    AInsideTheMainframeGameState* GetMainframeGameState() const;
};