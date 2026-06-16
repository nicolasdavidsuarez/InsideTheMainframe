#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InsideTheMainframePlayerState.h"
#include "InsideTheMainframePlayerController.generated.h"

// Forward declarations
class AInsideTheMainframeGameState;
class UInsideTheMainframeHUD;          // Widget principal — lo crearemos después

UCLASS()
class INSIDETHEMAINFRAME_API AInsideTheMainframePlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AInsideTheMainframePlayerController();

   
    virtual void OnPossess(APawn* InPawn) override;

    virtual void BeginPlay() override;

    virtual void OnRep_PlayerState() override;

   
    UFUNCTION(Client, Reliable)
    void Client_ShowRoleNotification(EPlayerRole NewRole);

    UFUNCTION(Client, Reliable)
    void Client_ShowEndScreen(bool bVirusWon);

    UFUNCTION(Client, Reliable)
    void Client_OnInfected();

  
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_NotifyReady();
    
    UFUNCTION(Client, Reliable)
    void Client_ShowRePlay();
    
    UFUNCTION(Server,Reliable)
    void Server_RequestRestartLevel();


    EPlayerRole PendingRole;
    bool bHasPendingRole = false;
    
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void CreateHUD();

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateHUDTimer(float TimeRemaining);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateHUDCounters(int32 VirusCount, int32 AntivirusCount);

       UFUNCTION(BlueprintPure, Category = "Player")
    AInsideTheMainframePlayerState* GetMainframePlayerState() const;

protected:
    // Clase del widget HUD — asignada desde el Blueprint hijo
    UPROPERTY(EditDefaultsOnly, Category = "HUD")
    TSubclassOf<UUserWidget> HUDWidgetClass;

    // Referencia al widget instanciado
    UPROPERTY()
    UUserWidget* HUDWidgetInstance;

    // Flag para no crear el HUD dos veces
    bool bHUDCreated;
public:
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateHUDInfectionWarning(bool bBeingInfected, float Progress);
};