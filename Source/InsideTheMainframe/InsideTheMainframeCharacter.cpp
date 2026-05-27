// Copyright Epic Games, Inc. All Rights Reserved.

#include "InsideTheMainframeCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InsideTheMainframe.h"
#include "InsideTheMainframePlayerState.h"
#include "InsideTheMainframePlayerController.h"
#include "InsideTheMainframeGameState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AInsideTheMainframeCharacter::AInsideTheMainframeCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw =true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 1200.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 250.0f;
	//CameraBoom->bInheritPitch = false;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = true;

	
	// Valores de gameplay
	MaxHealth         = 100.f;
	Health            = MaxHealth;
	bIsAlive          = true;
	bIsVirus          = false;
	InfectionRadius   = 150.f;
	InfectionCooldown = 1.5f;
	LastInfectionTime = -999.f;
	
	
	//energia
	MaxEnergy             = 100.f;
	Energy                = MaxEnergy;
	PassiveRechargeRate   = 5.f;     
	CapacitorRechargeAmount = 50.f;  
	AbilityCost           = 30.f; 
	
	//cono de infeccion
	ConeAngle      = 60.f;
	ConeRange      = 300.f;
	InfectionTime  = 1.75f;
	bConeActive    = false;
	bBeingInfected = false;
	InfectionProgress = 0.f;
	
	ConeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ConeMesh"));
	ConeMesh->SetupAttachment(RootComponent);
	ConeMesh->SetVisibility(false);          // Invisible por default
	ConeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	//Niagara de infeccin
	ConeEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ConeEffect"));
	ConeEffectComponent->SetupAttachment(RootComponent);
	ConeEffectComponent->SetAutoActivate(false); 
	
}

void AInsideTheMainframeCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
				PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(
			EnergyRechargeTimerHandle,
			this,
			&AInsideTheMainframeCharacter::PassiveEnergyTick,
			1.f,
			true
		);
	}
}

void AInsideTheMainframeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AInsideTheMainframeCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AInsideTheMainframeCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered,this, &AInsideTheMainframeCharacter::Look);
		EnhancedInputComponent->BindAction(InfectAction, ETriggerEvent::Started,this, &AInsideTheMainframeCharacter::ActivateCone);
		//EnhancedInputComponent->BindAction(InfectAction, ETriggerEvent::Started,this, &AInsideTheMainframeCharacter::TryInfectNearby);
		
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started,this, &AInsideTheMainframeCharacter::StartAim);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed,this, &AInsideTheMainframeCharacter::StopAim);
	}
	else
	{
		UE_LOG(LogInsideTheMainframe, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AInsideTheMainframeCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AInsideTheMainframeCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AInsideTheMainframeCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AInsideTheMainframeCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		//AddControllerPitchInput(Pitch);
	}
}

void AInsideTheMainframeCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AInsideTheMainframeCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}
// -------------------------------------------------------------------------
// GetLifetimeReplicatedProps
// -------------------------------------------------------------------------
void AInsideTheMainframeCharacter::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AInsideTheMainframeCharacter, Health);
    DOREPLIFETIME(AInsideTheMainframeCharacter, bIsAlive);
    DOREPLIFETIME(AInsideTheMainframeCharacter, bIsVirus);
	DOREPLIFETIME(AInsideTheMainframeCharacter, Energy);
	DOREPLIFETIME(AInsideTheMainframeCharacter, bBeingInfected);
	DOREPLIFETIME(AInsideTheMainframeCharacter, InfectionProgress);
}

// -------------------------------------------------------------------------
// TakeDamage
// -------------------------------------------------------------------------
float AInsideTheMainframeCharacter::TakeDamage(float DamageAmount,
    FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (!HasAuthority()) return 0.f;
    if (!bIsAlive) return 0.f;

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent,
        EventInstigator, DamageCauser);

    Health = FMath::Max(0.f, Health - ActualDamage);
    if (Health <= 0.f) Die();

    return ActualDamage;
}

