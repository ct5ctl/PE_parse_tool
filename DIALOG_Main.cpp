// Arw_PETool.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "DIALOG_Main.h"

//==================================================================================================================
PVOID g_pFileBuffer;
DWORD g_FileSize;
//全局变量hDlg用以存储 应用程序实例句柄(ImageBase)
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


//**************************************************入口函数									
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	g_hDlg = hInstance;
	//告诉PE loader需要用到的类有哪些
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&icex);
	//创建主窗口
	DialogBox(hInstance,(LPCTSTR)IDD_DIALOG_MAIN,NULL,MainDlgProc);
	return 0;
}

//**************************************************主窗口回调函数
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

//**************************************************主窗口进程列表初始化函数，hDlg为父窗口句柄
void InitProcessListView(void){
	//初始化[process]列表列名信息：
	LV_COLUMN lv;				
	//初始化					
	memset(&lv,0,sizeof(LV_COLUMN));					
	//获取IDC_LIST_PROCESS句柄					
	g_hListProcess = GetDlgItem(g_hDialogMain,IDC_LIST_PROCESS);					
	//设置整行选中					
	SendMessage(g_hListProcess,LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_FULLROWSELECT,LVS_EX_FULLROWSELECT);					
	
	//第一列					
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;		//选择需要设置的属性				
	lv.pszText = TEXT("进程");				//列标题	
	lv.cx = 250;					
	lv.iSubItem = 0;					
	//ListView_InsertColumn(hListProcess, 0, &lv);					
	SendMessage(g_hListProcess,LVM_INSERTCOLUMN,0,(DWORD)&lv);					
	//第二列					
	lv.pszText = TEXT("PID");					
	lv.cx = 112;					
	lv.iSubItem = 1;					
	//ListView_InsertColumn(hListProcess, 1, &lv);					
	SendMessage(g_hListProcess,LVM_INSERTCOLUMN,1,(DWORD)&lv);					
	//第三列					
	lv.pszText = TEXT("镜像基址");					
	lv.cx = 130;					
	lv.iSubItem = 2;					
	ListView_InsertColumn(g_hListProcess, 2, &lv);					
	//第四列					
	lv.pszText = TEXT("镜像大小");					
	lv.cx = 130;					
	lv.iSubItem = 3;					
	ListView_InsertColumn(g_hListProcess, 3, &lv);					
	
// 	//初始化[module]列表列名信息
// 	LV_COLUMN lv_1;	
// 	memset(&lv_1,0,sizeof(LV_COLUMN));
// 	g_hListModule = GetDlgItem(g_hDialogMain,IDC_LIST_MOUDLE);
// 	SendMessage(g_hListModule,LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_FULLROWSELECT,LVS_EX_FULLROWSELECT);
// 
// 	lv_1.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
// 	lv_1.pszText = TEXT("模块名称");
// 	lv_1.cx = 300;
// 	lv_1.iSubItem = 0;
// 	SendMessage(g_hListModule,LVM_INSERTCOLUMN,0,(DWORD)&lv_1);
// 	lv_1.pszText = TEXT("模块位置");
// 	lv_1.cx = 344;
// 	lv_1.iSubItem = 1;
// 	SendMessage(g_hListModule,LVM_INSERTCOLUMN,1,(DWORD)&lv_1);

	//获取当前所有进程并填入[process]列表，hListProcess为[process]列表句柄
	InitProcessListContent();	
}

