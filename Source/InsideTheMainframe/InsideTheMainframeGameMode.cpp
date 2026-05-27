// Copyright Epic Games, Inc. All Rights Reserved.
#include "InsideTheMainframeGameMode.h"
#include "InsideTheMainframeGameState.h"
#include "InsideTheMainframePlayerState.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"            // TActorIterator
#include "TimerManager.h"
#include "InsideTheMainframePlayerController.h"
#include "InsideTheMainframeCharacter.h"

AInsideTheMainframeGameMode::AInsideTheMainframeGameMode()
{
    // Le decimos a Unreal qué clase usar para cada rol
    GameStateClass        = AInsideTheMainframeGameState::StaticClass();
    PlayerStateClass      = AInsideTheMainframePlayerState::StaticClass();
    PlayerControllerClass = AInsideTheMainframePlayerController::StaticClass();
    DefaultPawnClass      = AInsideTheMainframeCharacter::StaticClass();

    MinPlayersToStart = 2;  
    MatchDuration     = 60.f;
    bMatchStarted     = false;
}


void AInsideTheMainframeGameMode::BeginPlay()
{
    Super::BeginPlay();
}


void AInsideTheMainframeGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);   
    UE_LOG(LogTemp, Log, TEXT("PostLogin: jugador conectado. Total: %d"), GetNumPlayers());
    TryStartMatch();
}


void AInsideTheMainframeGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

    UE_LOG(LogTemp, Log, TEXT("Logout: jugador desconectado. Quedan: %d"), GetNumPlayers());

}


void AInsideTheMainframeGameMode::TryStartMatch()
{
    if (bMatchStarted) return;
    
    /*AGameStateBase* GS = GetGameState<AGameStateBase>();
    GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
        FString::Printf(TEXT("PlayerArray.Num() = %d / GetNumPlayers() = %d"),
            GS ? GS->PlayerArray.Num() : -1, GetNumPlayers()));*/
    
    if (GetNumPlayers() < MinPlayersToStart) return;

    bMatchStarted = true;
    
    FTimerHandle StartDelayHandle;
    GetWorldTimerManager().SetTimer(StartDelayHandle, this,
        &AInsideTheMainframeGameMode::StartMatchDelayed, 1.f, false);
    
    /*
    if (AInsideTheMainframeGameState* GS = GetMainframeGameState())
    {
        GS->TimeRemaining    = MatchDuration;
        GS->bMatchInProgress = true;
        GS->AntivirusCount = GetNumPlayers();
        GS->VirusCount     = 0;
    }
    
    
    SelectInitialVirus();
    UpdatePlayerCounts();  // ← DESPUÉS de asignar roles

    GetWorldTimerManager().SetTimer(
        MatchTimerHandle, this,
        &AInsideTheMainframeGameMode::OnMatchTick,
        1.f, true);
        */
}

void AInsideTheMainframeGameMode::StartMatchDelayed()
{
    AGameStateBase* GS = GetGameState<AGameStateBase>();
    GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
        FString::Printf(TEXT("[START] PlayerArray=%d GetNumPlayers=%d"),
            GS ? GS->PlayerArray.Num() : -1, GetNumPlayers()));

    // Verificar cada PlayerState
    if (GS)
    {
        for (int32 i = 0; i < GS->PlayerArray.Num(); i++)
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Orange,
                FString::Printf(TEXT("[PS %d] = %s"), i,
                    GS->PlayerArray[i] ? *GS->PlayerArray[i]->GetPlayerName() : TEXT("NULL")));
        }
    AInsideTheMainframeGameState* MGS = Cast<AInsideTheMainframeGameState>(GS);
        MGS->TimeRemaining    = MatchDuration;
        MGS->bMatchInProgress = true;
        MGS->AntivirusCount   = GetNumPlayers();
        MGS->VirusCount       = 0;
    }

    SelectInitialVirus();
    UpdatePlayerCounts();

    GetWorldTimerManager().SetTimer(
        MatchTimerHandle, this,
        &AInsideTheMainframeGameMode::OnMatchTick,
        1.f, true);
}