// -------------------------------------------------------------------------
// SetupPlayerInputComponent — agregamos InfectAction al binding existente
// -------------------------------------------------------------------------
// NOTA: no reemplaces el tuyo, solo agregá esta línea dentro del if de tu
// SetupPlayerInputComponent, después del binding de LookAction:
//
//   EnhancedInputComponent->BindAction(InfectAction, ETriggerEvent::Started,
//       this, &AInsideTheMainframeCharacter::TryInfectNearby);
//

// -------------------------------------------------------------------------
// TryInfectNearby
// -------------------------------------------------------------------------
void AInsideTheMainframeCharacter::TryInfectNearby()
{UE_LOG(LogTemp, Warning, TEXT("[INFECT] bIsVirus=%s bIsAlive=%s"),
		bIsVirus ? TEXT("true") : TEXT("false"),
		bIsAlive ? TEXT("true") : TEXT("false"));

	if (!bIsVirus || !bIsAlive) return;

	float CurrentTime = GetWorld()->GetTimeSeconds();
	UE_LOG(LogTemp, Warning, TEXT("[INFECT] Cooldown check: %.1f / %.1f"),
		CurrentTime - LastInfectionTime, InfectionCooldown);

	if (CurrentTime - LastInfectionTime < InfectionCooldown) return;
	LastInfectionTime = CurrentTime;

	AInsideTheMainframeCharacter* Target = FindNearbyAntivirus();
	UE_LOG(LogTemp, Warning, TEXT("[INFECT] Target encontrado: %s"),
		Target ? *Target->GetName() : TEXT("NINGUNO"));

	if (!Target) return;
	Server_TryInfect(Target);
}

// -------------------------------------------------------------------------
// FindNearbyAntivirus
// -------------------------------------------------------------------------
AInsideTheMainframeCharacter* AInsideTheMainframeCharacter::FindNearbyAntivirus() const
{TArray<AActor*> NearbyActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(),
		AInsideTheMainframeCharacter::StaticClass(), NearbyActors);

	AInsideTheMainframeCharacter* ClosestTarget = nullptr;
	float ClosestDistance = InfectionRadius;

	for (AActor* Actor : NearbyActors)
	{
		if (Actor == this) continue;

		AInsideTheMainframeCharacter* Other = Cast<AInsideTheMainframeCharacter>(Actor);
		if (!Other || !Other->bIsAlive) continue;

		// Leer el rol desde el PlayerState — más confiable que bIsVirus del Character
		AInsideTheMainframePlayerState* OtherPS =
			Other->GetPlayerState<AInsideTheMainframePlayerState>();
		if (!OtherPS) continue;
		if (OtherPS->IsVirus()) continue;   // Ya es Virus, saltear

		float Distance = FVector::Dist(GetActorLocation(), Other->GetActorLocation());
		UE_LOG(LogTemp, Warning, TEXT("[FIND] %s a distancia %.0f (radio: %.0f)"),
			*Other->GetName(), Distance, InfectionRadius);

		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestTarget   = Other;
		}
	}
	return ClosestTarget;
}

// -------------------------------------------------------------------------
// Server_TryInfect
// -------------------------------------------------------------------------
bool AInsideTheMainframeCharacter::Server_TryInfect_Validate(
    AInsideTheMainframeCharacter* TargetCharacter)
{
    if (!TargetCharacter) return false;
    if (!bIsVirus || !bIsAlive) return false;
    if (!TargetCharacter->bIsAlive || TargetCharacter->bIsVirus) return false;

    float Distance = FVector::Dist(GetActorLocation(), TargetCharacter->GetActorLocation());
    return Distance <= InfectionRadius * 1.5f;
}

void AInsideTheMainframeCharacter::Server_TryInfect_Implementation(
    AInsideTheMainframeCharacter* TargetCharacter)
{
	TargetCharacter->OnBecomeVirus();

	// Notificar al PlayerController del infectado
	if (AInsideTheMainframePlayerController* PC =
		TargetCharacter->GetMainframePlayerController())
	{
		PC->Client_OnInfected();
	}

	if (AInsideTheMainframePlayerState* MyPS = GetMainframePlayerState())
		MyPS->AddInfectionPoint();

	TargetCharacter->Multicast_PlayInfectionEffect();

	UE_LOG(LogTemp, Warning, TEXT("[SERVER] %s infectó a %s"),
		*GetName(), *TargetCharacter->GetName());
	
}

