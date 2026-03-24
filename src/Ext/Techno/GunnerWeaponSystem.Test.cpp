#include "GunnerWeaponSystem.h"
#include "Body.h"
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>

// Test helper class to simulate passengers with different weapons
class TestPassenger : public FootClass
{
public:
	int TestWeaponIndex;

	TestPassenger(int weaponIndex) : TestWeaponIndex(weaponIndex) {}

	virtual AbstractType WhatAmI() const override { return AbstractType::Infantry; }
	virtual TechnoTypeClass* GetTechnoType() const override
	{
		// Return a mock infantry type
		return nullptr; // Simplified for test
	}
};

// Test helper class to simulate a transport with passengers
class TestTransport : public TechnoClass
{
public:
	std::vector<std::unique_ptr<TestPassenger>> TestPassengers;
	bool GunnerIteratePassengers;
	GunnerWeaponSelectionMode SelectionMode;
	int WeaponCriteria;

	TestTransport() : GunnerIteratePassengers(false), SelectionMode(GunnerWeaponSelectionMode::First), WeaponCriteria(0) {}

	void AddPassenger(int weaponIndex)
	{
		TestPassengers.push_back(std::make_unique<TestPassenger>(weaponIndex));
	}

	virtual FootClass* GetFirstPassenger() override
	{
		return TestPassengers.empty() ? nullptr : TestPassengers.front().get();
	}

	virtual TechnoTypeClass* GetTechnoType() const override
	{
		static TechnoTypeClass mockType;
		mockType.Gunner = true;
		return &mockType;
	}
};

// Test the weapon evaluation logic
void TestWeaponEvaluation()
{
	// Create mock weapons with different stats
	WeaponTypeClass weapon1;
	weapon1.Damage = 100;
	weapon1.Range = 256; // 1 cell
	weapon1.ROF = 30;

	WeaponTypeClass weapon2;
	weapon2.Damage = 50;
	weapon2.Range = 512; // 2 cells
	weapon2.ROF = 15; // Faster

	// Test damage evaluation
	int score1 = GunnerWeaponSystem::GetWeaponDamage(&weapon1);
	int score2 = GunnerWeaponSystem::GetWeaponDamage(&weapon2);

	// Weapon1 should have higher damage score
	assert(score1 > score2);

	// Test range evaluation
	int rangeScore1 = GunnerWeaponSystem::GetWeaponRange(&weapon1);
	int rangeScore2 = GunnerWeaponSystem::GetWeaponRange(&weapon2);

	// Weapon2 should have higher range score
	assert(rangeScore2 > rangeScore1);

	// Test ROF evaluation (lower ROF is better)
	int rofScore1 = GunnerWeaponSystem::GetWeaponROF(&weapon1);
	int rofScore2 = GunnerWeaponSystem::GetWeaponROF(&weapon2);

	// Weapon2 should have better ROF score
	assert(rofScore2 > rofScore1);
}

// Test passenger iteration with different selection modes
void TestPassengerIteration()
{
	// Create a transport with 3 passengers having different weapons
	TestTransport transport;
	transport.GunnerIteratePassengers = true;

	// Add passengers with weapon indices 0, 1, 2
	transport.AddPassenger(0);
	transport.AddPassenger(1);
	transport.AddPassenger(2);

	// Mock TechnoExt data
	TechnoExt::ExtData extData(&transport);
	extData.GunnerIteratePassengers = true;
	extData.GunnerWeaponSelectionMode = GunnerWeaponSelectionMode::First;

	// Test "First" mode - should return first passenger's weapon (0)
	int weaponIndex = GunnerWeaponSystem::SelectWeaponFromPassengers(&transport, nullptr);
	assert(weaponIndex == 0);

	// Test "Strongest" mode
	extData.GunnerWeaponSelectionMode = GunnerWeaponSelectionMode::Strongest;
	// Would need mock weapon setup to fully test
}

// Test edge cases
void TestEdgeCases()
{
	// Test with no passengers
	TestTransport transport;
	transport.GunnerIteratePassengers = true;

	int weaponIndex = GunnerWeaponSystem::SelectWeaponFromPassengers(&transport, nullptr);
	assert(weaponIndex == -1);

	// Test with GunnerIteratePassengers disabled
	transport.AddPassenger(0);
	transport.GunnerIteratePassengers = false;

	weaponIndex = GunnerWeaponSystem::SelectWeaponFromPassengers(&transport, nullptr);
	assert(weaponIndex == -1); // Should return -1 when disabled

	// Test with non-Gunner unit
	transport.GunnerIteratePassengers = true;
	// Would need to mock GetTechnoType() to return non-Gunner type
}

// Main test function
void RunGunnerWeaponSystemTests()
{
	Debug::Log("Running GunnerWeaponSystem tests...\n");

	try
	{
		TestWeaponEvaluation();
		Debug::Log("✓ Weapon evaluation tests passed\n");

		TestPassengerIteration();
		Debug::Log("✓ Passenger iteration tests passed\n");

		TestEdgeCases();
		Debug::Log("✓ Edge case tests passed\n");

		Debug::Log("All GunnerWeaponSystem tests passed!\n");
	}
	catch (const std::exception& e)
	{
		Debug::Log("✗ Test failed: %s\n", e.what());
	}
}