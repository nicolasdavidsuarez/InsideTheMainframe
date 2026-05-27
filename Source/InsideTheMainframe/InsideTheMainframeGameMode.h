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

      // Estado interno del GameMode  
    
    bool bMatchStarted;

    // Handle del timer que descuenta el tiempo
    FTimerHandle MatchTimerHandle;

   //jugadores Suficientes?
    public:
    void TryStartMatch();
    void StartMatchDelayed();
    void UpdatePlayerCounts();

    // Elige al azar un jugador y lo marca como Virus inicial
    void SelectInitialVirus();

    // Se llama cada segundo via Timer — descuenta tiempo y chequea victoria
    void OnMatchTick();

    // Recorre los PlayerStates y evalúa si alguien ganó
    void CheckVictoryCondition();

    // Termina la partida y notifica a todos los clientes
    // bVirusWon: true = ganaron los Virus, false = ganaron los Antivirus
    void EndMatch(bool bVirusWon);

private:
    // Helper: devuelve el GameState casteado a nuestro tipo
    AInsideTheMainframeGameState* GetMainframeGameState() const;
};