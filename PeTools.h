#ifndef DEF_PETOOLS_20220215
#define DEF_PETOOLS_20220215
#pragma once

#include "string.h"
#include <malloc.h>

#include <stdio.h>
#include <stdlib.h>

// exe->filebuffer  ����ֵΪ���������ļ���С
DWORD ReadPEFile(IN LPSTR lpszFile, OUT LPVOID* ppFileBuffer);
//���ڴ�ƫ��ת��Ϊ�ļ�ƫ��
DWORD RvaToFileOffset(IN LPVOID pFileBuffer, IN DWORD dwRva);
//���ļ�ƫ��ת��Ϊ�ڴ�ƫ��
DWORD FoaToRva(IN PVOID pFileBuffer, IN DWORD Foa); 


//**************************************************************************
//CopyFileBufferToImageBuffer:���ļ���FileBuffer���Ƶ�ImageBuffer
//����˵����
//pFileBuffer  FileBufferָ��
//pImageBuffer ImageBufferָ��
//����ֵ˵����
//��ȡʧ�ܷ���0  ���򷵻ظ��ƵĴ�С
//**************************************************************************
DWORD CopyFileBufferToImageBuffer(IN LPVOID pFileBuffer, OUT LPVOID* pImageBuffer);
//**************************************************************************
//CopyImageBufferToNewBuffer:��ImageBuffer�е����ݸ��Ƶ��µĻ�����
//����˵����
//pImageBuffer ImageBufferָ��
//pNewBuffer NewBufferָ��
//����ֵ˵����
//��ȡʧ�ܷ���0  ���򷵻ظ��ƵĴ�С
//**************************************************************************
DWORD CopyImageBufferToNewBuffer(IN LPVOID pImageBuffer, OUT LPVOID* pNewBuffer);
//**************************************************************************
//MemeryTOFile:���ڴ��е����ݸ��Ƶ��ļ�
//����˵����
//pMemBuffer �ڴ������ݵ�ָ��
//size Ҫ���ƵĴ�С
//lpszFile Ҫ�洢���ļ�·��
//����ֵ˵����
//��ȡʧ�ܷ���0  ���򷵻ظ��ƵĴ�С
//**************************************************************************
bool MemeryTOFile(IN LPVOID pMemBuffer, IN size_t size, OUT LPSTR lpszFile);
//**************************************************************************
//RvaToFileOffset:���ڴ�ƫ��ת��Ϊ�ļ�ƫ��
//����˵����
//pFileBuffer FileBufferָ��
//dwRva RVA��ֵ
//����ֵ˵����
//����ת�����FOA��ֵ  ���ʧ�ܷ���0
//**************************************************************************
//����ֵΪ����Ӵ���������̬ʱ���ڴ��λ��
DWORD addCodeToCodeSection(PVOID pImageBuffer, BYTE* shellCode);
//��Ӵ��뵽��������ִ�к���
void add_code(void);

//pFrom��pTo���ֵȫ0����1����֮�򷵻�0
int isAllZero(PVOID pFrom, PVOID pTo);
//�Ҷ���ֵ������valueΪ������ݴ�С��SectionAlignmentΪ�ڴ����ֵ������ʵ�ʶ���ֵ
DWORD Align(DWORD value, DWORD SectionAlignment);


//**************************************************��ӡpe�ļ�������ʧ�ܷ���0���ɹ�����1
DWORD print_exportTable(IN PVOID pFileBuffer);
//**************************************************rvaתfoa��ʧ�ܷ���0���ɹ�����foa
DWORD RvaToFoa(IN PVOID pFileBuffer,OUT DWORD dwRva);

#endif	//DEF_PETOOLS_20220215




















