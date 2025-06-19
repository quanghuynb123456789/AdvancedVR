#pragma once
#include "CoreMinimal.h"
#include "BaseXRComponent.h"
#include "UObject/SoftObjectPtr.h"
#include "AdvancedVRSettings.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAdvancedVRSettings, Log, All);

DECLARE_MULTICAST_DELEGATE(FOnSettingsUpdated);

// This enum is used to determine the platform being used.
UENUM(BlueprintType)
enum class EPlatformType : uint8
{
	Unknown		UMETA(DisplayName = "Unknown"),
	// Meta Oculus
	Oculus		UMETA(DisplayName = "Oculus"),
	// Pico
	Pico		UMETA(DisplayName = "Pico"),
	// Vive Focus
	Vive		UMETA(DisplayName = "Vive"),
	// Windows
	Windows		UMETA(DisplayName = "Windows"),
	// Android Mobile
	Mobile		UMETA(DisplayName = "Mobile")
};

USTRUCT(BlueprintType)
struct FGameBuildConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Config", meta = (ToolTip = "Name of Map"))
	FString MapName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Config", meta = (ToolTip = "Map's FilePath", RelativeToGameContentDir, LongPackageName))
	FFilePath MapPath;

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Config", meta = (ToolTip = "Enabled by Default"))
	bool bDefault = false;*/
};

// Smart pointer definitions for FGameBuildConfig
typedef TSharedPtr<FGameBuildConfig, ESPMode::ThreadSafe> FGameBuildConfigPtr;
typedef TSharedRef<FGameBuildConfig, ESPMode::ThreadSafe> FGameBuildConfigRef;

UCLASS(config = Engine, defaultconfig)
class ADVANCEDVR_API UAdvancedVRSettings : public UObject
{
	GENERATED_BODY()

public:
	UAdvancedVRSettings(const FObjectInitializer& ObjectInitializer);

	// Type of current platform
	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "Platform")
	EPlatformType PlatformType;

	// XRComponentClass
	UPROPERTY(config, BlueprintReadWrite, EditAnywhere, Category = "Platform")
	TSoftClassPtr<UBaseXRComponent> XRComponentClass;

	//All game maps
	UPROPERTY(config, EditAnywhere, Category = "Maps To Cook Settings", meta = (ToolTip = "All Game Map"))
	TArray<FGameBuildConfig> AllGameMaps;

	// Game Map Setting
	UPROPERTY(config, EditAnywhere, Category = "Maps To Package")
	TArray<FString> MapsToPackage;

	UFUNCTION(BlueprintCallable, Category = "PlatformType")
	static FString GetPlatformTypeAsString(EPlatformType Platform);

	// Get current platform
	UFUNCTION(BlueprintPure, Category = "AdvancedVRSettings")
	static EPlatformType GetPlatformType();

	// Get current XRComponentClass
	UFUNCTION(BlueprintPure, Category = "AdvancedVRSettings")
	static TSoftClassPtr<UBaseXRComponent> GetXRComponentClass();

	// Get All Game Build Config (May Not Be Built)
	UFUNCTION(BlueprintPure, Category = "AdvancedVRSettings")
	static TArray<FGameBuildConfig> GetAllGames();

	// Get All Map Names (Game Name) of AllGameMaps (May Not Be Built)
	UFUNCTION(BlueprintPure, Category = "AdvancedVRSettings")
	static TArray<FString> GetAllMapNames();

	// Get Game Build Config that will be included in the PACKAGED Game
	UFUNCTION(BlueprintPure, Category = "AdvancedVRSettings")
	static TArray<FGameBuildConfig> GetPackagedGames();

	// Get Map Names (Game Name) that will be included in the PACKAGED Game
	UFUNCTION(BlueprintPure, Category = "AdvancedVRSettings")
	static TArray<FString> GetPackagedMapNames();

	virtual void PostInitProperties() override;

	static FOnSettingsUpdated OnSettingsUpdated;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// Sync MapsToCook in UProjectPackagingSettings by MapsToPackage
	void SyncMapsToCook();

private:
	// Get Game Build Config By MapName
	static bool GetGameBuildConfigByMapName(const UAdvancedVRSettings* AdvancedVRSettings,const FString& MapName, FGameBuildConfig& GameBuildConfig);
};
