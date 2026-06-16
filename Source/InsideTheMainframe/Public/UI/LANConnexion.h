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
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Create;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Join;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> EditableTextBox_Ip;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Confirm;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Back;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Status;

	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> Switcher_Panels;

	
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSoftObjectPtr<UWorld> World;

	
	UFUNCTION() void OnButtonCreateClicked();
	UFUNCTION() void OnButtonJoinClicked();
	UFUNCTION() void OnButtonConfirmClicked();
	UFUNCTION() void OnButtonBackClicked();

	
	void SetStatus(const FString& Message);
};