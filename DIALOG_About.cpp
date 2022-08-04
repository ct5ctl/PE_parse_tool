#include "stdafx.h"
//#include "DIALOG_About.h"
#include "DIALOG_Main.h"


//**************************************************关于窗口回调函数
BOOL CALLBACK AboutDlgProc(									
						  HWND hwndDlg,  // handle to dialog box			
						  UINT uMsg,     // message			
						  WPARAM wParam, // first message parameter			
						  LPARAM lParam  // second message parameter			
						  ){
	g_hDialogAbout = hwndDlg;
	switch(uMsg){
// 	case WM_INITDIALOG:				//若要添加图片再写初始化函数
// 		InitAboutWindow(hwndDlg);
// 		return TRUE;
	case WM_CLOSE:
		EndDialog(hwndDlg,0);
		return TRUE;
	}
	return FALSE;
}
//**************************************************初始化函数_关于窗口，hwndDlg为父窗口句柄
// void InitAboutWindow(void){
// 	HWND hSpecificationGroup; 
// 	hSpecificationGroup = GetDlgItem(hwndDlg,IDC_STATIC_SPECIFICATION);
// }