//**************************************************初始化函数_主窗口模块列表
void InitModuleListView(void){
	//初始化[list]列表列名信息：
	LV_COLUMN lv;				
	//初始化					
	memset(&lv,0,sizeof(LV_COLUMN));					
	//获取IDC_LIST_MOUDLE句柄					
	g_hListModule = GetDlgItem(g_hDialogMain,IDC_LIST_MOUDLE);					
	//设置整行选中					
	SendMessage(g_hListModule,LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_FULLROWSELECT,LVS_EX_FULLROWSELECT);					
	
	//第一列					
	lv.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;					
	lv.pszText = TEXT("模块名称");				//列标题	
	lv.cx = 200;					
	lv.iSubItem = 0;					
	//ListView_InsertColumn(hListProcess, 0, &lv);					
	//SendMessage(g_hListModule,LVM_INSERTCOLUMN,0,(DWORD)&lv);					
	ListView_InsertColumn(g_hListModule,0,&lv);
	//第二列					
	lv.pszText = TEXT("模块位置");					
	lv.cx = 1000;					
	lv.iSubItem = 1;					
	//ListView_InsertColumn(hListProcess, 1, &lv);					
	//SendMessage(g_hListModule,LVM_INSERTCOLUMN,1,(DWORD)&lv);		
	ListView_InsertColumn(g_hListModule,1,&lv);
}
//**************************************************获取当前所有进程并填入[process]列表，hListProcess为[process]列表句柄
void InitProcessListContent(void){
	//1.获得当前所有进程名字、pid，若在内存中，则还要获取其镜像基址和镜像大小
	//2.将数据填入[process]列表中
	
	//给系统进程拍快照
	HANDLE lpSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);		//需要头文件"tlhelp32.h"
    if (lpSnapshot == INVALID_HANDLE_VALUE)
    {
        MessageBox(g_hDialogMain, TEXT("创建快照失败"), TEXT("ERROR"), MB_OK);
        return;
    }
	//初始化
	LV_ITEM lv;
	memset(&lv,0,sizeof(LV_ITEM));
	lv.mask = LVIF_TEXT;
	//遍历快照
	PROCESSENTRY32 lppe;
	lppe.dwSize = sizeof(lppe);							//如果没对dwSize初始化，pricess32First将会失败。
	BOOL bRet = Process32First(lpSnapshot,&lppe);		//Process32First若返回1，则第一个进程结构已被存入lppe		BOOL是int类型
	for(int row=0;bRet;row++){
		lv.iItem = row;
		
		//填入进程名
		LPSTR name = lppe.szExeFile;	//szExeFile （一个数组）（进程的可执行文件名称)
		lv.iSubItem = 0;
		lv.pszText = lppe.szExeFile;
		ListView_InsertItem(g_hListProcess,&lv);
		
		//填入PID
		TCHAR pBuffer[20] = {0};
		//strcpy(dwPID,lppe.th32ProcessID);
		lv.iSubItem = 1;
		DWORD dwPID = lppe.th32ProcessID;
		wsprintf(pBuffer,TEXT("%d"),dwPID);		//wsprintf用以将【参3】指向的buffer的任何内容以【参2】的格式复制到【参1】指向的buffer
		lv.pszText = pBuffer;
		ListView_SetItem(g_hListProcess,&lv);
		
		//填入镜像基址和大小
		MODULEINFO modInf = {0};
		BOOL miResult = FALSE;
		miResult = GetMainModuleInfo(dwPID, &modInf);
		if(miResult){
			//若获取进程主模块信息成功，则填入镜像基址和大小
			lv.iSubItem = 2;
			wsprintf(lv.pszText,TEXT("%p"),modInf.lpBaseOfDll);
			//lv.pszText = (char*)modInf.lpBaseOfDll;
			ListView_SetItem(g_hListProcess,&lv);

			lv.iSubItem = 3;
			wsprintf(lv.pszText,TEXT("%p"),modInf.SizeOfImage);
			//lv.pszText = (char*)modInf.SizeOfImage;
			ListView_SetItem(g_hListProcess,&lv);
		}
		bRet = Process32Next(lpSnapshot,&lppe);				//Process32Next若返回1，则下一个进程结构已被存入lppe
	}
}
//**************************************************获得pid对应的进程主模块信息=============================================================
BOOL GetMainModuleInfo(DWORD dwPID,MODULEINFO* modInf){
	//OpenProcess根据pid打开进程并获取进程句柄，若失败返回null
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
    if (hProcess == NULL)
    {
        return FALSE;
    }
	//EnumProcessModules根据进程句柄获取进程主模块句柄，若失败返回null，成功返回ture，参数2、4为输出值，前者为模块句柄数组，后者为进程的实际模块数量，参数3为参数2数组的大小
    HMODULE hModules[1];		//只取主模块即可
    DWORD cbNeeded;
    if (EnumProcessModules(hProcess, hModules, sizeof(hModules), &cbNeeded) == FALSE)
    {
        CloseHandle(hProcess);
        return FALSE;
    }
	//获取进程第一个模块信息传入modInfTemp的结构
    MODULEINFO modInfTemp;
    DWORD cb = sizeof(modInfTemp);
    if (GetModuleInformation(hProcess, hModules[0], &modInfTemp,cb) == FALSE)
    {
        CloseHandle(hProcess);
        return FALSE;
    }
	
    *modInf = modInfTemp;
	//关闭进程
    CloseHandle(hProcess);
    return TRUE;
}
//**************************************************遍历选中进程的所有模块，并将信息填入模块列表
void EnumModules(WPARAM wParam,LPARAM lParam){
	TCHAR szPid[0x20];
	LV_ITEM lv;
	DWORD dwRowId;
	//初始化
	memset(&lv,0,sizeof(LV_ITEM));
	memset(szPid,0,0x20);
	//获取所选行号
	dwRowId = SendMessage(g_hListProcess,LVM_GETNEXTITEM,-1,LVNI_SELECTED);
	if(dwRowId == -1){
		MessageBox(NULL,TEXT("请选择进程"),TEXT("出错啦!"),MB_OK);
		return;
	}
	//获取行号对应PID
	lv.iSubItem = 1;		//要获取的数据在第几列
	lv.pszText = szPid;		//缓冲区指针
	lv.cchTextMax = 0x20;	//缓冲区大小
	SendMessage(g_hListProcess,LVM_GETITEMTEXT,dwRowId,(DWORD)&lv);	//lv又为IN又为OUT
	//MessageBox(NULL,lv.pszText,TEXT("PID"),MB_OK);
	//DWORD dwPID = lv.pszText;
	DWORD dwPID = StrToInt(lv.pszText);
	bool bRet = FALSE;
	PostModuleListContent(dwPID);
}
//**************************************************根据【processList】选中的进程的dwPID，将进程的所有模块填入【moduleList】
void PostModuleListContent(DWORD dwPID){
	//0.清空数据
	//1.根据pid获取进程所有模块的句柄
	//2.根据各个模块句柄将信息填入【moduleList】

    ListView_DeleteAllItems(g_hListModule);	// 清空所有数据行
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
// 	//1.给进程拍快照
// 	//2.遍历快照
// 	//3.填入进程的所有模块名字和路径
// 
// 	//给系统进程拍快照
// 	HANDLE lpSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);		//需要头文件"tlhelp32.h"
//     if (lpSnapshot == INVALID_HANDLE_VALUE)
//     {
//         MessageBox(g_hDialogMain, TEXT("创建快照失败"), TEXT("ERROR"), MB_OK);
//         return;
//     }
// 	//初始化
// 	LV_ITEM lv;
// 	memset(&lv,0,sizeof(LV_ITEM));
// 	lv.mask = LVIF_TEXT;
// 	//遍历快照
// 	PROCESSENTRY32 lppe;
// 	lppe.dwSize = sizeof(lppe);							//如果没对dwSize初始化，pricess32First将会失败。
// 	BOOL bRet = Process32First(lpSnapshot,&lppe);		//Process32First若返回1，则第一个进程结构已被存入lppe		BOOL是int类型
// 	for(int row=0;bRet;row++){
// 		lv.iItem = row;
// 		//填入模块名
// 		lv.iSubItem = 0;
// 		lv.pszText = lppe.szExeFile;
// 		ListView_InsertItem(g_hListModule,&lv);
// 
// 		bRet = Process32Next(lpSnapshot,&lppe);
// 	}
}
//**************************************************根据pid获取进程所有模块的句柄
void GetAllModuleHandle(IN DWORD dwPID,OUT HMODULE* hMoudle,IN DWORD maxSize,OUT PDWORD num_Module){
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
	if(!hProcess){
		CloseHandle(hProcess);
		//MessageBox(g_hDialogMain,TEXT("进程句柄不存在"),TEXT("error"),MB_OK);
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


	
