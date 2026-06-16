#include "InsideTheMainframePlayerController.h"
#include "InsideTheMainframeGameState.h"
#include "Blueprint/UserWidget.h"
#include "Net/UnrealNetwork.h"
#include "Public/UI/InsideTheMainframeHUD.h"


AInsideTheMainframePlayerController::AInsideTheMainframePlayerController()
{
    HUDWidgetClass    = nullptr;
    HUDWidgetInstance = nullptr;
    bHUDCreated       = false;
}


void AInsideTheMainframePlayerController::BeginPlay()
{
    Super::BeginPlay();

    // IsLocalController() es true solo en el cliente dueño
    // Nunca crear HUD en el servidor ni en otros clientes
    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
    SetShowMouseCursor(false);
    if (IsLocalController())
    {
        CreateHUD();
    }
}


void AInsideTheMainframePlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    UE_LOG(LogTemp, Log, TEXT("[SERVER] PlayerController poseyó un pawn"));
}


void AInsideTheMainframePlayerController::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    // El PlayerState ya está disponible — podemos actualizar el HUD con los datos
    if (AInsideTheMainframePlayerState* PS = GetMainframePlayerState())
    {
        UE_LOG(LogTemp, Log, TEXT("[CLIENT] PlayerState recibido para: %s"),
            *PS->GetPlayerName());

        // Actualizar HUD con el rol una vez que el PlayerState llegó
        // UpdateHUDRole(PS->PlayerRole);
    }
}


void AInsideTheMainframePlayerController::Client_ShowRePlay_Implementation()
{
    if (UInsideTheMainframeHUD* HUD = 
       Cast<UInsideTheMainframeHUD>(HUDWidgetInstance))
    {
        HUD->ShowRepeatGame();
    }
}

void AInsideTheMainframePlayerController::Server_RequestRestartLevel_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("Server_RequestRestartLevel_Implementation"));

    UWorld* World = GetWorld();
    if (World)
    {
        FString CurrentMap = World->GetMapName();
        CurrentMap.RemoveFromStart(World->StreamingLevelsPrefix);
        World->ServerTravel(CurrentMap + "?listen", false);
    }

}

void AInsideTheMainframePlayerController::CreateHUD()
{
    if (bHUDCreated) return;
    if (!HUDWidgetClass) 
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerController: HUDWidgetClass no asignado en el BP"));
        return;
    }

    HUDWidgetInstance = CreateWidget<UUserWidget>(this, HUDWidgetClass);
    if (HUDWidgetInstance)
    {
        HUDWidgetInstance->AddToViewport();
        bHUDCreated = true;
        UE_LOG(LogTemp, Log, TEXT("[CLIENT] HUD creado y añadido al viewport"));
    }
    
    HUDWidgetInstance = CreateWidget<UUserWidget>(this, HUDWidgetClass);
    if (HUDWidgetInstance)
    {
        HUDWidgetInstance->AddToViewport();
        bHUDCreated = true;

        // Mostrar rol pendiente si llegó antes que el HUD
        if (bHasPendingRole)
        {
            if (UInsideTheMainframeHUD* HUD = 
                Cast<UInsideTheMainframeHUD>(HUDWidgetInstance))
            {
                HUD->ShowRoleNotification(PendingRole);
                bHasPendingRole = false;
            }
        }
    }
}


void AInsideTheMainframePlayerController::UpdateHUDTimer(float TimeRemaining)
{
    if (!IsLocalController()) return;

   

    UE_LOG(LogTemp, Verbose, TEXT("[CLIENT] HUD Timer: %.0f"), TimeRemaining);
}

void AInsideTheMainframePlayerController::UpdateHUDCounters(
    int32 VirusCount, int32 AntivirusCount)
{
    if (!IsLocalController()) return;

    UE_LOG(LogTemp, Log, TEXT("[CLIENT] HUD Contadores — Virus: %d | Antivirus: %d"),
        VirusCount, AntivirusCount);
}


void AInsideTheMainframePlayerController::Client_ShowRoleNotification_Implementation(
    EPlayerRole NewRole)
{
    GEngine->AddOnScreenDebugMessage(-1,10.f, FColor::Red,
         TEXT("[ROL] Mostrando rol pendiente"));
    if (UInsideTheMainframeHUD* HUD = 
        Cast<UInsideTheMainframeHUD>(HUDWidgetInstance))
    {
        HUD->ShowRoleNotification(NewRole);
    }
    else
    {
        // HUD todavía no existe, guardar para después
        PendingRole = NewRole;
        bHasPendingRole = true;
        UE_LOG(LogTemp, Warning, TEXT("[ROL] HUD null, guardando rol pendiente"));
        GEngine->AddOnScreenDebugMessage(-1,10.f, FColor::Red,
            TEXT("[ROL] HUD null, guardando rol pendiente"));
    }
}


void AInsideTheMainframePlayerController::Client_ShowEndScreen_Implementation(bool bVirusWon)
{
    bool bIWon = false;

    if (AInsideTheMainframePlayerState* PS = GetMainframePlayerState())
    {
        bIWon = (PS->IsVirus() == bVirusWon);
    }
    
    if (UInsideTheMainframeHUD* HUD =
        Cast<UInsideTheMainframeHUD>(HUDWidgetInstance))
    {
        HUD->ShowEndScreen(bIWon);
    }

    SetShowMouseCursor(true);
    SetInputMode(FInputModeUIOnly());
}


void AInsideTheMainframePlayerController::Client_OnInfected_Implementation()
{
    
    UE_LOG(LogTemp, Warning, TEXT("[CLIENT] Fui infectado — ahora soy Virus"));

    if (UInsideTheMainframeHUD* HUD =
        Cast<UInsideTheMainframeHUD>(HUDWidgetInstance))
    {
        HUD->ShowRoleNotification(EPlayerRole::Virus);
    }
}


bool AInsideTheMainframePlayerController::Server_NotifyReady_Validate()
{
    return true;   
}

void AInsideTheMainframePlayerController::Server_NotifyReady_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("[SERVER] Jugador listo: %s"),
        PlayerState ? *PlayerState->GetPlayerName() : TEXT("desconocido"));

}


AInsideTheMainframePlayerState* AInsideTheMainframePlayerController::GetMainframePlayerState() const
{
    return Cast<AInsideTheMainframePlayerState>(PlayerState);
}

void AInsideTheMainframePlayerController::UpdateHUDInfectionWarning(
    bool bBeingInfected, float Progress)
{
    if (!IsLocalController()) return;

    if (UInsideTheMainframeHUD* HUD =
        Cast<UInsideTheMainframeHUD>(HUDWidgetInstance))
    {
        HUD->ShowInfectionWarning(bBeingInfected, Progress);
    }
}