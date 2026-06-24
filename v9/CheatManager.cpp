#include "includes.hpp"

void CheatManager::ResetSessionState()
{
	PlayerInfos.clear();
	espTargets.clear();
	deadActors.clear();
	forcedVisibleActors.clear();
	playerNameCache.clear();
	TeleportTarget = nullptr;
	trackedWorld = nullptr;
	trackedLevelName.clear();
	lastEspTickMs = 0;
}

void CheatManager::PruneActorCaches(const std::unordered_set<SDK::AActor*>& currentActors)
{
	for (auto it = deadActors.begin(); it != deadActors.end(); )
		it = currentActors.count(*it) ? ++it : deadActors.erase(it);

	for (auto it = forcedVisibleActors.begin(); it != forcedVisibleActors.end(); )
		it = currentActors.count(*it) ? ++it : forcedVisibleActors.erase(it);

	for (auto it = playerNameCache.begin(); it != playerNameCache.end(); )
		it = currentActors.count(it->first) ? ++it : playerNameCache.erase(it);
}

bool CheatManager::IsSafeActor(SDK::AActor* actor) const
{
	if (!actor)
		return false;
	if (actor->IsActorBeingDestroyed())
		return false;
	if (!actor->IsA(SDK::ABP_FirstPersonCharacter_cLeon_Character_C::StaticClass()))
		return false;
	return true;
}

bool CheatManager::IsSafeMesh(SDK::USkeletalMeshComponent* mesh) const
{
	return mesh && mesh->SkeletalMesh && mesh->SkeletalMesh->Skeleton;
}

void CheatManager::Init()
{
	if (!ResolveContext())
		return;

	if (cfg->bFovChanger)
		PlayerController->FOV(cfg->fFovValue);

	const ULONGLONG now = GetTickCount64();
	if (lastEspTickMs == 0 || (now - lastEspTickMs) >= kEspIntervalMs)
	{
		lastEspTickMs = now;
		RefreshTargets();
	}

	RenderTargets();
}

void CheatManager::RefreshTargets()
{
	PlayerInfos.clear();
	espTargets.clear();

	if (!MyPlayer || !PlayerController || !UGStatics || !gWorld)
		return;

	const auto MyLocation = MyPlayer->K2_GetActorLocation();

	SDK::TArray<SDK::AActor*> Players;
	UGStatics->GetAllActorsOfClass(gWorld, SDK::ABP_FirstPersonCharacter_cLeon_Character_C::StaticClass(), &Players);

	std::unordered_set<SDK::AActor*> currentActors;
	const int playerCount = Players.Num();
	currentActors.reserve(static_cast<std::size_t>(playerCount > 0 ? playerCount : 1));
	espTargets.reserve(static_cast<std::size_t>(playerCount > 0 ? playerCount : 1));

	for (int i = 0; i < playerCount; i++)
	{
		if (!Players.IsValidIndex(i))
			continue;

		objActor = Players[i];
		if (!IsSafeActor(objActor))
			continue;

		BaseClass = static_cast<SDK::ABP_FirstPersonCharacter_cLeon_Character_C*>(objActor);
		if (!BaseClass)
			continue;

		currentActors.insert(objActor);

		if (IsDead())
			continue;

		const auto Location = BaseClass->K2_GetActorLocation();
		const std::string PlayerName = ResolvePlayerName();

		if (objActor == MyPlayer)
			continue;

		if (!IsSafeActor(objActor))
			continue;

		const bool IsVisible = PlayerController->LineOfSightTo(objActor, { 0, 0, 0 }, false);
		const bool bIsEnemy = IsEnemy();

		PlayerInfos.push_back({ PlayerName, Location, objActor });
		UpdateForcedVisibility();

		espTargets.push_back({ objActor, PlayerName, Location, IsVisible, bIsEnemy });
	}

	PruneActorCaches(currentActors);
	HandleTeleport(currentActors);
}

