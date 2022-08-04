#include "stdafx.h"
#include "DIALOG_Main.h"

//**************************************************创建一个文件选择窗口，并创建pe窗口
void CreatePeDialog(void){
	OPENFILENAME stOpenFile = {0};
	TCHAR szPeFileExt[100] = "*.exe;*.dll;*.sys;*.scr;*.drv";
	TCHAR szFileName[256] = {0};
	//弹出文件选择窗口，并获取用户选择的文件信息
	memset(&stOpenFile,0,sizeof(OPENFILENAME));
	stOpenFile.lStructSize = sizeof(OPENFILENAME);
	stOpenFile.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    stOpenFile.hwndOwner = g_hDialogMain;
    stOpenFile.lpstrFilter = szPeFileExt;
    stOpenFile.lpstrFile = szFileName;
    stOpenFile.nMaxFile = MAX_PATH;
	if(!GetOpenFileName(&stOpenFile)){
		MessageBox(0,TEXT("未选择文件"),0,MB_OK);
		return;
	}
	//打开文件到内存
	g_pFileBuffer = NULL;
	size_t g_FileSize = ReadPEFile(szFileName,&g_pFileBuffer);
	if(!g_FileSize){
		MessageBox(0,TEXT("非指定文件!"),0,MB_OK);
		return;
	}
	//创建Pe信息窗口
	DialogBox(g_hDlg,(LPCTSTR)IDD_DIALOG_PE,g_hDialogMain,PeDialogProc);
	return;
	//MessageBox(0,szFileName,0,0);
}
//**************************************************回调函数_pe窗口
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
//**************************************************初始化pe窗口，将选中文件的pe信息填入相应位置
void InitPeDialog(void){
	// 初始化PE头部结构体
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	// 初始化IMAGE_BUFFER指针(temparay)
	LPVOID pTempImagebuffer = NULL;
	//判断exe->内存是否成功
	if (!g_pFileBuffer)
	{
		MessageBox(g_hDialogPe,TEXT("打开文件失败!"),0,MB_OK);
		EndDialog(g_hDialogPe,0);
		return;
	}
	// 判断是否是可执行文件
	if (*((PWORD)g_pFileBuffer) != IMAGE_DOS_SIGNATURE)  // IMAGE_DOS_SIGNATURE是4字节，将pFileBuffer强制类型转换为4字节指针类型（PWORD）
	{
		MessageBox(g_hDialogPe,TEXT("不含MZ标志,非指定文件!"),0,MB_OK);
		EndDialog(g_hDialogPe,0);
		return;
	}
	//强制结构体类型转换pDosHeader
	pDosHeader = (PIMAGE_DOS_HEADER)g_pFileBuffer;
	//判断是否含有PE标志       
	if (*((PDWORD)((DWORD)g_pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE) 
	{																			 
		MessageBox(g_hDialogPe,TEXT("无效的PE标志!"),0,MB_OK);
		EndDialog(g_hDialogPe,0);
		return;
	}
	// 强制结构体类型转换
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)g_pFileBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	//将选中文件的pe信息填入相应位置
	TCHAR Buffer[100] = {0};
	//入口点
	wsprintf(Buffer,TEXT("%p"),pOptionHeader->AddressOfEntryPoint);
	SetEditContent(IDC_EDIT_1,Buffer);
	//镜像地址
	wsprintf(Buffer,TEXT("%p"),pOptionHeader->ImageBase);
	SetEditContent(IDC_EDIT_2,Buffer);
	//镜像大小
	wsprintf(Buffer,TEXT("%p"),pOptionHeader->SizeOfImage);
	SetEditContent(IDC_EDIT_3,Buffer);
	//代码基址
	wsprintf(Buffer,TEXT("%p"),pOptionHeader->BaseOfCode);
	SetEditContent(IDC_EDIT_4,Buffer);
	//数据基址
	wsprintf(Buffer,TEXT("%p"),pOptionHeader->BaseOfData);
	SetEditContent(IDC_EDIT_5,Buffer);
	//内存对齐
	wsprintf(Buffer,TEXT("%p"),pOptionHeader->SectionAlignment);
	SetEditContent(IDC_EDIT_6,Buffer);
	//文件对齐
	wsprintf(Buffer,TEXT("%p"),pOptionHeader->FileAlignment);
	SetEditContent(IDC_EDIT_7,Buffer);
	//标志字
	wsprintf(Buffer,TEXT("%p"),pOptionHeader->Magic);
	SetEditContent(IDC_EDIT_8,Buffer);
	//子系统
	wsprintf(Buffer,TEXT("%p"),pOptionHeader->Subsystem);
	SetEditContent(IDC_EDIT_9,Buffer);
	//区段数目
	wsprintf(Buffer,TEXT("%p"),pPEHeader->NumberOfSections);
	SetEditContent(IDC_EDIT_10,Buffer);
	//时间戳
	wsprintf(Buffer,TEXT("%p"),pPEHeader->TimeDateStamp);
	SetEditContent(IDC_EDIT_11,Buffer);
	//PE头大小
	wsprintf(Buffer,TEXT("%p"),pOptionHeader->SizeOfHeaders);
	SetEditContent(IDC_EDIT_12,Buffer);
	//特征值
	wsprintf(Buffer,TEXT("%p"),pPEHeader->Characteristics);
	SetEditContent(IDC_EDIT_13,Buffer);
	//校验和
	wsprintf(Buffer,TEXT("%p"),pOptionHeader->CheckSum);
	SetEditContent(IDC_EDIT_14,Buffer);
	//可选PE头
	wsprintf(Buffer,TEXT("%p"),pPEHeader->SizeOfOptionalHeader);
	SetEditContent(IDC_EDIT_15,Buffer);
	//目录项数目
	wsprintf(Buffer,TEXT("%p"),pOptionHeader->NumberOfRvaAndSizes);
	SetEditContent(IDC_EDIT_16,Buffer);
	return;
}

//**************************************************将pBuffer的内容写入edit控件
void SetEditContent(DWORD dwEditID,TCHAR* pBuffer){	
	HWND hEdit = GetDlgItem(g_hDialogPe,dwEditID);
	SetWindowText(hEdit,pBuffer);
}