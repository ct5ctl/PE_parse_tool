#include "stdafx.h"
#include "DIALOG_Main.h"


//**************************************************�ص�����_ExportTable����
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
//**************************************************��ʼ����Դ���
void InitResourceDialog(void){
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
	pDirectoryHeader = (PIMAGE_DATA_DIRECTORY)&pOptionHeader->DataDirectory[2];
	if(pDirectoryHeader->Size == 0){
		//�ñ�����
		MessageBox(g_hDialogDirection,TEXT("��Դ������"),0,MB_OK);
		EndDialog(g_hDialogResource,0);
		return;
	}
	
	DWORD rva_ResourceDirectory = pDirectoryHeader->VirtualAddress;
	PIMAGE_RESOURCE_DIRECTORY pResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)((DWORD)pDosHeader + RvaToFoa(pDosHeader,rva_ResourceDirectory));
	//��ӡ��Դ������
	PIMAGE_RESOURCE_DIRECTORY pTempResourceDirectory_1 = pResourceDirectory;
	PIMAGE_RESOURCE_DIRECTORY pTempResourceDirectory_2 = NULL;
	PIMAGE_RESOURCE_DIRECTORY pTempResourceDirectory_3 = NULL;
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pTempResourceDirectoryEntry_1 = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pTempResourceDirectory_1 + 1);
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pTempResourceDirectoryEntry_2 = NULL;
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pTempResourceDirectoryEntry_3 = NULL;
	PIMAGE_RESOURCE_DIR_STRING_U pNameStringDir = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	//��ӡ��Դ������
	PVOID buffer = malloc(1024*1024);
	memset(buffer,0,1024*1024);
	TCHAR* row = (TCHAR*)buffer;
	//��ӡ���͵Ļ�������
	wsprintf(row += _tcslen(row),TEXT("(��һ��)�ܲ���:\r\n"));
	wsprintf(row += _tcslen(row),TEXT("%-30s%-30x\r\n"),TEXT("��Դ����ʱ��(TimeDateStamp):"),pTempResourceDirectory_1->TimeDateStamp);
	wsprintf(row += _tcslen(row),TEXT("%-30s%-30x\r\n"),TEXT("��Դ����(Characteristics):"),pTempResourceDirectory_1->Characteristics);
	wsprintf(row += _tcslen(row),TEXT("%-30s%-30d\r\n"),TEXT("������������������(NumberOfNameEntries):"),pTempResourceDirectory_1->NumberOfNamedEntries);
	wsprintf(row += _tcslen(row),TEXT("%-30s%-30d\r\n"),TEXT("�Ա��������������(NumberOfNameEntries):"),pTempResourceDirectory_1->NumberOfIdEntries);
	wsprintf(row += _tcslen(row),TEXT("%-30s%-30d\r\n"),TEXT("��Դ����ʱ��(NumberOfIdEntries):"),pTempResourceDirectory_1->NumberOfIdEntries);
	for(int i=0;i<pTempResourceDirectory_1->NumberOfIdEntries + pTempResourceDirectory_1->NumberOfNamedEntries;i++,pTempResourceDirectoryEntry_1++){
		wsprintf(row += _tcslen(row),TEXT("%s%s%d%s%s\r\n"),TEXT("======================="),TEXT("��"),i + 1,TEXT("��"),TEXT("======================="));
		//�����1��
		//wsprintf(row += _tcslen(row),TEXT("------------------------------------------------------------------------------------------------\r\n"));
		if(pTempResourceDirectoryEntry_1->NameIsString == 1){			//��һ�ڶ�����ʵ���Բ��ж����λ(��Ϊ1)
			DWORD foa_NameStringDir = RvaToFoa(g_pFileBuffer,pTempResourceDirectoryEntry_1->NameOffset);
			pNameStringDir = (PIMAGE_RESOURCE_DIR_STRING_U)((DWORD)g_pFileBuffer + foa_NameStringDir);
 			wsprintf(row += _tcslen(row),TEXT("%-30s%*s\r\n"),TEXT("����:"),pNameStringDir->Length,pNameStringDir->NameString);
		}else{
			wsprintf(row += _tcslen(row),TEXT("%-30s%x\r\n"),TEXT("����:"),pTempResourceDirectoryEntry_1->Name);
		}
 		//�����2��
		pTempResourceDirectory_2 = (PIMAGE_RESOURCE_DIRECTORY)(pTempResourceDirectoryEntry_1->OffsetToDirectory + (DWORD)pResourceDirectory);
		pTempResourceDirectoryEntry_2 = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pTempResourceDirectory_2 + 1);
		//��ӡ��2��Ļ�������
		wsprintf(row += _tcslen(row),TEXT("(�ڶ���)�����:\r\n"));
		wsprintf(row += _tcslen(row),TEXT("%-30s%-30x\r\n"),TEXT("��Դ����ʱ��(TimeDateStamp):"),pTempResourceDirectory_2->TimeDateStamp);
		wsprintf(row += _tcslen(row),TEXT("%-30s%-30x\r\n"),TEXT("��Դ����(Characteristics):"),pTempResourceDirectory_2->Characteristics);
		wsprintf(row += _tcslen(row),TEXT("%-30s%-30d\r\n"),TEXT("��������������Դ����(NumberOfNameEntries):"),pTempResourceDirectory_2->NumberOfNamedEntries);
		wsprintf(row += _tcslen(row),TEXT("%-30s%-30d\r\n"),TEXT("�Ա����������Դ����(NumberOfNameEntries):"),pTempResourceDirectory_2->NumberOfIdEntries);
		wsprintf(row += _tcslen(row),TEXT("%-30s%-30d\r\n"),TEXT("��Դ����ʱ��(NumberOfIdEntries):"),pTempResourceDirectory_2->NumberOfIdEntries);
		for(int j=0;j<pTempResourceDirectory_2->NumberOfIdEntries + pTempResourceDirectory_2->NumberOfNamedEntries;j++,pTempResourceDirectoryEntry_2++){
			wsprintf(row += _tcslen(row),TEXT("------------------------------------------------------------------------------------------------\r\n"));
			if(/*pTempResourceDirectoryEntry_2->NameIsString == 1*/pTempResourceDirectoryEntry_2->Name & 0x80000000 == 0x80000000){			//��һ�ڶ�����ʵ���Բ��ж����λ(��Ϊ1)
				DWORD foa_NameStringDir_2 = RvaToFoa(g_pFileBuffer,pTempResourceDirectoryEntry_2->NameOffset);
				pNameStringDir = (PIMAGE_RESOURCE_DIR_STRING_U)((DWORD)g_pFileBuffer + foa_NameStringDir_2);
				wsprintf(row += _tcslen(row),TEXT("%-30s%*s\r\n"),TEXT("��Դ��:"),pNameStringDir->Length,pNameStringDir->NameString);
			}else{
				wsprintf(row += _tcslen(row),TEXT("%-30s%x\r\n"),TEXT("��Դ���:"),pTempResourceDirectoryEntry_2->Name);
			}
			//�����3��
			pTempResourceDirectory_3 = (PIMAGE_RESOURCE_DIRECTORY)(pTempResourceDirectoryEntry_2->OffsetToDirectory + (DWORD)pResourceDirectory);
			pTempResourceDirectoryEntry_3 = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pTempResourceDirectory_3 + 1);
			//��ӡ���������
			wsprintf(row += _tcslen(row),TEXT("(������)��Դ����:\r\n"));
			wsprintf(row += _tcslen(row),TEXT("%-30s%-30x\r\n"),TEXT("��Դ����ʱ��(TimeDateStamp):"),pTempResourceDirectory_3->TimeDateStamp);
			wsprintf(row += _tcslen(row),TEXT("%-30s%-30x\r\n"),TEXT("��Դ����(Characteristics):"),pTempResourceDirectory_3->Characteristics);
			wsprintf(row += _tcslen(row),TEXT("%-30s%-30d\r\n"),TEXT("�����������Ĵ���ҳ����(NumberOfNameEntries):"),pTempResourceDirectory_3->NumberOfNamedEntries);
			wsprintf(row += _tcslen(row),TEXT("%-30s%-30d\r\n"),TEXT("�Ա�������Ĵ���ҳ����(NumberOfNameEntries):"),pTempResourceDirectory_3->NumberOfIdEntries);
			wsprintf(row += _tcslen(row),TEXT("%-30s%-30d\r\n"),TEXT("��Դ����ʱ��(NumberOfIdEntries):"),pTempResourceDirectory_3->NumberOfIdEntries);
			wsprintf(row += _tcslen(row),TEXT("%-30s%-30d\r\n"),TEXT("������:"),pTempResourceDirectory_3->NumberOfIdEntries + pTempResourceDirectory_3->NumberOfNamedEntries);

			for(int j=0;j<pTempResourceDirectory_3->NumberOfIdEntries + pTempResourceDirectory_3->NumberOfNamedEntries;j++,pTempResourceDirectoryEntry_3++){
				if(pTempResourceDirectoryEntry_3->NameIsString == 1){			//��һ�ڶ�����ʵ���Բ��ж����λ(��Ϊ1)
					DWORD foa_NameStringDir_3 = RvaToFoa(g_pFileBuffer,pTempResourceDirectoryEntry_3->NameOffset);
					pNameStringDir = (PIMAGE_RESOURCE_DIR_STRING_U)((DWORD)g_pFileBuffer + foa_NameStringDir_3);
					wsprintf(row += _tcslen(row),TEXT("%-30s%*s\r\n"),TEXT("����ҳ��:"),pNameStringDir->Length,pNameStringDir->NameString);
				}else{
					wsprintf(row += _tcslen(row),TEXT("%-30s%x\r\n"),TEXT("����ҳ��:"),pTempResourceDirectoryEntry_3->Name);
				}
				if(pTempResourceDirectoryEntry_3->DataIsDirectory){
					MessageBox(0,TEXT("�е�4��,error"),0,0);
					return;
				}else{
					pDataDirectory = (PIMAGE_DATA_DIRECTORY)(pTempResourceDirectoryEntry_3->OffsetToData + (DWORD)pResourceDirectory);
					wsprintf(row += _tcslen(row),TEXT("%-30s%-30x%-30s%-30x\r\n"),TEXT("����ҳ��ַ(RVA):"),pDataDirectory->VirtualAddress,TEXT("����ҳ��С:"),pDataDirectory->Size);
				}
			}
		}
	}
	HWND hEdit = GetDlgItem(g_hDialogResource,IDC_EDIT_RESOURCETABLE);
	SetWindowText(hEdit,(TCHAR*)buffer);
}
