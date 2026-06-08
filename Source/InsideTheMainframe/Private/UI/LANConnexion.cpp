// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/LANConnexion.h"
#include "Kismet/GameplayStatics.h"

void ULANConnexion::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Input y cursor en modo UI
    FInputModeUIOnly InputMode;
    GetOwningPlayer()->SetInputMode(InputMode);
    GetOwningPlayer()->SetShowMouseCursor(true);

    // Panel principal al arrancar
    Switcher_Panels->SetActiveWidgetIndex(0);
    SetStatus(TEXT(""));

    // IP por defecto para testear en la misma PC
    EditableTextBox_Ip->SetText(FText::FromString(TEXT("127.0.0.1")));

    // Vincular botones
    Button_Create ->OnClicked.AddDynamic(this, &ThisClass::OnButtonCreateClicked);
    Button_Join   ->OnClicked.AddDynamic(this, &ThisClass::OnButtonJoinClicked);
    Button_Confirm->OnClicked.AddDynamic(this, &ThisClass::OnButtonConfirmClicked);
    Button_Back   ->OnClicked.AddDynamic(this, &ThisClass::OnButtonBackClicked);
}

// ─── Panel principal ──────────────────────────────────────────────────────────

void ULANConnexion::OnButtonCreateClicked()
{
    SetStatus(TEXT("Creando servidor..."));

    FInputModeGameOnly InputMode;
    GetOwningPlayer()->SetInputMode(InputMode);
    GetOwningPlayer()->SetShowMouseCursor(false);

    // Abre el nivel como Listen Server (otros pueden conectarse)
    UGameplayStatics::OpenLevelBySoftObjectPtr(this, World, true, TEXT("listen"));
}

void ULANConnexion::OnButtonJoinClicked()
{
    // Mostrar panel de IP
    Switcher_Panels->SetActiveWidgetIndex(1);
    SetStatus(TEXT("Ingresá la IP del host."));
}

// ─── Panel "Unirse" ───────────────────────────────────────────────────────────

void ULANConnexion::OnButtonConfirmClicked()
{
    const FString IpStr = EditableTextBox_Ip->GetText().ToString().TrimStartAndEnd();

    if (IpStr.IsEmpty())
    {
        SetStatus(TEXT("Escribí una IP antes de confirmar."));
        return;
    }

    SetStatus(FString::Printf(TEXT("Conectando a %s..."), *IpStr));

    FInputModeGameOnly InputMode;
    GetOwningPlayer()->SetInputMode(InputMode);
    GetOwningPlayer()->SetShowMouseCursor(false);

    // Viaja a la IP del host (misma PC = 127.0.0.1, red = 192.168.x.x)
    UGameplayStatics::OpenLevel(this, *IpStr);
}

void ULANConnexion::OnButtonBackClicked()
{
    Switcher_Panels->SetActiveWidgetIndex(0);
    SetStatus(TEXT(""));
}

// ─── Helper ───────────────────────────────────────────────────────────────────

void ULANConnexion::SetStatus(const FString& Message)
{
    if (Text_Status)
    {
        Text_Status->SetText(FText::FromString(Message));
    }
}