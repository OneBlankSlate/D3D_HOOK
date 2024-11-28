#include "DX11.h"
#include <d3d11.h>
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "engine.h"
HWND g_hWnd;
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

typedef HRESULT(STDMETHODCALLTYPE* ResizeBuffers)(IDXGISwapChain* This, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);

typedef HRESULT(STDMETHODCALLTYPE* Present)(IDXGISwapChain* This, UINT SyncInterval, UINT Flags);
typedef LRESULT(WINAPI* WndProc)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

ResizeBuffers oResizeBuffers = nullptr;
DWORD64* Vtb = nullptr;
Present OldPresent = nullptr;
WndProc OldWndProc = nullptr;  //原窗口消息
_GameWindow_ GameWindow;  //正确设置在DX11Hook中
// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam); //这个函数用于处理ImGui相关的Windows消息。
LRESULT WINAPI HOOKWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)  //新窗口过程函数，它将被用来替换默认的窗口过程函数
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	//return ::DefWindowProcW(hWnd, msg, wParam, lParam);//默认的窗口过程函数DefWindowProcW
	return ::CallWindowProcA(OldWndProc, hWnd, msg, wParam, lParam);   //因为我们为了使用Imgui而hook了原来的窗口消息，如果不call原来的窗口消息，是无法点击原游戏窗口的！
}

bool GetBoneScreenPos(AActor* Actor, int boneIndex, Vector2& screenPos)
{
	int Index = Actor->Mesh->BoneTransform[0].Data ? 0 : 1;
	if (!Actor->Mesh->BoneTransform[Index].Data)
		return false;

	FMatrix Matrix = Actor->Mesh->BoneTransform[Index].Data[boneIndex].ToMatrixWithScale() *
		Actor->Mesh->ComponentToWorld.ToMatrixWithScale();
	Vector3 Pos;
	Pos.X = Matrix._41;
	Pos.Y = Matrix._42;
	Pos.Z = Matrix._43;
	return	 WorldToScreen(Pos, screenPos);
}
void MoveMouseRelative(int dx, int dy) {
	INPUT input = { 0 };
	input.type = INPUT_MOUSE;
	input.mi.dx = dx; // 相对x坐标移动
	input.mi.dy = dy; // 相对y坐标移动
	input.mi.mouseData = 0;
	input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE; // 设置移动标志
	input.mi.time = 0; // 系统会自动填充
	input.mi.dwExtraInfo = 0; // 可以设置为GetMessageExtraInfo()的结果，通常设置为0

	SendInput(1, &input, sizeof(INPUT)); // 发送输入
}

void DrawBone(AActor* Actor, int begin, int end, ImU32 color)  //绘制骨骼   经常崩溃！！！！
{
	int Index = Actor->Mesh->BoneTransform[0].Data ? 0 : 1;
	if (!Actor->Mesh->BoneTransform[Index].Data)
		return;

	FMatrix Matrix_begin = Actor->Mesh->BoneTransform[Index].Data[begin].ToMatrixWithScale() *
		Actor->Mesh->ComponentToWorld.ToMatrixWithScale();  //计算起始位置的变换矩阵
	Vector3 Pos_begin;   //从变换矩阵中提取骨骼起始位置的坐标
	Pos_begin.X = Matrix_begin._41;
	Pos_begin.Y = Matrix_begin._42;
	Pos_begin.Z = Matrix_begin._43;
	Vector2 BoneScreen_begin = { 0 };  //屏幕坐标 

	FMatrix Matrix_end = Actor->Mesh->BoneTransform[Index].Data[end].ToMatrixWithScale() *
		Actor->Mesh->ComponentToWorld.ToMatrixWithScale();  //结束位置的变换矩阵     //运行过程中会存在非法访问
	Vector3 Pos_end;
	Pos_end.X = Matrix_end._41;
	Pos_end.Y = Matrix_end._42;
	Pos_end.Z = Matrix_end._43;

	Vector2 BoneScreen_end = { 0 };

	if (WorldToScreen(Pos_begin, BoneScreen_begin) && WorldToScreen(Pos_end, BoneScreen_end))
	{
		ImGui::GetBackgroundDrawList()->AddLine(
			ImVec2((float)BoneScreen_begin.X, (float)BoneScreen_begin.Y),
			ImVec2((float)BoneScreen_end.X, (float)BoneScreen_end.Y),
			color);
	}
};

