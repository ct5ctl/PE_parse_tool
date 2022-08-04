#include "stdafx.h"
#include "DIALOG_Main.h"

//**************************************************�ص�����_�󶨵������
BOOL CALLBACK BoundImportDialogProc(									
						  HWND hwndDlg,  // handle to dialog box			
						  UINT uMsg,     // message			
						  WPARAM wParam, // first message parameter			
						  LPARAM lParam  // second message parameter			
						  ){
	g_hDialogBoundImport = hwndDlg;
	switch(uMsg){
	case WM_INITDIALOG:
		InitBoundImportDialog();
		return TRUE;
	case WM_CLOSE:
		EndDialog(g_hDialogBoundImport,0);
		return TRUE;		
	}
	return FALSE;
}
//**************************************************��ʼ���󶨵������
void InitBoundImportDialog(void){
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
	pDirectoryHeader = (PIMAGE_DATA_DIRECTORY)&pOptionHeader->DataDirectory[11];
	if(pDirectoryHeader->Size == 0){
		//�ñ�����
		MessageBox(g_hDialogDirection,TEXT("�󶨵��������"),0,MB_OK);
		EndDialog(g_hDialogBoundImport,0);
		return;
	}
	//��ӡ�󶨵�������� 
	PVOID buffer = malloc(1024*1024);
	memset(buffer,0,1024*1024);		//�����ʼ�����򡱶ֶֶ֡�
	TCHAR* row = (TCHAR*)buffer;
	PIMAGE_BOUND_IMPORT_DESCRIPTOR pBoundImportDescriptor = NULL;
	PIMAGE_BOUND_IMPORT_DESCRIPTOR pFirstBoundImportDescriptor = NULL;
	IMAGE_BOUND_IMPORT_DESCRIPTOR Descriptor_null = {0};
	PIMAGE_BOUND_FORWARDER_REF pBoundImportRef = NULL;
	DWORD Foa = RvaToFileOffset(g_pFileBuffer, pDirectoryHeader->VirtualAddress);
	pFirstBoundImportDescriptor = (PIMAGE_BOUND_IMPORT_DESCRIPTOR)((DWORD)g_pFileBuffer + Foa);
	pBoundImportDescriptor = pFirstBoundImportDescriptor;
	for(int index=0;memcmp(pBoundImportDescriptor,&Descriptor_null,sizeof(IMAGE_BOUND_IMPORT_DESCRIPTOR)) != 0		//�󶨵���������־ΪPIMAGE_BOUND_IMPORT_DESCRIPTOR��С��ȫ0
		;index++,pBoundImportDescriptor += (pBoundImportDescriptor->NumberOfModuleForwarderRefs + 1)){
	
		wsprintf(row += _tcslen(row),TEXT("%s%s%d%s%s\r\n"),TEXT("======================="),TEXT("��"),index + 1,TEXT("��DESCRIPTOR"),TEXT("======================="));
		TCHAR* pName = (TCHAR*)((DWORD)pFirstBoundImportDescriptor + pBoundImportDescriptor->OffsetModuleName);
		wsprintf(row += _tcslen(row),TEXT("%-30s%-30s\r\n"),TEXT("DLL��:"),pName);
		wsprintf(row += _tcslen(row),TEXT("%-30s%-30x\r\n"),TEXT("TimeDateStamp:"),pBoundImportDescriptor->TimeDateStamp);
		wsprintf(row += _tcslen(row),TEXT("%-30s%-30x\r\n"),TEXT("���DLL��ص�DLL����(NumberOfModuleForwarderRefs):"),pBoundImportDescriptor->NumberOfModuleForwarderRefs);
		//�������dll�����ӡ
		if(pBoundImportDescriptor->NumberOfModuleForwarderRefs){
			wsprintf(row += _tcslen(row),TEXT("��ص�DLL:\r\n"));
			pBoundImportRef = (PIMAGE_BOUND_FORWARDER_REF)(pBoundImportDescriptor + 1);
			for(int i=0;i<pBoundImportDescriptor->NumberOfModuleForwarderRefs;i++,pBoundImportRef++){
				TCHAR* pNameRef = (TCHAR*)((DWORD)pFirstBoundImportDescriptor + pBoundImportRef->OffsetModuleName);
				wsprintf(row += _tcslen(row),TEXT("%s%s%d%s%s\r\n"),TEXT("----------"),TEXT("��"),i + 1,TEXT("�����DLL"),TEXT("----------"));
				wsprintf(row += _tcslen(row),TEXT("%-30s%-30s\r\n"),TEXT("DLL��:"),pNameRef);
				wsprintf(row += _tcslen(row),TEXT("%-30s%-30x\r\n"),TEXT("TimeDateStamp:"),pBoundImportRef->TimeDateStamp);
			}
		}	
	}
	HWND hEdit = GetDlgItem(g_hDialogBoundImport,IDC_EDIT_BOUNDIMPORT);
	SetWindowText(hEdit,(TCHAR*)buffer);
}