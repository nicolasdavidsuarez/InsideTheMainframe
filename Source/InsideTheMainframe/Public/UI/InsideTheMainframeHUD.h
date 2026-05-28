// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InsideTheMainframePlayerState.h"
#include "InsideTheMainframeHUD.generated.h"

// Forward declarations
class UTextBlock;
class UProgressBar;
class UImage;
class UOverlay;
class UVerticalBox;

UCLASS()
class INSIDETHEMAINFRAME_API UInsideTheMainframeHUD : public UUserWidget
{
    GENERATED_BODY()

public:
    // -------------------------------------------------------------------------
    // Lifecycle
    // -------------------------------------------------------------------------

    // Se llama cuando el widget es creado e inicializado
    virtual void NativeConstruct() override;

    // Se llama cada frame — acá actualizamos el HUD con datos del GameState
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    void UpdateEnergyBar();

    // -------------------------------------------------------------------------
    // Funciones públicas — llamadas desde PlayerController
    // -------------------------------------------------------------------------

    // Actualiza el texto del timer en pantalla
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateTimer(float TimeRemaining);

    // Actualiza los contadores de equipos
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateCounters(int32 VirusCount, int32 AntivirusCount);

    // Muestra la notificación de rol al inicio (ej: "ERES EL VIRUS")
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void ShowRoleNotification(EPlayerRole NewRole);

    // Muestra la pantalla de fin de partida
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void ShowEndScreen(bool bLocalPlayerWon);

    // Oculta la pantalla de fin de partida
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void HideEndScreen();

    // -------------------------------------------------------------------------
    // Bindings con el widget UMG
    // Los nombres tienen que coincidir EXACTAMENTE con los widgets en el editor
    // -------------------------------------------------------------------------

    // Timer
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* Text_Timer;

    // Contadores de equipos
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* Text_VirusCount;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* Text_AntivirusCount;

    // Rol del jugador
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* Text_PlayerRole;

    // Notificación de rol al inicio (overlay que se oculta después)
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UOverlay* Overlay_RoleNotification;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* Text_RoleNotification;

    // Panel de fin de partida
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UOverlay* Overlay_EndScreen;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* Text_EndResult;
    
    // Overlay que aparece cuando estás siendo infectado
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UOverlay* Overlay_InfectionWarning;

    // Barra de progreso de infección 0-1
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UProgressBar* ProgressBar_Infection;
    
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UProgressBar* ProgressBar_Energy;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* Text_Energy; 

    // Texto de advertencia
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* Text_InfectionWarning;
    
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void ShowInfectionWarning(bool bShow, float Progress);
    void UpdatePlayerRole();

protected:
    // Timer para ocultar la notificación de rol después de unos segundos
    FTimerHandle RoleNotificationTimer;

    // Oculta la notificación de rol — llamado por el timer
    void HideRoleNotification();

    // Formatea segundos a MM:SS para mostrar en pantalla
    FText FormatTime(float Seconds) const;
    
    
};