void AInsideTheMainframeGameMode::UpdatePlayerCounts()
{
    AInsideTheMainframeGameState* GS = GetMainframeGameState();
    if (!GS) return;

    int32 Virus = 0;
    int32 Antivirus = 0;

    for (APlayerState* PS : GS->PlayerArray)
    {
        AInsideTheMainframePlayerState* MPS =
            Cast<AInsideTheMainframePlayerState>(PS);
        if (!MPS) continue;
        if (MPS->IsVirus()) Virus++;
        else                Antivirus++;
    }

    GS->VirusCount     = Virus;
    GS->AntivirusCount = Antivirus;

     GEngine->AddOnScreenDebugMessage(-1,10.0f, FColor::Yellow, 
        FString::Printf(TEXT("Contadores: Virus=%d Antivirus=%d"), Virus, Antivirus));
    
    
}

void AInsideTheMainframeGameMode::SelectInitialVirus()
{
    
    AGameStateBase* GS = GetGameState<AGameStateBase>();
    if (!GS) return;
    if (GS->PlayerArray.Num() == 0) return;

    int32 VirusIndex = FMath::RandRange(0, GS->PlayerArray.Num() - 1);

    for (int32 i = 0; i < GS->PlayerArray.Num(); i++)
    {
        AInsideTheMainframePlayerState* PS =
            Cast<AInsideTheMainframePlayerState>(GS->PlayerArray[i]);

        if (!PS) continue;

        if (i == VirusIndex)
        {
            PS->SetAsVirus();
            UE_LOG(LogTemp, Warning, TEXT("Virus inicial: %s"), *PS->GetPlayerName());
        }
    }
}

void AInsideTheMainframeGameMode::OnMatchTick()
{
    AInsideTheMainframeGameState* GS = GetMainframeGameState();
    if (!GS) return;

    // Descontar 1 segundo
    GS->TimeRemaining = FMath::Max(0.f, GS->TimeRemaining - 1.f);

    UE_LOG(LogTemp, Verbose, TEXT("Tiempo restante: %.0f"), GS->TimeRemaining);

    // Chequear si alguien ganó
    CheckVictoryCondition();
}


void AInsideTheMainframeGameMode::CheckVictoryCondition()
{
    AInsideTheMainframeGameState* GS = GetMainframeGameState();
    if (!GS || !GS->bMatchInProgress) return;
    
    if (GS->TimeRemaining <= 0.f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Tiempo agotado — ganan los Antivirus"));
        EndMatch(false);
        return;
    }

    int32 AntivirusAlive = 0;

    for (APlayerState* PS : GS->PlayerArray)
    {
        AInsideTheMainframePlayerState* MPS =
            Cast<AInsideTheMainframePlayerState>(PS);

        if (MPS && !MPS->IsVirus())
        {
            AntivirusAlive++;
        }
    }

    if (AntivirusAlive == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Todos infectados — ganan los Virus"));
        EndMatch(true);
    }
}


void AInsideTheMainframeGameMode::EndMatch(bool bVirusWon)
{
    GetWorldTimerManager().ClearTimer(MatchTimerHandle);

    if (AInsideTheMainframeGameState* GS = GetMainframeGameState())
    {
        GS->bMatchInProgress = false;
        GS->bVirusWon        = bVirusWon;
        GS->Multicast_OnMatchEnded(bVirusWon);
    }

    // Notificar a cada PlayerController individualmente
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (AInsideTheMainframePlayerController* PC =
            Cast<AInsideTheMainframePlayerController>(It->Get()))
        {
            PC->Client_ShowEndScreen(bVirusWon);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Partida terminada. Virus ganaron: %s"),
        bVirusWon ? TEXT("SI") : TEXT("NO"))
}

AInsideTheMainframeGameState* AInsideTheMainframeGameMode::GetMainframeGameState() const
{
    return GetGameState<AInsideTheMainframeGameState>();
}