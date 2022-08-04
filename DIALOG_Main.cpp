// Arw_PETool.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "DIALOG_Main.h"

//==================================================================================================================
PVOID g_pFileBuffer;
DWORD g_FileSize;
//ȫ�ֱ���hDlg���Դ洢 Ӧ�ó���ʵ�����(ImageBase)
HINSTANCE g_hDlg;
HWND g_hListProcess;
HWND g_hListModule;
HWND g_hDialogMain;
HWND g_hDialogAbout;
HWND g_hDialogPe;
HWND g_hDialogSectionTable;
HWND g_hDialogDirection;
HWND g_hDialogDetail;
HWND g_hDialogExport;
HWND g_hDialogImport;
HWND g_hDialogResource;
HWND g_hDialogBaseReloc;
HWND g_hDialogBoundImport;
HWND g_hDialogIAT;
//==================================================================================================================


//**************************************************��ں���									
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	g_hDlg = hInstance;
	//����PE loader��Ҫ�õ���������Щ
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&icex);
	//����������
	DialogBox(hInstance,(LPCTSTR)IDD_DIALOG_MAIN,NULL,MainDlgProc);
	return 0;
}

//**************************************************�����ڻص�����
BOOL CALLBACK MainDlgProc(									
						  HWND hwndDlg,  // handle to dialog box			
						  UINT uMsg,     // message			
						  WPARAM wParam, // first message parameter			
						  LPARAM lParam  // second message parameter			
						  )
{
	g_hDialogMain = hwndDlg;
	switch(uMsg){
	case WM_INITDIALOG:
		InitProcessListView();
		InitModuleListView();
		return TRUE;
	case WM_CLOSE:
		EndDialog(g_hDialogMain,0);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDC_BUTTON_QUIT:
			EndDialog(g_hDialogMain,0);
			return TRUE;
		case IDC_BUTTON_PE:
			CreatePeDialog();
			return TRUE;
		case IDC_BUTTON_ABOUT:
			DialogBox(g_hDlg,(LPCTSTR)IDD_DIALOG_ABOUT,g_hDialogMain,AboutDlgProc);
			return TRUE;
		case IDC_BUTTON_SHELL:
			CreateShellDialog();
			return TRUE;
		case IDC_BUTTON_DLL_INJECT_REMOTE_THREAD:
			DialogBox(g_hDlg,(LPCTSTR)IDD_DIALOG_DLL_THREAD,g_hDialogMain,DllThreadDlgProc);
			return TRUE;
		case IDC_BUTTON_DLL_INJECT_IMPORT_TABLE:
			DialogBox(g_hDlg,(LPCTSTR)IDD_DIALOG_DLL_IMPORT,g_hDialogMain,DllImportDlgProc);
			return TRUE;
 		}	
	case WM_NOTIFY:
		switch(wParam){
		case IDC_LIST_PROCESS:
			NMHDR* pNmhdr = (NMHDR*)lParam;
			if(pNmhdr->code == NM_CLICK){
				g_hListProcess = GetDlgItem(hwndDlg,IDC_LIST_PROCESS);
				EnumModules(wParam,lParam);
			}
		}
	}
	return FALSE;
}

