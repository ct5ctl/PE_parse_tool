#include "stdafx.h"
#include "DIALOG_Main.h"

//**************************************************����һ���ļ�ѡ�񴰿ڣ�������pe����
void CreatePeDialog(void){
	OPENFILENAME stOpenFile = {0};
	TCHAR szPeFileExt[100] = "*.exe;*.dll;*.sys;*.scr;*.drv";
	TCHAR szFileName[256] = {0};
	//�����ļ�ѡ�񴰿ڣ�����ȡ�û�ѡ����ļ���Ϣ
	memset(&stOpenFile,0,sizeof(OPENFILENAME));
	stOpenFile.lStructSize = sizeof(OPENFILENAME);
	stOpenFile.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    stOpenFile.hwndOwner = g_hDialogMain;
    stOpenFile.lpstrFilter = szPeFileExt;
    stOpenFile.lpstrFile = szFileName;
    stOpenFile.nMaxFile = MAX_PATH;
	if(!GetOpenFileName(&stOpenFile)){
		MessageBox(0,TEXT("δѡ���ļ�"),0,MB_OK);
		return;
	}
	//���ļ����ڴ�
	g_pFileBuffer = NULL;
	size_t g_FileSize = ReadPEFile(szFileName,&g_pFileBuffer);
	if(!g_FileSize){
		MessageBox(0,TEXT("��ָ���ļ�!"),0,MB_OK);
		return;
	}
	//����Pe��Ϣ����
	DialogBox(g_hDlg,(LPCTSTR)IDD_DIALOG_PE,g_hDialogMain,PeDialogProc);
	return;
	//MessageBox(0,szFileName,0,0);
}
//**************************************************�ص�����_pe����
BOOL CALLBACK PeDialogProc(									
						  HWND hwndDlg,  // handle to dialog box			
						  UINT uMsg,     // message			
						  WPARAM wParam, // first message parameter			
						  LPARAM lParam  // second message parameter			
						  ){
	g_hDialogPe = hwndDlg;
	switch(uMsg){
	case WM_INITDIALOG:
		InitPeDialog();
		return TRUE;
	case WM_CLOSE:
		free(g_pFileBuffer);
		g_pFileBuffer = NULL;
		EndDialog(g_hDialogPe,0);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDC_BUTTON_PE_CLOSE:
			free(g_pFileBuffer);
			g_pFileBuffer = NULL;
			EndDialog(g_hDialogPe,0);
			return TRUE;
		case IDC_BUTTON_PE_SECTIONS:
			DialogBox(g_hDlg,(LPCTSTR)IDD_DIALOG_SECTIONTABLE,g_hDialogPe,SectionTableDialogProc);
			return TRUE;
		case IDC_BUTTON_PE_DIRECTION:
			DialogBox(g_hDlg,(LPCTSTR)IDD_DIALOG_DIRCTION,g_hDialogPe,DirectionDialogProc);
			return TRUE;
 		}
	}		
	return FALSE;
}
//**************************************************��ʼ��pe���ڣ���ѡ���ļ���pe��Ϣ������Ӧλ��
void InitPeDialog(void){
	// ��ʼ��PEͷ���ṹ��
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	// ��ʼ��IMAGE_BUFFERָ��(temparay)
	LPVOID pTempImagebuffer = NULL;
	//�ж�exe->�ڴ��Ƿ�ɹ�
	if (!g_pFileBuffer)
	{
		MessageBox(g_hDialogPe,TEXT("���ļ�ʧ��!"),0,MB_OK);
		EndDialog(g_hDialogPe,0);
		return;
	}
	// �ж��Ƿ��ǿ�ִ���ļ�
	if (*((PWORD)g_pFileBuffer) != IMAGE_DOS_SIGNATURE)  // IMAGE_DOS_SIGNATURE��4�ֽڣ���pFileBufferǿ������ת��Ϊ4�ֽ�ָ�����ͣ�PWORD��
	{
		MessageBox(g_hDialogPe,TEXT("����MZ��־,��ָ���ļ�!"),0,MB_OK);
		EndDialog(g_hDialogPe,0);
		return;
	}
	//ǿ�ƽṹ������ת��pDosHeader
	pDosHeader = (PIMAGE_DOS_HEADER)g_pFileBuffer;
	//�ж��Ƿ���PE��־       
	if (*((PDWORD)((DWORD)g_pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE) 
	{																			 
		MessageBox(g_hDialogPe,TEXT("��Ч��PE��־!"),0,MB_OK);
		EndDialog(g_hDialogPe,0);
		return;
	}
	// ǿ�ƽṹ������ת��
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)g_pFileBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	//��ѡ���ļ���pe��Ϣ������Ӧλ��
	TCHAR Buffer[100] = {0};
	//��ڵ�
	wsprintf(Buffer,TEXT("%p"),pOptionHeader->AddressOfEntryPoint);
	SetEditContent(IDC_EDIT_1,Buffer);
	//�����ַ
	wsprintf(Buffer,TEXT("%p"),pOptionHeader->ImageBase);
	SetEditContent(IDC_EDIT_2,Buffer);
	//�����С
	wsprintf(Buffer,TEXT("%p"),pOptionHeader->SizeOfImage);
	SetEditContent(IDC_EDIT_3,Buffer);
	//�����ַ
	wsprintf(Buffer,TEXT("%p"),pOptionHeader->BaseOfCode);
	SetEditContent(IDC_EDIT_4,Buffer);
	//���ݻ�ַ
	wsprintf(Buffer,TEXT("%p"),pOptionHeader->BaseOfData);
	SetEditContent(IDC_EDIT_5,Buffer);
	//�ڴ����
	wsprintf(Buffer,TEXT("%p"),pOptionHeader->SectionAlignment);
	SetEditContent(IDC_EDIT_6,Buffer);
	//�ļ�����
	wsprintf(Buffer,TEXT("%p"),pOptionHeader->FileAlignment);
	SetEditContent(IDC_EDIT_7,Buffer);
	//��־��
	wsprintf(Buffer,TEXT("%p"),pOptionHeader->Magic);
	SetEditContent(IDC_EDIT_8,Buffer);
	//��ϵͳ
	wsprintf(Buffer,TEXT("%p"),pOptionHeader->Subsystem);
	SetEditContent(IDC_EDIT_9,Buffer);
	//������Ŀ
	wsprintf(Buffer,TEXT("%p"),pPEHeader->NumberOfSections);
	SetEditContent(IDC_EDIT_10,Buffer);
	//ʱ���
	wsprintf(Buffer,TEXT("%p"),pPEHeader->TimeDateStamp);
	SetEditContent(IDC_EDIT_11,Buffer);
	//PEͷ��С
	wsprintf(Buffer,TEXT("%p"),pOptionHeader->SizeOfHeaders);
	SetEditContent(IDC_EDIT_12,Buffer);
	//����ֵ
	wsprintf(Buffer,TEXT("%p"),pPEHeader->Characteristics);
	SetEditContent(IDC_EDIT_13,Buffer);
	//У���
	wsprintf(Buffer,TEXT("%p"),pOptionHeader->CheckSum);
	SetEditContent(IDC_EDIT_14,Buffer);
	//��ѡPEͷ
	wsprintf(Buffer,TEXT("%p"),pPEHeader->SizeOfOptionalHeader);
	SetEditContent(IDC_EDIT_15,Buffer);
	//Ŀ¼����Ŀ
	wsprintf(Buffer,TEXT("%p"),pOptionHeader->NumberOfRvaAndSizes);
	SetEditContent(IDC_EDIT_16,Buffer);
	return;
}

//**************************************************��pBuffer������д��edit�ؼ�
void SetEditContent(DWORD dwEditID,TCHAR* pBuffer){	
	HWND hEdit = GetDlgItem(g_hDialogPe,dwEditID);
	SetWindowText(hEdit,pBuffer);
}