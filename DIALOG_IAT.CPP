#include "stdafx.h"
#include "DIALOG_Main.h"


//**************************************************回调函数_BaseReloc窗口
BOOL CALLBACK IATDialogProc(									
						  HWND hwndDlg,  // handle to dialog box			
						  UINT uMsg,     // message			
						  WPARAM wParam, // first message parameter			
						  LPARAM lParam  // second message parameter			
						  ){
	g_hDialogIAT = hwndDlg;
	switch(uMsg){
	case WM_INITDIALOG:
		InitIATDialog();
		return TRUE;
	case WM_CLOSE:
		EndDialog(g_hDialogIAT,0);
		return TRUE;		
	}
	return FALSE;
}
//**************************************************初始化IAT窗口
void InitIATDialog(void){
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
	pDirectoryHeader = (PIMAGE_DATA_DIRECTORY)&pOptionHeader->DataDirectory[12];
	if(pDirectoryHeader->Size == 0){
		//该表不存在
		MessageBox(g_hDialogDirection,TEXT("IAT不存在"),0,MB_OK);
		EndDialog(g_hDialogIAT,0);
		return;
	}
	//打印IAT内容 
	DWORD Foa = RvaToFileOffset(g_pFileBuffer, pDirectoryHeader->VirtualAddress);
	PIMAGE_THUNK_DATA32 pIAT = (PIMAGE_THUNK_DATA32)((DWORD)g_pFileBuffer + Foa);
	PDWORD pTempIAT = (PDWORD)pIAT;
	PVOID buffer = malloc(1024*1024);
	memset(buffer,0,1024*1024);		//必须初始化否则”吨吨吨“
	TCHAR* row = (TCHAR*)buffer;
	for(int index=0;*pTempIAT;index++,pTempIAT++){
		wsprintf(row += _tcslen(row),TEXT("%s%d%s%30p\r\n"),TEXT("第"),index + 1,TEXT("个函数地址(RVA):"),*pTempIAT);
	}
	HWND hEdit = GetDlgItem(g_hDialogIAT,IDC_EDIT_IAT);
	SetWindowText(hEdit,(TCHAR*)buffer);
}
