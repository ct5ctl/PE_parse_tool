#include "stdafx.h"
#include "DIALOG_Main.h"


//**************************************************回调函数_ExportTable窗口
BOOL CALLBACK ExportTableDialogProc(									
						  HWND hwndDlg,  // handle to dialog box			
						  UINT uMsg,     // message			
						  WPARAM wParam, // first message parameter			
						  LPARAM lParam  // second message parameter			
						  ){
	g_hDialogExport = hwndDlg;
	switch(uMsg){
	case WM_INITDIALOG:
		InitExportDialog();
		return TRUE;
	case WM_CLOSE:
		EndDialog(g_hDialogExport,0);
		return TRUE;		
	}
	return FALSE;
}
//**************************************************初始化导出表
void InitExportDialog(void){
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
	if(pDirectoryHeader->Size == 0){
		//该表不存在
		MessageBox(g_hDialogDirection,TEXT("导出表不存在"),0,MB_OK);
		EndDialog(g_hDialogExport,0);
		return;
	}

	DWORD Foa = RvaToFileOffset(g_pFileBuffer, pDirectoryHeader->VirtualAddress);
	PIMAGE_EXPORT_DIRECTORY pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((DWORD)g_pFileBuffer + Foa);
	//打印导出表内容 
	
	
	PVOID buffer = malloc(1024*1024);
	memset(buffer,0,sizeof(TCHAR)*1024*1024);
	TCHAR* row = (TCHAR*)buffer;
// 	wsprintf(row,TEXT("%-30s%-40x\r\n"),TEXT("Name:"),pExportDirectory->Name);
// 	//row += 1024;
// 	row += _tcslen(row);		
// 	wsprintf(row,TEXT("%-30s%-40x\r\n"),TEXT("Base:"),pExportDirectory->Base);
// 	row += _tcslen(row);
	wsprintf(row += _tcslen(row),TEXT("==================================================================================================================\r\n"));
	wsprintf(row += _tcslen(row),TEXT("基本参数:\r\n"));
	PDWORD pName = (PDWORD)(RvaToFileOffset(g_pFileBuffer, pExportDirectory->Name) + (DWORD)pDosHeader);
	wsprintf(row += _tcslen(row), TEXT("%-30s%-30s\r\n"), TEXT("Name:"), pName);		//_tcslen返回的是从指针位置到0之间的字符数，加上该值得到下一行开头位置
	wsprintf(row += _tcslen(row), TEXT("%-30s%-30x\r\n"), TEXT("Base:"), pExportDirectory->Base);
	wsprintf(row += _tcslen(row), TEXT("%-30s%-30x\r\n"), TEXT("NumberOfFunctions:"), pExportDirectory->NumberOfFunctions);
	wsprintf(row += _tcslen(row), TEXT("%-30s%-30x\r\n"), TEXT("NumberOfNames:"), pExportDirectory->NumberOfNames);
	//打印三张表
	wsprintf(row += _tcslen(row),TEXT("==================================================================================================================\r\n"));
	wsprintf(row += _tcslen(row),TEXT("导出函数表:\r\n"));
	PDWORD pAddressTable = (PDWORD)(RvaToFileOffset(g_pFileBuffer, pExportDirectory->AddressOfFunctions) + (DWORD)pDosHeader);
	PDWORD pNameTable = (PDWORD)(RvaToFileOffset(g_pFileBuffer, pExportDirectory->AddressOfNames) + (DWORD)pDosHeader);
	PWORD pOrdinalTable = (PWORD)(RvaToFileOffset(g_pFileBuffer, pExportDirectory->AddressOfNameOrdinals) + (DWORD)pDosHeader);
	wsprintf(row += _tcslen(row), TEXT("%-30s%-30s%-30s\r\n"), TEXT("Adress"), TEXT("Ordinal"), TEXT("Name"));
	for (DWORD index=0;index<pExportDirectory->NumberOfFunctions;index++)
	{
		DWORD dwAddress = *(pAddressTable + index);
		WORD Ordinal = *(pOrdinalTable + index);

		TCHAR* pNameT = (TCHAR*)(RvaToFileOffset(g_pFileBuffer,*(pNameTable + index)) + (DWORD)pDosHeader);
		wsprintf(row += _tcslen(row),TEXT("%-30x%-30x%-30s\r\n"),dwAddress,Ordinal,pNameT);
	}
	HWND hEdit = GetDlgItem(g_hDialogExport,IDC_EDIT_EXPORTTABLE);
	SetWindowText(hEdit,(TCHAR*)buffer);
}