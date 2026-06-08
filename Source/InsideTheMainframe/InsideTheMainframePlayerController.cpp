#include "InsideTheMainframePlayerController.h"
#include "InsideTheMainframeGameState.h"
#include "Blueprint/UserWidget.h"
#include "Net/UnrealNetwork.h"
#include "Public/UI/InsideTheMainframeHUD.h"

// -------------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------------
AInsideTheMainframePlayerController::AInsideTheMainframePlayerController()
{
    HUDWidgetClass    = nullptr;
    HUDWidgetInstance = nullptr;
    bHUDCreated       = false;
}

// -------------------------------------------------------------------------
// BeginPlay — solo corre en el cliente dueño de este controller
// -------------------------------------------------------------------------
void AInsideTheMainframePlayerController::BeginPlay()
{
    Super::BeginPlay();

    // IsLocalController() es true solo en el cliente dueño
    // Nunca crear HUD en el servidor ni en otros clientes
    if (IsLocalController())
    {
        CreateHUD();
    }
}

// -------------------------------------------------------------------------
// OnPossess — el controller tomó control de un pawn
// Solo corre en el servidor
// -------------------------------------------------------------------------
void AInsideTheMainframePlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    UE_LOG(LogTemp, Log, TEXT("[SERVER] PlayerController poseyó un pawn"));
}

// -------------------------------------------------------------------------
// OnRep_PlayerState
// Se llama en el cliente cuando el PlayerState llega replicado
// Es el momento más seguro para leer datos del PlayerState en el cliente
// -------------------------------------------------------------------------
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

// -------------------------------------------------------------------------
// CreateHUD — instancia el widget y lo agrega a la pantalla
// Solo corre en el cliente (verificado antes de llamar)
// -------------------------------------------------------------------------
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

// -------------------------------------------------------------------------
// UpdateHUDTimer
// -------------------------------------------------------------------------
void AInsideTheMainframePlayerController::UpdateHUDTimer(float TimeRemaining)
{
    if (!IsLocalController()) return;

    // Cuando tengas el widget específico, casteás HUDWidgetInstance
    // y llamás a la función correspondiente:
    // if (UInsideTheMainframeHUD* HUD = Cast<UInsideTheMainframeHUD>(HUDWidgetInstance))
    // {
    //     HUD->SetTimerValue(TimeRemaining);
    // }

    UE_LOG(LogTemp, Verbose, TEXT("[CLIENT] HUD Timer: %.0f"), TimeRemaining);
}

// -------------------------------------------------------------------------
// UpdateHUDCounters
// -------------------------------------------------------------------------
void AInsideTheMainframePlayerController::UpdateHUDCounters(
    int32 VirusCount, int32 AntivirusCount)
{
    if (!IsLocalController()) return;

    UE_LOG(LogTemp, Log, TEXT("[CLIENT] HUD Contadores — Virus: %d | Antivirus: %d"),
        VirusCount, AntivirusCount);
}

// -------------------------------------------------------------------------
// Client_ShowRoleNotification
// Se ejecuta en el cliente dueño cuando el servidor le asigna un rol
// -------------------------------------------------------------------------

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