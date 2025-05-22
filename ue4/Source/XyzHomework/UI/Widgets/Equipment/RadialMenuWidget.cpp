// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/Equipment/RadialMenuWidget.h"

#include "Actors/Equipment/EquipmentItem.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/ScaleBox.h"
#include "Components/TextBlock.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

void URadialMenuWidget::InitializeWidget(UCharacterEquipmentComponent* EquipmentComponent_In)
{
	EquipmentComponent = EquipmentComponent_In;
}

void URadialMenuWidget::UpdateMenuSegmentWidgets()
{
	if (!IsValid(EquipmentComponent))
	{
		return;
	}

	for (int32 i = 0; i < ItemSlotsInMenuSegments.Num(); ++i)
	{
		if (UImage* ImageIcon = MenuSegmentWidgets[i]->WidgetTree->FindWidget<UImage>(SegmentItemIconName))
		{
			const AEquipmentItem* EquipmentItem = EquipmentComponent->GetEquippedItem((uint32)ItemSlotsInMenuSegments[i]);
			if (IsValid(EquipmentItem) && IsValid(EquipmentItem->GetLinkedInventoryItem()))
			{
				ImageIcon->SetBrushFromTexture(EquipmentItem->GetLinkedInventoryItem()->GetItemDescription().Icon);
			}
			else
			{
				ImageIcon->SetBrushFromTexture(nullptr);
			}
		}
	}
}

void URadialMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ItemSlotsInMenuSegments.Num() > 0)
	{
		RadialMenuOffsetAngle = -PI / ItemSlotsInMenuSegments.Num();
	}

	if (RadialBackground)
	{
		RadialBackground->SetRenderTransformAngle(FMath::RadiansToDegrees(RadialMenuOffsetAngle));
		if (!BackgroundMaterial)
		{
			BackgroundMaterial = RadialBackground->GetDynamicMaterial();
		}
		BackgroundMaterial->SetScalarParameterValue(SegmentCountMaterialParamName, ItemSlotsInMenuSegments.Num());
	}

	CreateMenuSegmentWidgets();
	GenerateMenuSegmentGraph();
	OnNewSegmentSelected(0);
}

FReply URadialMenuWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (IsVisible() && InMouseEvent.GetCursorDelta().SizeSquared() > MinMouseDeltaSquared)
	{
		FVector2D WidgetCenterPosition = InGeometry.GetAbsolutePosition() + InGeometry.GetAbsoluteSize() * .5f;
		int32 NewSegmentIndex = GetSelectedMenuSegmentIndex(WidgetCenterPosition, InMouseEvent.GetScreenSpacePosition());
		OnNewSegmentSelected(NewSegmentIndex);
	}

	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

void URadialMenuWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (IsVisible())
	{
		FVector2D WidgetCenterPosition = InGeometry.GetAbsolutePosition() + InGeometry.GetAbsoluteSize() * .5f;
		int32 NewSegmentIndex = GetSelectedMenuSegmentIndex(WidgetCenterPosition, InMouseEvent.GetScreenSpacePosition());
		OnNewSegmentSelected(NewSegmentIndex);
	}
}

