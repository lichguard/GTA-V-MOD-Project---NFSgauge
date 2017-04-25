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
	 int GetGear(int handle) const;
	 float GetClutch(int handle) const;
	 uint32_t GetTopGear(int handle) const;
private:
	static uintptr_t FindPattern(const char *pattern, const char *mask);
	static uintptr_t GetAddressOfItemInPool(const MemoryPool *PoolAddress, int Handle);
	const char *EntityPoolOpcodeMask = "xxx????xxxxxxx";
	const char *EntityPoolOpcodePattern = "\x4C\x8B\x0D\x00\x00\x00\x00\x44\x8B\xC1\x49\x8B\x41\x08";
	MemoryPool **sAddressEntityPool = nullptr;
};

#endif