// -------------------------------------------------------------------------
// OnBecomeVirus
// -------------------------------------------------------------------------
void AInsideTheMainframeCharacter::OnBecomeVirus()
{
   
	if (!HasAuthority()) return;
    
	// Solo actualizar la variable del Character
	// NO llamar a PS->SetAsVirus() — ya venimos desde ahí
	bIsVirus = true;

	UE_LOG(LogTemp, Warning, TEXT("[CHARACTER] %s bIsVirus seteado a true"), *GetName());
}

// -------------------------------------------------------------------------
// Die
// -------------------------------------------------------------------------
void AInsideTheMainframeCharacter::Die()
{
    if (!HasAuthority()) return;
    bIsAlive = false;

    if (AInsideTheMainframePlayerState* PS = GetMainframePlayerState())
        PS->SetAsDead();

    Multicast_OnDeath();
}

// -------------------------------------------------------------------------
// UpdateVisuals
// -------------------------------------------------------------------------
void AInsideTheMainframeCharacter::UpdateVisuals()
{
    UMaterialInterface* Mat = bIsVirus ? VirusMaterial : AntivirusMaterial;
    if (Mat && GetMesh())
        GetMesh()->SetMaterial(0, Mat);
	
}

// -------------------------------------------------------------------------
// Multicast_PlayInfectionEffect
// -------------------------------------------------------------------------
void AInsideTheMainframeCharacter::Multicast_PlayInfectionEffect_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("[MULTICAST] Efecto de infección en %s"), *GetName());
}

// -------------------------------------------------------------------------
// Multicast_OnDeath
// -------------------------------------------------------------------------
void AInsideTheMainframeCharacter::Multicast_OnDeath_Implementation()
{
    GetCharacterMovement()->DisableMovement();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    UE_LOG(LogTemp, Warning, TEXT("[MULTICAST] Muerte de %s"), *GetName());
}

// -------------------------------------------------------------------------
// RepNotifies
// -------------------------------------------------------------------------
void AInsideTheMainframeCharacter::OnRep_Health()
{
    UE_LOG(LogTemp, Verbose, TEXT("[CLIENT] Health: %.0f"), Health);
}