FReply URadialMenuWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FKey MouseBtn = InMouseEvent.GetEffectingButton();
	if (MouseBtn == EKeys::LeftMouseButton)
	{
		ConfirmSegmentSelection();

		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void URadialMenuWidget::CreateMenuSegmentWidgets()
{
	if (!Canvas || !RadialMenu || !MenuSegmentWidgetClass.LoadSynchronous())
	{
		return;
	}

	const UCanvasPanelSlot* RadialMenuSlot = Cast<UCanvasPanelSlot>(RadialMenu->Slot);
	if (!RadialMenuSlot)
	{
		return;
	}

	int32 MenuSegmentsNum = ItemSlotsInMenuSegments.Num();
	MenuSegmentWidgets.Reserve(MenuSegmentsNum);

	float AngleStep = FMath::RadiansToDegrees(RadialMenuArcAngle / MenuSegmentsNum);
	for (int32 i = 0; i < MenuSegmentsNum; ++i)
	{
		UUserWidget* MenuSegmentWidget = CreateWidgetInstance(*this, MenuSegmentWidgetClass.LoadSynchronous(), FName(FString::Printf(TEXT("MenuSegmentImage%i"), i)));
		if (!MenuSegmentWidget)
		{
			continue;
		}
		Canvas->AddChildToCanvas(MenuSegmentWidget);

		float PositionAngle = AngleStep * i + RadialMenuOffsetAngle;
		UCanvasPanelSlot* SegmentImageSlot = Cast<UCanvasPanelSlot>(MenuSegmentWidget->Slot);
		if (!SegmentImageSlot)
		{
			Canvas->RemoveChild(MenuSegmentWidget);
			continue;
		}
		SegmentImageSlot->SetAutoSize(true);
		SegmentImageSlot->SetAnchors(FAnchors(.5f, .5f, .5f, .5f));
		SegmentImageSlot->SetAlignment(FVector2D(.5f, .5f));
		FVector2D NewImagePosition = SegmentImageSlot->GetPosition() + UpVector.GetRotated(PositionAngle) * (RadialMenuSlot->GetSize().Y * .35f);
		SegmentImageSlot->SetPosition(NewImagePosition);

		MenuSegmentWidgets.Add(MenuSegmentWidget);
	}

	UpdateMenuSegmentWidgets();
}

void URadialMenuWidget::ConfirmSegmentSelection()
{
	if (IsValid(EquipmentComponent) && CurrentSegmentIndex >= 0 && CurrentSegmentIndex < ItemSlotsInMenuSegments.Num())
	{
		EEquipmentItemSlot SelectedSlot = ItemSlotsInMenuSegments[CurrentSegmentIndex];
		EquipmentComponent->EquipItemBySlot(SelectedSlot);
		if (OnMenuSegmentSelectedEvent.IsBound())
		{
			OnMenuSegmentSelectedEvent.Broadcast();
		}
	}
}

int32 URadialMenuWidget::GetSelectedMenuSegmentIndex(FVector2D WidgetAbsolutePosition, FVector2D MouseScreenPosition) const
{
	FVector2D RelativeMousePositionDirection = MouseScreenPosition - WidgetAbsolutePosition;
	RelativeMousePositionDirection.Normalize();
	int32 DotSign = RelativeMousePositionDirection.X >= 0 ? 1 : -1; // Converting from 0..PI range to 0..2PI range
	float AngleOffset = RelativeMousePositionDirection.X >= 0.f ? 0.f : PI; // Converting from 0..PI range to 0..2PI range
	float RelativeMousePositionAngle = AngleOffset + FMath::Acos(DotSign * FVector2D::DotProduct(UpVector, RelativeMousePositionDirection)) - RadialMenuOffsetAngle;
	RelativeMousePositionAngle = RelativeMousePositionAngle > 2 * PI ? RelativeMousePositionAngle - 2 * PI : RelativeMousePositionAngle;
	TSharedPtr<FMenuSegment> SelectedSegment = FindNode(MenuSegments, RelativeMousePositionAngle);
	if (SelectedSegment)
	{
		return SelectedSegment->MenuSegmentIndex;
	}

	return -1;
}

void URadialMenuWidget::OnNewSegmentSelected(int32 NewSegmentIndex)
{
	if (!EquippedItemText || NewSegmentIndex == CurrentSegmentIndex || NewSegmentIndex < 0 || NewSegmentIndex >= ItemSlotsInMenuSegments.Num())
	{
		return;
	}

	CurrentSegmentIndex = NewSegmentIndex;
	BackgroundMaterial->SetScalarParameterValue(SegmentIndexMaterialParamName, NewSegmentIndex);
	
	if (!IsValid(EquipmentComponent))
	{
		return;
	}

	const AEquipmentItem* EquipmentItem = EquipmentComponent->GetEquippedItem((uint32)ItemSlotsInMenuSegments[CurrentSegmentIndex]);
	if (IsValid(EquipmentItem) && IsValid(EquipmentItem->GetLinkedInventoryItem()))
	{
		EquippedItemText->SetText(EquipmentItem->GetLinkedInventoryItem()->GetItemDescription().Name);
	}
	else
	{
		EquippedItemText->SetText(FText());
	}
}

#pragma region MENU SEGMENTS GRAPH

void URadialMenuWidget::GenerateMenuSegmentGraph()
{
	if (bGraphGenerated)
	{
		return;
	}

	MaxGraphDepth = FMath::Log2(ItemSlotsInMenuSegments.Num()) - 1; // decreased by one to have at least 2 segments in one node
	MenuSegments = MakeShared<FMenuSegment>();
	MenuSegments->RightEdgeAngle = RadialMenuArcAngle;
	AddChildNodes(MenuSegments, RadialMenuArcAngle);

	float AngleStep = RadialMenuArcAngle / ItemSlotsInMenuSegments.Num();
	for (int32 i = 0; i < ItemSlotsInMenuSegments.Num(); ++i)
	{
		TSharedPtr<FMenuSegment> NewSegment = MakeShared<FMenuSegment>();
		NewSegment->LeftEdgeAngle = AngleStep * i;
		NewSegment->RightEdgeAngle = AngleStep * (i + 1);
		NewSegment->MenuSegmentIndex = i;
		AddLeafNode(MenuSegments, NewSegment);
	}

	bGraphGenerated = true;
}

void URadialMenuWidget::AddChildNodes(const TSharedPtr<FMenuSegment>& ParentNode, float ArcAngle, uint32 Depth/* = 0*/)
{
	if (Depth >= MaxGraphDepth)
	{
		return;
	}

	float RotationAngle = ArcAngle / GraphNodeDivisions;
	for (uint32 i = 0; i < GraphNodeDivisions; ++i)
	{
		TSharedPtr<FMenuSegment> NewSegment = MakeShared<FMenuSegment>();
		NewSegment->LeftEdgeAngle = ParentNode->LeftEdgeAngle + RotationAngle * i;
		NewSegment->RightEdgeAngle = ParentNode->LeftEdgeAngle + RotationAngle * (i + 1);
		AddChildNodes(NewSegment, RotationAngle, Depth + 1);
		ParentNode->EnclosedSegments.Add(NewSegment);
	}
}

void URadialMenuWidget::AddLeafNode(const TSharedPtr<FMenuSegment>& ParentNode, const TSharedPtr<FMenuSegment>& LeafNode, uint32 Depth/* = 0*/)
{
	if ((LeafNode->LeftEdgeAngle >= ParentNode->LeftEdgeAngle && LeafNode->LeftEdgeAngle < ParentNode->RightEdgeAngle)
		|| (LeafNode->RightEdgeAngle > ParentNode->LeftEdgeAngle && LeafNode->RightEdgeAngle <= ParentNode->RightEdgeAngle))
	{
		if (Depth >= MaxGraphDepth)
		{
			ParentNode->EnclosedSegments.Add(LeafNode);
			return;
		}

		for (const TSharedPtr<FMenuSegment>& ChildNode : ParentNode->EnclosedSegments)
		{
			AddLeafNode(ChildNode, LeafNode, Depth + 1);
		}
	}
}

TSharedPtr<FMenuSegment> URadialMenuWidget::FindNode(const TSharedPtr<FMenuSegment>& ParentNode, float RelativeMousePositionAngle, uint32 Depth/* = 0*/) const
{
	if (RelativeMousePositionAngle >= ParentNode->LeftEdgeAngle && RelativeMousePositionAngle < ParentNode->RightEdgeAngle)
	{
		if (Depth > MaxGraphDepth)
		{
			return ParentNode;
		}

		for (TSharedPtr<FMenuSegment>& ChildNode : ParentNode->EnclosedSegments)
		{
			TSharedPtr<FMenuSegment> Result = FindNode(ChildNode, RelativeMousePositionAngle, Depth + 1);
			if (Result)
			{
				return Result;
			}
		}
	}

	return nullptr;
}
#pragma endregion
