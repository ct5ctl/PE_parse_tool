#ifndef DEF_PETOOLS_20220215
#define DEF_PETOOLS_20220215
#pragma once

#include "string.h"
#include <malloc.h>

#include <stdio.h>
#include <stdlib.h>

// exe->filebuffer  返回值为计算所得文件大小
DWORD ReadPEFile(IN LPSTR lpszFile, OUT LPVOID* ppFileBuffer);
//将内存偏移转换为文件偏移
DWORD RvaToFileOffset(IN LPVOID pFileBuffer, IN DWORD dwRva);
//将文件偏移转换为内存偏移
DWORD FoaToRva(IN PVOID pFileBuffer, IN DWORD Foa); 


//**************************************************************************
//CopyFileBufferToImageBuffer:将文件从FileBuffer复制到ImageBuffer
//参数说明：
//pFileBuffer  FileBuffer指针
//pImageBuffer ImageBuffer指针
//返回值说明：
//读取失败返回0  否则返回复制的大小
//**************************************************************************
DWORD CopyFileBufferToImageBuffer(IN LPVOID pFileBuffer, OUT LPVOID* pImageBuffer);
//**************************************************************************
//CopyImageBufferToNewBuffer:将ImageBuffer中的数据复制到新的缓冲区
//参数说明：
//pImageBuffer ImageBuffer指针
//pNewBuffer NewBuffer指针
//返回值说明：
//读取失败返回0  否则返回复制的大小
//**************************************************************************
DWORD CopyImageBufferToNewBuffer(IN LPVOID pImageBuffer, OUT LPVOID* pNewBuffer);
//**************************************************************************
//MemeryTOFile:将内存中的数据复制到文件
//参数说明：
//pMemBuffer 内存中数据的指针
//size 要复制的大小
//lpszFile 要存储的文件路径
//返回值说明：
//读取失败返回0  否则返回复制的大小
//**************************************************************************
bool MemeryTOFile(IN LPVOID pMemBuffer, IN size_t size, OUT LPSTR lpszFile);
//**************************************************************************
//RvaToFileOffset:将内存偏移转换为文件偏移
//参数说明：
//pFileBuffer FileBuffer指针
//dwRva RVA的值
//返回值说明：
//返回转换后的FOA的值  如果失败返回0
//**************************************************************************
//返回值为所添加代码在运行态时在内存的位置
DWORD addCodeToCodeSection(PVOID pImageBuffer, BYTE* shellCode);
//添加代码到代码区的执行函数
void add_code(void);

//pFrom到pTo间的值全0返回1，反之则返回0
int isAllZero(PVOID pFrom, PVOID pTo);
//找对齐值函数，value为最大数据大小，SectionAlignment为内存对其值，返回实际对齐值
DWORD Align(DWORD value, DWORD SectionAlignment);


//**************************************************打印pe文件导出表，失败返回0，成功返回1
DWORD print_exportTable(IN PVOID pFileBuffer);
//**************************************************rva转foa，失败返回0，成功返回foa
DWORD RvaToFoa(IN PVOID pFileBuffer,OUT DWORD dwRva);

#endif	//DEF_PETOOLS_20220215




















