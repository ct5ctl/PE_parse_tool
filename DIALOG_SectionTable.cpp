#include "stdafx.h"
#include "DIALOG_Main.h"


//**************************************************�ص�����_SectionTable����
BOOL CALLBACK SectionTableDialogProc(									
						  HWND hwndDlg,  // handle to dialog box			
						  UINT uMsg,     // message			
						  WPARAM wParam, // first message parameter			
						  LPARAM lParam  // second message parameter			
						  ){
	g_hDialogSectionTable = hwndDlg;
	switch(uMsg){
	case WM_INITDIALOG:
		InitSectionTableDialog();
		return TRUE;
	case WM_CLOSE:
		EndDialog(g_hDialogSectionTable,0);
		return TRUE;
	}
	return FALSE;
}
//**************************************************��ʼ���ڱ���
void InitSectionTableDialog(void)
{
	//����SectionList����������IDC_LIST_SECTIONTABLE
	HWND hListSectionTable = GetDlgItem(g_hDialogSectionTable,IDC_LIST_SECTIONTABLE);
	LV_COLUMN lv;
	memset(&lv,0,sizeof(LV_COLUMN));
	SendMessage(hListSectionTable,LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_FULLROWSELECT,LVS_EX_FULLROWSELECT);
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	
	lv.pszText = TEXT("����");
	lv.iSubItem = 0;
	lv.cx = 150;
	ListView_InsertColumn(hListSectionTable,0,&lv);
	lv.pszText = TEXT("�ļ�ƫ��");
	lv.iSubItem = 1;
	lv.cx = 120;
	ListView_InsertColumn(hListSectionTable,1,&lv);
	lv.pszText = TEXT("�ļ���С");
	lv.iSubItem = 2;
	lv.cx = 120;
	ListView_InsertColumn(hListSectionTable,2,&lv);
	lv.pszText = TEXT("�ڴ�ƫ��");
	lv.iSubItem = 3;
	lv.cx = 120;
	ListView_InsertColumn(hListSectionTable,3,&lv);
	lv.pszText = TEXT("�ڴ��С");
	lv.iSubItem = 4;
	lv.cx = 120;
	ListView_InsertColumn(hListSectionTable,4,&lv);
	lv.pszText = TEXT("�ڱ�����");
	lv.iSubItem = 5;
	lv.cx = 120;
	ListView_InsertColumn(hListSectionTable,5,&lv);

	//�������Ϣ
	// ��ʼ��PEͷ���ṹ��
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	pDosHeader = (PIMAGE_DOS_HEADER)g_pFileBuffer;
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)g_pFileBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	PIMAGE_SECTION_HEADER pTempSectionHeader = pSectionHeader;
	LV_ITEM lvItem;
	memset(&lvItem,0,sizeof(LV_ITEM));
	lvItem.mask = LVIF_TEXT|LVCFMT_LEFT;
	TCHAR Buffer[16];
	for(int row=0;row<pPEHeader->NumberOfSections;row++,pTempSectionHeader++)
	{
		lvItem.iItem = row;
		char nameBuffer[16] = {0};
		wsprintf(nameBuffer,TEXT("%s"),pTempSectionHeader->Name);
// 		if (sizeof(TCHAR) == 2)
//         {
//             // UNICODE�汾��Ҫ�� ���ֽ�ת�ɿ��ֽ�
//             memset(Buffer,0,sizeof(TCHAR)* 16);
//             MultiByteToWideChar(CP_ACP, 0,nameBuffer,-1,(unsigned short *)Buffer,IMAGE_SIZEOF_SHORT_NAME);
//             lvItem.pszText = Buffer;
//         }
//         else
//         {
//             // ANSI�汾����ֱ��ʹ��խ�ַ�
//             lvItem.pszText = nameBuffer;
//         }
		lvItem.pszText = nameBuffer;
		lvItem.iSubItem = 0;
		ListView_InsertItem(hListSectionTable,&lvItem);
		
		lvItem.iSubItem = 1;
		wsprintf(Buffer,TEXT("%p"),pTempSectionHeader->PointerToRawData);
		lvItem.pszText = Buffer;
		ListView_SetItem(hListSectionTable,&lvItem);

		lvItem.iSubItem = 2;
		wsprintf(lvItem.pszText,TEXT("%p"),pTempSectionHeader->SizeOfRawData);
		ListView_SetItem(hListSectionTable,&lvItem);

		lvItem.iSubItem = 3;
		wsprintf(lvItem.pszText,TEXT("%p"),pTempSectionHeader->VirtualAddress);
		ListView_SetItem(hListSectionTable,&lvItem);

		lvItem.iSubItem = 4;
		wsprintf(lvItem.pszText,TEXT("%p"),pTempSectionHeader->Misc.VirtualSize);
		ListView_SetItem(hListSectionTable,&lvItem);

		lvItem.iSubItem = 5;
		wsprintf(lvItem.pszText,TEXT("%p"),pTempSectionHeader->Characteristics);
		ListView_SetItem(hListSectionTable,&lvItem);
		
	}
}