//**************************************************�����ڽ����б��ʼ��������hDlgΪ�����ھ��
void InitProcessListView(void){
	//��ʼ��[process]�б�������Ϣ��
	LV_COLUMN lv;				
	//��ʼ��					
	memset(&lv,0,sizeof(LV_COLUMN));					
	//��ȡIDC_LIST_PROCESS���					
	g_hListProcess = GetDlgItem(g_hDialogMain,IDC_LIST_PROCESS);					
	//��������ѡ��					
	SendMessage(g_hListProcess,LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_FULLROWSELECT,LVS_EX_FULLROWSELECT);					
	
	//��һ��					
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;		//ѡ����Ҫ���õ�����				
	lv.pszText = TEXT("����");				//�б���	
	lv.cx = 250;					
	lv.iSubItem = 0;					
	//ListView_InsertColumn(hListProcess, 0, &lv);					
	SendMessage(g_hListProcess,LVM_INSERTCOLUMN,0,(DWORD)&lv);					
	//�ڶ���					
	lv.pszText = TEXT("PID");					
	lv.cx = 112;					
	lv.iSubItem = 1;					
	//ListView_InsertColumn(hListProcess, 1, &lv);					
	SendMessage(g_hListProcess,LVM_INSERTCOLUMN,1,(DWORD)&lv);					
	//������					
	lv.pszText = TEXT("�����ַ");					
	lv.cx = 130;					
	lv.iSubItem = 2;					
	ListView_InsertColumn(g_hListProcess, 2, &lv);					
	//������					
	lv.pszText = TEXT("�����С");					
	lv.cx = 130;					
	lv.iSubItem = 3;					
	ListView_InsertColumn(g_hListProcess, 3, &lv);					
	
// 	//��ʼ��[module]�б�������Ϣ
// 	LV_COLUMN lv_1;	
// 	memset(&lv_1,0,sizeof(LV_COLUMN));
// 	g_hListModule = GetDlgItem(g_hDialogMain,IDC_LIST_MOUDLE);
// 	SendMessage(g_hListModule,LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_FULLROWSELECT,LVS_EX_FULLROWSELECT);
// 
// 	lv_1.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
// 	lv_1.pszText = TEXT("ģ������");
// 	lv_1.cx = 300;
// 	lv_1.iSubItem = 0;
// 	SendMessage(g_hListModule,LVM_INSERTCOLUMN,0,(DWORD)&lv_1);
// 	lv_1.pszText = TEXT("ģ��λ��");
// 	lv_1.cx = 344;
// 	lv_1.iSubItem = 1;
// 	SendMessage(g_hListModule,LVM_INSERTCOLUMN,1,(DWORD)&lv_1);

	//��ȡ��ǰ���н��̲�����[process]�б�hListProcessΪ[process]�б���
	InitProcessListContent();	
}

