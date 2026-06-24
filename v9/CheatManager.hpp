class CheatManager
{
private:
	SDK::UWorld* gWorld;
	SDK::UWorld* trackedWorld{ nullptr };
	std::string trackedLevelName;
	ULONGLONG lastEspTickMs{ 0 };
	static constexpr ULONGLONG kEspIntervalMs = 100;

	struct EspTarget {
		SDK::AActor* Actor{ nullptr };
		std::string Name;
		SDK::FVector Location{};
		bool IsVisible{ false };
		bool bIsEnemy{ false };
	};
	std::vector<EspTarget> espTargets;

	SDK::APlayerController* PlayerController;
	SDK::ULocalPlayer* LocalPlayer;
	SDK::UGameInstance* OwningGameInstance;
	SDK::UGameViewportClient* GameViewportClient;
	SDK::AActor* objActor;
	SDK::UGameplayStatics* UGStatics;
	SDK::APawn* MyPlayer;
	SDK::ABP_FirstPersonCharacter_cLeon_Character_C* BaseClass;
	SDK::UKismetMathLibrary* MathLib;
	int x, y = 0;

	bool ResolveContext();
	void PruneActorCaches(const std::unordered_set<SDK::AActor*>& currentActors);
	bool IsSafeActor(SDK::AActor* actor) const;
	bool IsSafeMesh(SDK::USkeletalMeshComponent* mesh) const;
	std::string ResolvePlayerName();
	void UpdateForcedVisibility();
	bool IsDead();
	bool IsDead(SDK::AActor* actor);
	bool IsSurvivor();
	bool IsSurvivor(SDK::AActor* actor);
	bool IsSurvivor(SDK::ABP_FirstPersonCharacter_cLeon_Character_C* baseClass);
	bool IsHunter();
	bool IsHunter(SDK::AActor* actor);
	bool IsHunter(SDK::ABP_FirstPersonCharacter_cLeon_Character_C* baseClass);
	bool IsEnemy();
	void RefreshTargets();
	void RenderTargets();
	void DrawSkeleton(ImU32 colEsp);
	bool ComputeBoundingBox(SDK::FVector2D& BoxMin, SDK::FVector2D& BoxMax);
	void DrawEsp(const std::string& PlayerName, SDK::FVector Location, SDK::FVector MyLocation, bool IsVisible);
	void HandleTeleport(const std::unordered_set<SDK::AActor*>& currentActors);
	SDK::AActor* TeleportTarget = nullptr;

public:
	struct PlayerInfo {
		std::string Name;
		SDK::FVector Location;
		SDK::AActor* Actor;
	};
	std::vector<PlayerInfo> PlayerInfos;
	void RequestTeleport(SDK::AActor* Actor) { TeleportTarget = Actor; }
	std::unordered_set<SDK::AActor*> forcedVisibleActors;
	std::unordered_set<SDK::AActor*> deadActors;
	std::unordered_map<SDK::AActor*, std::string> playerNameCache;
	void ResetSessionState();
	void Init();
	void DumpBones();
};
