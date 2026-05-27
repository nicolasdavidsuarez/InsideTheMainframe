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

    // -------------------------------------------------------------------------
    // Overrides
    // -------------------------------------------------------------------------

    // Llamado cuando el controller toma posesión de un pawn
    virtual void OnPossess(APawn* InPawn) override;

    // Llamado en el cliente cuando el controller está listo
    virtual void BeginPlay() override;

    // Llamado en el cliente cuando recibe/actualiza su PlayerState
    virtual void OnRep_PlayerState() override;

    // -------------------------------------------------------------------------
    // Client RPCs — el servidor le habla a este cliente específico
    // -------------------------------------------------------------------------

    // El servidor le dice al cliente que muestre su rol al inicio
    UFUNCTION(Client, Reliable)
    void Client_ShowRoleNotification(EPlayerRole NewRole);

    // El servidor le dice al cliente que muestre la pantalla de fin de partida
    UFUNCTION(Client, Reliable)
    void Client_ShowEndScreen(bool bVirusWon);

    // El servidor le avisa que fue infectado (para efectos locales)
    UFUNCTION(Client, Reliable)
    void Client_OnInfected();

    // -------------------------------------------------------------------------
    // Server RPCs — el cliente le habla al servidor
    // -------------------------------------------------------------------------

    // El cliente le avisa al servidor que está listo para jugar
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_NotifyReady();

    // -------------------------------------------------------------------------
    // Funciones de HUD (corren en el cliente)
    // -------------------------------------------------------------------------

    // Crea e inicializa el widget HUD en pantalla
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void CreateHUD();

    // Actualiza el timer en el HUD — llamado desde OnRep_TimeRemaining del GameState
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateHUDTimer(float TimeRemaining);

    // Actualiza los contadores de Virus/Antivirus en el HUD
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateHUDCounters(int32 VirusCount, int32 AntivirusCount);

    // -------------------------------------------------------------------------
    // Helpers
    // -------------------------------------------------------------------------

    // Devuelve el PlayerState casteado — conveniente para no castear en todos lados
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