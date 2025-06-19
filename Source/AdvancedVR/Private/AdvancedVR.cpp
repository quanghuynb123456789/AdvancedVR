#include "AdvancedVR.h"
#include "AdvancedVRSettings.h"
#if WITH_EDITOR
#include "AdvancedVRSettingsCustomization.h"
#endif
#include "ISettingsContainer.h"
#include "ISettingsModule.h"

#define LOCTEXT_NAMESPACE "FAdvancedVRModule"

void FAdvancedVRModule::StartupModule()
{
    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        ISettingsContainerPtr SettingsContainer = SettingsModule->GetContainer("Project");

        SettingsModule->RegisterSettings("Project", "Plugins", "AdvancedVR",
            LOCTEXT("AdvancedVRSettingsName", "AdvancedVR Settings"),
            LOCTEXT("AdvancedVRSettingsDescription", "Configure AdvancedVR Settings for the AdvancedVR plugin"),
            GetMutableDefault<UAdvancedVRSettings>());
    }

#if WITH_EDITOR
    if (UAdvancedVRSettings* AdvancedVRSettings = GetMutableDefault<UAdvancedVRSettings>())
    {
        AdvancedVRSettings->SyncMapsToCook();
    }

    FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    PropertyEditorModule.RegisterCustomClassLayout(
        "AdvancedVRSettings",
        FOnGetDetailCustomizationInstance::CreateStatic(&FAdvancedVRSettingsCustomization::MakeInstance)
    );

    PropertyEditorModule.NotifyCustomizationModuleChanged();
#endif
}

void FAdvancedVRModule::ShutdownModule()
{
    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        SettingsModule->UnregisterSettings("Project", "Plugins", "AdvancedVR");
    }

#if WITH_EDITOR
    if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
    {
        FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
        PropertyEditorModule.UnregisterCustomClassLayout("AdvancedVRSettings");
    }
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAdvancedVRModule, AdvancedVR)
