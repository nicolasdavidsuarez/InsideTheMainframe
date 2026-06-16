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
   

    bMatchStarted = false;
    if (AInsideTheMainframeGameState* GS = GetMainframeGameState())
    {
        GS->bMatchInProgress = false;
        GS->bVirusWon        = false;
        GS->TimeRemaining    = 0.f;
        GS->VirusCount       = 0;
        GS->AntivirusCount   = 0;
    } 
}


void AInsideTheMainframeGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);   
   
    // Delay para asegurarse que ambos PostLogin llegaron antes de iniciar
    FTimerHandle TryStartHandle;
    GetWorldTimerManager().SetTimer(TryStartHandle, this,
        &AInsideTheMainframeGameMode::TryStartMatch, 0.5f, false);
}


void AInsideTheMainframeGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

    UE_LOG(LogTemp, Log, TEXT("Logout: jugador desconectado. Quedan: %d"), GetNumPlayers());

}


void AInsideTheMainframeGameMode::TryStartMatch()
{
    if (bMatchStarted) return;
    
    
    
    if (GetNumPlayers() < MinPlayersToStart) return;

    bMatchStarted = true;
    
    FTimerHandle StartDelayHandle;
    GetWorldTimerManager().SetTimer(StartDelayHandle, this,
        &AInsideTheMainframeGameMode::StartMatchDelayed, 1.f, false);
    
    
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

    int32 Count = 0;
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (AInsideTheMainframePlayerController* PC =
            Cast<AInsideTheMainframePlayerController>(It->Get()))
        {
            Count++;
            GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green,
                FString::Printf(TEXT("[ENDMATCH] Enviando a PC %d: %s"),
                    Count, *PC->GetName()));
            PC->Client_ShowEndScreen(bVirusWon);
            PC->Client_ShowRePlay();
        }
    }

    GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green,
        FString::Printf(TEXT("[ENDMATCH] Total PCs notificados: %d"), Count));
}

AInsideTheMainframeGameState* AInsideTheMainframeGameMode::GetMainframeGameState() const
{
    return GetGameState<AInsideTheMainframeGameState>();
}