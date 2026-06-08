// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "LANConnexion.generated.h"

UCLASS()
class INSIDETHEMAINFRAME_API ULANConnexion : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

private:
	// ─── Panel principal (índice 0 del Switcher) ──────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Create;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Join;

	// ─── Panel "Unirse" (índice 1 del Switcher) ───────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> EditableTextBox_Ip;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Confirm;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Back;

	// ─── Texto de estado (visible en ambos paneles) ────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Status;

	// ─── Switcher que alterna entre los dos paneles ────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> Switcher_Panels;

	// ─── Nivel a cargar como host ──────────────────────────────────────────
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSoftObjectPtr<UWorld> World;

	// ─── Callbacks ────────────────────────────────────────────────────────
	UFUNCTION() void OnButtonCreateClicked();
	UFUNCTION() void OnButtonJoinClicked();
	UFUNCTION() void OnButtonConfirmClicked();
	UFUNCTION() void OnButtonBackClicked();

	// ─── Helper ───────────────────────────────────────────────────────────
	void SetStatus(const FString& Message);
};