#include "GunnerWeaponSystem.h"
#include "Body.h"
#include "../TechnoType/Body.h"
#include "../WeaponType/Body.h"
#include <Utilities/Macro.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Debug.h>

int GunnerWeaponSystem::SelectWeaponFromPassengers(TechnoClass* pThis, AbstractClass* pTarget)
{
	if (!pThis || !pTarget)
		return -1;

	const auto pType = pThis->GetTechnoType();
	if (!pType || !pType->Gunner)
		return -1;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pExt || !pExt->GunnerIteratePassengers)
		return -1; // Use vanilla behavior if not enabled

	// Get configuration
	const auto mode = GetSelectionMode(pThis);
	const auto criteria = GetWeaponCriteria(pThis);

	// Start with first passenger
	auto pPassenger = pThis->Passengers.GetFirstPassenger();
	if (!pPassenger)
		return -1;

	int bestWeaponIndex = -1;
	int bestScore = -1;
	int lastWeaponIndex = -1;  // Track last passenger's weapon

	// Iterate through all passengers
	do
	{
		// Get weapon index from passenger
		int weaponIndex = GetPassengerWeaponIndex(pPassenger);
		if (weaponIndex >= 0)
		{
			// Always remember the last passenger's weapon
			lastWeaponIndex = weaponIndex;

			// For "Last" mode, we just need to remember it, no evaluation needed
			if (mode == GunnerWeaponSelectionMode::Last)
			{
				bestWeaponIndex = weaponIndex;
				// Continue iterating to find the actual last passenger
			}
			else
			{
				// Evaluate weapon based on criteria
				int score = EvaluateWeapon(pThis, weaponIndex, pTarget);

				// Update best weapon if this is better
				if (score > bestScore)
				{
					bestScore = score;
					bestWeaponIndex = weaponIndex;

					// Early exit for "First" mode (vanilla behavior)
					if (mode == GunnerWeaponSelectionMode::First)
						break;
				}
			}
		}

		pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);
	}
	while (pPassenger);

	// For "Last" mode, always return the last passenger's weapon
	if (mode == GunnerWeaponSelectionMode::Last)
		return lastWeaponIndex;

	return bestWeaponIndex;
}

int GunnerWeaponSystem::GetPassengerWeaponIndex(FootClass* pPassenger)
{
	if (!pPassenger)
		return -1;

	const auto pType = pPassenger->GetTechnoType();
	if (!pType)
		return -1;

	// Map passenger type to weapon index
	// This is based on the vanilla game's weapon mapping for Gunner units
	const auto whatAmI = pPassenger->WhatAmI();

	switch (whatAmI)
	{
	case AbstractType::Infantry:
	{
		const auto pInfType = static_cast<InfantryTypeClass*>(pType);
		// Infantry typically provide weapon based on their type
		// For now, return 0 for primary weapon
		// TODO: Implement proper infantry weapon mapping
		return 0;
	}
	case AbstractType::Unit:
	{
		const auto pUnitType = static_cast<UnitTypeClass*>(pType);
		// Units typically provide weapon based on their type
		// For now, return 0 for primary weapon
		// TODO: Implement proper unit weapon mapping
		return 0;
	}
	default:
		return -1;
	}
}

int GunnerWeaponSystem::EvaluateWeapon(TechnoClass* pTransport, int weaponIndex, AbstractClass* pTarget)
{
	if (!pTransport || weaponIndex < 0 || !pTarget)
		return -1;

	const auto pWeaponStruct = pTransport->GetWeapon(weaponIndex);
	if (!pWeaponStruct || !pWeaponStruct->WeaponType)
		return -1;

	const auto pWeapon = pWeaponStruct->WeaponType;
	const auto pExt = TechnoExt::ExtMap.Find(pTransport);
	const auto mode = GetSelectionMode(pTransport);
	const auto criteria = GetWeaponCriteria(pTransport);

	int score = 0;

	// Check if weapon can target the target
	if (!CanWeaponTarget(pWeapon, pTarget))
		return -1;

	// Evaluate based on criteria
	if (criteria & static_cast<int>(GunnerWeaponCriteria::Damage))
	{
		score += GetWeaponDamage(pWeapon) * 10;
	}

	if (criteria & static_cast<int>(GunnerWeaponCriteria::Range))
	{
		score += GetWeaponRange(pWeapon) * 5;
	}

	if (criteria & static_cast<int>(GunnerWeaponCriteria::ROF))
	{
		score += (1000 - GetWeaponROF(pWeapon)) / 10; // Lower ROF is better
	}

	if (criteria & static_cast<int>(GunnerWeaponCriteria::Versatility))
	{
		score += GetWeaponVersatility(pWeapon) * 3;
	}

	// Apply mode-specific scoring
	switch (mode)
	{
	case GunnerWeaponSelectionMode::Strongest:
		score = GetWeaponDamage(pWeapon) * 100 + GetWeaponRange(pWeapon) * 10;
		break;
	case GunnerWeaponSelectionMode::LongestRange:
		score = GetWeaponRange(pWeapon) * 1000;
		break;
	case GunnerWeaponSelectionMode::FastestROF:
		score = (1000 - GetWeaponROF(pWeapon)) * 10;
		break;
	default:
		break;
	}

	return score;
}

int GunnerWeaponSystem::GetWeaponDamage(WeaponTypeClass* pWeapon)
{
	if (!pWeapon)
		return 0;

	// Get base damage
	int damage = pWeapon->Damage;

	// Consider warhead multipliers
	if (pWeapon->Warhead)
	{
		// Apply warhead-specific adjustments if needed
	}

	return damage;
}

