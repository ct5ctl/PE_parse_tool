#include "stdafx.h"
#include "DIALOG_Main.h"

//**************************************************回调函数_Direction窗口
BOOL CALLBACK DirectionDialogProc(									
						  HWND hwndDlg,  // handle to dialog box			
						  UINT uMsg,     // message			
						  WPARAM wParam, // first message parameter			
						  LPARAM lParam  // second message parameter			
						  ){
	g_hDialogDirection = hwndDlg;
	switch(uMsg){
	case WM_INITDIALOG:
		InitDirectionDialog();
		return TRUE;
	case WM_CLOSE:
		EndDialog(g_hDialogDirection,0);
		return TRUE;
	case WM_COMMAND:
		switch(wParam)
		{
		case IDC_BUTTON_DIRECTION_CLOSE:
			EndDialog(g_hDialogDirection,0);
			return TRUE;
		case IDC_BUTTON_DIRECTION_OUTPORT:
			DialogBox(g_hDlg,(LPCTSTR)IDD_DIALOG_EXPORTTABLE,g_hDialogDirection,ExportTableDialogProc);
			return TRUE;
		case IDC_BUTTON_DIRECTION_IMPORT:	
			DialogBox(g_hDlg,(LPCTSTR)IDD_DIALOG_IMPORTTABLE,g_hDialogDirection,ImportTableDialogProc);
			return TRUE;
		case IDC_BUTTON_DIRECTION_RESOURCETABLE:	
			DialogBox(g_hDlg,(LPCTSTR)IDD_DIALOG_RESOURCETABLE,g_hDialogDirection,ResourceTableDialogProc);
			return TRUE;
		case IDC_BUTTON_DIRECTION_BASERELOC:
			DialogBox(g_hDlg,(LPCTSTR)IDD_DIALOG_BASERELOC,g_hDialogDirection,BaseRelocTableDialogProc);
			return TRUE;
		case IDC_BUTTON_DIRECTION_BOUNDIMPORT:
			DialogBox(g_hDlg,(LPCTSTR)IDD_DIALOG_BOUNDIMPORT,g_hDialogDirection,BoundImportDialogProc);
			return TRUE;
		case IDC_BUTTON_DIRECTION_IAT:
			DialogBox(g_hDlg,(LPCTSTR)IDD_DIALOG_IAT,g_hDialogDirection,IATDialogProc);
			return TRUE;
		}
	}
	return FALSE;
}
//**************************************************初始化Direction窗口
void InitDirectionDialog(void){
	// 初始化PE头部结构体
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDirectoryHeader = NULL;
	pDosHeader = (PIMAGE_DOS_HEADER)g_pFileBuffer;
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)g_pFileBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pDirectoryHeader = (PIMAGE_DATA_DIRECTORY)pOptionHeader->DataDirectory;
	//将数据填入eidt控件
	TCHAR rvaBuffer[100] = {0};
	TCHAR sizeBuffer[100] = {0};
	//1
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[0])->VirtualAddress);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_R1,rvaBuffer);
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[0])->Size);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_S1,rvaBuffer);
	//2
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[1])->VirtualAddress);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_R2,rvaBuffer);
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[1])->Size);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_S2,rvaBuffer);
	//3
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[2])->VirtualAddress);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_R3,rvaBuffer);
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[2])->Size);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_S3,rvaBuffer);
	//4
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[3])->VirtualAddress);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_R4,rvaBuffer);
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[3])->Size);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_S4,rvaBuffer);
	//5
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[4])->VirtualAddress);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_R5,rvaBuffer);
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[4])->Size);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_S5,rvaBuffer);
	//6
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[5])->VirtualAddress);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_R6,rvaBuffer);
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[5])->Size);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_S6,rvaBuffer);
	//7
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[6])->VirtualAddress);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_R7,rvaBuffer);
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[6])->Size);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_S7,rvaBuffer);
	//8
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[7])->VirtualAddress);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_R8,rvaBuffer);
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[7])->Size);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_S8,rvaBuffer);
	//9
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[8])->VirtualAddress);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_R9,rvaBuffer);
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[8])->Size);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_S9,rvaBuffer);
	//10
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[9])->VirtualAddress);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_R10,rvaBuffer);
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[9])->Size);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_S10,rvaBuffer);
	//11
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[10])->VirtualAddress);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_R11,rvaBuffer);
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[10])->Size);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_S11,rvaBuffer);
	//12
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[11])->VirtualAddress);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_R12,rvaBuffer);
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[11])->Size);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_S12,rvaBuffer);
	//13
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[12])->VirtualAddress);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_R13,rvaBuffer);
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[12])->Size);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_S13,rvaBuffer);
	//14
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[13])->VirtualAddress);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_R14,rvaBuffer);
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[13])->Size);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_S14,rvaBuffer);
	//15
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[14])->VirtualAddress);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_R15,rvaBuffer);
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[14])->Size);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_S15,rvaBuffer);
	//16
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[15])->VirtualAddress);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_R16,rvaBuffer);
	wsprintf(rvaBuffer,TEXT("%p"),(&pDirectoryHeader[15])->Size);
	SetDirectionEditContent(IDC_EDIT_DIRECTION_S16,rvaBuffer);


}
//**************************************************将pBuffer的内容写入目录窗口的edit控件
void SetDirectionEditContent(DWORD dwEditID,TCHAR* pBuffer){	
	HWND hEdit = GetDlgItem(g_hDialogDirection,dwEditID);
	SetWindowText(hEdit,pBuffer);
}