void AInsideTheMainframeCharacter::OnRep_IsAlive()
{
    if (!bIsAlive)
    {
        GetCharacterMovement()->DisableMovement();
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void AInsideTheMainframeCharacter::OnRep_IsVirus()
{
    UpdateVisuals();
    UE_LOG(LogTemp, Warning, TEXT("[CLIENT] %s es ahora %s"),
        *GetName(), bIsVirus ? TEXT("VIRUS") : TEXT("ANTIVIRUS"));
	if (AInsideTheMainframePlayerState* PS = GetMainframePlayerState())
	{
		bIsVirus = PS->IsVirus();
	}
	BP_VirusChange(bIsVirus);
}

// -------------------------------------------------------------------------
// Helpers
// -------------------------------------------------------------------------
AInsideTheMainframePlayerState* AInsideTheMainframeCharacter::GetMainframePlayerState() const
{
    return GetPlayerState<AInsideTheMainframePlayerState>();
}

AInsideTheMainframePlayerController* AInsideTheMainframeCharacter::GetMainframePlayerController() const
{
    return Cast<AInsideTheMainframePlayerController>(GetController());
}

// -------------------------------------------------------------------------
// TrySpendEnergy
// -------------------------------------------------------------------------
bool AInsideTheMainframeCharacter::TrySpendEnergy(float Amount)
{
    if (!HasAuthority()) return false;
    if (Energy < Amount) return false;
 
    Energy = FMath::Max(0.f, Energy - Amount);
 
    UE_LOG(LogTemp, Log, TEXT("[ENERGY] %s gastó %.0f energía. Restante: %.0f"),
        *GetName(), Amount, Energy);
 
    return true;
}
 
// -------------------------------------------------------------------------
// RechargeFromCapacitor — llamado desde el BP del capacitor
// como es un BP quien llama, necesitamos el Server RPC
// -------------------------------------------------------------------------
void AInsideTheMainframeCharacter::RechargeFromCapacitor()
{
    // El BP llama esto en el cliente — mandamos al servidor
    Server_RechargeFromCapacitor();
}
 
bool AInsideTheMainframeCharacter::Server_RechargeFromCapacitor_Validate()
{
    return true;
}
 
void AInsideTheMainframeCharacter::Server_RechargeFromCapacitor_Implementation()
{
    float OldEnergy = Energy;
    Energy = FMath::Min(MaxEnergy, Energy + CapacitorRechargeAmount);
 
    UE_LOG(LogTemp, Warning, TEXT("[ENERGY] %s recargó en capacitor: %.0f → %.0f"),
        *GetName(), OldEnergy, Energy);
}
 
// -------------------------------------------------------------------------
// PassiveEnergyTick — recarga lenta cada segundo
// -------------------------------------------------------------------------
void AInsideTheMainframeCharacter::PassiveEnergyTick()
{
    if (!HasAuthority()) return;
    if (!bIsAlive) return;
    if (Energy >= MaxEnergy) return;
 
    Energy = FMath::Min(MaxEnergy, Energy + PassiveRechargeRate);
}
 
// -------------------------------------------------------------------------
// OnRep_Energy — el cliente recibió el nuevo valor de energía
// -------------------------------------------------------------------------
void AInsideTheMainframeCharacter::OnRep_Energy()
{
    // Actualizar la barra de energía en el HUD
    // Cuando tengas el HUD actualizado:
    // if (AInsideTheMainframePlayerController* PC = GetMainframePlayerController())
    //     PC->UpdateHUDEnergy(Energy, MaxEnergy);
 
    UE_LOG(LogTemp, Verbose, TEXT("[CLIENT] Energía actualizada: %.0f / %.0f"),
        Energy, MaxEnergy);
}
// -------------------------------------------------------------------------
// ActivateCone — input del cliente, manda al servidor
// -------------------------------------------------------------------------
void AInsideTheMainframeCharacter::ActivateCone()
{
    if (!bIsVirus || !bIsAlive) return;
    if (bConeActive) return;   // Ya está activo
 
    Server_ActivateCone();
}
 
// -------------------------------------------------------------------------
// Server_ActivateCone
// -------------------------------------------------------------------------
bool AInsideTheMainframeCharacter::Server_ActivateCone_Validate()
{
    if (!bIsVirus || !bIsAlive) return false;
    if (Energy < AbilityCost) return false;
    return true;
}
 
void AInsideTheMainframeCharacter::Server_ActivateCone_Implementation()
{
    // Gastar energía
    if (!TrySpendEnergy(AbilityCost)) return;
 
    bConeActive = true;
    InfectionTimers.Empty();
 
    UE_LOG(LogTemp, Warning, TEXT("[CONE] %s activó el cono de infección"), *GetName());
 
    // Efecto visual en todos los clientes
    Multicast_PlayConeEffect();
 
    // Arrancar el tick del cono — cada 0.1 segundos
    GetWorldTimerManager().SetTimer(
        ConeTickHandle,
        this,
        &AInsideTheMainframeCharacter::ConeTickUpdate,
        0.1f,
        true
    );
 
    // El cono dura máximo 2 segundos aunque no infecte a nadie
    FTimerHandle ConeLifetimeHandle;
    GetWorldTimerManager().SetTimer(
        ConeLifetimeHandle,
        [this]()
        {
            // Cancelar infecciones pendientes
            TArray<AInsideTheMainframeCharacter*> Keys;
            InfectionTimers.GetKeys(Keys);
            for (AInsideTheMainframeCharacter* Target : Keys)
            {
                CancelInfectionOnTarget(Target);
            }
            GetWorldTimerManager().ClearTimer(ConeTickHandle);
            bConeActive = false;
            InfectionTimers.Empty();
        },
        2.f,
        false
    );
}
 
// -------------------------------------------------------------------------
// ConeTickUpdate — corre cada 0.1s en el servidor mientras el cono está activo
// -------------------------------------------------------------------------
void AInsideTheMainframeCharacter::ConeTickUpdate()
{
    if (!HasAuthority() || !bConeActive) return;
 
    // Obtener targets actuales en el cono
    TArray<AInsideTheMainframeCharacter*> CurrentTargets = GetTargetsInCone();
 
    // Chequear targets que salieron del cono — cancelar su infección
    TArray<AInsideTheMainframeCharacter*> Keys;
    InfectionTimers.GetKeys(Keys);
    for (AInsideTheMainframeCharacter* TrackedTarget : Keys)
    {
        if (!CurrentTargets.Contains(TrackedTarget))
        {
            CancelInfectionOnTarget(TrackedTarget);
        }
    }
 
    // Actualizar timers de targets que están en el cono
    for (AInsideTheMainframeCharacter* Target : CurrentTargets)
    {
        if (!Target || !Target->bIsAlive || Target->bIsVirus) continue;
 
        // Inicializar timer si es la primera vez que entra al cono
        if (!InfectionTimers.Contains(Target))
        {
            InfectionTimers.Add(Target, 0.f);
            // Avisar al target que está siendo infectado
            Target->Multicast_StartBeingInfected(true);
            UE_LOG(LogTemp, Warning, TEXT("[CONE] %s entró al cono"), *Target->GetName());
        }
 
        // Incrementar el timer
        InfectionTimers[Target] += 0.1f;
 
        // Actualizar progreso replicado para el HUD
        Target->InfectionProgress = InfectionTimers[Target] / InfectionTime;
 
        UE_LOG(LogTemp, Verbose, TEXT("[CONE] %s progreso: %.1f / %.1f"),
            *Target->GetName(), InfectionTimers[Target], InfectionTime);
 
        // ¿Se cumplió el tiempo de infección?
        if (InfectionTimers[Target] >= InfectionTime)
        {
            UE_LOG(LogTemp, Warning, TEXT("[CONE] %s infectado!"), *Target->GetName());
            Target->Multicast_StartBeingInfected(false);
            Target->InfectionProgress = 0.f;
            InfectionTimers.Remove(Target);
            Target->OnBecomeVirus();
 
            // Notificar al PlayerController del infectado
            if (AInsideTheMainframePlayerController* PC =
                Target->GetMainframePlayerController())
            {
                PC->Client_OnInfected();
            }
 
            // Sumar punto al Virus
            if (AInsideTheMainframePlayerState* MyPS = GetMainframePlayerState())
                MyPS->AddInfectionPoint();
 
            Target->Multicast_PlayInfectionEffect();
        }
    }
}
 
// -------------------------------------------------------------------------
// GetTargetsInCone — devuelve todos los Antivirus dentro del cono
// -------------------------------------------------------------------------
TArray<AInsideTheMainframeCharacter*> AInsideTheMainframeCharacter::GetTargetsInCone() const
{
    TArray<AInsideTheMainframeCharacter*> Result;
    TArray<AActor*> AllCharacters;
 
    UGameplayStatics::GetAllActorsOfClass(GetWorld(),
        AInsideTheMainframeCharacter::StaticClass(), AllCharacters);
 
    for (AActor* Actor : AllCharacters)
    {
        if (Actor == this) continue;
 
        AInsideTheMainframeCharacter* Other = Cast<AInsideTheMainframeCharacter>(Actor);
        if (!Other || !Other->bIsAlive || Other->bIsVirus) continue;
 
        if (IsInCone(Other))
            Result.Add(Other);
    }
 
    return Result;
}
 
// -------------------------------------------------------------------------
// IsInCone — verifica si un target está dentro del cono
// -------------------------------------------------------------------------
bool AInsideTheMainframeCharacter::IsInCone(AInsideTheMainframeCharacter* Target) const
{
    if (!Target) return false;
 
    FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
    float Distance   = ToTarget.Size();
 
    // Verificar distancia
    if (Distance > ConeRange) return false;
 
    // Verificar ángulo — comparamos con la dirección hacia adelante
    ToTarget.Normalize();
   // FVector Forward = GetActorForwardVector();
	FVector Forward = GetBaseAimRotation().Vector();
 
    float DotProduct  = FVector::DotProduct(Forward, ToTarget);
    float AngleRad    = FMath::Acos(DotProduct);
    float AngleDeg    = FMath::RadiansToDegrees(AngleRad);
 
    // ConeAngle es el ángulo total, dividimos por 2 para comparar con el half-angle
    return AngleDeg <= (ConeAngle * 0.5f);
}
 
// -------------------------------------------------------------------------
// CancelInfectionOnTarget — el target salió del cono
// -------------------------------------------------------------------------
void AInsideTheMainframeCharacter::CancelInfectionOnTarget(
    AInsideTheMainframeCharacter* Target)
{
    if (!Target) return;
 
    InfectionTimers.Remove(Target);
    Target->InfectionProgress = 0.f;
    Target->Multicast_StartBeingInfected(false);
 
    UE_LOG(LogTemp, Warning, TEXT("[CONE] %s escapó del cono"), *Target->GetName());
}

void AInsideTheMainframeCharacter::StartAim()
{
	bIsAim = true;
	bUseControllerRotationYaw = true;
}

void AInsideTheMainframeCharacter::StopAim()
{
	bIsAim = false;
	bUseControllerRotationYaw = false;
}

// -------------------------------------------------------------------------
// Multicast_PlayConeEffect — efecto visual del cono
// -------------------------------------------------------------------------
void AInsideTheMainframeCharacter::Multicast_PlayConeEffect_Implementation()
{
    // Acá reproducís el Niagara del cono, sonido, etc.
    UE_LOG(LogTemp, Warning, TEXT("[MULTICAST] Efecto de cono activado por %s"), *GetName());
	if (ConeEffectComponent && ConeNiagaraSystem)
	{
		ConeEffectComponent->SetAsset(ConeNiagaraSystem);
		ConeEffectComponent->Activate(true);  // true = reset y arranca
	}
	
	if (ConeMesh)
	{
		ConeMesh->SetVisibility(true);

		// Ocultarlo después de 2 segundos — igual que la duración del cono
		FTimerHandle HideHandle;
		GetWorldTimerManager().SetTimer(HideHandle, [this]()
		{
			if (ConeMesh)
				ConeMesh->SetVisibility(false);
		}, 2.f, false);
	}
}
 
// -------------------------------------------------------------------------
// Multicast_StartBeingInfected — avisa al target si está siendo infectado
// -------------------------------------------------------------------------
void AInsideTheMainframeCharacter::Multicast_StartBeingInfected_Implementation(bool bStart)
{
    bBeingInfected = bStart;
 
    if (!bStart)
        InfectionProgress = 0.f;
 
    UE_LOG(LogTemp, Warning, TEXT("[MULTICAST] %s siendo infectado: %s"),
        *GetName(), bStart ? TEXT("SI") : TEXT("NO"));
}
 
// -------------------------------------------------------------------------
// OnRep_bBeingInfected — el cliente recibe el estado de infección
// -------------------------------------------------------------------------
void AInsideTheMainframeCharacter::OnRep_bBeingInfected()
{
    // Si este es el jugador local, mostrar/ocultar el indicador de infección en el HUD
    if (IsLocallyControlled())
    {
        // Cuando tengas el HUD actualizado:
    	if (AInsideTheMainframePlayerController* PC = GetMainframePlayerController())
    	{
    		    		PC->UpdateHUDInfectionWarning(bBeingInfected, InfectionProgress);

    	}
 
        UE_LOG(LogTemp, Warning, TEXT("[CLIENT] %s — siendo infectado: %s"),
            *GetName(), bBeingInfected ? TEXT("SI") : TEXT("NO"));
    }
}
 
// -------------------------------------------------------------------------
// OnRep_InfectionProgress — actualizar barra de progreso en el HUD
// -------------------------------------------------------------------------
void AInsideTheMainframeCharacter::OnRep_InfectionProgress()
{
    if (IsLocallyControlled())
    {
        // Cuando tengas el HUD actualizado:
        
    	if (AInsideTheMainframePlayerController* PC = GetMainframePlayerController())
    	{
    		    		PC->UpdateHUDInfectionWarning(bBeingInfected, InfectionProgress);

    	}
        UE_LOG(LogTemp, Verbose, TEXT("[CLIENT] Progreso infección: %.0f%%"),
            InfectionProgress * 100.f);
    }
}
