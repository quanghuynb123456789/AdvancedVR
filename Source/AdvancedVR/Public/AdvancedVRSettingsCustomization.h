#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "AdvancedVRSettings.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "PropertyHandle.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SMultipleOptionTable.h"

#define LOCTEXT_NAMESPACE "FAdvancedVRSettingsCustomization"

class SMapPickerRowWidget : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SMapPickerRowWidget) {}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, FGameBuildConfigPtr InGameBuildConfig)
	{
        GameBuildConfig = InGameBuildConfig;

		ChildSlot
			[
				SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(FMargin(3.0, 2.0))
					.VAlign(VAlign_Center)
					[
						// Warning Icon for whether or not this culture has localization data.
						SNew(SImage)
							.Image(FCoreStyle::Get().GetBrush("Icons.Warning"))
							.Visibility(this, &SMapPickerRowWidget::HandleWarningImageVisibility)
							.ToolTipText(LOCTEXT("FilePathEmpty", "FilePath Is Empty!"))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
							.Text(FText::FromString(GameBuildConfig->MapName))
							.ToolTipText(FText::FromString(GameBuildConfig->MapPath.FilePath))
					]
			];
	}

    EVisibility HandleWarningImageVisibility() const
    {
        return GameBuildConfig->MapPath.FilePath.IsEmpty() ? EVisibility::Visible : EVisibility::Collapsed;
    }

private:
    FGameBuildConfigPtr GameBuildConfig;
};

/**
 * Custom UI for AdvancedVRSettings to add checkboxes for MapsToPackage.
 */
