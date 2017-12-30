#ifndef _NATIVEMEMORY_
#define _NATIVEMEMORY_

#include <cstdint>

struct MemoryPool {
	uintptr_t ListAddr;
	char *BoolAdr;
	int MaxCount;
	int ItemSize;
};

class MemoryAccess {
public:
	MemoryAccess();

	static int HandleToIndex(int Handle);
	uintptr_t GetAddressOfEntity(int Handle) const;

	uint32_t Get_Memory(int handle, int offset) const;
	 float GetVehicleRPM(int handle) const;
	 uint16_t GetGear(int handle) const;
	 float GetClutch(int handle) const;
	 unsigned char GetTopGear(int handle) const;
private:
	static uintptr_t FindPattern(const char *pattern, const char *mask);
	static uintptr_t GetAddressOfItemInPool(const MemoryPool *PoolAddress, int Handle);
	const char *EntityPoolOpcodeMask = "xxx????xxxxxxx";
	const char *EntityPoolOpcodePattern = "\x4C\x8B\x0D\x00\x00\x00\x00\x44\x8B\xC1\x49\x8B\x41\x08";
	MemoryPool **sAddressEntityPool = nullptr;
};

enum G_GameVersion : int {
	G_VER_1_0_335_2_STEAM, // 00
	G_VER_1_0_335_2_NOSTEAM, // 01

	G_VER_1_0_350_1_STEAM, // 02
	G_VER_1_0_350_2_NOSTEAM, // 03

	G_VER_1_0_372_2_STEAM, // 04
	G_VER_1_0_372_2_NOSTEAM, // 05

	G_VER_1_0_393_2_STEAM, // 06
	G_VER_1_0_393_2_NOSTEAM, // 07

	G_VER_1_0_393_4_STEAM, // 08
	G_VER_1_0_393_4_NOSTEAM, // 09

	G_VER_1_0_463_1_STEAM, // 10
	G_VER_1_0_463_1_NOSTEAM, // 11

	G_VER_1_0_505_2_STEAM, // 12
	G_VER_1_0_505_2_NOSTEAM, // 13

	G_VER_1_0_573_1_STEAM, // 14
	G_VER_1_0_573_1_NOSTEAM, // 15

	G_VER_1_0_617_1_STEAM, // 16
	G_VER_1_0_617_1_NOSTEAM, // 17

	G_VER_1_0_678_1_STEAM, // 18
	G_VER_1_0_678_1_NOSTEAM, // 19

	G_VER_1_0_757_2_STEAM, // 20
	G_VER_1_0_757_2_NOSTEAM, // 21

	G_VER_1_0_757_4_STEAM, // 22
	G_VER_1_0_757_4_NOSTEAM, // 23

	G_VER_1_0_791_2_STEAM, // 24
	G_VER_1_0_791_2_NOSTEAM, // 25

	G_VER_1_0_877_1_STEAM, // 26
	G_VER_1_0_877_1_NOSTEAM, // 27

	G_VER_1_0_944_2_STEAM, // 28
	G_VER_1_0_944_2_NOSTEAM, // 29

	G_VER_1_0_1011_1_STEAM, // 30
	G_VER_1_0_1011_1_NOSTEAM, // 31

	G_VER_1_0_1032_1_STEAM, // 32
	G_VER_1_0_1032_1_NOSTEAM, // 33

	G_VER_1_0_1103_2_STEAM, // 34
	G_VER_1_0_1103_2_NOSTEAM, // 35

	G_VER_1_0_1180_2_STEAM, // 36
	G_VER_1_0_1180_2_NOSTEAM, // 37
};
#endif