bool BoneESP = true;
float AIM_FOV = 200.f;
bool Is_AIM = true;
HRESULT STDMETHODCALLTYPE MyPresent(IDXGISwapChain* This, UINT SyncInterval, UINT Flags)  //HackPresent  自实现的绘制函数
{

	ImU32 color = ImGui::ColorConvertFloat4ToU32(ImVec4({ 1.0f, 0.0f, 1.0f, 1.0f })); //我们的绘制颜色，后续绘制都使用这个
	//printf("hooked\n");

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();  //调用ImGui的DirectX 11后端实现，为新的一帧开始做准备
	ImGui_ImplWin32_NewFrame();  //调用ImGui的Win32平台后端实现，为新的一帧开始做准备。
	ImGui::NewFrame();   //开始一个新的ImGui帧。
	 
	ImGui::Begin("Hello, world!");  //开启一个新的ImGui窗口，设置窗口标题

	ImGui::Checkbox("BoneESP", &BoneESP);
	ImGui::Checkbox("AIM", &Is_AIM);
	ImGui::SliderFloat("FOV", &AIM_FOV, 0.0f, 300.0f);
	ImGui::End();
	//在中心添加瞄准的判断范围
	ImGui::GetForegroundDrawList()->AddCircle({ (float)GameWindow.GameCenterX, (float)GameWindow.GameCenterY }, AIM_FOV, ImColor(255, 255, 255));
	UWorld* World = GetWorld();
	static bool first = true;
	if (first)
	{
		DWORD Prptect;
		VirtualProtect(World, 0x1000, PAGE_EXECUTE_READWRITE, &Prptect);
		first = false;
	}
	//自瞄数据
	AActor* AIM_Actor = nullptr;
	float temp_fov = AIM_FOV;
	//取屏幕中心坐标


	static int ScreenCenterX = GameWindow.GameCenterX;
	static int ScreenCenterY = GameWindow.GameCenterY;

	//TArray<AActor*>& Actors = World->PersistentLevel->Actors;//游戏结束了,被清空,强行访问报错,解决方案,判断,是否在游戏


	TArray<APlayerState*> Actors = GetWorld()->GameState->PlayerArray;   //新的解决方案
	for (size_t i = 0; i < Actors.Count; i++)
	{
		APlayerState* PlayerState = Actors.Data[i];


		if (!PlayerState || !PlayerState->PawnPrivate)
		{
			continue;
		}

		
		AActor* Actor = PlayerState->PawnPrivate;
		AActor* Player = World->OwningGameInstance->GetMyPawn();  //得自己角色对象
		string ClassName = Actor->Get_Object_Class_Name();  //类名
		/*if (GetWorld()->OwningGameInstance->GetPlayerController() == Actor)
		{
			char buffer[1024];
			sprintf(buffer, "%p", Actor);
			ImGui::Text(Actor->GetName().c_str());
			ImGui::Text(buffer);

		}*/

		if (ClassName.find("BotPawn_C") == string::npos)  //过滤掉自己
		{
			continue;
		}

		FTransform Transform = Actor->RootComponent->ComponentToWorld;
		Vector2 screen = { 0 };

		if (WorldToScreen(Transform.Translation, screen))  //Transform中的Translation是对象的世界坐标
		{
			//char buffer[1024];
			//sprintf(buffer, "%p", Actor);
			//测试代码
			//ImU32 color = ImGui::ColorConvertFloat4ToU32(ImVec4({ 1.0f, 0.0f, 1.0f, 1.0f }));
			//ImGui::GetBackgroundDrawList()->AddText(ImVec2((float)screen.X, (float)screen.Y), color, buffer);
			//ImGui::GetBackgroundDrawList()->AddText(ImVec2((float)screen.X, (float)screen.Y), color, Actor->GetName().c_str());
			//ImGui::Text(Actor->GetName().c_str());
			//ImGui::Text(buffer);
			//printf("%s\n", Actor->GetName().c_str());
		}
		//分析骨骼id
		/*for (size_t i = 0; i < Actor->Mesh->BoneTransform.Count; i++)
		{

			FMatrix Matrix = Actor->Mesh->BoneTransform.Data[i].ToMatrixWithScale() *   //骨骼相对坐标矩阵*ComponentToWorld转换矩阵=世界坐标系矩阵
				Actor->Mesh->ComponentToWorld.ToMatrixWithScale();
			Vector3 Pos;
			Pos.X = Matrix._41;
			Pos.Y = Matrix._42;
			Pos.Z = Matrix._43;

			Vector2 BoneScreen = { 0 };
			if (WorldToScreen(Pos, BoneScreen))
			{
				char buffer[1024];
				sprintf(buffer, "%d", i);
				ImU32 color = ImGui::ColorConvertFloat4ToU32(ImVec4({ 1.0f, 0.0f, 1.0f, 1.0f }));
				ImGui::GetBackgroundDrawList()->AddText(ImVec2((float)BoneScreen.X, (float)BoneScreen.Y), color, buffer);
			}

		}*/
		if (BoneESP) 
		{
			//绘制ID
			char wbuffer[1024];
			sprintf(wbuffer, "%ws", Actor->PlayerState->Name);
			ImGui::GetBackgroundDrawList()->AddText(ImVec2((float)screen.X - 15.0f, (float)screen.Y - 25.0f), color, wbuffer);


			//绘制骨骼
			/*DrawBone(Actor, 5, 6, color);
			DrawBone(Actor, 6, 3, color);
			DrawBone(Actor, 3, 1, color);*/
			DrawBone(Actor, 5, 1, color);


			/*DrawBone(Actor, 6, 7, color);
			DrawBone(Actor, 7, 9, color);
			DrawBone(Actor, 9, 11, color);*/
			DrawBone(Actor, 6, 11, color);

			/*DrawBone(Actor, 6, 34, color);
			DrawBone(Actor, 34, 36, color);
			DrawBone(Actor, 36, 39, color);*/
			DrawBone(Actor, 6, 39, color);

			/*DrawBone(Actor, 1, 61, color);
			DrawBone(Actor, 61, 62, color);
			DrawBone(Actor, 62, 63, color);*/
			DrawBone(Actor, 1, 63, color);

			/*DrawBone(Actor, 1, 66, color);
			DrawBone(Actor, 66, 67, color);
			DrawBone(Actor, 67, 68, color);*/
			DrawBone(Actor, 1, 68, color);

		}


		if (Is_AIM)
		{

			Vector2 AimPos = { 0 };
			if (GetBoneScreenPos(Actor, 5, AimPos))
			{
				float xx = ScreenCenterX - AimPos.X;
				float yy = ScreenCenterY - AimPos.Y;

				float dis = sqrt(xx * xx + yy * yy);
				if (dis < temp_fov)
				{
					temp_fov = dis;
					AIM_Actor = Actor;

				}

			}

		}
	}

	//判断按键是否按下

	if (AIM_Actor)
	{
		//获取指定骨骼的屏幕坐标
		Vector2 AimPos = { 0 };
		if (GetBoneScreenPos(AIM_Actor, 5, AimPos))
		{
			//PFS游戏中,鼠标相对位置为(0,0)在屏幕中心,,

			//获取鼠标相对位置
			int X = AimPos.X - ScreenCenterX;
			int Y = AimPos.Y - ScreenCenterY;
			//printf("%d,%d\n", X, Y);
			ImGui::GetBackgroundDrawList()->AddLine(
				ImVec2((float)ScreenCenterX, (float)ScreenCenterY),
				ImVec2((float)AimPos.X, (float)AimPos.Y),
				ImColor(255, 0, 0));

			//模拟鼠标相对移动
			if (GetAsyncKeyState(2))  //VK_RBUTTON  2
			{//MoveMouseRelative(X,Y);
				mouse_event(MOUSEEVENTF_MOVE, X, Y, 0, 0);
			}
		}
	}

	// Rendering
	ImGui::Render();   //渲染当前ImGui帧的内容。
	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr); //设置Direct3D设备上下文的渲染目标，这里设置为之前创建的g_mainRenderTargetView。
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());//将ImGui的绘制数据提交给DirectX 11后端，以便渲染ImGui内容。
	return OldPresent(This, SyncInterval, Flags);   //调用原始的Present函数，以完成帧的呈现。这里OldPresent是之前保存的原始Present函数的指针。
}

