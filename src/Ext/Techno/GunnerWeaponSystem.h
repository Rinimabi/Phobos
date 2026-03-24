#pragma once

#include <YRpp/TechnoClass.h>
#include <YRpp/FootClass.h>
#include <YRpp/WeaponTypeClass.h>

enum class GunnerWeaponSelectionMode : int
{
	First = 0,      // Vanilla behavior - first passenger
	Last = 5,       // Last passenger (most recently added)
	Strongest = 1,  // Highest damage weapon
	LongestRange = 2, // Longest weapon range
	FastestROF = 3, // Fastest rate of fire
	Custom = 4      // Custom implementation
};

enum class GunnerWeaponCriteria : int
{
	Damage = 1,
	Range = 2,
	ROF = 4,
	Versatility = 8  // Against multiple target types
};

class GunnerWeaponSystem
{
public:
	// Main function to select weapon from passengers
	static int SelectWeaponFromPassengers(TechnoClass* pThis, AbstractClass* pTarget);

	// Configuration getters
	static GunnerWeaponSelectionMode GetSelectionMode(TechnoClass* pThis);
	static int GetWeaponCriteria(TechnoClass* pThis);
	static bool ShouldIteratePassengers(TechnoClass* pThis);

	// Test function to verify the system works
	static void RunTests();

private:
	// Helper functions
	static int GetPassengerWeaponIndex(FootClass* pPassenger);
	static int EvaluateWeapon(TechnoClass* pTransport, int weaponIndex, AbstractClass* pTarget);
	static int GetWeaponDamage(WeaponTypeClass* pWeapon);
	static int GetWeaponRange(WeaponTypeClass* pWeapon);
	static int GetWeaponROF(WeaponTypeClass* pWeapon);
	static int GetWeaponVersatility(WeaponTypeClass* pWeapon);
	static bool CanWeaponTarget(WeaponTypeClass* pWeapon, AbstractClass* pTarget);
};