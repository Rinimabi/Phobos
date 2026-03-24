# Gunner Passenger Iteration System Implementation

## Overview

This implementation adds a new passenger iteration system for Gunner-type units (like the FV - 多功能步兵车) that allows them to consider weapons from ALL passengers, not just the first one as in the vanilla game.

## Problem Statement

In the vanilla game, Gunner units only use the weapon from the first passenger. This limitation means:
- Only the first passenger's weapon is considered
- Other passengers' weapons are ignored
- No way to select the "best" weapon among all passengers

## Solution

The new system implements:
1. **Passenger Iteration**: Loop through all passengers to evaluate their weapons
2. **Configurable Selection Mode**: Choose how to select the best weapon
3. **Multiple Criteria**: Evaluate weapons based on damage, range, rate of fire, and versatility
4. **INI Configuration**: Configure the behavior per-unit-type

## Implementation Details

### Files Added/Modified

1. **GunnerWeaponSystem.h** - New header defining enums and the GunnerWeaponSystem class
2. **GunnerWeaponSystem.cpp** - Implementation of the weapon selection logic
3. **Body.h** - Added configuration fields to TechnoExt::ExtData
4. **Body.cpp** - Added serialization for the new fields
5. **Hooks.GunnerWeapon.cpp** - Hooks to integrate with the game's weapon selection

### Configuration Options

In the INI file, you can configure:

```ini
[SomeGunnerUnit]  ; Your Gunner unit type
Gunner.IteratePassengers=true  ; Enable passenger iteration (default: false)
Gunner.WeaponSelectionMode=Strongest  ; Selection mode: First, Strongest, LongestRange, FastestROF, Custom
Gunner.WeaponCriteria=Damage,Range  ; Criteria for Custom mode: Damage, Range, ROF, Versatility
```

### Selection Modes

- **First**: Vanilla behavior - uses first passenger's weapon (early exit)
- **Last**: Uses last passenger's weapon (most recently added)
- **Strongest**: Selects weapon with highest damage
- **LongestRange**: Selects weapon with longest range
- **FastestROF**: Selects weapon with fastest rate of fire
- **Custom**: Uses specified criteria with weighted scoring

### Weapon Criteria

When using Custom mode, weapons are scored based on:
- **Damage**: Weapon's base damage × 10
- **Range**: Weapon's range in cells × 5
- **ROF**: (1000 - ROF) / 10 (lower ROF is better)
- **Versatility**: Count of armor types the weapon can damage

## Usage Example

To enable the system for an FV unit:

```ini
[FV]
Gunner.IteratePassengers=true
Gunner.WeaponSelectionMode=Strongest
```

This will make the FV iterate through all passengers and select the weapon with the highest damage.

## Technical Implementation

### Hook Points

The system hooks into:
- `0x6F3451` - TechnoClass::SelectWeapon to override vanilla Gunner behavior

### Memory Safety

- All pointer checks are performed
- Early returns for invalid states
- Maintains compatibility with vanilla behavior when disabled

### Performance Considerations

- Early exit for "First" mode to maintain vanilla performance
- Minimal overhead when system is disabled (default)
- Efficient passenger iteration using the game's existing passenger list

## Future Enhancements

Potential improvements that could be added:
1. Per-passenger weapon priority settings
2. Dynamic criteria based on target type
3. Passenger type-specific weapon preferences
4. Configurable scoring weights for Custom mode

## Backward Compatibility

The system is fully backward compatible:
- Default behavior unchanged (disabled by default)
- Vanilla Gunner units work as before
- No impact on non-Gunner units
- Can be enabled selectively per unit type