void CheatManager::RenderTargets()
{
	if (!MyPlayer || !PlayerController || espTargets.empty())
		return;

	const auto MyLocation = MyPlayer->K2_GetActorLocation();

	for (const EspTarget& target : espTargets)
	{
		if (!IsSafeActor(target.Actor))
			continue;

		if (cfg->bEnemyOnly && !target.bIsEnemy)
			continue;

		objActor = target.Actor;
		BaseClass = static_cast<SDK::ABP_FirstPersonCharacter_cLeon_Character_C*>(objActor);
		if (!BaseClass)
			continue;

		const auto Location = BaseClass->K2_GetActorLocation();
		DrawEsp(target.Name, Location, MyLocation, target.IsVisible);
	}
}

bool CheatManager::ResolveContext()
{
	gWorld = SDK::UWorld::GetWorld();
	if (!gWorld)
	{
		if (trackedWorld)
			ResetSessionState();
		return false;
	}

	if (gWorld != trackedWorld)
	{
		ResetSessionState();
		trackedWorld = gWorld;
	}

	OwningGameInstance = gWorld->OwningGameInstance;
	if (!OwningGameInstance)
		return false;

	if (OwningGameInstance->LocalPlayers.Num() <= 0)
		return false;
	LocalPlayer = OwningGameInstance->LocalPlayers[0];
	if (!LocalPlayer)
		return false;

	GameViewportClient = LocalPlayer->ViewportClient;
	if (!GameViewportClient)
		return false;

	PlayerController = LocalPlayer->PlayerController;
	if (!PlayerController)
		return false;

	PlayerController->GetViewportSize(&x, &y);

	MyPlayer = PlayerController->K2_GetPawn();
	if (!MyPlayer)
		return false;

	UGStatics = (SDK::UGameplayStatics*)SDK::UGameplayStatics::StaticClass();
	if (!UGStatics)
		return false;

	const std::string levelName = UGStatics->GetCurrentLevelName(gWorld, true).ToString();
	if (!levelName.empty() && levelName != trackedLevelName)
	{
		if (!trackedLevelName.empty())
			ResetSessionState();
		trackedLevelName = levelName;
		trackedWorld = gWorld;
	}

	MathLib = (SDK::UKismetMathLibrary*)SDK::UKismetMathLibrary::StaticClass();
	if (!MathLib)
		return false;

	return true;
}

std::string CheatManager::ResolvePlayerName()
{
	if (!BaseClass->PlayerState)
	{
		auto it = playerNameCache.find(objActor);
		return it != playerNameCache.end() ? it->second : "Unknown";
	}

	SDK::FString* Name = &BaseClass->PlayerState->PlayerNamePrivate;
	if (BaseClass->PlayerState->IsA(SDK::ABP_FirstPersonPlayerState_Online_C::StaticClass()))
	{
		auto* ps = static_cast<SDK::ABP_FirstPersonPlayerState_Online_C*>(BaseClass->PlayerState);
		if (ps->CustomPlayerName.IsValid())
			Name = &ps->CustomPlayerName;
	}

	if (Name->IsValid())
	{
		std::string PlayerName = Name->ToString();
		playerNameCache[objActor] = PlayerName;
		return PlayerName;
	}

	return "Unknown";
}

void CheatManager::UpdateForcedVisibility()
{
	if (!cfg->bForceCharacterVisibility)
		return;

	if (!g_OnRepBodyVisibilityFunc)
		g_OnRepBodyVisibilityFunc = SDK::ABP_FirstPersonCharacter_cLeon_Character_C::StaticClass()->GetFunction("BP_FirstPersonCharacter_cLeon_Character_C", "OnRep_BodyVisibility");

	if (!BaseClass->BodyVisibility)
	{
		BaseClass->BodyVisibility = true;
		BaseClass->OnRep_BodyVisibility();
		forcedVisibleActors.insert(objActor);
	}
}

bool CheatManager::IsDead()
{
	if (!IsSafeMesh(BaseClass->Mesh))
		return deadActors.count(objActor) > 0;

	if (BaseClass->Mesh->IsAnySimulatingPhysics())
		deadActors.insert(objActor);
	return deadActors.count(objActor) > 0;
}

bool CheatManager::IsDead(SDK::AActor* actor)
{
	if (!IsSafeActor(actor))
		return true;

	auto* baseClass = static_cast<SDK::ABP_FirstPersonCharacter_cLeon_Character_C*>(actor);
	if (!IsSafeMesh(baseClass->Mesh))
		return deadActors.count(actor) > 0;

	if (baseClass->Mesh->IsAnySimulatingPhysics())
		deadActors.insert(actor);
	return deadActors.count(actor) > 0;
}

