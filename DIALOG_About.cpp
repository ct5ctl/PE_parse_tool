#include "stdafx.h"
//#include "DIALOG_About.h"
#include "DIALOG_Main.h"


//**************************************************���ڴ��ڻص�����
BOOL CALLBACK AboutDlgProc(									
						  HWND hwndDlg,  // handle to dialog box			
						  UINT uMsg,     // message			
						  WPARAM wParam, // first message parameter			
						  LPARAM lParam  // second message parameter			
						  ){
	g_hDialogAbout = hwndDlg;
	switch(uMsg){
// 	case WM_INITDIALOG:				//��Ҫ���ͼƬ��д��ʼ������
// 		InitAboutWindow(hwndDlg);
// 		return TRUE;
	case WM_CLOSE:
		EndDialog(hwndDlg,0);
		return TRUE;
	}
	return FALSE;
}
//**************************************************��ʼ������_���ڴ��ڣ�hwndDlgΪ�����ھ��
// void InitAboutWindow(void){
// 	HWND hSpecificationGroup; 
// 	hSpecificationGroup = GetDlgItem(hwndDlg,IDC_STATIC_SPECIFICATION);
// }