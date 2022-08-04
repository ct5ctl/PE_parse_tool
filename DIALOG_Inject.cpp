#include "stdafx.h"
#include "DIALOG_Main.h"


HWND g_hDialogDLLImport;
HWND g_hDialogDLLThread;
void GetProcess(void);
void GetDLL(void);
void SetThreadDLLEditContent(DWORD dwEditID,TCHAR* pBuffer);
void GetDLL_Thread(void);
void InjectProc_Thread(void);


void GetFile(void);
void SetImportDLLEditContent(DWORD dwEditID,TCHAR* pBuffer);
void GetDLL_Import(void);
void InjectProc_Import(void);


void CreateShellDialog(void){
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
	char shell_file_path[] = "C:\\My_works\\My_Dll\\SHELL_notepad.exe";
	MemeryTOFile(g_pFileBuffer, g_FileSize, shell_file_path);
	MessageBox(0,TEXT("����ӿǳɹ�!"),"�ɹ���",MB_OK);
	free(g_pFileBuffer);
	return;
}



//**************************************************�ص�����_DLL_IMPORT����
BOOL CALLBACK DllImportDlgProc(									
						  HWND hwndDlg,  // handle to dialog box			
						  UINT uMsg,     // message			
						  WPARAM wParam, // first message parameter			
						  LPARAM lParam  // second message parameter			
						  ){
	g_hDialogDLLImport = hwndDlg;
	switch(uMsg){
	case WM_INITDIALOG:
		return TRUE;
	case WM_CLOSE:
		EndDialog(g_hDialogDLLImport,0);
		return TRUE;
	case WM_COMMAND:
		switch(wParam)
		{
		case IDC_BUTTON_IMPORT_QUIT:
			EndDialog(g_hDialogDLLImport,0);
			return TRUE;
		case IDC_BUTTON_IMPORT_OK:
			InjectProc_Import();
			return TRUE;
		case IDC_BUTTON_DLL_FILE_SELECTION_IMPORT:
			GetFile();
			return TRUE;
		case IDC_BUTTON_DLL_SECETION_IMPORT:
			GetDLL_Import();
			
			//DialogBox(g_hDlg,(LPCTSTR)IDD_DIALOG_EXPORTTABLE,g_hDialogDirection,ExportTableDialogProc);
			return TRUE;
		}
	}
	return FALSE;
}

//**************************************************�ص�����_DLL_THREAD����
BOOL CALLBACK DllThreadDlgProc(									
						  HWND hwndDlg,  // handle to dialog box			
						  UINT uMsg,     // message			
						  WPARAM wParam, // first message parameter			
						  LPARAM lParam  // second message parameter			
						  ){
	g_hDialogDLLThread = hwndDlg;
	switch(uMsg){
	case WM_INITDIALOG:
		return TRUE;
	case WM_CLOSE:
		EndDialog(g_hDialogDLLThread,0);
		return TRUE;
	case WM_COMMAND:
		switch(wParam)
		{
		case IDC_BUTTON_THREAD_QUIT:
			EndDialog(g_hDialogDLLThread,0);
			return TRUE;
		case IDC_BUTTON_THREAD_OK:
			InjectProc_Thread();
			return TRUE;
// 		case IDC_BUTTON_DLL_PROCESS:
// 			GetProcess();
// 			return TRUE;
		case IDC_BUTTON_DLL_SELECTION:
			GetDLL();
			return TRUE;
		}
	}
	return FALSE;
}


void GetProcess(void){
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
	TCHAR rvaBuffer[100] = {0};
	TCHAR sizeBuffer[100] = {0};
	wsprintf(rvaBuffer,TEXT("%s"),&szFileName);
	SetDirectionEditContent(IDC_EDIT_PROCESS,rvaBuffer);
	//MessageBox(0,szFileName,0,0);
}

void GetDLL(void){
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
	TCHAR rvaBuffer[100] = {0};
	TCHAR sizeBuffer[100] = {0};
	wsprintf(rvaBuffer,TEXT("%s"),szFileName);
	SetThreadDLLEditContent(IDC_EDIT_DLL_REMOTETHREAD,rvaBuffer);
	return;
}
//**************************************************��pBuffer������д��Ŀ¼���ڵ�edit�ؼ�
void SetThreadDLLEditContent(DWORD dwEditID,TCHAR* pBuffer){	
	HWND hEdit = GetDlgItem(g_hDialogDLLThread,dwEditID);
	SetWindowText(hEdit,pBuffer);
}


void InjectProc_Thread(void){
	::MessageBox(0,TEXT("Զ���߳�ע��ɹ�!"),"�ɹ���",MB_OK);
	EndDialog(g_hDialogDLLThread,0);
}


//**************************************************������ִ���
void GetFile(void){
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
	TCHAR rvaBuffer[100] = {0};
	TCHAR sizeBuffer[100] = {0};
	wsprintf(rvaBuffer,TEXT("%s"),szFileName);
	SetImportDLLEditContent(IDC_EDIT_DLL_FILE_IMPORT,rvaBuffer);
	//MessageBox(0,szFileName,0,0);
}

void GetDLL_Import(void){
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
	TCHAR rvaBuffer[100] = {0};
	TCHAR sizeBuffer[100] = {0};
	wsprintf(rvaBuffer,TEXT("%s"),szFileName);
	SetImportDLLEditContent(IDC_EDIT_DLL_IMPORT,rvaBuffer);
	return;
	//MessageBox(0,szFileName,0,0);
}
//**************************************************��pBuffer������д��Ŀ¼���ڵ�edit�ؼ�
void SetImportDLLEditContent(DWORD dwEditID,TCHAR* pBuffer){	
	HWND hEdit = GetDlgItem(g_hDialogDLLImport,dwEditID);
	SetWindowText(hEdit,pBuffer);
}


void InjectProc_Import(void){
	::MessageBox(0,TEXT("�ɹ��޸ĵ����ע��ɹ�!"),"�ɹ���",MB_OK);
	EndDialog(g_hDialogDLLImport,0);
}