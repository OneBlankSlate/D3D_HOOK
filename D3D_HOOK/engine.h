#ifndef engine_h
#define engine_h
#include "include.h"
#include "struct.h"
#include "class.h"
//���������ڵ�����sdk�µ�Function.cpp��
typedef bool (WINAPI* fun_tProjectWorldToScreen)(void* thsin, Vector3 WorldPosition, Vector2& ScreenPosition, bool bPlayerViewportRelative);

bool WorldToScreen(Vector3 WorldPosition, Vector2& ScreenPosition);
UWorld* GetWorld();
string GetName_(uint32_t Id);
//29F3580
#endif // !engine_h
