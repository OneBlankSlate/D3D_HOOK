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
	char pa_00[0x1c0];     //UE5DumperҲû�е���,hack����
	FTransform ComponentToWorld;//0X30  //Transform�ṹ����һ��ת���������������һ������ʾ������Ӿֲ�����ϵת������������ϵ,���ڽ��������������ת�������������Ա����
};

class USkeletalMeshComponent :public USceneComponent
{
public:
	char pa_00[0x2C0];                  //0X4B0-0X1C0-0X30=
	TArray<FTransform> BoneTransform[2];//����������
	                                    //����ԭ��ƫ����4B0��sdkδ�����������ڼ̳���USceneComponent�����ƫ�Ʊ��޸���  

};

class AActor;

class APlayerState
{
public:
	char pa_00[0x280];
	AActor* PawnPrivate;// 0x0280(0x0008)

	char pa_01[0x78];
	wchar_t* Name;//0x300   //������Ȼ��һ��������wchar_t*���͵�ָ�룬��ʵ��ռ16�ֽڣ���0x10����Ϊ���ڴ�������ʵ������FString���ͣ���һ��TArray
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


	uint32_t GetID();  //����Ƕ����������е�id��ֵ��������������������
	string Get_My_Name();    //û���õ�  ���Դ���
	string Get_Object_Class_Name();//ID������  WeapGun_C  BotPawn_C
};

class UGameInstance
{
public:
	void* GetPlayerController();
	AActor* GetMyPawn();
};
class ULevel {
public:
	char pa_00[0x98];        //�����Class.h�е�class ULevel�в�δ�ҵ�����hack�е�Offset.h�����ҵ�
	TArray<AActor*> Actors;	 //���������ж��󣬲�ֹ����
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