//**************************************************��ʼ������_������ģ���б�
void InitModuleListView(void){
	//��ʼ��[list]�б�������Ϣ��
	LV_COLUMN lv;				
	//��ʼ��					
	memset(&lv,0,sizeof(LV_COLUMN));					
	//��ȡIDC_LIST_MOUDLE���					
	g_hListModule = GetDlgItem(g_hDialogMain,IDC_LIST_MOUDLE);					
	//��������ѡ��					
	SendMessage(g_hListModule,LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_FULLROWSELECT,LVS_EX_FULLROWSELECT);					
	
	//��һ��					
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;					
	lv.pszText = TEXT("ģ������");				//�б���	
	lv.cx = 200;					
	lv.iSubItem = 0;					
	//ListView_InsertColumn(hListProcess, 0, &lv);					
	//SendMessage(g_hListModule,LVM_INSERTCOLUMN,0,(DWORD)&lv);					
	ListView_InsertColumn(g_hListModule,0,&lv);
	//�ڶ���					
	lv.pszText = TEXT("ģ��λ��");					
	lv.cx = 1000;					
	lv.iSubItem = 1;					
	//ListView_InsertColumn(hListProcess, 1, &lv);					
	//SendMessage(g_hListModule,LVM_INSERTCOLUMN,1,(DWORD)&lv);		
	ListView_InsertColumn(g_hListModule,1,&lv);
}
//**************************************************��ȡ��ǰ���н��̲�����[process]�б�hListProcessΪ[process]�б���
void InitProcessListContent(void){
	//1.��õ�ǰ���н������֡�pid�������ڴ��У���Ҫ��ȡ�侵���ַ�;����С
	//2.����������[process]�б���
	
	//��ϵͳ�����Ŀ���
	HANDLE lpSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);		//��Ҫͷ�ļ�"tlhelp32.h"
    if (lpSnapshot == INVALID_HANDLE_VALUE)
    {
        MessageBox(g_hDialogMain, TEXT("��������ʧ��"), TEXT("ERROR"), MB_OK);
        return;
    }
	//��ʼ��
	LV_ITEM lv;
	memset(&lv,0,sizeof(LV_ITEM));
	lv.mask = LVIF_TEXT;
	//��������
	PROCESSENTRY32 lppe;
	lppe.dwSize = sizeof(lppe);							//���û��dwSize��ʼ����pricess32First����ʧ�ܡ�
	BOOL bRet = Process32First(lpSnapshot,&lppe);		//Process32First������1�����һ�����̽ṹ�ѱ�����lppe		BOOL��int����
	for(int row=0;bRet;row++){
		lv.iItem = row;
		
		//���������
		LPSTR name = lppe.szExeFile;	//szExeFile ��һ�����飩�����̵Ŀ�ִ���ļ�����)
		lv.iSubItem = 0;
		lv.pszText = lppe.szExeFile;
		ListView_InsertItem(g_hListProcess,&lv);
		
		//����PID
		TCHAR pBuffer[20] = {0};
		//strcpy(dwPID,lppe.th32ProcessID);
		lv.iSubItem = 1;
		DWORD dwPID = lppe.th32ProcessID;
		wsprintf(pBuffer,TEXT("%d"),dwPID);		//wsprintf���Խ�����3��ָ���buffer���κ������ԡ���2���ĸ�ʽ���Ƶ�����1��ָ���buffer
		lv.pszText = pBuffer;
		ListView_SetItem(g_hListProcess,&lv);
		
		//���뾵���ַ�ʹ�С
		MODULEINFO modInf = {0};
		BOOL miResult = FALSE;
		miResult = GetMainModuleInfo(dwPID, &modInf);
		if(miResult){
			//����ȡ������ģ����Ϣ�ɹ��������뾵���ַ�ʹ�С
			lv.iSubItem = 2;
			wsprintf(lv.pszText,TEXT("%p"),modInf.lpBaseOfDll);
			//lv.pszText = (char*)modInf.lpBaseOfDll;
			ListView_SetItem(g_hListProcess,&lv);

			lv.iSubItem = 3;
			wsprintf(lv.pszText,TEXT("%p"),modInf.SizeOfImage);
			//lv.pszText = (char*)modInf.SizeOfImage;
			ListView_SetItem(g_hListProcess,&lv);
		}
		bRet = Process32Next(lpSnapshot,&lppe);				//Process32Next������1������һ�����̽ṹ�ѱ�����lppe
	}
}
//**************************************************���pid��Ӧ�Ľ�����ģ����Ϣ=============================================================
BOOL GetMainModuleInfo(DWORD dwPID,MODULEINFO* modInf){
	//OpenProcess����pid�򿪽��̲���ȡ���̾������ʧ�ܷ���null
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
    if (hProcess == NULL)
    {
        return FALSE;
    }
	//EnumProcessModules���ݽ��̾����ȡ������ģ��������ʧ�ܷ���null���ɹ�����ture������2��4Ϊ���ֵ��ǰ��Ϊģ�������飬����Ϊ���̵�ʵ��ģ������������3Ϊ����2����Ĵ�С
    HMODULE hModules[1];		//ֻȡ��ģ�鼴��
    DWORD cbNeeded;
    if (EnumProcessModules(hProcess, hModules, sizeof(hModules), &cbNeeded) == FALSE)
    {
        CloseHandle(hProcess);
        return FALSE;
    }
	//��ȡ���̵�һ��ģ����Ϣ����modInfTemp�Ľṹ
    MODULEINFO modInfTemp;
    DWORD cb = sizeof(modInfTemp);
    if (GetModuleInformation(hProcess, hModules[0], &modInfTemp,cb) == FALSE)
    {
        CloseHandle(hProcess);
        return FALSE;
    }
	
    *modInf = modInfTemp;
	//�رս���
    CloseHandle(hProcess);
    return TRUE;
}
//**************************************************����ѡ�н��̵�����ģ�飬������Ϣ����ģ���б�
void EnumModules(WPARAM wParam,LPARAM lParam){
	TCHAR szPid[0x20];
	LV_ITEM lv;
	DWORD dwRowId;
	//��ʼ��
	memset(&lv,0,sizeof(LV_ITEM));
	memset(szPid,0,0x20);
	//��ȡ��ѡ�к�
	dwRowId = SendMessage(g_hListProcess,LVM_GETNEXTITEM,-1,LVNI_SELECTED);
	if(dwRowId == -1){
		MessageBox(NULL,TEXT("��ѡ�����"),TEXT("������!"),MB_OK);
		return;
	}
	//��ȡ�кŶ�ӦPID
	lv.iSubItem = 1;		//Ҫ��ȡ�������ڵڼ���
	lv.pszText = szPid;		//������ָ��
	lv.cchTextMax = 0x20;	//��������С
	SendMessage(g_hListProcess,LVM_GETITEMTEXT,dwRowId,(DWORD)&lv);	//lv��ΪIN��ΪOUT
	//MessageBox(NULL,lv.pszText,TEXT("PID"),MB_OK);
	//DWORD dwPID = lv.pszText;
	DWORD dwPID = StrToInt(lv.pszText);
	bool bRet = FALSE;
	PostModuleListContent(dwPID);
}
//**************************************************���ݡ�processList��ѡ�еĽ��̵�dwPID�������̵�����ģ�����롾moduleList��
void PostModuleListContent(DWORD dwPID){
	//0.�������
	//1.����pid��ȡ��������ģ��ľ��
	//2.���ݸ���ģ��������Ϣ���롾moduleList��

    ListView_DeleteAllItems(g_hListModule);	// �������������
	HMODULE hMoudle[1000] = {0};
	DWORD num_Module = 0;
	GetAllModuleHandle(dwPID,hMoudle,1000,&num_Module);
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
	if(num_Module == 0 || hMoudle[0] == 0 || !hProcess){
		return;
	}
	for(DWORD row=0;row < num_Module;row++){
		TCHAR NameBuffer[1000] = {0};
		TCHAR PathBuffer[1000] = {0};
		GetModuleBaseName(hProcess,hMoudle[row],NameBuffer,1000);
		GetModuleFileNameEx(hProcess,hMoudle[row],PathBuffer,1000);
		LV_ITEM lv = {0};
		lv.mask = LVIF_TEXT;
		lv.iItem = row;
		
		lv.iSubItem = 0;
		lv.pszText = NameBuffer;
		ListView_InsertItem(g_hListModule,&lv);
		lv.iSubItem = 1;
		lv.pszText = PathBuffer;
		ListView_SetItem(g_hListModule,&lv);
	}
	CloseHandle(hProcess);
// 	//1.�������Ŀ���
// 	//2.��������
// 	//3.������̵�����ģ�����ֺ�·��
// 
// 	//��ϵͳ�����Ŀ���
// 	HANDLE lpSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);		//��Ҫͷ�ļ�"tlhelp32.h"
//     if (lpSnapshot == INVALID_HANDLE_VALUE)
//     {
//         MessageBox(g_hDialogMain, TEXT("��������ʧ��"), TEXT("ERROR"), MB_OK);
//         return;
//     }
// 	//��ʼ��
// 	LV_ITEM lv;
// 	memset(&lv,0,sizeof(LV_ITEM));
// 	lv.mask = LVIF_TEXT;
// 	//��������
// 	PROCESSENTRY32 lppe;
// 	lppe.dwSize = sizeof(lppe);							//���û��dwSize��ʼ����pricess32First����ʧ�ܡ�
// 	BOOL bRet = Process32First(lpSnapshot,&lppe);		//Process32First������1�����һ�����̽ṹ�ѱ�����lppe		BOOL��int����
// 	for(int row=0;bRet;row++){
// 		lv.iItem = row;
// 		//����ģ����
// 		lv.iSubItem = 0;
// 		lv.pszText = lppe.szExeFile;
// 		ListView_InsertItem(g_hListModule,&lv);
// 
// 		bRet = Process32Next(lpSnapshot,&lppe);
// 	}
}
//**************************************************����pid��ȡ��������ģ��ľ��
void GetAllModuleHandle(IN DWORD dwPID,OUT HMODULE* hMoudle,IN DWORD maxSize,OUT PDWORD num_Module){
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
	if(!hProcess){
		CloseHandle(hProcess);
		//MessageBox(g_hDialogMain,TEXT("���̾��������"),TEXT("error"),MB_OK);
		return;
	}
	DWORD cbNeeded;
    if (!EnumProcessModules(hProcess, hMoudle,maxSize,&cbNeeded))
    {
        CloseHandle(hProcess);
        return;
    }
	*num_Module = cbNeeded/sizeof(HMODULE);
	CloseHandle(hProcess);
}


	
