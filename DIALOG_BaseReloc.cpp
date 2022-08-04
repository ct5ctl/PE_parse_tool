#include "stdafx.h"
#include "DIALOG_Main.h"

//**************************************************�ص�����_BaseReloc����
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
//**************************************************��ʼ�������
void InitBaseRelocDialog(void){
	// ��ʼ��PEͷ���ṹ��
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
		//�ñ�����
		MessageBox(g_hDialogDirection,TEXT("�ض�λ������"),0,MB_OK);
		EndDialog(g_hDialogBaseReloc,0);
		return;
	}
	
	//ʹָ��ָ���ض�λ��
	DWORD Foa_relocationTable = RvaToFileOffset(g_pFileBuffer, pDirectoryHeader->VirtualAddress);
	PIMAGE_BASE_RELOCATION pRelocationTable = (PIMAGE_BASE_RELOCATION)((DWORD)g_pFileBuffer + Foa_relocationTable);
	//��ӡ�ض�λ������
	PVOID buffer = malloc(1024*1024);
	memset(buffer,0,1024*1024);		//�����ʼ�����򡱶ֶֶ֡�
 	TCHAR* row = (TCHAR*)buffer;
	for (int i = 0; pRelocationTable->SizeOfBlock != 0 && pRelocationTable->VirtualAddress != 0; i++, pRelocationTable = (PIMAGE_BASE_RELOCATION)((DWORD)pRelocationTable + pRelocationTable->SizeOfBlock)) {
		wsprintf(row += _tcslen(row),TEXT("%s%s%d%s%s\r\n"),TEXT("======================="),TEXT("��"),i + 1,TEXT("���ڴ�ҳ"),TEXT("======================="));
		wsprintf(row += _tcslen(row),TEXT("%-30s%-30p\r\n"),TEXT("VirtualAddress:"),pRelocationTable->VirtualAddress);
		wsprintf(row += _tcslen(row),TEXT("%-30s%-30p\r\n"),TEXT("SizeOfBlock:"),pRelocationTable->SizeOfBlock);
		DWORD num = (pRelocationTable->SizeOfBlock - 8) / 2;
		wsprintf(row += _tcslen(row),TEXT("%-30s%-30d\r\n"),TEXT("��ǰҳ�ľ���������:"),num);
		wsprintf(row += _tcslen(row),TEXT("������:\r\n"));
		PWORD pData = (PWORD)((DWORD)pRelocationTable + 8);
		DWORD num_needmod = 0;
		for (DWORD k = 0; k < num; k++, pData++) {
			if (*pData & 0xF000 == 0x3000) {
				wsprintf(row += _tcslen(row),TEXT("%s%d%s%30x\r\n"),TEXT("��"),k+1,TEXT("��ƫ�Ƶ�ַ(��Ҫ�޸�):"),*pData & 0x0fff);
				num_needmod++;
			}else{
				wsprintf(row += _tcslen(row),TEXT("%s%d%s%30x\r\n"),TEXT("��"),k+1,TEXT("��ƫ�Ƶ�ַ:"),*pData & 0x0fff);
			}
		}
		wsprintf(row += _tcslen(row),TEXT("%s%d%s\r\n"),TEXT("��"),num_needmod,TEXT("�����޸�"));
	}
	HWND hEdit = GetDlgItem(g_hDialogBaseReloc,IDC_EDIT_BASERELOC);
	SetWindowText(hEdit,(TCHAR*)buffer);
}