class FAdvancedVRSettingsCustomization : public IDetailCustomization
{
public:
    /** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override
	{
        const UAdvancedVRSettings* AdvancedVRSettings = GetDefault<UAdvancedVRSettings>();

        IDetailCategoryBuilder& MapsCategory = DetailBuilder.EditCategory("AdvancedVRSettings");
        MapsPropertyHandle = DetailBuilder.GetProperty("MapsToPackage", UAdvancedVRSettings::StaticClass());
        MapsPropertyHandle->MarkHiddenByCustomization();
        MapsPropertyArrayHandle = MapsPropertyHandle->AsArray();

        //Init GameBuildConfigList
        GameBuildConfigList.Reset();
        for (const FGameBuildConfig& GameBuildConfig : AdvancedVRSettings->AllGameMaps)
        {
            GameBuildConfigList.Add(MakeShared<FGameBuildConfig>(GameBuildConfig));
        }

        MapsCategory.AddCustomRow(LOCTEXT("MapsToPackageLabel", "Maps To Package"))
            .NameContent()
            .HAlign(HAlign_Fill)
            .VAlign(VAlign_Top)
            [
                SNew(SHorizontalBox)

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(SImage)
                            .Image(FAppStyle::GetBrush(TEXT("Icons.Settings")))
                            .ToolTipText(LOCTEXT("MapsWarning", "Ensure you have selected at least one map for packaging."))
                    ]

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(STextBlock).Text(LOCTEXT("MapsToPackageHeader", "Select Maps to Package:"))
                    ]

            ]
            .ValueContent()
            .HAlign(HAlign_Fill)
            .VAlign(VAlign_Fill)
            [
                SNew(SVerticalBox)

                    /*+ SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 4.0f)
                    .VAlign(VAlign_Center)
                    [
                        SNew(SHorizontalBox)

                            + SHorizontalBox::Slot()
                            .AutoWidth()
                            [
                                SNew(SCheckBox)
                                    .IsChecked(ECheckBoxState::Checked)
                                    [
                                        SNew(STextBlock).Text(LOCTEXT("AllMapsCheckBoxText", "Select All Maps"))
                                    ]
                            ]
                    ]*/

                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SAssignNew(Table, SMultipleOptionTable<FGameBuildConfigPtr>, &GameBuildConfigList)
                            .OnPreBatchSelect(this, &FAdvancedVRSettingsCustomization::OnPreBatchSelect)
                            .OnPostBatchSelect(this, &FAdvancedVRSettingsCustomization::OnPostBatchSelect)
                            .OnGenerateOptionWidget(this, &FAdvancedVRSettingsCustomization::GenerateWidgetForMap)
                            .OnOptionSelectionChanged(this, &FAdvancedVRSettingsCustomization::OnMapSelectionChanged)
                            .IsOptionSelected(this, &FAdvancedVRSettingsCustomization::IsMapSelected)
                            .ListHeight(300.0f)
                    ]

            ];

        UAdvancedVRSettings::OnSettingsUpdated.AddRaw(this, &FAdvancedVRSettingsCustomization::RefreshTable);
	}

    /** Factory method to create an instance of this customization */
    static TSharedRef<IDetailCustomization> MakeInstance()
    {
        return MakeShareable(new FAdvancedVRSettingsCustomization());
    }

    void RefreshTable()
    {
        UE_LOG(LogTemp, Log, TEXT("RefreshTable in AdvancedVRSettingsCustomization"));
        const UAdvancedVRSettings* AdvancedVRSettings = GetDefault<UAdvancedVRSettings>();

        GameBuildConfigList.Reset();

        for (const FGameBuildConfig& GameBuildConfig : AdvancedVRSettings->AllGameMaps)
        {
            UE_LOG(LogTemp, Log, TEXT("AdvancedVRSettingsCustomization.GameBuildConfigList Add %s"), *GameBuildConfig.MapName);
            GameBuildConfigList.Add(MakeShared<FGameBuildConfig>(GameBuildConfig));
        }

        if (Table.IsValid())
        {
            Table->RequestTableRefresh();
        }

        if (!IsInBatchSelectOperation)
        {
            MapsPropertyHandle->NotifyPostChange(EPropertyChangeType::Unspecified);
        }
    }

    void AddMap(FString MapName)
    {
        if (!IsInBatchSelectOperation)
        {
            MapsPropertyHandle->NotifyPreChange();
        }
        TArray<void*> RawData;
        MapsPropertyHandle->AccessRawData(RawData);
        TArray<FString>* RawMapNameStringArray = reinterpret_cast<TArray<FString>*>(RawData[0]);
        if (!RawMapNameStringArray->Contains(MapName))
        {
            RawMapNameStringArray->Add(MapName);
        }

        const UAdvancedVRSettings* AdvancedVRSettings = GetDefault<UAdvancedVRSettings>();
        if (AdvancedVRSettings)
        {
            RawMapNameStringArray->Sort([&](const FString& A, const FString& B)
                {
                    int32 IndexA = AdvancedVRSettings->AllGameMaps.IndexOfByPredicate([&](const FGameBuildConfig& Config) { return Config.MapName == A; });
                    int32 IndexB = AdvancedVRSettings->AllGameMaps.IndexOfByPredicate([&](const FGameBuildConfig& Config) { return Config.MapName == B; });

                    return IndexA < IndexB;
                });
        }

        if (!IsInBatchSelectOperation)
        {
            MapsPropertyHandle->NotifyPostChange(EPropertyChangeType::ArrayAdd);
        }
    }

    void RemoveMap(FString MapName)
    {
        if (!IsInBatchSelectOperation)
        {
            MapsPropertyHandle->NotifyPreChange();
        }
        TArray<void*> RawData;
        MapsPropertyHandle->AccessRawData(RawData);
        TArray<FString>* RawMapNameStringArray = reinterpret_cast<TArray<FString>*>(RawData[0]);
        RawMapNameStringArray->Remove(MapName);
        if (!IsInBatchSelectOperation)
        {
            MapsPropertyHandle->NotifyPostChange(EPropertyChangeType::ArrayRemove);
        }
    }

    void OnPreBatchSelect()
    {
        IsInBatchSelectOperation = true;
        MapsPropertyHandle->NotifyPreChange();
    }

    void OnPostBatchSelect()
    {
        MapsPropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
        IsInBatchSelectOperation = false;
    }

    TSharedRef<SWidget> GenerateWidgetForMap(FGameBuildConfigPtr GameBuildConfig)
    {
        return SNew(SMapPickerRowWidget, GameBuildConfig);
    }

    void OnMapSelectionChanged(bool IsSelected, FGameBuildConfigPtr GameBuildConfig)
    {
        if (IsSelected)
        {
            AddMap(GameBuildConfig->MapName);
        }
        else
        {
            RemoveMap(GameBuildConfig->MapName);
        }

    }

    bool IsMapSelected(FGameBuildConfigPtr GameBuildConfig)
    {
        uint32 ElementCount;
        MapsPropertyArrayHandle->GetNumElements(ElementCount);
        for (uint32 Index = 0; Index < ElementCount; ++Index)
        {
            const TSharedRef<IPropertyHandle> PropertyHandle = MapsPropertyArrayHandle->GetElement(Index);
            FString MapNameAtIndex;
            PropertyHandle->GetValue(MapNameAtIndex);
            if (MapNameAtIndex == GameBuildConfig->MapName)
            {
                return true;
            }
        }

        return false;
    }

private:
    TArray<FGameBuildConfigPtr> GameBuildConfigList;
    TSharedPtr<IPropertyHandle> MapsPropertyHandle;
    TSharedPtr<IPropertyHandleArray> MapsPropertyArrayHandle;
    TSharedPtr< SMultipleOptionTable<FGameBuildConfigPtr> > Table;
    bool IsInBatchSelectOperation;
};


#undef LOCTEXT_NAMESPACE