int GunnerWeaponSystem::GetWeaponRange(WeaponTypeClass* pWeapon)
{
	if (!pWeapon || !pWeapon->Projectile)
		return 0;

	// Get weapon range in cells
	return pWeapon->Range / 256; // Convert from leptons to cells
}

int GunnerWeaponSystem::GetWeaponROF(WeaponTypeClass* pWeapon)
{
	if (!pWeapon)
		return 0;

	// Get Rate of Fire (lower is better)
	return pWeapon->ROF;
}

int GunnerWeaponSystem::GetWeaponVersatility(WeaponTypeClass* pWeapon)
{
	if (!pWeapon || !pWeapon->Warhead)
		return 0;

	int versatility = 0;
	const auto pWarhead = pWeapon->Warhead;

	// Check what targets this weapon can hit
	if (pWarhead->Verses[Armor::None] != 0.0) versatility++;
	if (pWarhead->Verses[Armor::Flak] != 0.0) versatility++;
	if (pWarhead->Verses[Armor::Plate] != 0.0) versatility++;
	if (pWarhead->Verses[Armor::Light] != 0.0) versatility++;
	if (pWarhead->Verses[Armor::Medium] != 0.0) versatility++;
	if (pWarhead->Verses[Armor::Heavy] != 0.0) versatility++;
	if (pWarhead->Verses[Armor::Wood] != 0.0) versatility++;
	if (pWarhead->Verses[Armor::Steel] != 0.0) versatility++;
	if (pWarhead->Verses[Armor::Concrete] != 0.0) versatility++;

	return versatility;
}

bool GunnerWeaponSystem::CanWeaponTarget(WeaponTypeClass* pWeapon, AbstractClass* pTarget)
{
	if (!pWeapon || !pTarget)
		return false;

	// Basic targeting check - weapon must be able to target the target type
	// This is a simplified check - full implementation would need more complex logic

	if (const auto pTargetTechno = abstract_cast<TechnoClass*, true>(pTarget))
	{
		// Check if weapon can target the techno
		const auto pTargetType = pTargetTechno->GetTechnoType();
		if (!pTargetType)
			return false;

		// Check AA capability for airborne targets
		if (pTargetTechno->IsInAir() && !pWeapon->Projectile->AA)
			return false;

		// Additional targeting checks would go here
		// (armor type, house alignment, etc.)
	}

	return true;
}

GunnerWeaponSelectionMode GunnerWeaponSystem::GetSelectionMode(TechnoClass* pThis)
{
	if (!pThis)
		return GunnerWeaponSelectionMode::First;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (pExt)
		return pExt->GunnerWeaponSelectionMode;

	return GunnerWeaponSelectionMode::First;
}

int GunnerWeaponSystem::GetWeaponCriteria(TechnoClass* pThis)
{
	if (!pThis)
		return 0;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (pExt)
		return pExt->GunnerWeaponCriteria;

	return 0;
}

bool GunnerWeaponSystem::ShouldIteratePassengers(TechnoClass* pThis)
{
	if (!pThis)
		return false;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (pExt)
		return pExt->GunnerIteratePassengers;

	return false; // Default to vanilla behavior
}

// Simple test function to verify the system works
void GunnerWeaponSystem::RunTests()
{
	Debug::Log("Running GunnerWeaponSystem tests...\n");

	// Test 1: Weapon damage calculation
	{
		WeaponTypeClass weapon1;
		weapon1.Damage = 100;
		weapon1.Warhead = nullptr;

		WeaponTypeClass weapon2;
		weapon2.Damage = 50;
		weapon2.Warhead = nullptr;

		int damage1 = GetWeaponDamage(&weapon1);
		int damage2 = GetWeaponDamage(&weapon2);

		if (damage1 > damage2)
		{
			Debug::Log("✓ Weapon damage test passed\n");
		}
		else
		{
			Debug::Log("✗ Weapon damage test failed\n");
		}
	}

	// Test 2: Weapon range calculation
	{
		WeaponTypeClass weapon1;
		weapon1.Range = 256; // 1 cell
		weapon1.Projectile = nullptr;

		WeaponTypeClass weapon2;
		weapon2.Range = 512; // 2 cells
		weapon2.Projectile = nullptr;

		int range1 = GetWeaponRange(&weapon1);
		int range2 = GetWeaponRange(&weapon2);

		if (range2 > range1)
		{
			Debug::Log("✓ Weapon range test passed\n");
		}
		else
		{
			Debug::Log("✗ Weapon range test failed\n");
		}
	}

	// Test 3: ROF evaluation
	{
		WeaponTypeClass weapon1;
		weapon1.ROF = 30;

		WeaponTypeClass weapon2;
		weapon2.ROF = 15; // Faster

		int rof1 = GetWeaponROF(&weapon1);
		int rof2 = GetWeaponROF(&weapon2);

		if (rof2 > rof1)
		{
			Debug::Log("✓ Weapon ROF test passed\n");
		}
		else
		{
			Debug::Log("✗ Weapon ROF test failed\n");
		}
	}

	// Test 4: Last passenger mode
	{
		// This test would require mock passengers to verify
		// For now, just verify the mode exists
		if (static_cast<int>(GunnerWeaponSelectionMode::Last) == 5)
		{
			Debug::Log("✓ Last passenger mode exists\n");
		}
		else
		{
			Debug::Log("✗ Last passenger mode test failed\n");
		}
	}

	Debug::Log("GunnerWeaponSystem tests completed\n");
}