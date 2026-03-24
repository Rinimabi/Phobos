# Phobos Project Analysis

## Project Overview
Phobos is a community engine extension for Command & Conquer: Yuri's Revenge that provides new features and fixes. It's designed to work alongside Ares rather than replace it.

**Key Details:**
- Type: C++20 Windows DLL game extension
- License: GPL v3
- Status: Seeking active maintainers
- Documentation: https://phobos.readthedocs.io
- Repository: https://github.com/Phobos-developers/Phobos

## Technical Analysis

### Build System
- Platform: Windows only (Win32)
- Language: C++20
- IDE: Visual Studio 2022 minimum
- Build Types: Debug, DevBuild, Release
- Dependencies: YRpp submodule, Syringe injector

### Project Structure
```
Phobos/
├── src/                    # 313 C++ source files
│   ├── Phobos.cpp/h        # Extension bootstrap
│   ├── Commands/           # Hotkey commands
│   ├── Ext/                # Vanilla class extensions
│   ├── New/                # New types and entities
│   ├── Misc/               # Uncategorized hooks
│   └── Utilities/          # Shared infrastructure
├── YRpp/                   # Game binary headers (submodule)
├── docs/                   # Sphinx documentation
└── scripts/                # Build scripts
```

### Code Quality Assessment

**Strengths:**
- Excellent documentation (comprehensive, multilingual)
- Well-organized structure with clear separation of concerns
- Consistent naming conventions and code style
- Active development with recent commits
- Strong community support

**Critical Issues:**
- **Zero unit tests** - No automated testing framework
- Large source files (20+ files >1000 lines, largest 3,240 lines)
- Disabled C++ features (exceptions and RTTI off)
- Seeking active maintainers

## FV Weapon Selection Analysis

### Issue: Why FV Units Only Replace Weapons for the First Passenger

**Finding**: The limitation is in the vanilla game engine, not Phobos extensions.

**Analysis**:
1. **Gunner Detection Logic** (in `Hooks.MultiWeapon.cpp`):
   ```cpp
   if (isUnit
       && !pType->IsGattling && pType->TurretCount > 0
       && (pType->Gunner || !pTypeExt->MultiWeapon))
   {
       return UnitGunner; // 0x7431E4
   }
   ```

2. **Vanilla Game Limitation**: The actual weapon selection logic that determines which passenger's weapon to use is implemented in the vanilla game engine (addresses like 0x6F3451, 0x6F3474), not in Phobos hooks.

3. **Root Cause**: The vanilla Yuri's Revenge engine's Gunner weapon selection logic:
   - Only checks the **first passenger** when determining weapon replacement
   - Does not iterate through all passengers to find the best weapon match
   - This was a design decision to simplify the weapon selection algorithm

**Technical Details**:
- Hook at `0x7431C9` intercepts weapon selection for units
- Gunner vehicles are redirected to special handling at `0x7431E4`
- The actual passenger-to-weapon mapping happens in vanilla code that Phobos doesn't modify
- Phobos adds MultiWeapon support as an alternative system, but doesn't override the core Gunner behavior

**Conclusion**: This limitation exists because the original game developers implemented Gunner weapon replacement as a simple "first passenger wins" system to maintain compatibility with vanilla gameplay mechanics.

### Key Features
1. **AttachEffect System**: Advanced temporary effects on units
2. **Enhanced Game Mechanics**: Improved building, techno, infantry behaviors
3. **AI Improvements**: Enhanced scripting and mapping capabilities
4. **Visual Enhancements**: New animations, effects, UI features
5. **Compatibility**: Works with Ares and CnCNet5

### Documentation Quality
- Comprehensive Sphinx documentation
- Available in English and Chinese
- Well-organized with clear categories
- Version switching support
- Active maintenance

### Health Score: 66/100 (Moderate)

**Recommendations:**
1. **Critical**: Implement unit testing framework (Google Test/Catch2)
2. **High**: Refactor large files (>1000 lines)
3. **Medium**: Add static analysis to CI pipeline
4. **Low**: Modernize C++ usage (constexpr, noexcept)

### Installation Process
1. Install Visual Studio 2022 with dependencies from `.vsconfig`
2. Clone repository recursively (includes YRpp submodule)
3. Build using VS, VSCode, or command line scripts
4. Place Phobos.dll and Phobos.pdb in game folder
5. Launch via Syringe.exe injection

### Community
- International Discord: C&C Mod Haven
- Chinese Forum: Phobos CN
- Large contributor base with tiered maintainers
- Active PR merging and documentation updates

## Development Notes
- No network communication or external dependencies
- Uses runtime code injection via Syringe
- Maintains save game compatibility within versions
- Designed for modding community
- Warning: Not actively maintained enough - seeking help

## Quick Commands
```bash
# Build debug version
scripts/build_debug.bat

# Build documentation
pip install -r docs/requirements.txt
scripts/build_docs.bat

# Check git status
git status
```

---

## Recent Implementation: Gunner Passenger Iteration System

### Overview
Implemented a new passenger iteration system for Gunner-type units (FV/IFV) that allows considering weapons from ALL passengers, not just the first one as in vanilla.

### Key Features
- **Configurable Weapon Selection**: Choose from First (vanilla), Strongest, LongestRange, FastestROF, or Custom modes
- **Multiple Evaluation Criteria**: Damage, Range, ROF (Rate of Fire), and Versatility
- **Per-Unit Configuration**: Enable via INI settings per unit type
- **Backward Compatible**: Disabled by default, no impact on existing behavior

### Implementation Files
- `src/Ext/Techno/GunnerWeaponSystem.h/cpp` - Core weapon selection logic
- `src/Ext/Techno/Body.h/cpp` - Added configuration fields to TechnoExt
- `src/Ext/Techno/Hooks.GunnerWeapon.cpp` - Integration hooks
- `GUNNER_PASSENGER_ITERATION.md` - Detailed documentation

### Usage Example
```ini
[FV]
Gunner.IteratePassengers=true
Gunner.WeaponSelectionMode=Strongest
Gunner.WeaponCriteria=Damage,Range
```

### Technical Details
- Hooks into TechnoClass::SelectWeapon (0x6F3451)
- Iterates through all passengers using game's passenger list
- Evaluates weapons based on configurable criteria
- Maintains performance with early exit for "First" mode

---
*Analysis updated: 2026-03-24*
