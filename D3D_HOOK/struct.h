#ifndef STRUCT_H
#define STRUCT_H
#include "include.h"
struct Vector2
{
	float X;
	float Y;
};
struct Vector3 :public Vector2
{
	float Z;
};
struct Vector4 :public Vector3
{
	float W;
};
struct FMatrix
{
	float _11, _12, _13, _14;
	float _21, _22, _23, _24;
	float _31, _32, _33, _34;
	float _41, _42, _43, _44;
	FMatrix operator*(const FMatrix& other);


};
struct FTransform
{

	Vector4 Rotation;   //x y z w 
	Vector4 Translation;
	Vector4 Scale3D;
	FMatrix ToMatrixWithScale();
};


struct FNameEntry
{
	uint16_t bIsWide : 1;
	uint16_t LowercaseProbeHash : 5;
	uint16_t Len : 10;
	union
	{
		char AnsiName[1024];
		wchar_t WideName[1024];


	};


};

struct _GameWindow_
{
	HWND GameHWND;
	DWORD GameLeft, GameTop, GameWidth, GameHight, GameCenterX, GameCenterY;
	RECT GameRect;
};
#endif // !pch_h
