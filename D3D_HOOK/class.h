#ifndef CLASS_H
#define CLASS_H

#include"struct.h"
#include"include.h"


template<class T>
class TArray
{
public:
	T* Data;
	int Count;
	int Max;
};

class FString
{
public:
	TArray<TCHAR> DataType;
};



class USceneComponent
{
public:
	char pa_00[0x1c0];     //UE5Dumper也没有导出,hack中有
	FTransform ComponentToWorld;//0X30  //Transform结构代表一种转换，就像其变量名一样，表示将组件从局部坐标系转换到世界坐标系,用在将骨骼的相对坐标转换到世界坐标以便绘制
};

class USkeletalMeshComponent :public USceneComponent
{
public:
	char pa_00[0x2C0];                  //0X4B0-0X1C0-0X30=
	TArray<FTransform> BoneTransform[2];//骨骼有两套
	                                    //这里原本偏移是4B0，sdk未导出，但由于继承了USceneComponent，因此偏移被修改了  

};

class AActor;

class APlayerState
{
public:
	char pa_00[0x280];
	AActor* PawnPrivate;// 0x0280(0x0008)

	char pa_01[0x78];
	wchar_t* Name;//0x300   //这里虽然是一个正常的wchar_t*类型的指针，但实际占16字节，即0x10，因为在内存中这里实际上是FString类型，即一个TArray
	char pa_18[0x10];
	int Team;//0x320

};

class AActor
{
public:
	char pa_00[0x130];
	USceneComponent* RootComponent;//8  0x280-0x138=0x148

	char pa_108[0x108];

	APlayerState* PlayerState;  // 0x0240(0x0008)
	char pa_01[0x38];// 
	USkeletalMeshComponent* Mesh;  // 0x0280(0x0008)


	uint32_t GetID();  //这个是对象在世界中的id数值，不是真正的人物名称
	string Get_My_Name();    //没有用到  测试代码
	string Get_Object_Class_Name();//ID得类名  WeapGun_C  BotPawn_C
};

class UGameInstance
{
public:
	void* GetPlayerController();
	AActor* GetMyPawn();
};
class ULevel {
public:
	char pa_00[0x98];        //这个在Class.h中的class ULevel中并未找到，在hack中的Offset.h中能找到
	TArray<AActor*> Actors;	 //世界中所有对象，不止人物
};


class AGameStateBase
{
public:
	char pa_00[0x238];
	TArray<APlayerState*>   PlayerArray;                                                  // 0x0238(0x0010)

};
class UWorld
{
public:
	char pa_00[0x30];
	ULevel* PersistentLevel;//8
	char pa_E8[0xE8];
	AGameStateBase* GameState;// 0x0120(0x0008)

	char pa_01[0x58];
	UGameInstance* OwningGameInstance;

};



#endif // !1
