#include "Body.h"
#include "GunnerWeaponSystem.h"

// Hook into TechnoClass::SelectWeapon to override vanilla Gunner behavior
// This allows iterating through all passengers instead of just the first one
DEFINE_HOOK(0x6F3451, TechnoClass_SelectWeapon_GunnerPassengerIteration, 0x5)
{
	GET_STACK(TechnoClass*, pThis, 0x4);
	GET_STACK(AbstractClass*, pTarget, 0x8);

	// Check if this is a Gunner unit with passenger iteration enabled
	const auto pType = pThis->GetTechnoType();
	if (!pType || !pType->Gunner)
		return 0; // Not a Gunner unit, use vanilla behavior

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pExt || !pExt->GunnerIteratePassengers)
		return 0; // Passenger iteration not enabled, use vanilla behavior

	// Use our new passenger iteration system
	int weaponIndex = GunnerWeaponSystem::SelectWeaponFromPassengers(pThis, pTarget);

	if (weaponIndex >= 0)
	{
		// Found a weapon from passengers, return it
		R->EAX(weaponIndex);
		return 0x6F3474; // Skip vanilla weapon selection
	}

	// No suitable weapon found from passengers, continue with vanilla behavior
	return 0;
}