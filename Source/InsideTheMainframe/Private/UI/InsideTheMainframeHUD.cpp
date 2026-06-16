// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/UI/InsideTheMainframeHUD.h"

#include "InsideTheMainframeCharacter.h"
#include "InsideTheMainframeGameState.h"
#include "InsideTheMainframePlayerController.h"
#include "InsideTheMainframePlayerState.h"
#include "Components/TextBlock.h"
#include "Components/Overlay.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "TimerManager.h"
#include "AI/NavigationSystemBase.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"


void UInsideTheMainframeHUD::RestartGame()
{
    APlayerController* PC = GetOwningPlayer();  
    if (!PC) return;

    AInsideTheMainframePlayerController* ITMController = 
        Cast<AInsideTheMainframePlayerController>(PC);
    if (ITMController)
    {
        ITMController->Server_RequestRestartLevel();
    }
    //Game State Restart
}

void UInsideTheMainframeHUD::ExitGame()
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;

    // Salir del juego es siempre local
    UKismetSystemLibrary::QuitGame(
        GetWorld(),
        PC,
        EQuitPreference::Quit,
        false // bIgnorePlatformRestrictions
    );
    //GameMode? ExitGame
}

void UInsideTheMainframeHUD::NativeConstruct()
{
    Super::NativeConstruct();

    // Ocultar el end screen al inicio
    if (Overlay_EndScreen)
        Overlay_EndScreen->SetVisibility(ESlateVisibility::Hidden);

    // Ocultar la notificación de rol al inicio
    if (Overlay_RoleNotification)
        Overlay_RoleNotification->SetVisibility(ESlateVisibility::Hidden);
    
    Button_RepeatPlay->OnClicked.AddDynamic(this, &ThisClass::RestartGame);
    Button_Salir->OnClicked.AddDynamic(this, &ThisClass::ExitGame);

    UE_LOG(LogTemp, Log, TEXT("[HUD] Widget inicializado"));
}


void UInsideTheMainframeHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (UWorld* World = GetWorld())
    {
        if (AInsideTheMainframeGameState* GS =
            World->GetGameState<AInsideTheMainframeGameState>())
        {
            GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Cyan,
                FString::Printf(TEXT("[HUD] bMatchInProgress=%d Virus=%d Antivirus=%d"),
                    GS->bMatchInProgress, GS->VirusCount, GS->AntivirusCount));

            if (GS->bMatchInProgress)
            {
                UpdateTimer(GS->TimeRemaining);
                UpdateCounters(GS->VirusCount, GS->AntivirusCount);
                UpdatePlayerRole(); 
                UpdateEnergyBar();
            }
        }
    }
}


void UInsideTheMainframeHUD::UpdateEnergyBar()
{
    if (!ProgressBar_Energy) return;

    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;

    AInsideTheMainframeCharacter* Character =
        Cast<AInsideTheMainframeCharacter>(PC->GetPawn());
    if (!Character) return;

    float Percent = Character->Energy / Character->MaxEnergy;
    ProgressBar_Energy->SetPercent(Percent);

    // Opcional — texto numérico
    if (Text_Energy)
        Text_Energy->SetText(FText::FromString(
            FString::Printf(TEXT("%.0f / %.0f"), Character->Energy, Character->MaxEnergy)));
}

void UInsideTheMainframeHUD::UpdateTimer(float TimeRemaining)
{
    if (Text_Timer)
        Text_Timer->SetText(FText::FromString(
       TEXT("SCAN C: Disk... Remainig time:  ") + FormatTime(TimeRemaining).ToString()));
}


void UInsideTheMainframeHUD::UpdateCounters(int32 VirusCount, int32 AntivirusCount)
{
    if (Text_VirusCount)
        Text_VirusCount->SetText(FText::FromString(
            FString::Printf(TEXT("Virus: %d"), VirusCount)));

    if (Text_AntivirusCount)
        Text_AntivirusCount->SetText(FText::FromString(
            FString::Printf(TEXT("Antivirus: %d"), AntivirusCount)));
}


void UInsideTheMainframeHUD::ShowRoleNotification(EPlayerRole NewRole)
{
    if (!Overlay_RoleNotification || !Text_RoleNotification) return;

    // Texto según el rol
    FString RoleText = (NewRole == EPlayerRole::Virus)
        ? TEXT("ERES EL VIRUS\nInfectá a todos antes de que se acabe el tiempo")
        : TEXT("ERES UN ANTIVIRUS\nSobreviví hasta el final");

    Text_RoleNotification->SetText(FText::FromString(RoleText));
    Overlay_RoleNotification->SetVisibility(ESlateVisibility::Visible);

    
    GetWorld()->GetTimerManager().SetTimer(
        RoleNotificationTimer,
        this,
        &UInsideTheMainframeHUD::HideRoleNotification,
        3.f,
        false
    );
}


void UInsideTheMainframeHUD::HideRoleNotification()
{
    if (Overlay_RoleNotification)
        Overlay_RoleNotification->SetVisibility(ESlateVisibility::Hidden);
}


void UInsideTheMainframeHUD::ShowEndScreen(bool bLocalPlayerWon)
{
    if (!Overlay_EndScreen || !Text_EndResult) return;
    
    if (AInsideTheMainframePlayerState* PS = Cast<AInsideTheMainframePlayerState>(GetOwningPlayerState(false))) //GetMainframePlayerState())
    {
        FString ResultText;
        if (PS->bIsVirus)
        {
            ResultText = bLocalPlayerWon
        ? TEXT("SISTEMA CORROMPIDO-MOODLE CAIDO\nGanaste!!!")
        : TEXT("SISTEMA SCANEADO-PROTEGIDO\nPerdiste!!!");
        }else
        {
            ResultText = bLocalPlayerWon
        ? TEXT("SISTEMA SCANEADO-PROTEGIDO\nGanaste")
        : TEXT("SISTEMA CORROMPIDO-MOODLE CAIDO\nPerdiste");
        }
        Text_EndResult->SetText(FText::FromString(ResultText));
        Overlay_RoleNotification->SetVisibility(ESlateVisibility::Hidden);
        Overlay_InfectionWarning->SetVisibility(ESlateVisibility::Hidden);
        Overlay_EndScreen->SetVisibility(ESlateVisibility::Visible);
    }
}


void UInsideTheMainframeHUD::HideEndScreen()
{
    if (Overlay_EndScreen)
        Overlay_EndScreen->SetVisibility(ESlateVisibility::Hidden);
}

void UInsideTheMainframeHUD::ShowRepeatGame()
{
    if (Overlay_RePlay)
        Overlay_RePlay->SetVisibility(ESlateVisibility::Visible);
}

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
void UInsideTheMainframeHUD::UpdatePlayerRole()
{
    if (!Text_PlayerRole) return;

    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;

    AInsideTheMainframePlayerState* PS =
        PC->GetPlayerState<AInsideTheMainframePlayerState>();
    if (!PS) return;

    FString RoleText = PS->IsVirus() ? TEXT("VIRUS") : TEXT("ANTIVIRUS");
    Text_PlayerRole->SetText(FText::FromString(RoleText));
}