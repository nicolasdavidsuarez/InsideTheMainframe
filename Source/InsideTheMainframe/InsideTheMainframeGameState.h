// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "InsideTheMainframeGameState.generated.h"

UCLASS()
class INSIDETHEMAINFRAME_API AInsideTheMainframeGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    AInsideTheMainframeGameState();

    UPROPERTY(ReplicatedUsing = OnRep_TimeRemaining, BlueprintReadOnly, Category = "Match")
    float TimeRemaining;
    
    UPROPERTY()
    AActor* RamRef=nullptr;
   
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
    bool bMatchInProgress;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
    bool bVirusWon;
 
    
    UPROPERTY(ReplicatedUsing = OnRep_Counters, BlueprintReadOnly, Category = "Match")
    int32 AntivirusCount;

    UPROPERTY(ReplicatedUsing = OnRep_Counters, BlueprintReadOnly, Category = "Match")
    int32 VirusCount;

    
    UFUNCTION()
    void OnRep_Counters();
    
    UFUNCTION()
    void CheckAllInfected();
    
    UFUNCTION(BlueprintCallable, Category = "Match")
    void BrokedRam(AActor* ram);
    
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_BrokedRam(AActor* ram);
    
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Match")
    void Server_BrokedRam(AActor* ram);
    

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnMatchStarted();

  
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnMatchEnded(bool bVirusWonMatch);

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_OnPlayerInfected(APlayerState* InfectedPlayer);

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    UFUNCTION()
    void OnRep_TimeRemaining();
};