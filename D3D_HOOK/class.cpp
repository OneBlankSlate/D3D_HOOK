#include "class.h"
#include"engine.h"
void* UGameInstance::GetPlayerController()
{
	DWORD64 p = *(DWORD64*)(this + 0x38);  //hack中也有
	
	DWORD64 player = *(DWORD64*)(p);
	return *(void**)(player + 0x30);
}

AActor* UGameInstance::GetMyPawn()
{
	DWORD64 Data = *(DWORD64*)(this + 0x38);  //得本地玩家数组，没有机器人   TArray<ULocalPlayer*>
	DWORD64 Player = *(DWORD64*)(Data);       //本地玩家数组的第一个为我们自己
	DWORD64 PlayerController = *(DWORD64*)(Player + 0x30);  //ULocalPlayer的0x30处为玩家控制器 APlayerController*    PlayerController

	return *(AActor**)(PlayerController + 0x250);    //玩家控制器的0x250处为Apawn对象

}

uint32_t AActor::GetID()
{
	return 	 *(uint32_t*)((uint8_t*)this + 0x18);  //AActor继承自UObject，但UObject结构没有被dump出来，在hack中的Offset.h中可以得到该偏移0x18
}


string AActor::Get_My_Name()    //没有用到  测试代码
{
	/*DWORD64 PlayerState = *(DWORD64*)((uint8_t*)this + 0x240);
	DWORD64 FString = ((ULONG_PTR)PlayerState + 0x300);
	wchar_t* name = (wchar_t*)((ULONG_PTR)FString + 0);*/
	UWorld* UWorld = GetWorld();
	DWORD64 OwningGameInstance = (DWORD64)(UWorld->OwningGameInstance);
	//printf("OwningGameInstance:%p\r\n", OwningGameInstance);
	DWORD64 PlayerTarray = *(DWORD64*)(OwningGameInstance + 0x38);  //玩家数组
	//printf("PlayerTarray:%p\r\n", PlayerTarray);
	DWORD64 SelfPlayer = *(DWORD64*)(PlayerTarray + 0);       //第一个为自己
	//printf("SelfPlayer:%p\r\n", SelfPlayer);
	DWORD64 PlayerController = *(DWORD64*)(SelfPlayer + 0x30); //PlayerController
	//printf("PlayerController:%p\r\n", PlayerController);
	DWORD64 Pawn = *(DWORD64*)(PlayerController + 0x250);  //APawn* Pawn
	//printf("Pawn:%p\r\n", Pawn);
	DWORD64 PlayerState = *(DWORD64*)(Pawn + 0x240);  //APlayerState* PlayerState
	//printf("PlayerState:%p\r\n", PlayerState);
	DWORD64 NameAddress = *(DWORD64*)(PlayerState + 0x300);  //FString->TArray -> T* Data
	//printf("NameAddress:%p\r\n", NameAddress);
	DWORD64 NameCharCount = *(DWORD*)(PlayerState + 0x300 + 0x8);//FString->TArray -> int count
	//printf("NameCharCount:%d\r\n", NameCharCount);
	DWORD64 NameLength = (NameCharCount + 1) * sizeof(wchar_t); //名字长度
	//printf("NameLength:%d\r\n", NameLength);

	wchar_t* Name = new wchar_t[NameLength];
	memcpy(Name, (void*)NameAddress, NameLength);

	PVOID v5 = malloc(NameLength);
	memset(v5, 0, NameLength);
	WideCharToMultiByte(CP_ACP, 0, Name, -1, (LPSTR)v5, wcslen(Name), NULL, NULL);
	string NameString = (char*)v5;
	return NameString;

}

string AActor::Get_Object_Class_Name()
{

	uint32_t Id = *(uint32_t*)((uint8_t*)this + 0x18);

	return GetName_(Id);
}