bool CheatManager::IsSurvivor() { return IsSurvivor(BaseClass); }

bool CheatManager::IsSurvivor(SDK::AActor* actor)
{
	if (!IsSafeActor(actor))
		return false;
	return static_cast<SDK::ABP_FirstPersonCharacter_cLeon_Character_C*>(actor)->IsA(SDK::ABP_FirstPersonCharacter_cLeon_Character_Survivor_C::StaticClass());
}

bool CheatManager::IsSurvivor(SDK::ABP_FirstPersonCharacter_cLeon_Character_C* baseClass)
{
	return baseClass && baseClass->IsA(SDK::ABP_FirstPersonCharacter_cLeon_Character_Survivor_C::StaticClass());
}

bool CheatManager::IsHunter() { return IsHunter(BaseClass); }

bool CheatManager::IsHunter(SDK::AActor* actor)
{
	if (!IsSafeActor(actor))
		return false;
	return static_cast<SDK::ABP_FirstPersonCharacter_cLeon_Character_C*>(actor)->IsA(SDK::ABP_FirstPersonCharacter_cLeon_Character_Hunter_C::StaticClass());
}

bool CheatManager::IsHunter(SDK::ABP_FirstPersonCharacter_cLeon_Character_C* baseClass)
{
	return baseClass && baseClass->IsA(SDK::ABP_FirstPersonCharacter_cLeon_Character_Hunter_C::StaticClass());
}

bool CheatManager::IsEnemy()
{
	if (!MyPlayer->IsA(SDK::ABP_FirstPersonCharacter_cLeon_Character_C::StaticClass()))
		return false;
	auto* MyChar = static_cast<SDK::ABP_FirstPersonCharacter_cLeon_Character_C*>(MyPlayer);
	return MyChar->IsHunter != BaseClass->IsHunter;
}

void CheatManager::DrawSkeleton(ImU32 colEsp)
{
	if (!IsSafeMesh(BaseClass->Mesh) || !PlayerController)
		return;

	SDK::FVector2D BoneScreen, PrevBoneScreen;
	for (const std::pair<int, int>& Connection : skeleton::Connections)
	{
		if (Connection.first < 0 || Connection.second < 0)
			continue;

		const auto bone1 = BaseClass->Mesh->GetBoneName(Connection.first);
		const auto bone2 = BaseClass->Mesh->GetBoneName(Connection.second);
		if (bone1.IsNone() || bone2.IsNone())
			continue;

		const auto BoneLoc1 = BaseClass->Mesh->GetSocketLocation(bone1);
		const auto BoneLoc2 = BaseClass->Mesh->GetSocketLocation(bone2);
		if (PlayerController->ProjectWorldLocationToScreen(BoneLoc1, &BoneScreen, false) &&
			PlayerController->ProjectWorldLocationToScreen(BoneLoc2, &PrevBoneScreen, false))
			ImGui::GetForegroundDrawList()->AddLine(ImVec2(BoneScreen.X, BoneScreen.Y), ImVec2(PrevBoneScreen.X, PrevBoneScreen.Y), colEsp, 1.2f);
	}
}

bool CheatManager::ComputeBoundingBox(SDK::FVector2D& BoxMin, SDK::FVector2D& BoxMax)
{
	if (!IsSafeMesh(BaseClass->Mesh) || !PlayerController)
		return false;

	bool bHasBox = false;
	for (int BoneIdx = skeleton::amm; BoneIdx < skeleton::None; BoneIdx++)
	{
		const auto BoneLoc = BaseClass->Mesh->GetSocketLocation(BaseClass->Mesh->GetBoneName(BoneIdx));
		SDK::FVector2D BoneScreenPos;
		if (!PlayerController->ProjectWorldLocationToScreen(BoneLoc, &BoneScreenPos, false))
			continue;

		if (!bHasBox)
		{
			BoxMin = BoxMax = BoneScreenPos;
			bHasBox = true;
			continue;
		}

		if (BoneScreenPos.X < BoxMin.X) BoxMin.X = BoneScreenPos.X;
		if (BoneScreenPos.Y < BoxMin.Y) BoxMin.Y = BoneScreenPos.Y;
		if (BoneScreenPos.X > BoxMax.X) BoxMax.X = BoneScreenPos.X;
		if (BoneScreenPos.Y > BoxMax.Y) BoxMax.Y = BoneScreenPos.Y;
	}
	return bHasBox;
}

