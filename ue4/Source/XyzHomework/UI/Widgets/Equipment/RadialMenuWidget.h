// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "Blueprint/UserWidget.h"
#include "RadialMenuWidget.generated.h"

class UCanvasPanel;
class UCharacterEquipmentComponent;
class UImage;
class UScaleBox;
class UTextBlock;

/** Struct describing a segment of the radial menu. */
struct FMenuSegment
{
	float LeftEdgeAngle = 0.f;
	float RightEdgeAngle = 0.f;
	int32 MenuSegmentIndex = -1;
	TArray<TSharedPtr<FMenuSegment>> EnclosedSegments;
};

UCLASS()
class XYZHOMEWORK_API URadialMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE(FOnMenuSegmentSelectedEvent);
	FOnMenuSegmentSelectedEvent OnMenuSegmentSelectedEvent;

	void InitializeWidget(UCharacterEquipmentComponent* EquipmentComponent_In);
	void UpdateMenuSegmentWidgets();

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* Canvas;
	UPROPERTY(meta = (BindWidget))
	UScaleBox* RadialMenu;
	UPROPERTY(meta = (BindWidget))
	UImage* RadialBackground;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* EquippedItemText;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Radial Menu Widget")
	FName SegmentCountMaterialParamName = FName("Segments");
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Radial Menu Widget")
	FName SegmentIndexMaterialParamName = FName("Index");
	/** List of equipment slots presented in the radial menu. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Radial Menu Widget")
	TArray<EEquipmentItemSlot> ItemSlotsInMenuSegments;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Radial Menu Widget")
	TSoftClassPtr<UUserWidget> MenuSegmentWidgetClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Radial Menu Widget")
	FName SegmentItemIconName = FName("ItemIcon");

private:
	void CreateMenuSegmentWidgets();
	/** Instructs the equipment component to equip the equipment item represented in the selected menu segment. */
	void ConfirmSegmentSelection();
	int32 GetSelectedMenuSegmentIndex(FVector2D WidgetAbsolutePosition, FVector2D MouseScreenPosition) const;
	void OnNewSegmentSelected(int32 NewSegmentIndex);

	/** Index of the menu segment currently hovered over by the mouse cursor. */
	int32 CurrentSegmentIndex = -1;
	UPROPERTY()
	TArray<UUserWidget*> MenuSegmentWidgets;
	/** Root node of the menu segments graph. */
	TSharedPtr<FMenuSegment> MenuSegments;
	UPROPERTY()
	UMaterialInstanceDynamic* BackgroundMaterial;
	UPROPERTY()
	UCharacterEquipmentComponent* EquipmentComponent;
	/** Up direction in the canvas coordinates. */
	FVector2D UpVector = FVector2D(0.f, -1.f);
	/** Distance the mouse cursor should move to trigger the reevaluation of a currently selected menu segment.  */
	float MinMouseDeltaSquared = .1f;

#pragma region MENU SEGMENTS GRAPH
	/** Generates a graph of menu segments that is suitable for the binary search. */
	void GenerateMenuSegmentGraph();
	/** Splits the 'ParentNode' menu segment node into a 'GraphNodeDivisions' number of subsegments. */
	void AddChildNodes(const TSharedPtr<FMenuSegment>& ParentNode, float ArcAngle, uint32 Depth = 0);
	/** Adds new leaf nodes at the 'MaxGraphDepth' depth of the graph. These nodes represent the actual radial menu segments. */
	void AddLeafNode(const TSharedPtr<FMenuSegment>& ParentNode, const TSharedPtr<FMenuSegment>& LeafNode, uint32 Depth = 0);
	/** Finds and returns a leaf node located at the 'MaxGraphDepth' depth of the graph based on 'RelativeMousePositionAngle'. */
	TSharedPtr<FMenuSegment> FindNode(const TSharedPtr<FMenuSegment>& ParentNode, float RelativeMousePositionAngle, uint32 Depth = 0) const;

	bool bGraphGenerated = false;
	/** Radial menu circumference (360 degrees by default). */
	float RadialMenuArcAngle = 2 * PI;
	/** Menu segment with index 0 is centered at the top using this offset. This value is dynamically calculated in NativeConstruct(). */
	float RadialMenuOffsetAngle = 0.f;
	/** Every node area is split into this number of subsegments. 2 corresponds to the log(n) graph search complexity. */
	uint32 GraphNodeDivisions = 2;
	/** Dynamically calculated in NativeConstruct() based on the number of actual radial menu segments. */
	uint32 MaxGraphDepth = 1;
#pragma endregion
};
