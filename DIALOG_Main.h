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
//����ͨ�ÿؼ�
#include <commctrl.h>
#pragma comment(lib,"comctl32.lib")

//==================================================================================================================
extern PVOID g_pFileBuffer;
extern DWORD g_FileSize;
//ȫ�ֱ���hDlg���Դ洢 Ӧ�ó���ʵ�����(ImageBase)
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

////==============================================================================================������==================================================================================��������
//**************************************************�ص�����_������
BOOL CALLBACK MainDlgProc(									
						 HWND hwndDlg,  // handle to dialog box			
						 UINT uMsg,     // message			
						 WPARAM wParam, // first message parameter			
						 LPARAM lParam  // second message parameter			
						 );
//**************************************************��ʼ������_�����ڽ����б�
void InitProcessListView(void);
//**************************************************��ȡ��ǰ���н��̲�����[process]�б�
void InitProcessListContent(void);
//**************************************************ִ�к���_����ѡ�н��̵�����ģ�飬������Ϣ����ģ���б�
void EnumModules(WPARAM wParam,LPARAM lParam);
//**************************************************���pid��Ӧ�Ľ�����ģ����Ϣ
BOOL GetMainModuleInfo(DWORD dwPID,MODULEINFO* modInf);
//**************************************************��ʼ������_������ģ���б�
void InitModuleListView(void);
//**************************************************���ݡ�processList��ѡ�еĽ��̵�dwPID�������̵�����ģ�����롾moduleList��
void PostModuleListContent(DWORD dwPID);
//**************************************************����pid��ȡ��������ģ��ľ��
void GetAllModuleHandle(IN DWORD dwPID,OUT HMODULE* hMoudle,IN DWORD maxSize,OUT PDWORD num_Module);
//==============================================================================================About����==================================================================================
//**************************************************�ص�����_���ڴ���
BOOL CALLBACK AboutDlgProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
//**************************************************��ʼ������_���ڴ��ڣ�hwndDlgΪ�����ھ��
void InitAboutWindow(void);
//===============================================================================================pe����=================================================================================
//**************************************************����һ���ļ�ѡ�񴰿ڣ�������pe����
void CreatePeDialog(void);
//**************************************************�ص�����_pe����
BOOL CALLBACK PeDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
//**************************************************��ʼ��pe���ڣ���ѡ���ļ���pe��Ϣ������Ӧλ��
void InitPeDialog(void);
//**************************************************��content������д��edit�ؼ�
void SetEditContent(DWORD dwEditID,TCHAR* content);

//================================================================================================�ڱ���================================================================================
//**************************************************�ص�����_SectionTable����
BOOL CALLBACK SectionTableDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
//**************************************************��ʼ���ڱ���
void InitSectionTableDialog(void);	
//**************************************************SectionTable����SectionList�б��ʼ������
void InitSectionTableListView(void);					  
//=================================================================================================Ŀ¼����===============================================================================
//**************************************************�ص�����_Direction����
BOOL CALLBACK DirectionDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
//**************************************************��ʼ��Direction����
void InitDirectionDialog(void);
//**************************************************��pBuffer������д��Ŀ¼���ڵ�edit�ؼ�
void SetDirectionEditContent(DWORD dwEditID,TCHAR* pBuffer);











//=================================================================================================Ŀ¼����ϸ��Ϣ����===============================================================================
//=================================================================================================ExportTable����===============================================================================
//**************************************************�ص�����_ExportTable����
BOOL CALLBACK ExportTableDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
//**************************************************��ʼ��������
void InitExportDialog(void);
//=================================================================================================ImportTable����===============================================================================
//**************************************************�ص�����_ImportTable����
BOOL CALLBACK ImportTableDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
//**************************************************��ʼ�������
void InitImportDialog(void);
//=================================================================================================ResourceTable����===============================================================================
//**************************************************�ص�����_ResourceTable����
BOOL CALLBACK ResourceTableDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
//**************************************************��ʼ����Դ���
void InitResourceDialog(void);
//=================================================================================================ResourceTable����===============================================================================
//**************************************************�ص�����_BaseReloc����
BOOL CALLBACK BaseRelocTableDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
//**************************************************��ʼ����Դ���
void InitBaseRelocDialog(void);
//=================================================================================================IAT����===============================================================================
//**************************************************�ص�����_IAT����
BOOL CALLBACK IATDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
//**************************************************��ʼ��IAT
void InitIATDialog(void);
//=================================================================================================�󶨵������===============================================================================
//**************************************************�ص�����_�󶨵������
BOOL CALLBACK BoundImportDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
//**************************************************��ʼ���󶨵������
void InitBoundImportDialog(void);

void CreateShellDialog(void);
// void CreateRemoteThreadDLLDialog(void);
// void CreateImportTableDLLDialog(void);
//**************************************************�ص�����_DllThread����
BOOL CALLBACK DllThreadDlgProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
//**************************************************�ص�����_DllImport����
BOOL CALLBACK DllImportDlgProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);



#endif	//DIALOG_MAIN_20220214