void CheatManager::DrawEsp(const std::string& PlayerName, SDK::FVector Location, SDK::FVector MyLocation, bool IsVisible)
{
	if (!IsSafeActor(objActor) || !PlayerController)
		return;

	const ImU32 colEsp = ImGui::ColorConvertFloat4ToU32(IsVisible ? *(ImVec4*)cfg->colVisible : *(ImVec4*)cfg->colNotVisible);
	const ImU32 colLine = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)cfg->colLines);
	const float fff[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const ImU32 colWhite = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)fff);

	SDK::FVector2D BoxMin{}, BoxMax{};
	bool bHasBox = false;

	if (BaseClass->Mesh && IsSafeMesh(BaseClass->Mesh))
	{
		if (cfg->bSkeleton)
			DrawSkeleton(colEsp);
		bHasBox = ComputeBoundingBox(BoxMin, BoxMax);
	}

	if (bHasBox)
	{
		if (cfg->bNames)
			ImGui::GetForegroundDrawList()->AddText(ImVec2(BoxMin.X, BoxMin.Y - 15), colEsp, PlayerName.c_str());

		if (cfg->bRoles)
		{
			const char* roleText = IsHunter() ? "Hunter" : (IsSurvivor() ? "Survivor" : nullptr);
			if (roleText)
			{
				const float nameWidth = cfg->bNames ? ImGui::CalcTextSize(PlayerName.c_str()).x + 5 : 0.0f;
				ImGui::GetForegroundDrawList()->AddText(ImVec2(BoxMin.X + nameWidth, BoxMin.Y - 15), colWhite, roleText);
			}
		}

		if (cfg->bBox)
			draw->DrawBox(static_cast<int>(BoxMin.X), static_cast<int>(BoxMin.Y),
				static_cast<int>(BoxMax.X - BoxMin.X), static_cast<int>(BoxMax.Y - BoxMin.Y), colEsp, 1.0f);

		if (cfg->bDistance)
		{
			char DistanceText[32];
			snprintf(DistanceText, sizeof(DistanceText), "%.0fm", MyLocation.GetDistanceToInMeters(Location));
			const ImVec2 TextSize = ImGui::CalcTextSize(DistanceText);
			const float TextX = (BoxMin.X + BoxMax.X) * 0.5f - TextSize.x * 0.5f;
			ImGui::GetForegroundDrawList()->AddText(ImVec2(TextX, BoxMax.Y + 2), colEsp, DistanceText);
		}
	}

	SDK::FVector2D Screen;
	if (cfg->bLines && PlayerController->ProjectWorldLocationToScreen(Location, &Screen, false))
	{
		const auto& io = ImGui::GetIO();
		ImGui::GetForegroundDrawList()->AddLine(
			ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y),
			ImVec2(Screen.X, Screen.Y), colLine, 0.8f);
	}
}

void CheatManager::HandleTeleport(const std::unordered_set<SDK::AActor*>& currentActors)
{
	if (TeleportTarget && currentActors.count(TeleportTarget) && MyPlayer && IsSafeActor(TeleportTarget))
	{
		SDK::FRotator CurrentRotation = MyPlayer->K2_GetActorRotation();
		MyPlayer->K2_TeleportTo(TeleportTarget->K2_GetActorLocation(), CurrentRotation);
	}
	TeleportTarget = nullptr;
}

void CheatManager::DumpBones()
{
	if (!BaseClass || !BaseClass->Mesh || !BaseClass->Mesh->SkeletalMesh || !BaseClass->Mesh->SkeletalMesh->Skeleton)
		return;

	FILE* Log = fopen("C:\\v9\\bones.txt", "w");
	if (!Log)
		return;

	for (int i = 0; i < BaseClass->Mesh->SkeletalMesh->Skeleton->BoneTree.Num(); i++)
	{
		auto boneName = BaseClass->Mesh->GetBoneName(i);
		fprintf(Log, "%s = %d,\n", boneName.GetRawString().c_str(), i);
	}
	fclose(Log);
}
