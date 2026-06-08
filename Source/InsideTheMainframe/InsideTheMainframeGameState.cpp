// Fill out your copyright notice in the Description page of Project Settings.


#include "InsideTheMainframeGameState.h"

#include "InsideTheMainframeGameMode.h"
#include "InsideTheMainframePlayerController.h"
#include "Net/UnrealNetwork.h"    
#include "UI/InsideTheMainframeHUD.h"
#include "GameFramework/PlayerState.h"

// -------------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------------
AInsideTheMainframeGameState::AInsideTheMainframeGameState()
{
    TimeRemaining    = 0.f;
    bMatchInProgress = false;
    bVirusWon        = false;
    AntivirusCount   = 0;
    VirusCount       = 0;
}

void AInsideTheMainframeGameState::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AInsideTheMainframeGameState, bMatchInProgress);
    DOREPLIFETIME(AInsideTheMainframeGameState, bVirusWon);
    DOREPLIFETIME(AInsideTheMainframeGameState, AntivirusCount);
    DOREPLIFETIME(AInsideTheMainframeGameState, VirusCount);

    DOREPLIFETIME(AInsideTheMainframeGameState, TimeRemaining);
}

void AInsideTheMainframeGameState::OnRep_TimeRemaining()
{
    UE_LOG(LogTemp, Verbose, TEXT("[CLIENT] TimeRemaining actualizado: %.0f"), TimeRemaining);


/*     if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
     {
         if (AInsideTheMainframeHUD* HUD = PC->GetHUD<AInsideTheMainframeHUD>())
         {
             HUD->UpdateTimer(TimeRemaining);
         }
     }*/
}


void AInsideTheMainframeGameState::Multicast_OnMatchStarted_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("[MULTICAST] Partida iniciada en: %s"),
        HasAuthority() ? TEXT("Servidor") : TEXT("Cliente"));

}


void AInsideTheMainframeGameState::Multicast_OnMatchEnded_Implementation(bool bVirusWonMatch)
{
    UE_LOG(LogTemp, Warning, TEXT("[MULTICAST] Partida terminada. Virus ganaron: %s"),
        bVirusWonMatch ? TEXT("SI") : TEXT("NO"));

   
}


void AInsideTheMainframeGameState::Multicast_OnPlayerInfected_Implementation(
    APlayerState* InfectedPlayer)
{
    if (!InfectedPlayer) return;

    UE_LOG(LogTemp, Warning, TEXT("[MULTICAST] Jugador infectado: %s"),
        *InfectedPlayer->GetPlayerName());

}
void AInsideTheMainframeGameState::OnRep_Counters()
{
    // Notificar a todos los PlayerControllers locales que actualicen el HUD
    if (UWorld* World = GetWorld())
    {
        for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
        {
            if (AInsideTheMainframePlayerController* PC =
                Cast<AInsideTheMainframePlayerController>(It->Get()))
            {
                if (PC->IsLocalController())
                {
                    PC->UpdateHUDCounters(VirusCount, AntivirusCount);
                }
            }
        }
    }
}
void AInsideTheMainframeGameState::CheckAllInfected()
{
    if (!HasAuthority()) return;
    if (!bMatchInProgress) return;

    for (APlayerState* PS : PlayerArray)
    {
        AInsideTheMainframePlayerState* IPS =
            Cast<AInsideTheMainframePlayerState>(PS);
        if (IPS && !IPS->IsVirus())
            return;
    }

    // Todos son virus — avisarle al GameMode
    if (AInsideTheMainframeGameMode* GM =
        Cast<AInsideTheMainframeGameMode>(GetWorld()->GetAuthGameMode()))
    {
        GM->EndMatch(true);
    }
}

void AInsideTheMainframeGameState::BrokedRam(AActor* ram)
{
 
    if (!HasAuthority()) return;
    RamRef=ram;
     TimeRemaining = TimeRemaining+20.0f; //le da mas tiempo  
    Multicast_BrokedRam(ram);
   
}

void AInsideTheMainframeGameState::Server_BrokedRam_Implementation(AActor* ram)
{
    BrokedRam(ram);
}

void AInsideTheMainframeGameState::Multicast_BrokedRam_Implementation(AActor* ram)
{
    if (!ram) return;
    if (UFunction* Func = ram->FindFunction(TEXT("OnRamBroken")))
    {
        ram->ProcessEvent(Func, nullptr);
    }
}
