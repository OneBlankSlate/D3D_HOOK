// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "include.h"
#include "DX11.h"
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:

        AllocConsole();   //为调用进程创建一个新的控制台窗口
        freopen("CONOUT$", "w+", stdout);  //重新打开一个文件流，并将标准输出流 stdout 重定向到一个新的文件或设备(即上面创建的新的控制台窗口)
        //"CONOUT$" 是一个特殊文件名，用于引用当前控制台的标准输出流
        //"w+" 模式意味着打开文件用于读写（w 表示写入，+ 表示更新模式，即可以读写），如果文件不存在则创建它。
        //这里它实际上是以写入模式打开控制台，这会清空控制台的内容。
        /*当你在程序中调用 freopen("CONOUT$", "w+", stdout); 时，你实际上是在告诉程序将 stdout 重定向到控制台的标准输出流。这通常在程序已经通过
        AllocConsole() 创建了一个新的控制台之后使用，以确保程序的输出能够显示在新创建的控制台上。*/

        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DX11Hook, NULL, 0, NULL);

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

