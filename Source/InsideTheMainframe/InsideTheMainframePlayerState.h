// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "InsideTheMainframePlayerState.generated.h"

// Enum para el rol del jugador — más legible que un bool en el HUD y logs
UENUM(BlueprintType)
enum class EPlayerRole : uint8
{
    Antivirus   UMETA(DisplayName = "Antivirus"),
    Virus       UMETA(DisplayName = "Virus")
};

// Enum para el estado de vida — útil para respawn y animaciones
//no se usa, al final no se muere nunca o es virus o se acaba el tiempo
UENUM(BlueprintType)
enum class EPlayerStatus : uint8
{
    Alive       UMETA(DisplayName = "Alive"),
    Dead        UMETA(DisplayName = "Dead")
};

UCLASS()
class INSIDETHEMAINFRAME_API AInsideTheMainframePlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    AInsideTheMainframePlayerState();

   

    UPROPERTY(ReplicatedUsing = OnRep_PlayerRole, BlueprintReadOnly, Category = "Player")
    EPlayerRole PlayerRole;

    UPROPERTY(ReplicatedUsing = OnRep_PlayerStatus, BlueprintReadOnly, Category = "Player")
    EPlayerStatus PlayerStatus;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
    int32 InfectionScore;
    

    
    UPROPERTY(BlueprintReadOnly, Category = "Player")
    bool bIsVirus;

    
    UFUNCTION(Client, Reliable)
    void Client_NotifyRole(EPlayerRole NewRole);

    
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_RequestInfectPlayer(AInsideTheMainframePlayerState* TargetPlayer);

   
    void SetAsVirus();

    void SetAsDead();

    void AddInfectionPoint();

    UFUNCTION(BlueprintPure, Category = "Player")
    bool IsVirus() const { return PlayerRole == EPlayerRole::Virus; }

    UFUNCTION(BlueprintPure, Category = "Player")
    bool IsAlive() const { return PlayerStatus == EPlayerStatus::Alive; }

      virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    // RepNotify se ejecuta en el cliente cuando PlayerRole cambia
    UFUNCTION()
    void OnRep_PlayerRole();

    // RepNotify se ejecuta en el cliente cuando PlayerStatus cambia
    UFUNCTION()
    void OnRep_PlayerStatus();
};