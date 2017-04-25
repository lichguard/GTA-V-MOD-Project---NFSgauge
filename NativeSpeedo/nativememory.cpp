#include "NativeMemory.h"

#ifndef WIN32_LEAN_AND_MEAN 
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <Psapi.h>
#include "..\..\inc\main.h"



MemoryAccess::MemoryAccess()
{
	const uintptr_t patternAddress = FindPattern(EntityPoolOpcodePattern, EntityPoolOpcodeMask);

	// 3 bytes are opcode and its first argument, so we add it to get relative address to patternAddress. 7 bytes are length of opcode and its parameters.
	sAddressEntityPool = reinterpret_cast<MemoryPool **>(*reinterpret_cast<int *>(patternAddress + 3) + patternAddress + 7);
}

int MemoryAccess::HandleToIndex(int Handle) 
{
	return Handle >> 8; // == Handle / 256
}

uintptr_t MemoryAccess::GetAddressOfEntity(int Handle) const
{
	return *reinterpret_cast<uintptr_t*>(GetAddressOfItemInPool(*sAddressEntityPool, Handle) + 8);
}

uint32_t MemoryAccess::Get_Memory(int handle, int offset)const
{
	uintptr_t addr = GetAddressOfEntity(handle);
	if (addr == 0)
		return 0;
	
	return *reinterpret_cast<uint32_t*>(addr + offset);
}

float MemoryAccess::GetVehicleRPM(int handle) const 
{
	
	int offset = 1992;
	if (getGameVersion() > 27) // + 64
		offset = 2068; 	//0x814
	else if (getGameVersion() > 25) // +64
		offset = 2036; 	//0x7F4
	else if (getGameVersion() > 3)
		offset = 2004;

	uintptr_t addr = GetAddressOfEntity(handle);
	return addr == 0 ? 0.0f : *(float*)(addr + offset);

	// 2004 = location starting point
	// 1992 = power supply to engine // 1988 also does this but not sure why
	//2000 some kind of gear switch
	//1992 for legacy
	//2004 for latest
}

//uint32_t
int MemoryAccess::GetGear(int handle) const
{

	int offset = 0x792;

	if (getGameVersion() > 27) // + 64
		offset = 0x7E0; 	//0x7D4
	else if (getGameVersion() > 25) // +32
		offset = 0x7C0; 	//0x7D4
	else if (getGameVersion() > 3)
		offset = 0x7A0;


	const uint64_t address = GetAddressOfEntity(handle);
	if (address == 0)
		return 0;

	return (*reinterpret_cast<const uint32_t *>(address + offset)) & 0x0000FFFF ;

}

float MemoryAccess::GetClutch(int handle) const {

	int offset = 0x7D0;

	if (getGameVersion() > 27) // + 64
		offset = 0x820;
	else if (getGameVersion() > 25) // +32
		offset = 0x800; 
	else if (getGameVersion() > 3)
		offset = 0x7E0;


	const uint64_t address = GetAddressOfEntity(handle);

	//return address == 0 ? 0 : *reinterpret_cast<const uint32_t *>(address + offset);

	return address == 0 ? 0 : *reinterpret_cast<const float *>(address + offset);

}

uint32_t MemoryAccess::GetTopGear(int handle) const {

	int offset = 0x796;

	if (getGameVersion() > 27) // + 64
		offset = 0x7E6; 	//0x7D4
	else if (getGameVersion() > 25) // +32
		offset = 0x7C6; 	//0x7D4
	else if (getGameVersion() > 3)
		offset = 0x7A6;


	const uint64_t address = GetAddressOfEntity(handle);

	return address == 0 ? 0 : *reinterpret_cast<const uint32_t *>(address + offset);

}

uintptr_t MemoryAccess::FindPattern(const char *pattern, const char *mask)
{
	MODULEINFO modInfo = { 0 };
	GetModuleInformation(GetCurrentProcess(), GetModuleHandle(nullptr), &modInfo, sizeof(MODULEINFO));

	const char *start_offset = reinterpret_cast<const char *>(modInfo.lpBaseOfDll);
	const uintptr_t size = static_cast<uintptr_t>(modInfo.SizeOfImage);

	intptr_t pos = 0;
	const uintptr_t searchLen = static_cast<uintptr_t>(strlen(mask) - 1);

	for (const char *retAddress = start_offset; retAddress < start_offset + size; retAddress++)
	{
		if (*retAddress == pattern[pos] || mask[pos] == '?')
		{
			if (mask[pos + 1] == '\0')
			{
				return (reinterpret_cast<uintptr_t>(retAddress) - searchLen);
			}

			pos++;
		}
		else
		{
			pos = 0;
		}
	}

	return 0;
}

uintptr_t MemoryAccess::GetAddressOfItemInPool(const MemoryPool *PoolAddress, int Handle) 
{
	if (PoolAddress == nullptr)
	{
		return 0;
	}

	const int index = HandleToIndex(Handle);
	const int flag = PoolAddress->BoolAdr[index]; // flag should be equal to 2 if everything is ok

												  // parity check? (taken from ScriptHookDotNet for IV
	if (flag & 0x80 || flag != (Handle & 0xFF))
	{
		return 0;
	}

	return (PoolAddress->ListAddr + index * PoolAddress->ItemSize);
}
