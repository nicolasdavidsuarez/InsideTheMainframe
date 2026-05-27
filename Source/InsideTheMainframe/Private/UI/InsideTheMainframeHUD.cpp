// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/UI/InsideTheMainframeHUD.h"
#include "InsideTheMainframeGameState.h"
#include "InsideTheMainframePlayerState.h"
#include "Components/TextBlock.h"
#include "Components/Overlay.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "TimerManager.h"

// -------------------------------------------------------------------------
// NativeConstruct — el widget está listo en pantalla
// -------------------------------------------------------------------------
void UInsideTheMainframeHUD::NativeConstruct()
{
    Super::NativeConstruct();

    // Ocultar el end screen al inicio
    if (Overlay_EndScreen)
        Overlay_EndScreen->SetVisibility(ESlateVisibility::Hidden);

    // Ocultar la notificación de rol al inicio
    if (Overlay_RoleNotification)
        Overlay_RoleNotification->SetVisibility(ESlateVisibility::Hidden);

    UE_LOG(LogTemp, Log, TEXT("[HUD] Widget inicializado"));
}

// -------------------------------------------------------------------------
// NativeTick — actualizar HUD cada frame con datos del GameState
// -------------------------------------------------------------------------
void UInsideTheMainframeHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Leer el GameState directamente — siempre tiene los datos actualizados
    if (UWorld* World = GetWorld())
    {
        if (AInsideTheMainframeGameState* GS =
            World->GetGameState<AInsideTheMainframeGameState>())
        {
            if (GS->bMatchInProgress)
            {
                UpdateTimer(GS->TimeRemaining);
                UpdateCounters(GS->VirusCount, GS->AntivirusCount);
            }
        }
    }
}

// -------------------------------------------------------------------------
// UpdateTimer
// -------------------------------------------------------------------------
void UInsideTheMainframeHUD::UpdateTimer(float TimeRemaining)
{
    if (Text_Timer)
        Text_Timer->SetText(FormatTime(TimeRemaining));
}

// -------------------------------------------------------------------------
// UpdateCounters
// -------------------------------------------------------------------------
void UInsideTheMainframeHUD::UpdateCounters(int32 VirusCount, int32 AntivirusCount)
{
    if (Text_VirusCount)
        Text_VirusCount->SetText(FText::FromString(
            FString::Printf(TEXT("Virus: %d"), VirusCount)));

    if (Text_AntivirusCount)
        Text_AntivirusCount->SetText(FText::FromString(
            FString::Printf(TEXT("Antivirus: %d"), AntivirusCount)));
}

// -------------------------------------------------------------------------
// ShowRoleNotification — muestra por 3 segundos quién sos
// -------------------------------------------------------------------------
void UInsideTheMainframeHUD::ShowRoleNotification(EPlayerRole NewRole)
{
    if (!Overlay_RoleNotification || !Text_RoleNotification) return;

    // Texto según el rol
    FString RoleText = (NewRole == EPlayerRole::Virus)
        ? TEXT("ERES EL VIRUS\nInfectá a todos antes de que se acabe el tiempo")
        : TEXT("ERES UN ANTIVIRUS\nSobreviví hasta el final");

    Text_RoleNotification->SetText(FText::FromString(RoleText));
    Overlay_RoleNotification->SetVisibility(ESlateVisibility::Visible);

    // Ocultar después de 3 segundos
    GetWorld()->GetTimerManager().SetTimer(
        RoleNotificationTimer,
        this,
        &UInsideTheMainframeHUD::HideRoleNotification,
        3.f,
        false
    );
}

// -------------------------------------------------------------------------
// HideRoleNotification
// -------------------------------------------------------------------------
void UInsideTheMainframeHUD::HideRoleNotification()
{
    if (Overlay_RoleNotification)
        Overlay_RoleNotification->SetVisibility(ESlateVisibility::Hidden);
}

// -------------------------------------------------------------------------
// ShowEndScreen
// -------------------------------------------------------------------------
void UInsideTheMainframeHUD::ShowEndScreen(bool bLocalPlayerWon)
{
    if (!Overlay_EndScreen || !Text_EndResult) return;

    FString ResultText = bLocalPlayerWon
        ? TEXT("SISTEMA PROTEGIDO\nGanaste")
        : TEXT("SISTEMA COMPROMETIDO\nPerdiste");

    Text_EndResult->SetText(FText::FromString(ResultText));
    Overlay_EndScreen->SetVisibility(ESlateVisibility::Visible);
}

// -------------------------------------------------------------------------
// HideEndScreen
// -------------------------------------------------------------------------
void UInsideTheMainframeHUD::HideEndScreen()
{
    if (Overlay_EndScreen)
        Overlay_EndScreen->SetVisibility(ESlateVisibility::Hidden);
}

// -------------------------------------------------------------------------
// FormatTime — convierte segundos a MM:SS
// -------------------------------------------------------------------------
FText UInsideTheMainframeHUD::FormatTime(float Seconds) const
{
    int32 TotalSeconds = FMath::Max(0, FMath::FloorToInt(Seconds));
    int32 Minutes      = TotalSeconds / 60;
    int32 Secs         = TotalSeconds % 60;

    return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Secs));
}
void UInsideTheMainframeHUD::ShowInfectionWarning(bool bShow, float Progress)
{
    if (Overlay_InfectionWarning)
        Overlay_InfectionWarning->SetVisibility(
            bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

    if (ProgressBar_Infection)
    {
         ProgressBar_Infection->SetPercent(Progress);
        ProgressBar_Infection->SetVisibility(ESlateVisibility::Visible);
    }
       
}