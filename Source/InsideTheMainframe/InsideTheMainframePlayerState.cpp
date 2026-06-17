// Fill out your copyright notice in the Description page of Project Settings.



#include "InsideTheMainframePlayerState.h"

#include "InsideTheMainframeCharacter.h"
#include "InsideTheMainframeGameMode.h"
#include "InsideTheMainframeGameState.h"
#include "InsideTheMainframePlayerController.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameStateBase.h"


AInsideTheMainframePlayerState::AInsideTheMainframePlayerState()
{
    PlayerRole    = EPlayerRole::Antivirus;
    PlayerStatus  = EPlayerStatus::Alive;
    InfectionScore = 0;
    bIsVirus      = false;
}


void AInsideTheMainframePlayerState::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AInsideTheMainframePlayerState, PlayerRole);
    DOREPLIFETIME(AInsideTheMainframePlayerState, PlayerStatus);
    DOREPLIFETIME(AInsideTheMainframePlayerState, InfectionScore);
}


void AInsideTheMainframePlayerState::SetAsVirus()
{
    if (!HasAuthority()) return;

    GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
       TEXT("[SET_AS_VIRUS] Llamado"));
    
    PlayerRole = EPlayerRole::Virus;
    bIsVirus   = true;

    if (UWorld* World = GetWorld())
    {
        AInsideTheMainframeGameMode* GM =
            Cast<AInsideTheMainframeGameMode>(World->GetAuthGameMode());

        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
            FString::Printf(TEXT("[SET_AS_VIRUS] GM=%s"),
                GM ? TEXT("VALIDO") : TEXT("NULL")));

        if (GM) GM->UpdatePlayerCounts();
    }
    
    // Verificar que el Owner existe antes de castear
    AController* Controller = Cast<AController>(GetOwner());
    if (Controller)
    {
        APawn* Pawn = Controller->GetPawn();
        if (Pawn)
        {
            AInsideTheMainframeCharacter* Character =
                Cast<AInsideTheMainframeCharacter>(Pawn);
            if (Character)
            {
                Character->OnBecomeVirus();
            }
        }
    }

    // Actualizar contadores
    if (UWorld* World = GetWorld())
    {
        if (AInsideTheMainframeGameMode* GM =
           Cast<AInsideTheMainframeGameMode>(World->GetAuthGameMode()))
        {
            GM->UpdatePlayerCounts();
        }
    }

    Client_NotifyRole(PlayerRole);

    UE_LOG(LogTemp, Warning, TEXT("[SERVER] %s es ahora Virus"), *GetPlayerName());
    if (AInsideTheMainframeGameState* GS =
    GetWorld()->GetGameState<AInsideTheMainframeGameState>())
    {
        GS->CheckAllInfected();
    }
}


void AInsideTheMainframePlayerState::SetAsDead()
{
    if (!HasAuthority()) return;

    PlayerStatus = EPlayerStatus::Dead;

    UE_LOG(LogTemp, Warning, TEXT("[SERVER] %s murió"), *GetPlayerName());
}


void AInsideTheMainframePlayerState::AddInfectionPoint()
{
    if (!HasAuthority()) return;

    InfectionScore++;

    UE_LOG(LogTemp, Log, TEXT("[SERVER] %s tiene %d infecciones"),
        *GetPlayerName(), InfectionScore);
}


void AInsideTheMainframePlayerState::Client_NotifyRole_Implementation(
    EPlayerRole NewRole)
{
    UE_LOG(LogTemp, Warning, TEXT("[CLIENT] Mi rol es: %s"),
      NewRole == EPlayerRole::Virus ? TEXT("VIRUS") : TEXT("ANTIVIRUS"));

    if (AInsideTheMainframePlayerController* PC =
     Cast<AInsideTheMainframePlayerController>(GetPlayerController()))
    {
        PC->Client_ShowRoleNotification(NewRole);
    }
}


bool AInsideTheMainframePlayerState::Server_RequestInfectPlayer_Validate(
    AInsideTheMainframePlayerState* TargetPlayer)
{
    // Validación: solo un Virus puede infectar, y el target debe ser Antivirus
    if (!TargetPlayer) return false;
    if (PlayerRole != EPlayerRole::Virus) return false;
    if (TargetPlayer->PlayerRole != EPlayerRole::Antivirus) return false;
    return true;
}

void AInsideTheMainframePlayerState::Server_RequestInfectPlayer_Implementation(
    AInsideTheMainframePlayerState* TargetPlayer)
{
    // Llegamos acá solo si Validate devolvió true
    // Ejecutamos la infección con autoridad del servidor

    TargetPlayer->SetAsVirus();
    AddInfectionPoint();

    // Notificar a todos del evento de infección (efecto visual/sonido)
    if (UWorld* World = GetWorld())
    {
        if (AInsideTheMainframeGameState* GS =
            World->GetGameState<AInsideTheMainframeGameState>())
        {
            GS->Multicast_OnPlayerInfected(TargetPlayer);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[SERVER] %s infectó a %s"),
        *GetPlayerName(), *TargetPlayer->GetPlayerName());
}


void AInsideTheMainframePlayerState::OnRep_PlayerRole()
{
    // Sincronizar el alias bool con el enum
    bIsVirus = (PlayerRole == EPlayerRole::Virus);

    UE_LOG(LogTemp, Warning, TEXT("[CLIENT] Rol actualizado para %s: %s"),
        *GetPlayerName(),
        PlayerRole == EPlayerRole::Virus ? TEXT("VIRUS") : TEXT("ANTIVIRUS"));

    // Acá podés cambiar el material del personaje, mostrar un ícono en el HUD, etc.
    //o cambiar la iluminacion
}


void AInsideTheMainframePlayerState::OnRep_PlayerStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("[CLIENT] Estado actualizado para %s: %s"),
        *GetPlayerName(),
        PlayerStatus == EPlayerStatus::Alive ? TEXT("VIVO") : TEXT("MUERTO"));

    // al final no se usa para nada
}