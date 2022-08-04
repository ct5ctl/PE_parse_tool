#include "stdafx.h"
#include "DIALOG_Main.h"

//**************************************************回调函数_BaseReloc窗口
BOOL CALLBACK BaseRelocTableDialogProc(									
						  HWND hwndDlg,  // handle to dialog box			
						  UINT uMsg,     // message			
						  WPARAM wParam, // first message parameter			
						  LPARAM lParam  // second message parameter			
						  ){
	g_hDialogBaseReloc = hwndDlg;
	switch(uMsg){
	case WM_INITDIALOG:
		InitBaseRelocDialog();
		return TRUE;
	case WM_CLOSE:
		EndDialog(g_hDialogBaseReloc,0);
		return TRUE;		
	}
	return FALSE;
}
//**************************************************初始化导入表
void InitBaseRelocDialog(void){
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
	pDirectoryHeader = (PIMAGE_DATA_DIRECTORY)&pOptionHeader->DataDirectory[5];
	if(pDirectoryHeader->Size == 0){
		//该表不存在
		MessageBox(g_hDialogDirection,TEXT("重定位表不存在"),0,MB_OK);
		EndDialog(g_hDialogBaseReloc,0);
		return;
	}
	
	//使指针指向重定位表
	DWORD Foa_relocationTable = RvaToFileOffset(g_pFileBuffer, pDirectoryHeader->VirtualAddress);
	PIMAGE_BASE_RELOCATION pRelocationTable = (PIMAGE_BASE_RELOCATION)((DWORD)g_pFileBuffer + Foa_relocationTable);
	//打印重定位表数据
	PVOID buffer = malloc(1024*1024);
	memset(buffer,0,1024*1024);		//必须初始化否则”吨吨吨“
 	TCHAR* row = (TCHAR*)buffer;
	for (int i = 0; pRelocationTable->SizeOfBlock != 0 && pRelocationTable->VirtualAddress != 0; i++, pRelocationTable = (PIMAGE_BASE_RELOCATION)((DWORD)pRelocationTable + pRelocationTable->SizeOfBlock)) {
		wsprintf(row += _tcslen(row),TEXT("%s%s%d%s%s\r\n"),TEXT("======================="),TEXT("第"),i + 1,TEXT("号内存页"),TEXT("======================="));
		wsprintf(row += _tcslen(row),TEXT("%-30s%-30p\r\n"),TEXT("VirtualAddress:"),pRelocationTable->VirtualAddress);
		wsprintf(row += _tcslen(row),TEXT("%-30s%-30p\r\n"),TEXT("SizeOfBlock:"),pRelocationTable->SizeOfBlock);
		DWORD num = (pRelocationTable->SizeOfBlock - 8) / 2;
		wsprintf(row += _tcslen(row),TEXT("%-30s%-30d\r\n"),TEXT("当前页的具体项数量:"),num);
		wsprintf(row += _tcslen(row),TEXT("具体项:\r\n"));
		PWORD pData = (PWORD)((DWORD)pRelocationTable + 8);
		DWORD num_needmod = 0;
		for (DWORD k = 0; k < num; k++, pData++) {
			if (*pData & 0xF000 == 0x3000) {
				wsprintf(row += _tcslen(row),TEXT("%s%d%s%30x\r\n"),TEXT("第"),k+1,TEXT("项偏移地址(需要修改):"),*pData & 0x0fff);
				num_needmod++;
			}else{
				wsprintf(row += _tcslen(row),TEXT("%s%d%s%30x\r\n"),TEXT("第"),k+1,TEXT("项偏移地址:"),*pData & 0x0fff);
			}
		}
		wsprintf(row += _tcslen(row),TEXT("%s%d%s\r\n"),TEXT("共"),num_needmod,TEXT("项需修改"));
	}
	HWND hEdit = GetDlgItem(g_hDialogBaseReloc,IDC_EDIT_BASERELOC);
	SetWindowText(hEdit,(TCHAR*)buffer);
}