void GetDx11Ptr(IDXGISwapChain* This)
{
	g_pSwapChain = This;
	g_pSwapChain->GetDevice(__uuidof(g_pd3dDevice), (void**)&g_pd3dDevice);  //获取与交换链关联的Direct3D设备（ID3D11Device），并将其存储在全局变量g_pd3dDevice中。
	g_pd3dDevice->GetImmediateContext(&g_pd3dDeviceContext);//从获取到的设备中请求一个设备上下文（ID3D11DeviceContext），这是用于执行Direct3D 11绘制命令的对象，并将其存储在全局变量g_pd3dDeviceContext中。


	ID3D11Texture2D* pBackBuffer;  //用于存储交换链的背面缓冲区
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));   //获取交换链的第一个缓冲区（背面缓冲区），并将其接口指针存储在pBackBuffer中。
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);//使用背面缓冲区创建一个渲染目标视图（ID3D11RenderTargetView），这是将用于渲染操作的对象，并将其存储在全局变量g_mainRenderTargetView中。
	pBackBuffer->Release();
}


HRESULT STDMETHODCALLTYPE Init(IDXGISwapChain* This, UINT SyncInterval, UINT Flags)
{
	GetDx11Ptr(This);
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	ImGui::StyleColorsDark();


	OldWndProc = (WndProc)SetWindowLongPtrW(g_hWnd, GWLP_WNDPROC, (LONG_PTR)HOOKWndProc);

	ImGui_ImplWin32_Init(g_hWnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

	Vtb[8] = (DWORD64)MyPresent;
	return OldPresent(This, SyncInterval, Flags);
}


HRESULT STDMETHODCALLTYPE hkResizeBuffers(IDXGISwapChain* This, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	if (g_pd3dDevice)
	{
		g_pd3dDevice->Release();
		g_pd3dDevice = nullptr;
		g_mainRenderTargetView->Release();

		ImGui_ImplDX11_Shutdown();

		Vtb[8] = (DWORD64)Init;
	}


	return oResizeBuffers(This, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}




void DX11Hook()
{

	g_hWnd = FindWindowA("UnrealWindow", NULL);
	printf("g_hWnd %p\n", g_hWnd);
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;


	GetClientRect(g_hWnd, &GameWindow.GameRect);    //获取窗口客户区域大小，而不是窗口全部区域

	GameWindow.GameHight = GameWindow.GameRect.bottom;
	GameWindow.GameWidth = GameWindow.GameRect.right;
	GameWindow.GameCenterX = GameWindow.GameWidth * 0.5;
	GameWindow.GameCenterY = GameWindow.GameHight * 0.5;
	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
		res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);

	if (res != S_OK)
	{
		printf("D3D11CreateDeviceAndSwapChain failed.%d\n", res);
		MessageBoxA(NULL, "D3D11CreateDeviceAndSwapChain failed.", NULL, MB_ICONERROR);
		return;
	}
	Vtb = *(DWORD64**)g_pSwapChain;
	OldPresent = (Present)Vtb[8];
	DWORD Prptect;
	VirtualProtect(Vtb, 1, PAGE_EXECUTE_READWRITE, &Prptect);
	Vtb[8] = (DWORD64)Init;

	//HOOK窗口大小改变
	//oResizeBuffers = (ResizeBuffers)Vtb[13];
	//Vtb[13] = (DWORD64)hkResizeBuffers;
	g_pSwapChain->Release();
	//Vtb[8]  Present
	//IAT HOOK 
}
