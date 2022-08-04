#ifndef DIALOG_MAIN_20220214
#define DIALOG_MAIN_20220214
#pragma once

#include <tlhelp32.h>
#include "psapi.h"		//
#pragma comment (lib,"psapi.lib")	
#include <shlwapi.h>
#pragma comment (lib,"shlwapi.lib")
#include <commdlg.h>
#include "resource.h"
#include "PeTools.h"
#include <tchar.h>
//声明通用控件
#include <commctrl.h>
#pragma comment(lib,"comctl32.lib")

//==================================================================================================================
extern PVOID g_pFileBuffer;
extern DWORD g_FileSize;
//全局变量hDlg用以存储 应用程序实例句柄(ImageBase)
extern HINSTANCE g_hDlg;
extern HWND g_hListProcess;
extern HWND g_hListModule;
extern HWND g_hDialogMain;
extern HWND g_hDialogAbout;
extern HWND g_hDialogPe;
extern HWND g_hDialogSectionTable;
extern HWND g_hDialogDirection;
extern HWND g_hDialogDetail;
extern HWND g_hDialogExport;
extern HWND g_hDialogImport;
extern HWND g_hDialogResource;
extern HWND g_hDialogBaseReloc;
extern HWND g_hDialogBoundImport;
extern HWND g_hDialogIAT;
//==================================================================================================================

////==============================================================================================主窗口==================================================================================函数声明
//**************************************************回调函数_主窗口
BOOL CALLBACK MainDlgProc(									
						 HWND hwndDlg,  // handle to dialog box			
						 UINT uMsg,     // message			
						 WPARAM wParam, // first message parameter			
						 LPARAM lParam  // second message parameter			
						 );
//**************************************************初始化函数_主窗口进程列表
void InitProcessListView(void);
//**************************************************获取当前所有进程并填入[process]列表
void InitProcessListContent(void);
//**************************************************执行函数_遍历选中进程的所有模块，并将信息填入模块列表
void EnumModules(WPARAM wParam,LPARAM lParam);
//**************************************************获得pid对应的进程主模块信息
BOOL GetMainModuleInfo(DWORD dwPID,MODULEINFO* modInf);
//**************************************************初始化函数_主窗口模块列表
void InitModuleListView(void);
//**************************************************根据【processList】选中的进程的dwPID，将进程的所有模块填入【moduleList】
void PostModuleListContent(DWORD dwPID);
//**************************************************根据pid获取进程所有模块的句柄
void GetAllModuleHandle(IN DWORD dwPID,OUT HMODULE* hMoudle,IN DWORD maxSize,OUT PDWORD num_Module);
//==============================================================================================About窗口==================================================================================
//**************************************************回调函数_关于窗口
BOOL CALLBACK AboutDlgProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
//**************************************************初始化函数_关于窗口，hwndDlg为父窗口句柄
void InitAboutWindow(void);
//===============================================================================================pe窗口=================================================================================
//**************************************************创建一个文件选择窗口，并创建pe窗口
void CreatePeDialog(void);
//**************************************************回调函数_pe窗口
BOOL CALLBACK PeDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
//**************************************************初始化pe窗口，将选中文件的pe信息填入相应位置
void InitPeDialog(void);
//**************************************************将content的内容写入edit控件
void SetEditContent(DWORD dwEditID,TCHAR* content);

//================================================================================================节表窗口================================================================================
//**************************************************回调函数_SectionTable窗口
BOOL CALLBACK SectionTableDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
//**************************************************初始化节表窗口
void InitSectionTableDialog(void);	
//**************************************************SectionTable窗口SectionList列表初始化函数
void InitSectionTableListView(void);					  
//=================================================================================================目录窗口===============================================================================
//**************************************************回调函数_Direction窗口
BOOL CALLBACK DirectionDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
//**************************************************初始化Direction窗口
void InitDirectionDialog(void);
//**************************************************将pBuffer的内容写入目录窗口的edit控件
void SetDirectionEditContent(DWORD dwEditID,TCHAR* pBuffer);











//=================================================================================================目录项详细信息窗口===============================================================================
//=================================================================================================ExportTable窗口===============================================================================
//**************************************************回调函数_ExportTable窗口
BOOL CALLBACK ExportTableDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
//**************************************************初始化导出表
void InitExportDialog(void);
//=================================================================================================ImportTable窗口===============================================================================
//**************************************************回调函数_ImportTable窗口
BOOL CALLBACK ImportTableDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
//**************************************************初始化导入表
void InitImportDialog(void);
//=================================================================================================ResourceTable窗口===============================================================================
//**************************************************回调函数_ResourceTable窗口
BOOL CALLBACK ResourceTableDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
//**************************************************初始化资源表表
void InitResourceDialog(void);
//=================================================================================================ResourceTable窗口===============================================================================
//**************************************************回调函数_BaseReloc窗口
BOOL CALLBACK BaseRelocTableDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
//**************************************************初始化资源表表
void InitBaseRelocDialog(void);
//=================================================================================================IAT窗口===============================================================================
//**************************************************回调函数_IAT窗口
BOOL CALLBACK IATDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
//**************************************************初始化IAT
void InitIATDialog(void);
//=================================================================================================绑定导入表窗口===============================================================================
//**************************************************回调函数_绑定导入表窗口
BOOL CALLBACK BoundImportDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
//**************************************************初始化绑定导入表窗口
void InitBoundImportDialog(void);

void CreateShellDialog(void);
// void CreateRemoteThreadDLLDialog(void);
// void CreateImportTableDLLDialog(void);
//**************************************************回调函数_DllThread窗口
BOOL CALLBACK DllThreadDlgProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
//**************************************************回调函数_DllImport窗口
BOOL CALLBACK DllImportDlgProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);



#endif	//DIALOG_MAIN_20220214