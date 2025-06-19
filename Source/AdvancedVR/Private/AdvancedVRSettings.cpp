
#include "AdvancedVRSettings.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Interfaces/IProjectManager.h"
#if WITH_EDITOR
#include "Settings/ProjectPackagingSettings.h"
#endif

DEFINE_LOG_CATEGORY(LogAdvancedVRSettings);

TMulticastDelegate<void()> UAdvancedVRSettings::OnSettingsUpdated;

UAdvancedVRSettings::UAdvancedVRSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	PlatformType(EPlatformType::Unknown),
	XRComponentClass(UBaseXRComponent::StaticClass())
{
	
}

FString UAdvancedVRSettings::GetPlatformTypeAsString(EPlatformType Platform)
{
	switch (Platform)
	{
	case EPlatformType::Oculus:   return TEXT("Oculus");
	case EPlatformType::Pico:     return TEXT("Pico");
	case EPlatformType::Vive:     return TEXT("Vive");
	case EPlatformType::Windows:  return TEXT("Windows");
	case EPlatformType::Mobile:   return TEXT("Mobile");
	case EPlatformType::Unknown:  return TEXT("Unknown");
	default:                      return TEXT("Invalid");
	}
}

EPlatformType UAdvancedVRSettings::GetPlatformType()
{
	const UAdvancedVRSettings* AdvancedVRSettings = GetDefault<UAdvancedVRSettings>();
	return AdvancedVRSettings->PlatformType;
}


TSoftClassPtr<UBaseXRComponent> UAdvancedVRSettings::GetXRComponentClass()
{
	const UAdvancedVRSettings* AdvancedVRSettings = GetDefault<UAdvancedVRSettings>();
	return AdvancedVRSettings->XRComponentClass;
}

TArray<FGameBuildConfig> UAdvancedVRSettings::GetAllGames()
{
	TArray<FString> AllMapNames;
	const UAdvancedVRSettings* AdvancedVRSettings = GetDefault<UAdvancedVRSettings>();
	return AdvancedVRSettings->AllGameMaps;
}

TArray<FString> UAdvancedVRSettings::GetAllMapNames()
{
	TArray<FString> AllMapNames;
	const UAdvancedVRSettings* AdvancedVRSettings = GetDefault<UAdvancedVRSettings>();
	for (const FGameBuildConfig& GameBuildConfig : AdvancedVRSettings->AllGameMaps)
	{
		AllMapNames.Add(GameBuildConfig.MapName);
	}
	return AllMapNames;
}

TArray<FGameBuildConfig> UAdvancedVRSettings::GetPackagedGames()
{
	const UAdvancedVRSettings* AdvancedVRSettings = GetDefault<UAdvancedVRSettings>();
	TArray<FGameBuildConfig> GameBuildConfigArray;
	for (const FString& Map : AdvancedVRSettings->MapsToPackage)
	{
		FGameBuildConfig GameBuildConfig;
		if (GetGameBuildConfigByMapName(AdvancedVRSettings, Map, GameBuildConfig))
		{
			GameBuildConfigArray.Add(GameBuildConfig);
		}
	}
	return GameBuildConfigArray;
}

TArray<FString> UAdvancedVRSettings::GetPackagedMapNames()
{
	const UAdvancedVRSettings* AdvancedVRSettings = GetDefault<UAdvancedVRSettings>();
	return AdvancedVRSettings->MapsToPackage;
}

void UAdvancedVRSettings::PostInitProperties()
{
#if WITH_EDITOR
	// Not doing this currently, loading defaults is cool, and I may go back to it later when i get
	// controller offsets for vive/touch/MR vs each other.
	/*if (ControllerProfiles.Num() == 0)
	{
		ControllerProfiles.Add(FBPVRControllerProfile(TEXT("Vive_Wands")));
		ControllerProfiles.Add(FBPVRControllerProfile(TEXT("Oculus_Touch"), ControllerProfileStatics::OculusTouchStaticOffset));
		this->SaveConfig(CPF_Config, *this->GetDefaultConfigFilename());
	}*/
#endif

	Super::PostInitProperties();
}

#if WITH_EDITOR

void UAdvancedVRSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FProperty* PropertyThatChanged = PropertyChangedEvent.MemberProperty;

	UE_LOG(LogAdvancedVRSettings, Log, TEXT("PropertyThatChanged: %s"), *PropertyThatChanged->GetFName().ToString());

	if (PropertyThatChanged != nullptr)
	{
#if WITH_EDITORONLY_DATA
		if (PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(UAdvancedVRSettings, PlatformType))
		{
			UE_LOG(LogAdvancedVRSettings, Log, TEXT("Changed To PlatformType: %s"), *UAdvancedVRSettings::GetPlatformTypeAsString(GetPlatformType()));

			// Get Project Manager
			IProjectManager& ProjectManager = IProjectManager::Get();

			FText FailReason;

			// Modify plugins based on VRSettings->PlatformType
			switch (GetPlatformType())
			{
			case EPlatformType::Oculus:
			{
				ProjectManager.SetPluginEnabled(TEXT("OculusXR"), true, FailReason);
				ProjectManager.SetPluginEnabled(TEXT("PICOXR"), false, FailReason);
				ProjectManager.SetPluginEnabled(TEXT("ViveOpenXR"), false, FailReason);
				break;
			}
			case EPlatformType::Pico:
			{
				ProjectManager.SetPluginEnabled(TEXT("OculusXR"), false, FailReason);
				ProjectManager.SetPluginEnabled(TEXT("PICOXR"), true, FailReason);
				ProjectManager.SetPluginEnabled(TEXT("ViveOpenXR"), false, FailReason);
				break;
			}
			case EPlatformType::Vive:
			{
				ProjectManager.SetPluginEnabled(TEXT("OculusXR"), false, FailReason);
				ProjectManager.SetPluginEnabled(TEXT("PICOXR"), false, FailReason);
				ProjectManager.SetPluginEnabled(TEXT("ViveOpenXR"), true, FailReason);
				break;
			}
			default:
			{
				ProjectManager.SetPluginEnabled(TEXT("OculusXR"), false, FailReason);
				ProjectManager.SetPluginEnabled(TEXT("PICOXR"), false, FailReason);
				ProjectManager.SetPluginEnabled(TEXT("ViveOpenXR"), false, FailReason);
				break;
			}
			}

			// Save the changes to the .uproject file
			if (!ProjectManager.SaveCurrentProjectToDisk(FailReason))
			{
				UE_LOG(LogAdvancedVRSettings, Error, TEXT("Failed To Update Plugins In .uproject: %s"), *FailReason.ToString());
			}
			else 
			{
				UE_LOG(LogAdvancedVRSettings, Log, TEXT("Update Plugins In .uproject Successfully"));
			}
		}
		else if (PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(UAdvancedVRSettings, XRComponentClass))
		{
			UE_LOG(LogAdvancedVRSettings, Log, TEXT("Changed To XRComponentClass: %s"), *GetXRComponentClass().ToString());
		}
		else if (PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(UAdvancedVRSettings, AllGameMaps))
		{
			UE_LOG(LogAdvancedVRSettings, Log, TEXT("Changed AllGameMaps"));
			
			OnSettingsUpdated.Broadcast();
		}
		else if (PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(UAdvancedVRSettings, MapsToPackage))
		{
			UE_LOG(LogAdvancedVRSettings, Log, TEXT("Changed MapToPackage"));
			SyncMapsToCook();
		}
#endif
	}
}
#endif

#if WITH_EDITOR
void UAdvancedVRSettings::SyncMapsToCook()
{
	UProjectPackagingSettings* PackagingSettings = GetMutableDefault<UProjectPackagingSettings>();
	if (PackagingSettings)
	{
		// Clear the current MapsToCook
		PackagingSettings->MapsToCook.Empty();

		// Create a temporary array to store valid MapsToPackage
		TArray<FString> ValidMapsToPackage;

		// Add new maps from MapsToPackage
		for (const FString& MapToPackage : MapsToPackage)
		{
			UE_LOG(LogAdvancedVRSettings, Log, TEXT("Check MapToPackage %s"), *MapToPackage);
			FGameBuildConfig GameBuildConfig;
			if (GetGameBuildConfigByMapName(this, MapToPackage, GameBuildConfig))
			{
				PackagingSettings->MapsToCook.Add(GameBuildConfig.MapPath);
				ValidMapsToPackage.Add(MapToPackage);
			}
			else
			{
				UE_LOG(LogAdvancedVRSettings, Warning, TEXT("Failed to find config for map: %s"), *MapToPackage);
			}
		}

		// Replace MapsToPackage with only valid entries
		MapsToPackage = ValidMapsToPackage;

		// Log updated maps for debugging
		for (const FFilePath& Map : PackagingSettings->MapsToCook)
		{
			UE_LOG(LogAdvancedVRSettings, Log, TEXT("Update Map: %s"), *Map.FilePath);
		}

		// Save the updated settings to the config file
		PackagingSettings->TryUpdateDefaultConfigFile("", true);
	}
	else
	{
		UE_LOG(LogAdvancedVRSettings, Warning, TEXT("PackagingSettings is null. Unable to update maps."));
	}
}
#endif

bool UAdvancedVRSettings::GetGameBuildConfigByMapName(const UAdvancedVRSettings* AdvancedVRSettings, const FString& MapName, FGameBuildConfig& GameBuildConfig)
{
	for (const FGameBuildConfig& Game : AdvancedVRSettings->AllGameMaps)
	{
		if (Game.MapName.Equals(MapName))
		{
			GameBuildConfig = Game;
			return true;
		}
	}

	GameBuildConfig = FGameBuildConfig();
	return false;
}
