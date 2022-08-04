#include "stdafx.h"
#include "DIALOG_Main.h"


//**************************************************回调函数_ExportTable窗口
BOOL CALLBACK ResourceTableDialogProc(									
						  HWND hwndDlg,  // handle to dialog box			
						  UINT uMsg,     // message			
						  WPARAM wParam, // first message parameter			
						  LPARAM lParam  // second message parameter			
						  ){
	g_hDialogResource = hwndDlg;
	switch(uMsg){
	case WM_INITDIALOG:
		InitResourceDialog();
		return TRUE;
	case WM_CLOSE:
		EndDialog(g_hDialogResource,0);
		return TRUE;		
	}
	return FALSE;
}
//**************************************************初始化资源表表
void InitResourceDialog(void){
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
	pDirectoryHeader = (PIMAGE_DATA_DIRECTORY)&pOptionHeader->DataDirectory[2];
	if(pDirectoryHeader->Size == 0){
		//该表不存在
		MessageBox(g_hDialogDirection,TEXT("资源表不存在"),0,MB_OK);
		EndDialog(g_hDialogResource,0);
		return;
	}
	
	DWORD rva_ResourceDirectory = pDirectoryHeader->VirtualAddress;
	PIMAGE_RESOURCE_DIRECTORY pResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)((DWORD)pDosHeader + RvaToFoa(pDosHeader,rva_ResourceDirectory));
	//打印资源表内容
	PIMAGE_RESOURCE_DIRECTORY pTempResourceDirectory_1 = pResourceDirectory;
	PIMAGE_RESOURCE_DIRECTORY pTempResourceDirectory_2 = NULL;
	PIMAGE_RESOURCE_DIRECTORY pTempResourceDirectory_3 = NULL;
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pTempResourceDirectoryEntry_1 = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pTempResourceDirectory_1 + 1);
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pTempResourceDirectoryEntry_2 = NULL;
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pTempResourceDirectoryEntry_3 = NULL;
	PIMAGE_RESOURCE_DIR_STRING_U pNameStringDir = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	//打印资源表内容
	PVOID buffer = malloc(1024*1024);
	memset(buffer,0,1024*1024);
	TCHAR* row = (TCHAR*)buffer;
	//打印类型的基本参数
	wsprintf(row += _tcslen(row),TEXT("(第一层)总参数:\r\n"));
	wsprintf(row += _tcslen(row),TEXT("%-30s%-30x\r\n"),TEXT("资源创建时间(TimeDateStamp):"),pTempResourceDirectory_1->TimeDateStamp);
	wsprintf(row += _tcslen(row),TEXT("%-30s%-30x\r\n"),TEXT("资源属性(Characteristics):"),pTempResourceDirectory_1->Characteristics);
	wsprintf(row += _tcslen(row),TEXT("%-30s%-30d\r\n"),TEXT("以名称命名的类数量(NumberOfNameEntries):"),pTempResourceDirectory_1->NumberOfNamedEntries);
	wsprintf(row += _tcslen(row),TEXT("%-30s%-30d\r\n"),TEXT("以编号命名的类数量(NumberOfNameEntries):"),pTempResourceDirectory_1->NumberOfIdEntries);
	wsprintf(row += _tcslen(row),TEXT("%-30s%-30d\r\n"),TEXT("资源创建时间(NumberOfIdEntries):"),pTempResourceDirectory_1->NumberOfIdEntries);
	for(int i=0;i<pTempResourceDirectory_1->NumberOfIdEntries + pTempResourceDirectory_1->NumberOfNamedEntries;i++,pTempResourceDirectoryEntry_1++){
		wsprintf(row += _tcslen(row),TEXT("%s%s%d%s%s\r\n"),TEXT("======================="),TEXT("第"),i + 1,TEXT("类"),TEXT("======================="));
		//进入第1层
		//wsprintf(row += _tcslen(row),TEXT("------------------------------------------------------------------------------------------------\r\n"));
		if(pTempResourceDirectoryEntry_1->NameIsString == 1){			//第一第二层其实可以不判断最高位(必为1)
			DWORD foa_NameStringDir = RvaToFoa(g_pFileBuffer,pTempResourceDirectoryEntry_1->NameOffset);
			pNameStringDir = (PIMAGE_RESOURCE_DIR_STRING_U)((DWORD)g_pFileBuffer + foa_NameStringDir);
 			wsprintf(row += _tcslen(row),TEXT("%-30s%*s\r\n"),TEXT("类名:"),pNameStringDir->Length,pNameStringDir->NameString);
		}else{
			wsprintf(row += _tcslen(row),TEXT("%-30s%x\r\n"),TEXT("类编号:"),pTempResourceDirectoryEntry_1->Name);
		}
 		//进入第2层
		pTempResourceDirectory_2 = (PIMAGE_RESOURCE_DIRECTORY)(pTempResourceDirectoryEntry_1->OffsetToDirectory + (DWORD)pResourceDirectory);
		pTempResourceDirectoryEntry_2 = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pTempResourceDirectory_2 + 1);
		//打印第2层的基本参数
		wsprintf(row += _tcslen(row),TEXT("(第二层)类参数:\r\n"));
		wsprintf(row += _tcslen(row),TEXT("%-30s%-30x\r\n"),TEXT("资源创建时间(TimeDateStamp):"),pTempResourceDirectory_2->TimeDateStamp);
		wsprintf(row += _tcslen(row),TEXT("%-30s%-30x\r\n"),TEXT("资源属性(Characteristics):"),pTempResourceDirectory_2->Characteristics);
		wsprintf(row += _tcslen(row),TEXT("%-30s%-30d\r\n"),TEXT("以名称命名的资源数量(NumberOfNameEntries):"),pTempResourceDirectory_2->NumberOfNamedEntries);
		wsprintf(row += _tcslen(row),TEXT("%-30s%-30d\r\n"),TEXT("以编号命名的资源数量(NumberOfNameEntries):"),pTempResourceDirectory_2->NumberOfIdEntries);
		wsprintf(row += _tcslen(row),TEXT("%-30s%-30d\r\n"),TEXT("资源创建时间(NumberOfIdEntries):"),pTempResourceDirectory_2->NumberOfIdEntries);
		for(int j=0;j<pTempResourceDirectory_2->NumberOfIdEntries + pTempResourceDirectory_2->NumberOfNamedEntries;j++,pTempResourceDirectoryEntry_2++){
			wsprintf(row += _tcslen(row),TEXT("------------------------------------------------------------------------------------------------\r\n"));
			if(/*pTempResourceDirectoryEntry_2->NameIsString == 1*/pTempResourceDirectoryEntry_2->Name & 0x80000000 == 0x80000000){			//第一第二层其实可以不判断最高位(必为1)
				DWORD foa_NameStringDir_2 = RvaToFoa(g_pFileBuffer,pTempResourceDirectoryEntry_2->NameOffset);
				pNameStringDir = (PIMAGE_RESOURCE_DIR_STRING_U)((DWORD)g_pFileBuffer + foa_NameStringDir_2);
				wsprintf(row += _tcslen(row),TEXT("%-30s%*s\r\n"),TEXT("资源名:"),pNameStringDir->Length,pNameStringDir->NameString);
			}else{
				wsprintf(row += _tcslen(row),TEXT("%-30s%x\r\n"),TEXT("资源编号:"),pTempResourceDirectoryEntry_2->Name);
			}
			//进入第3层
			pTempResourceDirectory_3 = (PIMAGE_RESOURCE_DIRECTORY)(pTempResourceDirectoryEntry_2->OffsetToDirectory + (DWORD)pResourceDirectory);
			pTempResourceDirectoryEntry_3 = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pTempResourceDirectory_3 + 1);
			//打印第三层参数
			wsprintf(row += _tcslen(row),TEXT("(第三层)资源参数:\r\n"));
			wsprintf(row += _tcslen(row),TEXT("%-30s%-30x\r\n"),TEXT("资源创建时间(TimeDateStamp):"),pTempResourceDirectory_3->TimeDateStamp);
			wsprintf(row += _tcslen(row),TEXT("%-30s%-30x\r\n"),TEXT("资源属性(Characteristics):"),pTempResourceDirectory_3->Characteristics);
			wsprintf(row += _tcslen(row),TEXT("%-30s%-30d\r\n"),TEXT("以名称命名的代码页数量(NumberOfNameEntries):"),pTempResourceDirectory_3->NumberOfNamedEntries);
			wsprintf(row += _tcslen(row),TEXT("%-30s%-30d\r\n"),TEXT("以编号命名的代码页数量(NumberOfNameEntries):"),pTempResourceDirectory_3->NumberOfIdEntries);
			wsprintf(row += _tcslen(row),TEXT("%-30s%-30d\r\n"),TEXT("资源创建时间(NumberOfIdEntries):"),pTempResourceDirectory_3->NumberOfIdEntries);
			wsprintf(row += _tcslen(row),TEXT("%-30s%-30d\r\n"),TEXT("总数量:"),pTempResourceDirectory_3->NumberOfIdEntries + pTempResourceDirectory_3->NumberOfNamedEntries);

			for(int j=0;j<pTempResourceDirectory_3->NumberOfIdEntries + pTempResourceDirectory_3->NumberOfNamedEntries;j++,pTempResourceDirectoryEntry_3++){
				if(pTempResourceDirectoryEntry_3->NameIsString == 1){			//第一第二层其实可以不判断最高位(必为1)
					DWORD foa_NameStringDir_3 = RvaToFoa(g_pFileBuffer,pTempResourceDirectoryEntry_3->NameOffset);
					pNameStringDir = (PIMAGE_RESOURCE_DIR_STRING_U)((DWORD)g_pFileBuffer + foa_NameStringDir_3);
					wsprintf(row += _tcslen(row),TEXT("%-30s%*s\r\n"),TEXT("代码页名:"),pNameStringDir->Length,pNameStringDir->NameString);
				}else{
					wsprintf(row += _tcslen(row),TEXT("%-30s%x\r\n"),TEXT("代码页号:"),pTempResourceDirectoryEntry_3->Name);
				}
				if(pTempResourceDirectoryEntry_3->DataIsDirectory){
					MessageBox(0,TEXT("有第4层,error"),0,0);
					return;
				}else{
					pDataDirectory = (PIMAGE_DATA_DIRECTORY)(pTempResourceDirectoryEntry_3->OffsetToData + (DWORD)pResourceDirectory);
					wsprintf(row += _tcslen(row),TEXT("%-30s%-30x%-30s%-30x\r\n"),TEXT("代码页地址(RVA):"),pDataDirectory->VirtualAddress,TEXT("代码页大小:"),pDataDirectory->Size);
				}
			}
		}
	}
	HWND hEdit = GetDlgItem(g_hDialogResource,IDC_EDIT_RESOURCETABLE);
	SetWindowText(hEdit,(TCHAR*)buffer);
}
