#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "InsideTheMainframeCharacter.generated.h"
 


class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;
class AInsideTheMainframePlayerController;
class AInsideTheMainframePlayerState;
class UCapsuleComponent;

 

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS()
class AInsideTheMainframeCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AimAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ShotAction;
	

	// En la sección protected:
	UPROPERTY(VisibleAnywhere, Category = "Effects")
	UNiagaraComponent* ConeEffectComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UNiagaraSystem* ConeNiagaraSystem;
	
	UPROPERTY(VisibleAnywhere, Category = "Effects")
	UStaticMeshComponent* ConeMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UStaticMesh* ConeMeshAsset;

public:
    // -------------------------------------------------------------------------
    // Energía — replicada para que el HUD de cada cliente muestre su propia barra
    // -------------------------------------------------------------------------

    UPROPERTY(ReplicatedUsing = OnRep_Energy, BlueprintReadOnly, Category = "Energy")
    float Energy;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Energy")
    float MaxEnergy;

    UPROPERTY(EditDefaultsOnly, Category = "Energy")
    float PassiveRechargeRate;

    UPROPERTY(EditDefaultsOnly, Category = "Energy")
    float CapacitorRechargeAmount;

    UPROPERTY(EditDefaultsOnly, Category = "Energy")
    float AbilityCost;


    bool TrySpendEnergy(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Energy")
    void RechargeFromCapacitor();

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_RechargeFromCapacitor();
	
	// Clase del proyectil según rol — asignadas desde el BP hijo
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<AActor> VirusProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<AActor> AntivirusProjectileClass;

	// Punto de spawn del proyectil — un socket o un componente en el mesh
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	FName ProjectileSpawnSocket = TEXT("ProjectileSpawn");
	
	public:
    // -------------------------------------------------------------------------
    // Configuración del cono — editables desde el BP
    // -------------------------------------------------------------------------
 
    // Ángulo del cono en grados (60 = 30 grados a cada lado)
    UPROPERTY(EditDefaultsOnly, Category = "Infection|Cone")
    float ConeAngle;
 
    // Distancia máxima del cono
    UPROPERTY(EditDefaultsOnly, Category = "Infection|Cone")
    float ConeRange;
 
    // Tiempo que tiene que estar dentro del cono para infectarse
    UPROPERTY(EditDefaultsOnly, Category = "Infection|Cone")
    float InfectionTime;
 
    // -------------------------------------------------------------------------
    // Estado de infección por cono — replicado para el HUD del Antivirus
    // -------------------------------------------------------------------------
 
    // Si este jugador está siendo infectado actualmente
    UPROPERTY(ReplicatedUsing = OnRep_bBeingInfected, BlueprintReadOnly, Category = "Infection|Cone")
    bool bBeingInfected;
 
    // Progreso de infección 0-1 — para la barra del HUD
    UPROPERTY(ReplicatedUsing = OnRep_InfectionProgress, BlueprintReadOnly, Category = "Infection|Cone")
    float InfectionProgress;
 
    // -------------------------------------------------------------------------
    // Funciones del cono
    // -
    // Server RPC — el cliente pide activar el cono
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_ActivateCone();
 
    // Multicast — efecto visual del cono en todos los clientes
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayConeEffect();
 
    // Multicast — avisa al Antivirus que está siendo infectado
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_StartBeingInfected(bool bStart);
 
protected:
    // -------------------------------------------------------------------------
    // Estado interno del cono (solo servidor)
    // -------------------------------------------------------------------------
 
    // Mapa de target → tiempo acumulado de infección
    // Usamos TMap para trackear múltiples targets a la vez
    TMap<AInsideTheMainframeCharacter*, float> InfectionTimers;
 
    // Timer que tickea mientras el cono está activo
    FTimerHandle ConeTickHandle;
 
    // Si el cono está activo actualmente
    bool bConeActive;
 
    // -------------------------------------------------------------------------
    // Lógica interna
    // -------------------------------------------------------------------------
 
    // Activa el cono — llama al Server RPC
    void ActivateCone();
 
    // Tick del cono — corre en el servidor cada 0.1 segundos
    void ConeTickUpdate();
 
    // Devuelve todos los Antivirus dentro del cono en este momento
    TArray<AInsideTheMainframeCharacter*> GetTargetsInCone() const;
 
    // Verifica si un actor está dentro del cono
    bool IsInCone(AInsideTheMainframeCharacter* Target) const;
 
    // Cancela la infección de un target (salió del cono)
    void CancelInfectionOnTarget(AInsideTheMainframeCharacter* Target);
	
	bool bIsAim = false;
	
	void StartAim();
	void StopAim();
 
    // RepNotifies
    UFUNCTION()
    void OnRep_bBeingInfected();
 
    UFUNCTION()
    void OnRep_InfectionProgress();

protected:
    // RepNotify de energía — actualiza la barra del HUD en el cliente
    UFUNCTION()
    void OnRep_Energy();

    // Recarga pasiva — llamada cada segundo por un timer
    void PassiveEnergyTick();

    // Handle del timer de recarga pasiva
    FTimerHandle EnergyRechargeTimerHandle;
	
	//referencia a la ram
	UPROPERTY(EditDefaultsOnly, Category = "Ram")
	AActor* RamRef=nullptr;
	
	UFUNCTION(BlueprintCallable)
	void SetRamRef(AActor* ram);
	
	UFUNCTION(BlueprintCallable)
	void ClearRamRef();

	/** Constructor */
	AInsideTheMainframeCharacter();
	virtual void BeginPlay() override;

protected:
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Shooting();

	void Shooting();

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();
	
	

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "Health")
	float Health;

	UPROPERTY(ReplicatedUsing = OnRep_IsAlive, BlueprintReadOnly, Category = "Health")
	bool bIsAlive;

	UPROPERTY(ReplicatedUsing = OnRep_IsVirus, BlueprintReadOnly, Category = "Role")
	bool bIsVirus;
	
	public:
    // -------------------------------------------------------------------------
    // Configuración
    // -------------------------------------------------------------------------
    UPROPERTY(EditDefaultsOnly, Category = "Health")
    float MaxHealth;

    UPROPERTY(EditDefaultsOnly, Category = "Infection")
    float InfectionRadius;

    UPROPERTY(EditDefaultsOnly, Category = "Infection")
    float InfectionCooldown;

    UPROPERTY(EditDefaultsOnly, Category = "Visual")
    UMaterialInterface* VirusMaterial;

    UPROPERTY(EditDefaultsOnly, Category = "Visual")
    UMaterialInterface* AntivirusMaterial;

    // -------------------------------------------------------------------------
    // RPCs
    // -------------------------------------------------------------------------
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_TryInfect(AInsideTheMainframeCharacter* TargetCharacter);

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayInfectionEffect();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnDeath();
	
	UFUNCTION()
	void OnRep_IsVirus();
	
	UFUNCTION(BlueprintImplementableEvent)
	void BP_VirusChange(bool isVirus);
	


    UFUNCTION(BlueprintPure, Category = "Player")
    AInsideTheMainframePlayerState* GetMainframePlayerState() const;

    UFUNCTION(BlueprintPure, Category = "Player")
    AInsideTheMainframePlayerController* GetMainframePlayerController() const;

    void OnBecomeVirus();

    // -------------------------------------------------------------------------
    // Replicación
    // -------------------------------------------------------------------------
    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    // Input de infección
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* InfectAction;

    void TryInfectNearby();

    // Lógica de infección
    float LastInfectionTime;
    AInsideTheMainframeCharacter* FindNearbyAntivirus() const;

    // RepNotifies
    UFUNCTION()
    void OnRep_Health();

    UFUNCTION()
    void OnRep_IsAlive();

 

    // Lógica interna
    void Die();
    void UpdateVisuals();
};

