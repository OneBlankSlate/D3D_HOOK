#include "engine.h"
uint8_t* GameData = (uint8_t*)GetModuleHandleA(0);
UWorld** Uworld = (UWorld**)(GameData + 0x4C3AD60);
uint8_t** GName = (uint8_t**)(GameData + 0x4AB6650 - 0x10);
fun_tProjectWorldToScreen _fun_tProjectWorldToScreen = (fun_tProjectWorldToScreen)(GameData + 0x29F3580);

UWorld* GetWorld()
{
	return *Uworld;

}

bool WorldToScreen(Vector3 WorldPosition, Vector2& ScreenPosition)
{
	void* pthis = GetWorld()->OwningGameInstance->GetPlayerController();

	return _fun_tProjectWorldToScreen(pthis, WorldPosition, ScreenPosition,false);
}
string GetName_(uint32_t Id)   //该函数的算法是UE引擎中的
{ 
	uint32_t Block = Id >> 16;
	uint32_t Offset = Id & 0xFFFF;//65535

	FNameEntry* Info = (FNameEntry*)(GName[2 + Block] + 2 * Offset);

	return string(Info->AnsiName, Info->Len);
}