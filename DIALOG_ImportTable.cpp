#include "stdafx.h"
#include "DIALOG_Main.h"

//**************************************************回调函数_ExportTable窗口
BOOL CALLBACK ImportTableDialogProc(									
						  HWND hwndDlg,  // handle to dialog box			
						  UINT uMsg,     // message			
						  WPARAM wParam, // first message parameter			
						  LPARAM lParam  // second message parameter			
						  ){
	g_hDialogImport = hwndDlg;
	switch(uMsg){
	case WM_INITDIALOG:
		InitImportDialog();
		return TRUE;
	case WM_CLOSE:
		EndDialog(g_hDialogImport,0);
		return TRUE;		
	}
	return FALSE;
}
//**************************************************初始化导入表
void InitImportDialog(void){
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
	pDirectoryHeader = (PIMAGE_DATA_DIRECTORY)&pOptionHeader->DataDirectory[1];
	if(pDirectoryHeader->Size == 0){
		//该表不存在
		MessageBox(g_hDialogDirection,TEXT("导入表不存在"),0,MB_OK);
		EndDialog(g_hDialogImport,0);
		return;
	}

	DWORD Foa = RvaToFileOffset(g_pFileBuffer, pDirectoryHeader->VirtualAddress);
	PIMAGE_IMPORT_DESCRIPTOR pImportDirectory = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)g_pFileBuffer + Foa);
	PIMAGE_IMPORT_DESCRIPTOR pTempImportTable = pImportDirectory;
	//打印导入表内容 

	size_t size_ImportTable = pDirectoryHeader->Size;  //结束标志为sizeof(IMAGE_IMPORT_DESCRIPTOR)大小的全0
	size_t num_ImportTable = size_ImportTable/sizeof(IMAGE_IMPORT_DESCRIPTOR) - 1;
	
	//对所有调用的dll进行遍历
	PVOID buffer = malloc(1024*1024);
	memset(buffer,0,1024*1024);		//必须初始化否则”吨吨吨“
	TCHAR* row = (TCHAR*)buffer;
	for (DWORD i = 0;i <= num_ImportTable/*!isAllZero((PVOID)pTempImportTable, (PVOID)((DWORD)pTempImportTable + sizeof(IMAGE_IMPORT_DESCRIPTOR) - 1))*/; pTempImportTable++, i++) {
		wsprintf(row += _tcslen(row),TEXT("%s%s%d%s%s\r\n"),TEXT("======================="),TEXT("第"),i + 1,TEXT("个DLL"),TEXT("======================="));
		//打印基本参数
		DWORD foa_name = RvaToFileOffset(g_pFileBuffer, pTempImportTable->Name);
		TCHAR* name = (TCHAR*)((DWORD)g_pFileBuffer + foa_name);
		DWORD foa_OriginalFirstThunk = RvaToFileOffset(g_pFileBuffer, pTempImportTable->OriginalFirstThunk);
		PDWORD pOriginalFirstThunk = (PDWORD)((DWORD)g_pFileBuffer + foa_OriginalFirstThunk);
		DWORD foa_FirstThunk = RvaToFileOffset(g_pFileBuffer, pTempImportTable->FirstThunk);
		PDWORD pFirstThunk = (PDWORD)((DWORD)g_pFileBuffer + foa_FirstThunk);
		wsprintf(row += _tcslen(row),TEXT("参数:\r\n"));
		wsprintf(row += _tcslen(row),TEXT("%-30s%-30s\r\n"),TEXT("Name:"),name);
		wsprintf(row += _tcslen(row),TEXT("%-30s%-30x\r\n"),TEXT("TimeDateStamp:"),pTempImportTable->TimeDateStamp);
		wsprintf(row += _tcslen(row),TEXT("%-30s%-30x\r\n"),TEXT("ForwarderChain:"),pTempImportTable->ForwarderChain);
		wsprintf(row += _tcslen(row),TEXT("------------------------------------------------------------------------------------\r\n"));
		//打印int和iat表
		wsprintf(row += _tcslen(row),TEXT("%-10s%-60s%-30s\r\n"),TEXT(""),TEXT("INT"),TEXT("IAT"));
		
		for(int j=0;*pOriginalFirstThunk || *pFirstThunk;j++,pOriginalFirstThunk++,pFirstThunk++){
		
			if (*pOriginalFirstThunk & 0x80000000 == 0x80000000) {
				wsprintf(row += _tcslen(row),TEXT("%-10d%-60x%-30p\r\n"),j+1,*pOriginalFirstThunk & 0x0fff,*pFirstThunk);
			}
			else {
				DWORD foa_funtionName = RvaToFileOffset(g_pFileBuffer, *pOriginalFirstThunk);
				PIMAGE_IMPORT_BY_NAME pFuntionName = (PIMAGE_IMPORT_BY_NAME)((DWORD)g_pFileBuffer + foa_funtionName);
				wsprintf(row += _tcslen(row),TEXT("%-10d%-60s%-30p\r\n"),j+1,pFuntionName->Name,*pFirstThunk);
			}
		}
	}
	HWND hEdit = GetDlgItem(g_hDialogImport,IDC_EDIT_IMPORTTABLE);
	SetWindowText(hEdit,(TCHAR*)buffer);
}
