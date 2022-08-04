#include "stdafx.h"
#include "DIALOG_Main.h"


// exe->filebuffer  返回值为计算所得文件大小
DWORD ReadPEFile(IN LPSTR lpszFile, OUT LPVOID* ppFileBuffer) {
	FILE* pFile = NULL;  // 文件指针
	DWORD file_size = 0;
	LPVOID pTempFilebuffer = NULL;

	// 打开文件
	pFile = fopen(lpszFile, "rb");  // 如果有新的指针，就要进行判断
	if (!pFile)
	{
		printf("打开exe文件失败！\n");//如果分配失败就要关闭文件、释放动态内存、指针指向NULL
		return 0;
	}
	// 读取文件大小
	fseek(pFile, 0, SEEK_END);
	file_size = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);
	// 分配空间
	pTempFilebuffer = malloc(file_size);  // 如果有新的指针，就要进行判断
	if (!pTempFilebuffer)
	{
		printf("分配空间失败！\n");//如果分配失败就要关闭文件、释放动态内存、指针指向NULL
		fclose(pFile);
		return 0;
	}
	// 将数据读取到内存中
	size_t n = fread(pTempFilebuffer, file_size, 1, pFile);
	if (!n)
	{
		printf("数据读取到内存中失败！\n"); //如果分配失败就要关闭文件、释放动态内存、指针指向NULL
		fclose(pFile);
		free(pTempFilebuffer);
		return 0;
	}
	// 关闭文件(已经读取到内存了)
	*ppFileBuffer = pTempFilebuffer;
	pTempFilebuffer = NULL;
	fclose(pFile);
	return file_size;
}

// filebuffer -> imagebuffer
DWORD CopyFileBufferToImageBuffer(IN LPVOID pFileBuffer, OUT LPVOID* ppImageBuffer) {
	// 初始化PE头部结构体
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	// 初始化IMAGE_BUFFER指针(temparay)
	LPVOID pTempImagebuffer = NULL;
	//判断exe->内存是否成功
	if (!pFileBuffer)
	{
		printf("(2pimagebuffer阶段)读取到内存的pfilebuffer无效！\n");
		return 0;
	}
	// 判断是否是可执行文件
	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)  // IMAGE_DOS_SIGNATURE是4字节，将pFileBuffer强制类型转换为4字节指针类型（PWORD）
	{
		printf("(2pimagebuffer阶段)不含MZ标志，不是exe文件！\n");
		return 0;
	}
	//强制结构体类型转换pDosHeader
	pDosHeader = PIMAGE_DOS_HEADER(pFileBuffer);
	//判断是否含有PE标志       
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE) // 注意指针的加法是：去掉一个*后的类型相加。必须转换为DWORD类型再加减。
	{																			  //相加后的和 强制类型转换为4字节指针类型（PWORD） IMAGE_NT_SIGNATURE 4BYTES
		printf("(2pimagebuffer阶段)不是有效的PE标志！\n");
		return 0;
	}
	// 强制结构体类型转换
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pFileBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);

	// 分配动态内存
	pTempImagebuffer = malloc(pOptionHeader->SizeOfImage);
	if (!pTempImagebuffer)
	{
		printf("分配动态内存失败！\n");
		free(pTempImagebuffer);
		return 0;
	}
	//初始化所分配的内存空间
	memset(pTempImagebuffer, 0, pOptionHeader->SizeOfImage);
	//复制头和节表到imagebuffer
	memcpy(pTempImagebuffer, pFileBuffer, pOptionHeader->SizeOfHeaders);
	//将各节按对齐方式复制到ImageBuffer
	PIMAGE_SECTION_HEADER sectionTemp = pSectionHeader;
	for (int i = 0; i < pPEHeader->NumberOfSections; i++, sectionTemp++) {
		memcpy((PIMAGE_SECTION_HEADER)((DWORD)pTempImagebuffer + sectionTemp->VirtualAddress), (PIMAGE_SECTION_HEADER)((DWORD)pFileBuffer + sectionTemp->PointerToRawData), sectionTemp->SizeOfRawData);
	}			//这里只要是指针就行，不用管转的是什么类型的指针（只看位置）

	*ppImageBuffer = pTempImagebuffer;
	pTempImagebuffer = NULL;    //不写也行，因为是局部变量
	return pOptionHeader->SizeOfImage;
}

//imagebuffer->newbuffer
DWORD CopyImageBufferToNewBuffer(IN LPVOID pImageBuffer, OUT LPVOID* ppNewBuffer) {
	// 初始化PE头部结构体
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;

	// 初始化NEW_BUFFER指针(temparay)
	LPVOID pTempNewbuffer = NULL;

	// 判断pImageBuffer是否有效
	if (!pImageBuffer)
	{
		printf("(2pnewbuffer阶段)读取到内存的pimagebuffer无效！\n");
		return 0;
	}
	//判断是不是exe文件
	if (*((PWORD)pImageBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(2pnewbuffer阶段)不含MZ标志，不是exe文件！\n");
		return 0;
	}
	// 强制结构体类型转换
	pDosHeader = (PIMAGE_DOS_HEADER)pImageBuffer;
	if (*((PDWORD)((DWORD)pImageBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(2pnewbuffer阶段)不是有效的PE标志！\n");
		return 0;
	}
	// 强制结构体类型转换
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pImageBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // 这里必须强制类型转换
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);

	//获取new_buffer的大小
	int new_buffer_size = pOptionHeader->SizeOfHeaders;
	for (DWORD i = 0; i < pPEHeader->NumberOfSections; i++)
	{
		new_buffer_size += pSectionHeader[i].SizeOfRawData;  // pSectionHeader[i]另一种加法
	}
	// 分配内存（newbuffer）
	pTempNewbuffer = malloc(new_buffer_size);
	if (!pTempNewbuffer)
	{
		printf("(2pnewbuffer阶段)分配Newbuffer失败！\n");
		return 0;
	}
	memset(pTempNewbuffer, 0, new_buffer_size);
	// 拷贝头部
	memcpy(pTempNewbuffer, pDosHeader, pOptionHeader->SizeOfHeaders);
	// 循环拷贝节区
	PIMAGE_SECTION_HEADER pTempSectionHeader = pSectionHeader;
	for (DWORD j = 0; j < pPEHeader->NumberOfSections; j++, pTempSectionHeader++)
	{	//PointerToRawData节区在文件中的偏移,VirtualAddress节区在内存中的偏移地址,SizeOfRawData节在文件中对齐后的尺寸
		memcpy((PDWORD)((DWORD)pTempNewbuffer + pTempSectionHeader->PointerToRawData), (PDWORD)((DWORD)pImageBuffer + pTempSectionHeader->VirtualAddress), pTempSectionHeader->SizeOfRawData);
	}
	//返回数据
	*ppNewBuffer = pTempNewbuffer; //暂存的数据传给参数后释放
	pTempNewbuffer = NULL;
	return new_buffer_size;  // 返回计算得到的分配内存的大小
}


//newbuffer->存盘
bool MemeryTOFile(IN LPVOID pMemBuffer, IN size_t size, OUT LPSTR lpszFile) {
	FILE* pFile = fopen(lpszFile, "wb");
	if (!pFile) {
		printf("创建新文件失败/n");
		return false;
	}
	fwrite(pMemBuffer, size, 1, pFile);
	return true;
}

//将内存偏移转换为文件偏移
DWORD RvaToFileOffset(IN LPVOID pFileBuffer, IN DWORD dwRva) {
	// 初始化PE头部结构体
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;

	// 判断pFileBuffer是否有效
	if (!pFileBuffer)
	{
		printf("pFileBuffer指针无效\n");
		return 0;
	}
	//判断是不是exe文件
	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(RVA->FOA阶段)不含MZ标志，不是exe文件！\n");
		return 0;
	}
	// 强制结构体类型转换
	pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(RVA->FOA阶段)不是有效的PE标志！\n");
		return 0;
	}
	// 强制结构体类型转换
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pFileBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // 这里必须强制类型转换
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	PIMAGE_SECTION_HEADER tempSection = pSectionHeader;
	if (dwRva <= pOptionHeader->SizeOfHeaders) {
		return dwRva;
	}
	else {
		for (int i = 0; i < pPEHeader->NumberOfSections; i++, tempSection++) {
			if (dwRva >= tempSection->VirtualAddress && dwRva <= tempSection->VirtualAddress + tempSection->Misc.VirtualSize) {
				return dwRva - tempSection->VirtualAddress + tempSection->PointerToRawData;
			}
		}
	}
	printf("rva->foa失败\n");
	return 0;
}

//将内存偏移转换为文件偏移
DWORD FoaToRva(IN PVOID pFileBuffer, IN DWORD Foa) {
	// 初始化PE头部结构体
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;

	// 判断pFileBuffer是否有效
	if (!pFileBuffer)
	{
		printf("pFileBuffer指针无效\n");
		return 0;
	}
	//判断是不是exe文件
	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(FOA->RVA阶段)不含MZ标志，不是exe文件！\n");
		return 0;
	}
	// 强制结构体类型转换
	pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(FOA->RVA阶段)不是有效的PE标志！\n");
		return 0;
	}
	// 强制结构体类型转换
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pFileBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // 这里必须强制类型转换
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	PIMAGE_SECTION_HEADER tempSection = pSectionHeader;
	if (Foa <= pOptionHeader->SizeOfHeaders) {
		return Foa;
	}
	else {
		for (int i = 0; i < pPEHeader->NumberOfSections; i++, tempSection++) {
			if (Foa >= tempSection->PointerToRawData && Foa <= tempSection->PointerToRawData + tempSection->Misc.VirtualSize) {
				return Foa - tempSection->PointerToRawData + tempSection->VirtualAddress;
			}
		}
	}
	printf("foa->rva失败\n");
	return 0;
}

//执行函数
void operate_pe()
{   // 初始化操作
	PVOID pFileBuffer = NULL;
	PVOID pImageBuffer = NULL;
	PVOID pNewFileBuffer = NULL;
	DWORD NewFileBufferSize = 0;
	//char file_path[] = "D:\\Lib\\IPMSG2007.exe";
	char file_path[] = "C:\\Windows\\System32\\notepad.exe";
	char write_file_path[] = "C:\\My_works\\cp_notepad.exe";

	// exe->filebuffer
	DWORD ret1 = ReadPEFile(file_path, &pFileBuffer);  // &pFileBuffer(void**类型) 传递地址对其值可以进行修改
	printf("exe->filebuffer  返回值为计算所得文件大小：%#x\n", ret1);
	// filebuffer -> imagebuffer
	DWORD ret2 = CopyFileBufferToImageBuffer(pFileBuffer, &pImageBuffer);
	printf("filebuffer -> imagebuffer返回值为计算所得文件大小：%#x\n", ret2);
	//imagebuffer->newbuffer
	DWORD FileSize = CopyImageBufferToNewBuffer(pImageBuffer, &pNewFileBuffer);
	printf("imagebuffer->newbuffer返回值为计算所得文件大小：%#x\n", FileSize);
	//newbuffer->存盘
	bool ret4 = MemeryTOFile(pNewFileBuffer, FileSize, write_file_path);
	if (ret4) {
		printf("写入到新文件成功！");
	}
	//释放申请的空间
	free(pFileBuffer);
	free(pImageBuffer);
	free(pNewFileBuffer);
}

void rva_foa_transf(void) {
	DWORD Rva = 0x10000;
	// 初始化操作
	PVOID pFileBuffer = NULL;
	PVOID pImageBuffer = NULL;
	PVOID pNewFileBuffer = NULL;
	DWORD NewFileBufferSize = 0;
	//char file_path[] = "D:\\Lib\\IPMSG2007.exe";
	char file_path[] = "C:\\Windows\\System32\\notepad.exe";
	char write_file_path[] = "C:\\My_works\\cp_notepad.exe";

	// exe->filebuffer
	DWORD ret = ReadPEFile(file_path, &pFileBuffer);  // &pFileBuffer(void**类型) 传递地址对其值可以进行修改
	printf("exe->filebuffer  返回值为计算所得文件大小：%#x\n", ret);

	DWORD FOA = RvaToFileOffset(pFileBuffer, Rva);	//只需传pFileBuffer即可，应为无论是内存中的什么状态，其头部和节表均是不变的，而rva->foa的转化只需看节表的偏移地址（与是否拉伸无关）
	printf("文件偏移：%x\n", FOA);

	//释放申请的空间
	free(pFileBuffer);
	free(pImageBuffer);
	free(pNewFileBuffer);
}

//*************************************************************************************0201__
#define size_shellCode 0x12
#define MessageBoxAdress 0x77D5050B

DWORD addCodeToCodeSection(PVOID pImageBuffer, BYTE* shellCode) {
	// 初始化PE头部结构体
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;

	// 判断pFileBuffer是否有效
	if (!pImageBuffer)
	{
		printf("(addCode阶段)pImageBuffer指针无效\n");
		return 0;
	}
	//判断是不是exe文件
	if (*((PWORD)pImageBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(addCode阶段)不含MZ标志，不是exe文件！\n");
		return 0;
	}
	// 强制结构体类型转换
	pDosHeader = (PIMAGE_DOS_HEADER)pImageBuffer;
	if (*((PDWORD)((DWORD)pDosHeader + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(addCode阶段)不是有效的PE标志！\n");
		return 0;
	}
	// 强制结构体类型转换
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // 这里必须强制类型转换
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	//判断代码节的空白区空间是否足够
	if ((pSectionHeader->SizeOfRawData - pSectionHeader->Misc.VirtualSize) < size_shellCode) {
		printf("(addCode阶段)代码节空白区不足！\n");
		return 0;
	}
	//添加代码
	char* codeBegin = (char*)(pSectionHeader->VirtualAddress + pSectionHeader->Misc.VirtualSize + (DWORD)pImageBuffer);
	memcpy(codeBegin, shellCode, size_shellCode);
	//代码跳转地址修正
	DWORD callCode = 0;
	DWORD jmpCode = 0;
	callCode = MessageBoxAdress - ((DWORD)codeBegin + 0XD - (DWORD)pImageBuffer + pOptionHeader->ImageBase);
	//jmpCode = (pOptionHeader->ImageBase + pOptionHeader->AddressOfEntryPoint) - ((DWORD)codeBegin + 0X12 - (DWORD)pImageBuffer + pOptionHeader->ImageBase);
	jmpCode = (pOptionHeader->AddressOfEntryPoint) - ((DWORD)codeBegin + 0X12 - (DWORD)pImageBuffer);
	*(PDWORD)(codeBegin + 9) = callCode;   //必须是PDWORD 这样才能把4个字节写入
	*(PDWORD)(codeBegin + 0XE) = jmpCode;
	pOptionHeader->AddressOfEntryPoint = (DWORD)codeBegin - (DWORD)pImageBuffer;  //oep中填的是偏移
	return (DWORD)codeBegin - (DWORD)pImageBuffer + pOptionHeader->ImageBase;
}




void add_code(void) {
	BYTE shellCode[] = {
		0x6A,00,0x6A,00,0x6A,00,0x6A,00,
		0xE8,0x00,0x00,0x00,0x00,
		0xE9,0x00,0x00,0x00,0x00
	};

	// 初始化操作
	PVOID pFileBuffer = NULL;
	PVOID pImageBuffer = NULL;
	PVOID pNewFileBuffer = NULL;
	DWORD NewFileBufferSize = 0;
	//char file_path[] = "D:\\Lib\\IPMSG2007.exe";
	char file_path[] = "C:\\Windows\\System32\\notepad.exe";
	char write_file_path[] = "C:\\My_works\\add_notepad.exe";

	// exe->filebuffer
	DWORD ret1 = ReadPEFile(file_path, &pFileBuffer);  // &pFileBuffer(void**类型) 传递地址对其值可以进行修改
	printf("exe->filebuffer  返回值为计算所得文件大小：%#x\n", ret1);
	// filebuffer -> imagebuffer
	DWORD ret2 = CopyFileBufferToImageBuffer(pFileBuffer, &pImageBuffer);
	printf("filebuffer -> imagebuffer返回值为计算所得文件大小：%#x\n", ret2);
	//添加代码到代码节空白区
	DWORD place = addCodeToCodeSection(pImageBuffer, shellCode);
	printf("新代码的内存中运行位置：%x\n", place);
	//imagebuffer->newbuffer
	DWORD FileSize = CopyImageBufferToNewBuffer(pImageBuffer, &pNewFileBuffer);
	printf("imagebuffer->newbuffer返回值为计算所得文件大小：%#x\n", FileSize);
	//newbuffer->存盘
	bool ret4 = MemeryTOFile(pNewFileBuffer, FileSize, write_file_path);
	if (ret4) {
		printf("写入到新文件成功！");
	}
	//释放申请的空间
	free(pFileBuffer);
	free(pImageBuffer);
	free(pNewFileBuffer);
}

//***************************************************************************************************************0203___P46

#define addSection_needSpace 0x50
#define Size_NewSection 0x1000
//pFrom到pTo间的值全0返回1，反之则返回0
int isAllZero(PVOID pFrom, PVOID pTo) {
	while (pFrom < pTo) {
		if (*(PBYTE)pFrom != 0)return 0;
		pFrom = (PVOID)((DWORD)pFrom + 1);
	}
	return 1;
}


//找对齐值函数，value为最大数据大小，SectionAlignment为内存对其值，返回实际对齐值
DWORD Align(DWORD value, DWORD SectionAlignment) {
	for (int i = 1; value > SectionAlignment * i; i++);
	return i * SectionAlignment;
}

//找最大值
DWORD Max(DWORD A, DWORD B) {
	return A > B ? A : B;
}

//新增节的执行函数
DWORD addSection(PVOID pImageBuffer, PVOID* ppNewImageBuffer) {
	size_t size_NewBuffer = 0;
	// 初始化PE头部结构体
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;


	// 判断pImageBuffer是否有效
	if (!pImageBuffer)
	{
		printf("(addSection阶段)pImageBuffer指针无效\n");
		return 0;
	}
	//判断是不是exe文件
	if (*((PWORD)pImageBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(addSection阶段)不含MZ标志，不是exe文件！\n");
		return 0;
	}
	// 强制结构体类型转换
	pDosHeader = (PIMAGE_DOS_HEADER)pImageBuffer;
	if (*((PDWORD)((DWORD)pDosHeader + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(addSection阶段)不是有效的PE标志！\n");
		return 0;
	}
	// 强制结构体类型转换
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // 这里必须强制类型转换
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);

	PIMAGE_SECTION_HEADER pSectionTemp = pSectionHeader;
	//判断最后一个节表后是否还有两个节表的空间（0x50）
	for (int i = 0; i < pPEHeader->NumberOfSections; i++) {
		pSectionTemp++;
	}
	if ((pOptionHeader->SizeOfHeaders - ((DWORD)pSectionTemp + 0x28)) < addSection_needSpace || !isAllZero((PVOID)((DWORD)pSectionTemp + 0x28), (PVOID)((DWORD)pSectionTemp + 0x78))) {
		printf("添加新节表所需空间不足，需进行节表前移或扩大节\n");
		//moveHeaders()   exSection()
		return 0;
	}

	printf("添加新节表所需空间充足\n");
	//申请newImageBuffer空间
	size_t SizeOfNewImageBuffer = pOptionHeader->SizeOfImage + Size_NewSection;
	PVOID pTempNewImageBuffer = malloc(SizeOfNewImageBuffer);
	if (!pTempNewImageBuffer) {
		printf("(addSection阶段)分配NewImageBuffer内存空间失败\n");
		return 0;
	}
	memset(pTempNewImageBuffer, 0, pOptionHeader->SizeOfImage + 0x1000);
	memcpy(pTempNewImageBuffer, pImageBuffer, pOptionHeader->SizeOfImage);
	//这里可对新节进行赋值

	// 将头部指针指向NewImageBuffer的对应位置
	PIMAGE_DOS_HEADER NewpDosHeader = (PIMAGE_DOS_HEADER)pTempNewImageBuffer;
	PIMAGE_NT_HEADERS NewpNTHeader = (PIMAGE_NT_HEADERS)((DWORD)NewpDosHeader + NewpDosHeader->e_lfanew);
	PIMAGE_FILE_HEADER NewpPEHeader = (PIMAGE_FILE_HEADER)((DWORD)NewpNTHeader + 4);
	PIMAGE_OPTIONAL_HEADER NewpOptionHeader = (PIMAGE_OPTIONAL_HEADER)((DWORD)NewpPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	PIMAGE_SECTION_HEADER NewpSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)NewpOptionHeader + NewpPEHeader->SizeOfOptionalHeader);

	//进行添加新节操作
	//PIMAGE_SECTION_HEADER newSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)(pSectionTemp+1) - (DWORD)pImageBuffer + (DWORD)pTempNewImageBuffer);  //该指针指向newImageBuffer的新增节表位置
	PIMAGE_SECTION_HEADER newSectionHeader = NewpSectionHeader;
	for (i = 0; i < NewpPEHeader->NumberOfSections; i++, newSectionHeader++);
	memcpy(newSectionHeader, NewpSectionHeader, 0x28);
	memset(newSectionHeader + 1, 0, 0x28);       //规定最后一节之后要有一个节表的全0


	//修改头部信息
	NewpOptionHeader->SizeOfImage = NewpOptionHeader->SizeOfImage + 0x1000;
	NewpPEHeader->NumberOfSections++;
	//修改新节表信息
	//newSectionHeader->Name = ".haha\0";
	memcpy(newSectionHeader, ".haha", 8);
	newSectionHeader->SizeOfRawData = Size_NewSection;
	newSectionHeader->Misc.VirtualSize = Size_NewSection;
	pSectionTemp = newSectionHeader - 1;
	newSectionHeader->PointerToRawData = pSectionTemp->PointerToRawData + pSectionTemp->SizeOfRawData;

	DWORD max = pSectionTemp->SizeOfRawData;
	if (pSectionTemp->Misc.VirtualSize > pSectionTemp->SizeOfRawData)max = pSectionTemp->Misc.VirtualSize;
	DWORD size = Align(max, NewpOptionHeader->SectionAlignment);
	newSectionHeader->VirtualAddress = size + pSectionTemp->VirtualAddress;

	*ppNewImageBuffer = pTempNewImageBuffer;
	pTempNewImageBuffer = NULL;
	return SizeOfNewImageBuffer;
}




//新增节的外函数
void add_section(void) {
	// 初始化操作
	PVOID pFileBuffer = NULL;
	PVOID pImageBuffer = NULL;
	PVOID pNewImageBuffer = NULL;
	PVOID pNewFileBuffer = NULL;
	DWORD NewFileBufferSize = 0;
	//char file_path[] = "D:\\Lib\\IPMSG2007.exe";
	char file_path[] = "C:\\software__\\IPMSG2007.exe";
	char write_file_path[] = "C:\\software__\\addSection_IPMSG2007.exe";

	// exe->filebuffer
	DWORD ret1 = ReadPEFile(file_path, &pFileBuffer);  // &pFileBuffer(void**类型) 传递地址对其值可以进行修改
	printf("原文件大小：%#x\n", ret1);
	// filebuffer -> imagebuffer
	DWORD ret2 = CopyFileBufferToImageBuffer(pFileBuffer, &pImageBuffer);
	printf("filebuffer -> imagebuffer返回值为计算所得文件大小：%#x\n", ret2);
	//新增一个节
	DWORD ret3 = addSection(pImageBuffer, &pNewImageBuffer);
	printf("新增节后新文件的运行内存大小：%#x\n", ret3);
	//imagebuffer->newbuffer
	DWORD FileSize = CopyImageBufferToNewBuffer(pNewImageBuffer, &pNewFileBuffer);
	printf("imagebuffer->newbuffer返回值为计算所得文件大小：%#x\n", FileSize);
	//newbuffer->存盘
	bool ret4 = MemeryTOFile(pNewFileBuffer, FileSize, write_file_path);
	if (ret4) {
		printf("写入到新文件成功！");
	}
	//释放申请的空间
	free(pFileBuffer);
	free(pNewFileBuffer);
	free(pNewImageBuffer);
	free(pImageBuffer);
}


//**************************************************************************0202__p47扩大节合并节

#define size_expandSection 0x1000

//扩大节函数
DWORD expandLastSection(IN PVOID pImageBuffer, OUT PVOID* pExpandImageBuffer) {
	// 初始化PE头部结构体
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	// 判断pImageBuffer是否有效
	if (!pImageBuffer)
	{
		printf("(addSection阶段)pImageBuffer指针无效\n");
		return 0;
	}
	//判断是不是exe文件
	if (*((PWORD)pImageBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(addSection阶段)不含MZ标志，不是exe文件！\n");
		return 0;
	}
	pDosHeader = (PIMAGE_DOS_HEADER)pImageBuffer;
	if (*((PDWORD)((DWORD)pDosHeader + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(addSection阶段)不是有效的PE标志！\n");
		return 0;
	}
	// 强制结构体类型转换
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // 这里必须强制类型转换
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);


	//修改最后一个节表参数
	/*PIMAGE_SECTION_HEADER pLastSection = pSectionHeader + pPEHeader->NumberOfSections - 1;
	DWORD old_max = Max(pLastSection->Misc.VirtualSize,pLastSection->SizeOfRawData);
	DWORD old_size = Align(old_max,pOptionHeader->SectionAlignment);
	DWORD max = Max(pLastSection->Misc.VirtualSize + size_expandSection,pLastSection->SizeOfRawData);
	DWORD size = Align(max,pOptionHeader->SectionAlignment);
	pLastSection->Misc.VirtualSize = pLastSection->Misc.VirtualSize + size_expandSection;
	pLastSection->SizeOfRawData = size;
	pOptionHeader->SizeOfImage = pLastSection->Misc.VirtualSize + size;*/

	PIMAGE_SECTION_HEADER pLastSection = pSectionHeader + pPEHeader->NumberOfSections - 1;

	DWORD max = Max(pLastSection->Misc.VirtualSize, pLastSection->SizeOfRawData);
	DWORD size = Align(max, pOptionHeader->SectionAlignment);
	pLastSection->Misc.VirtualSize = size + size_expandSection;
	pLastSection->SizeOfRawData = size + size_expandSection;
	pOptionHeader->SizeOfImage = pLastSection->VirtualAddress + pLastSection->Misc.VirtualSize;
	//申请新空间
	size_t Size_ExpandImageBuffer = pOptionHeader->SizeOfImage;
	PVOID pTempExpandImageBuffer = malloc(Size_ExpandImageBuffer);
	if (!pTempExpandImageBuffer) {
		printf("（扩大最后一节阶段）分配内存空间失败\n");
		return 0;
	}
	memset(pTempExpandImageBuffer, 0, Size_ExpandImageBuffer);
	memcpy(pTempExpandImageBuffer, pImageBuffer, pOptionHeader->SizeOfImage - size_expandSection);
	//在最后一节填入新数据(要先改变头指针基址)
	//memcpy((char*)(pNewLastSection->VirtualAddress + pNewLastSection->Misc.VirtualSize),pNewData,size_expandSection);


	*pExpandImageBuffer = pTempExpandImageBuffer;
	return 1;
}

//合并节函数
DWORD MergeAllSection(IN PVOID pImageBuffer, OUT PVOID* pNewImageBuffer) {
	// 初始化PE头部结构体
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	// 判断pImageBuffer是否有效
	if (!pImageBuffer)
	{
		printf("(addSection阶段)pImageBuffer指针无效\n");
		return 0;
	}
	//判断是不是exe文件
	if (*((PWORD)pImageBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(addSection阶段)不含MZ标志，不是exe文件！\n");
		return 0;
	}
	pDosHeader = (PIMAGE_DOS_HEADER)pImageBuffer;
	if (*((PDWORD)((DWORD)pDosHeader + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(addSection阶段)不是有效的PE标志！\n");
		return 0;
	}
	// 强制结构体类型转换
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // 这里必须强制类型转换
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	PIMAGE_SECTION_HEADER pTempSectionHeader = pSectionHeader;
	//修改节表1的参数
	PIMAGE_SECTION_HEADER pLastSectionHeader = pSectionHeader + pPEHeader->NumberOfSections - 1;
	DWORD charactor = 0;
	for (int i = 0; i < pPEHeader->NumberOfSections; i++, pTempSectionHeader++) {
		charactor |= pTempSectionHeader->Characteristics;
	}
	pSectionHeader->Characteristics = charactor;
	pSectionHeader->Misc.VirtualSize = pOptionHeader->SizeOfImage - pOptionHeader->SizeOfHeaders;
	//pSectionHeader->SizeOfRawData = pLastSectionHeader->PointerToRawData - pSectionHeader->PointerToRawData + pLastSectionHeader->SizeOfRawData;  ***不能这样  因为只有一个节不能像多个节那样拉伸
	pSectionHeader->SizeOfRawData = pOptionHeader->SizeOfImage - pOptionHeader->SizeOfHeaders;
	//清空其他节表
	memset((char*)((DWORD)pSectionHeader + 0x28), 0, (pPEHeader->NumberOfSections - 1) * 0x28);
	//修改头部相应参数
	pPEHeader->NumberOfSections = 1;

	*pNewImageBuffer = pImageBuffer;
	pImageBuffer = NULL;
	return 1;
}

//扩大节合并节的外函数
void expandOrMerge_Section(void) {
	// 初始化操作
	PVOID pFileBuffer = NULL;
	PVOID pImageBuffer = NULL;
	PVOID pNewImageBuffer = NULL;
	PVOID pNewFileBuffer = NULL;
	DWORD NewFileBufferSize = 0;
	//char file_path[] = "D:\\Lib\\IPMSG2007.exe";
	char file_path[] = "C:\\software__\\IPMSG2007.exe";
	char write_file_path[] = "C:\\software__\\addSection_IPMSG2007.exe";

	// exe->filebuffer
	DWORD ret1 = ReadPEFile(file_path, &pFileBuffer);  // &pFileBuffer(void**类型) 传递地址对其值可以进行修改
	printf("原文件大小：%#x\n", ret1);
	// filebuffer -> imagebuffer
	DWORD ret2 = CopyFileBufferToImageBuffer(pFileBuffer, &pImageBuffer);
	printf("filebuffer -> imagebuffer返回值为计算所得文件大小：%#x\n", ret2);
	//新增一个节
	/*DWORD ret3 = expandLastSection(pImageBuffer,&pNewImageBuffer);
	if(ret3)
		printf("expand succeed\n");*/
		//合并所有节
	DWORD flag = MergeAllSection(pImageBuffer, &pNewImageBuffer);
	if (flag)printf("合并节表成功\n");
	//imagebuffer->newbuffer
	DWORD FileSize = CopyImageBufferToNewBuffer(pNewImageBuffer, &pNewFileBuffer);
	printf("imagebuffer->newbuffer返回值为计算所得文件大小：%#x\n", FileSize);
	//newbuffer->存盘
	bool ret4 = MemeryTOFile(pNewFileBuffer, FileSize, write_file_path);
	if (ret4) {
		printf("写入到新文件成功！");
	}
	//释放申请的空间
	free(pFileBuffer);
	//free(pImageBuffer);
	free(pNewImageBuffer);
	free(pNewFileBuffer);

}
//*****************************************************************************0205__P49打印导出表

//打印pe文件导出表，失败返回0，成功返回1
DWORD print_exportTable(IN PVOID pFileBuffer) {
	// 初始化PE头部结构体
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	PIMAGE_EXPORT_DIRECTORY pExportDirectory = NULL;
	//判断exe->内存是否成功
	if (!pFileBuffer)
	{
		printf("读取到内存的pfilebuffer失败！\n");
		return 0;
	}
	// 判断是否是可执行文件
	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)  // IMAGE_DOS_SIGNATURE是4字节，将pFileBuffer强制类型转换为4字节指针类型（PWORD）
	{
		printf("不含MZ标志，不是exe文件！\n");
		return 0;
	}
	//强制结构体类型转换pDosHeader
	pDosHeader = PIMAGE_DOS_HEADER(pFileBuffer);
	//判断是否含有PE标志       
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("不是有效的PE标志！\n");
		return 0;
	}
	// 强制结构体类型转换
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	pDataDirectory = (PIMAGE_DATA_DIRECTORY)pOptionHeader->DataDirectory;
	//判断导出表是否存在
	if (!pDataDirectory->VirtualAddress) {
		printf("导出表不存在\n");
		return 0;
	}
	DWORD Foa = RvaToFileOffset(pFileBuffer, pDataDirectory->VirtualAddress);
	pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((DWORD)pFileBuffer + Foa);
	//打印导出表内容                      
	printf("*****************************************************\n");
	printf("Name:%#x\n", pExportDirectory->Name);
	printf("Base:%d\n", pExportDirectory->Base);
	printf("NumberOfFunctions：%d\n", pExportDirectory->NumberOfFunctions);
	printf("NumberOfNames:%d\n", pExportDirectory->NumberOfNames);
	//打印函数地址表
	printf("*****************************************************地址表\n");
	PDWORD pAddressTable = (PDWORD)(RvaToFileOffset(pFileBuffer, pExportDirectory->AddressOfFunctions) + (DWORD)pDosHeader);
	for (DWORD i = 0; i < pExportDirectory->NumberOfFunctions; i++) {
		printf("%d的地址:%#x\n", i, *(pAddressTable + i));
	}
	//打印函数名称表
	printf("*****************************************************名称表\n");
	DWORD foa_AddressOfNames = RvaToFileOffset(pFileBuffer, pExportDirectory->AddressOfNames);
	PDWORD pNameTable = (PDWORD)(foa_AddressOfNames + (DWORD)pDosHeader);
	LPSTR name = (LPSTR)pNameTable;
	for (i = 0; i < pExportDirectory->NumberOfNames; i++, pNameTable++) {
		name = (LPSTR)pNameTable;
		printf("%d的名称:%s\n", i, name);
	}
	//打印函数序号表
	printf("*****************************************************序号表\n");
	PWORD pNameOrdinalsTable = (PWORD)(RvaToFileOffset(pFileBuffer, pExportDirectory->AddressOfNameOrdinals) + (DWORD)pDosHeader);
	for (i = 0; i < pExportDirectory->NumberOfNames; i++) {
		printf("%d的函数序号:%#x\n", i, *(pNameOrdinalsTable + i));
	}
	return 1;
}

//通过函数名字获得函数地址
DWORD GetFunctionAddrByName(IN PVOID pFileBuffer, IN LPSTR pFunctionName) {
	// 初始化PE头部结构体
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	PIMAGE_EXPORT_DIRECTORY pExportDirectory = NULL;
	//判断exe->内存是否成功
	if (!pFileBuffer)
	{
		printf("读取到内存的pfilebuffer失败！\n");
		return 0;
	}
	// 判断是否是可执行文件
	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)  // IMAGE_DOS_SIGNATURE是4字节，将pFileBuffer强制类型转换为4字节指针类型（PWORD）
	{
		printf("不含MZ标志！\n");
		return 0;
	}
	//强制结构体类型转换pDosHeader
	pDosHeader = PIMAGE_DOS_HEADER(pFileBuffer);
	//判断是否含有PE标志       
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("不是有效的PE标志！\n");
		return 0;
	}
	// 强制结构体类型转换
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	pDataDirectory = (PIMAGE_DATA_DIRECTORY)pOptionHeader->DataDirectory;
	//判断导出表是否存在
	if (!pDataDirectory->VirtualAddress) {
		printf("导出表不存在\n");
		return 0;
	}
	DWORD Foa = RvaToFileOffset(pFileBuffer, pDataDirectory->VirtualAddress);
	pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((DWORD)pFileBuffer + Foa);
	LPSTR nameTemp = (LPSTR)((DWORD)pFileBuffer + RvaToFileOffset(pFileBuffer, pExportDirectory->AddressOfNames));
	int index = -1;
	for (DWORD i = 0; i < pExportDirectory->NumberOfNames; i++, nameTemp++) {
		if (!strcmp(nameTemp, pFunctionName)) {
			index = i;
			break;
		}
	}
	//若找到相应函数名，则在序号表中找相同下标的值来访问地址表
	if (index >= 0) {
		PWORD indexTemp = (PWORD)((DWORD)pFileBuffer + RvaToFileOffset(pFileBuffer, pExportDirectory->AddressOfNameOrdinals));
		PDWORD addressTemp = (PDWORD)((DWORD)pFileBuffer + RvaToFileOffset(pFileBuffer, pExportDirectory->AddressOfFunctions));
		indexTemp += index;
		addressTemp += (DWORD)*indexTemp;
		return *addressTemp;
	}
	printf("未找到同名函数\n");
	return 0;
}

//通过函数序号获得函数地址
DWORD GetFunctionAddrByOrdinals(IN PVOID pFileBuffer, IN DWORD Ord) {
	// 初始化PE头部结构体
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	PIMAGE_EXPORT_DIRECTORY pExportDirectory = NULL;
	//判断exe->内存是否成功
	if (!pFileBuffer)
	{
		printf("读取到内存的pfilebuffer失败！\n");
		return 0;
	}
	// 判断是否是可执行文件
	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)  // IMAGE_DOS_SIGNATURE是4字节，将pFileBuffer强制类型转换为4字节指针类型（PWORD）
	{
		printf("不含MZ标志！\n");
		return 0;
	}
	//强制结构体类型转换pDosHeader
	pDosHeader = PIMAGE_DOS_HEADER(pFileBuffer);
	//判断是否含有PE标志       
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("不是有效的PE标志！\n");
		return 0;
	}
	// 强制结构体类型转换
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	pDataDirectory = (PIMAGE_DATA_DIRECTORY)pOptionHeader->DataDirectory;
	//判断导出表是否存在
	if (!pDataDirectory->VirtualAddress) {
		printf("导出表不存在\n");
		return 0;
	}
	DWORD Foa = RvaToFileOffset(pFileBuffer, pDataDirectory->VirtualAddress);
	pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((DWORD)pFileBuffer + Foa);
	DWORD index = Ord - pExportDirectory->Base;
	if (index >= pExportDirectory->NumberOfFunctions || index < 0) {
		printf("函数序号无效\n");
		return 0;
	}
	printf("index:%d\n", index);
	PDWORD addressTemp = (PDWORD)((DWORD)pFileBuffer + RvaToFileOffset(pFileBuffer, pExportDirectory->AddressOfFunctions));
	addressTemp += index;
	return *addressTemp;
}


void operate_exprotTable(void) {
	PVOID pFileBuffer = NULL;
	//char file_path[] = "C:\\software__\\IPMSG2007.exe";			
	//char file_path[] = "C:\\WINDOWS\\system32\\notepad.exe";	C:\My_works\nixiang_220120\dyDll\Debug
	char file_path[] = "C:\\My_works\\nixiang_220120\\dyDll\\Debug\\dyDll.dll";
	// exe->filebuffer
	DWORD ret1 = ReadPEFile(file_path, &pFileBuffer);  // &pFileBuffer(void**类型) 传递地址对其值可以进行修改
	if (!ret1) {
		printf("打开文件失败/n");
		return;
	}
	//打印导出表
	DWORD ret = print_exportTable(pFileBuffer);
	if (!ret) {
		printf("打印导出表失败/n");
		return;
	}
	printf("=================================================\n");
	LPSTR funName = "_Plus@8";
	DWORD adress = GetFunctionAddrByName(pFileBuffer, funName);
	printf("=================================================\n");
	DWORD Ord = 1;
	GetFunctionAddrByOrdinals(pFileBuffer, Ord);
	free(pFileBuffer);
	return;
}
//*****************************************************************************0205__p50重定位表
//打印重定位表所有数据
DWORD print_relocationTable(IN PVOID pFileBuffer) {
	// 初始化PE头部结构体
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	PIMAGE_EXPORT_DIRECTORY pExportDirectory = NULL;
	//判断exe->内存是否成功
	if (!pFileBuffer)
	{
		printf("读取到内存的pfilebuffer失败！\n");
		return 0;
	}
	// 判断是否是可执行文件
	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)  // IMAGE_DOS_SIGNATURE是4字节，将pFileBuffer强制类型转换为4字节指针类型（PWORD）
	{
		printf("不含MZ标志，不是exe文件！\n");
		return 0;
	}
	//强制结构体类型转换pDosHeader
	pDosHeader = PIMAGE_DOS_HEADER(pFileBuffer);
	//判断是否含有PE标志       
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("不是有效的PE标志！\n");
		return 0;
	}
	// 强制结构体类型转换
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	pDataDirectory = (PIMAGE_DATA_DIRECTORY)pOptionHeader->DataDirectory;
	if (!(pDataDirectory + 5)->VirtualAddress) {
		printf("重定位表不存在\n");
		return 0;
	}
	//使指针指向重定位表
	DWORD Foa_relocationTable = RvaToFileOffset(pFileBuffer, (pDataDirectory + 5)->VirtualAddress);
	PIMAGE_BASE_RELOCATION pRelocationTable = (PIMAGE_BASE_RELOCATION)((DWORD)pFileBuffer + Foa_relocationTable);
	//打印重定位表数据
	for (int i = 0; pRelocationTable->SizeOfBlock != 0 && pRelocationTable->VirtualAddress != 0; i++, pRelocationTable = (PIMAGE_BASE_RELOCATION)((DWORD)pRelocationTable + pRelocationTable->SizeOfBlock)) {
		printf("===================================================第%d块\n", i);
		printf("VirtualAddress:%#x\n", pRelocationTable->VirtualAddress);
		printf("SizeOfBlock:%#x\n", pRelocationTable->SizeOfBlock);
		DWORD num = (pRelocationTable->SizeOfBlock - 8) / 2;
		printf("当前块的具体项数量:%d\n", num);
		printf("需要修改的项如下:\n");
		PWORD pData = (PWORD)((DWORD)pRelocationTable + 8);
		for (DWORD k = 0; k < num; k++, pData++) {
			if (*pData & 0xF000 == 0x3000) {
				printf("第%d项:%#x\n", k, *pData & 0x0fff);
			}
		}
	}
	return 1;
}

//重定位表外函数
void operate_relocationTable(void) {
	PVOID pFileBuffer = NULL;
	//char file_path[] = "C:\\software__\\IPMSG2007.exe";			
	//char file_path[] = "C:\\WINDOWS\\system32\\notepad.exe";	C:\My_works\nixiang_220120\dyDll\Debug
	char file_path[] = "C:\\My_works\\nixiang_220120\\dyDll\\Debug\\dyDll.dll";
	// exe->filebuffer
	DWORD ret1 = ReadPEFile(file_path, &pFileBuffer);
	if (!ret1) {
		printf("打开文件失败/n");
		return;
	}
	//打印导出表
	DWORD ret = print_relocationTable(pFileBuffer);
	if (!ret) {
		printf("打印重定位表失败/n");
		return;
	}

	free(pFileBuffer);
	return;
}

//*****************************************************************************0205__p51移动导出表、重定位表
//按size的大小新增一个节，并返回新节的FOA,若新增节失败，返回0
DWORD addSection_returnFoa(IN PVOID pFileBuffer, OUT PVOID* ppNewFileBuffer, IN size_t size) {
	size_t size_NewBuffer = 0;
	// 初始化PE头部结构体
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;


	// 判断pFileBuffer是否有效
	if (!pFileBuffer)
	{
		printf("(addSection阶段)pImageBuffer指针无效\n");
		return 0;
	}
	//判断是不是exe文件
	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(addSection阶段)不含MZ标志！\n");
		return 0;
	}
	// 强制结构体类型转换
	pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	if (*((PDWORD)((DWORD)pDosHeader + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(addSection阶段)不是有效的PE标志！\n");
		return 0;
	}
	// 强制结构体类型转换
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // 这里必须强制类型转换
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);

	PIMAGE_SECTION_HEADER pSectionTemp = pSectionHeader;
	//判断最后一个节表后是否还有两个节表的空间（0x50）
	for (int i = 0; i < pPEHeader->NumberOfSections; i++) {
		pSectionTemp++;
	}
	if ((pOptionHeader->SizeOfHeaders - ((DWORD)pSectionTemp + 0x28)) < addSection_needSpace || !isAllZero((PVOID)((DWORD)pSectionTemp + 0x28), (PVOID)((DWORD)pSectionTemp + 0x78))) {
		printf("添加新节表所需空间不足，需进行节表前移或扩大节\n");
		//moveHeaders()   exSection()
		return 0;
	}

	printf("添加新节表所需空间充足\n");
	//申请newpFileBuffer空间
	size_t size_FileBuffer = (pSectionHeader + pPEHeader->NumberOfSections - 1)->PointerToRawData + (pSectionHeader + pPEHeader->NumberOfSections - 1)->SizeOfRawData;
	size_NewBuffer = Align(size, pOptionHeader->SectionAlignment) + size_FileBuffer;
	PVOID pTempNewFileBuffer = malloc(size_NewBuffer);
	if (!pTempNewFileBuffer) {
		printf("(addSection阶段)分配NewFileBuffer内存空间失败\n");
		return 0;
	}
	memset(pTempNewFileBuffer, 0, size_NewBuffer);
	memcpy(pTempNewFileBuffer, pFileBuffer, size_FileBuffer);
	//这里可对新节进行赋值

	// 将头部指针指向NewFileBuffer的对应位置
	PIMAGE_DOS_HEADER NewpDosHeader = (PIMAGE_DOS_HEADER)pTempNewFileBuffer;
	PIMAGE_NT_HEADERS NewpNTHeader = (PIMAGE_NT_HEADERS)((DWORD)NewpDosHeader + NewpDosHeader->e_lfanew);
	PIMAGE_FILE_HEADER NewpPEHeader = (PIMAGE_FILE_HEADER)((DWORD)NewpNTHeader + 4);
	PIMAGE_OPTIONAL_HEADER NewpOptionHeader = (PIMAGE_OPTIONAL_HEADER)((DWORD)NewpPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	PIMAGE_SECTION_HEADER NewpSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)NewpOptionHeader + NewpPEHeader->SizeOfOptionalHeader);

	//进行添加新节操作
	//PIMAGE_SECTION_HEADER newSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)(pSectionTemp+1) - (DWORD)pImageBuffer + (DWORD)pTempNewImageBuffer);  //该指针指向newImageBuffer的新增节表位置
	PIMAGE_SECTION_HEADER newSectionHeader = NewpSectionHeader;
	newSectionHeader = newSectionHeader + NewpPEHeader->NumberOfSections;
	memcpy(newSectionHeader, NewpSectionHeader, 0x28);
	memset(newSectionHeader + 1, 0, 0x28);       //规定最后一节之后要有一个节表的全0


	//修改头部信息
	NewpOptionHeader->SizeOfImage = NewpOptionHeader->SizeOfImage + Align(size, pOptionHeader->SectionAlignment);
	NewpPEHeader->NumberOfSections++;
	//修改新节表信息
	//newSectionHeader->Name = ".haha\0";
	memcpy(newSectionHeader, ".relo", 8);
	newSectionHeader->SizeOfRawData = Align(size, pOptionHeader->SectionAlignment);
	newSectionHeader->Misc.VirtualSize = Align(size, pOptionHeader->SectionAlignment);
	pSectionTemp = newSectionHeader - 1;
	newSectionHeader->PointerToRawData = pSectionTemp->PointerToRawData + pSectionTemp->SizeOfRawData;

	DWORD max = pSectionTemp->SizeOfRawData;
	if (pSectionTemp->Misc.VirtualSize > pSectionTemp->SizeOfRawData)max = pSectionTemp->Misc.VirtualSize;
	DWORD size1 = Align(max, NewpOptionHeader->SectionAlignment);
	newSectionHeader->VirtualAddress = size1 + pSectionTemp->VirtualAddress;

	*ppNewFileBuffer = pTempNewFileBuffer;
	pTempNewFileBuffer = NULL;
	DWORD foa = newSectionHeader->PointerToRawData;
	return foa;
}

//移动重定位表,失败返回0，成功返回新文件大小
DWORD move_relocationTable(IN PVOID pFileBuffer, OUT PVOID* ppNewFileBuffer) {
	// 初始化PE头部结构体
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	PIMAGE_BASE_RELOCATION pRelocationTable = NULL;

	PVOID pTempNewFileBuffer = NULL;
	//判断exe->内存是否成功
	if (!pFileBuffer)
	{
		printf("读取到内存的pfilebuffer失败！\n");
		return 0;
	}

	// 判断是否是可执行文件
	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)  // IMAGE_DOS_SIGNATURE是4字节，将pFileBuffer强制类型转换为4字节指针类型（PWORD）
	{
		printf("不含MZ标志！\n");
		return 0;
	}
	//强制结构体类型转换pDosHeader
	pDosHeader = PIMAGE_DOS_HEADER(pFileBuffer);
	//判断是否含有PE标志       
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("不是有效的PE标志！\n");
		return 0;
	}
	// 强制结构体类型转换
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	pDataDirectory = (PIMAGE_DATA_DIRECTORY)pOptionHeader->DataDirectory;
	if (!(pDataDirectory + 5)->VirtualAddress) {
		printf("重定位表不存在\n");
		return 0;
	}
	//使指针指向重定位表
	DWORD Foa_relocationTable = RvaToFileOffset(pFileBuffer, (pDataDirectory + 5)->VirtualAddress);
	pRelocationTable = (PIMAGE_BASE_RELOCATION)((DWORD)pFileBuffer + Foa_relocationTable);
	PIMAGE_BASE_RELOCATION pTempRelocationTable = pRelocationTable;

	//计算重定位表大小
	size_t size_RelocationTable = 0;
	for (; pTempRelocationTable->SizeOfBlock != 0 && pTempRelocationTable->VirtualAddress != 0; pTempRelocationTable = (PIMAGE_BASE_RELOCATION)((DWORD)pTempRelocationTable + pTempRelocationTable->SizeOfBlock)) {
		size_RelocationTable += pTempRelocationTable->SizeOfBlock;
	}
	size_RelocationTable += 8;		//结束标记占8字节
	//新增节，并返回新节的foa
	DWORD Foa_NewSection = addSection_returnFoa(pFileBuffer, &pTempNewFileBuffer, size_RelocationTable);
	PDWORD pNewSection = (PDWORD)((DWORD)pTempNewFileBuffer + Foa_NewSection);
	//复制重定位表到新节
	//printf("%#x\n",size_RelocationTable);
	memcpy(pNewSection, pRelocationTable, size_RelocationTable);
	//修改op头的目录项地址
	DWORD RvaRelocationTable = 0;
	/*if(*((PWORD)pTempNewFileBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("%s\n",((PWORD)pTempNewFileBuffer));
		return 0;
	}*/
	RvaRelocationTable = FoaToRva(pTempNewFileBuffer, Foa_NewSection);		//注意新旧Buffer指针的使用---------------------------------
	if (!RvaRelocationTable) {
		printf("RelocationTable的VirtualAddress错误\n");
		return 0;
	}
	(pDataDirectory + 5)->VirtualAddress = RvaRelocationTable;

	*ppNewFileBuffer = pTempNewFileBuffer;
	pTempNewFileBuffer = NULL;
	//计算新文件大小
	size_t sizeNewFileBuffer = Align(size_RelocationTable, pOptionHeader->SectionAlignment) + Foa_NewSection;
	return sizeNewFileBuffer;


}


//移动导出表、重定位表外函数
void operate_moveTable(void) {
	PVOID pFileBuffer = NULL;
	PVOID pNewFileBuffer = NULL;
	//char file_path[] = "C:\\software__\\IPMSG2007.exe";			
	//char file_path[] = "C:\\WINDOWS\\system32\\notepad.exe";	C:\My_works\nixiang_220120\dyDll\Debug
	char file_path[] = "C:\\My_works\\nixiang_220120\\dyDll\\Debug\\dyDll.dll";
	char write_file_path[] = "C:\\My_works\\nixiang_220120\\dyDll\\Debug\\dyDll_move.dll";
	// exe->filebuffer
	DWORD ret1 = ReadPEFile(file_path, &pFileBuffer);
	if (!ret1) {
		printf("打开文件失败/n");
		return;
	}
	//移动重定位表
	DWORD size_NewFileBuffer = move_relocationTable(pFileBuffer, &pNewFileBuffer);
	if (!size_NewFileBuffer) {
		printf("移动重定位表失败/n");
		return;
	}

	//存盘
	bool ret4 = MemeryTOFile(pNewFileBuffer, size_NewFileBuffer, write_file_path);
	if (ret4) {
		printf("写入到新文件成功！");
	}
	free(pFileBuffer);
	return;
}
//*****************************************************************************0206_____p52/53导入表、iat表
//打印导入表，失败返回0，成功返回dll个数
DWORD print_importTable(IN PVOID pFileBuffer) {
	// 初始化PE头部结构体
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	PIMAGE_IMPORT_DESCRIPTOR pImportTable = NULL;

	//判断exe->内存是否成功
	if (!pFileBuffer)
	{
		printf("读取到内存的pfilebuffer失败！\n");
		return 0;
	}

	// 判断是否是可执行文件
	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)  // IMAGE_DOS_SIGNATURE是4字节，将pFileBuffer强制类型转换为4字节指针类型（PWORD）
	{
		printf("不含MZ标志！\n");
		return 0;
	}
	//强制结构体类型转换pDosHeader
	pDosHeader = PIMAGE_DOS_HEADER(pFileBuffer);
	//判断是否含有PE标志       
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("不是有效的PE标志！\n");
		return 0;
	}
	// 强制结构体类型转换
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	pDataDirectory = (PIMAGE_DATA_DIRECTORY)pOptionHeader->DataDirectory;
	if (!(pDataDirectory + 1)->VirtualAddress) {
		printf("导入表不存在\n");
		return 0;
	}
	//使指针指向导入表
	DWORD Foa_importTable = RvaToFileOffset(pFileBuffer, (pDataDirectory + 1)->VirtualAddress);
	pImportTable = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)pFileBuffer + Foa_importTable);
	PIMAGE_IMPORT_DESCRIPTOR pTempImportTable = pImportTable;
	//对所有调用的dll进行遍历
	for (int i = 0; !isAllZero((PVOID)pTempImportTable, (PVOID)((DWORD)pTempImportTable + sizeof(IMAGE_IMPORT_DESCRIPTOR) - 1)); pTempImportTable++, i++) {

		printf("***************************************************第%d个dll\n", i + 1);
		DWORD foa_name = RvaToFileOffset(pFileBuffer, pTempImportTable->Name);
		PDWORD name = (PDWORD)((DWORD)pFileBuffer + foa_name);
		DWORD foa_OriginalFirstThunk = RvaToFileOffset(pFileBuffer, pTempImportTable->OriginalFirstThunk);
		PDWORD pOriginalFirstThunk = (PDWORD)((DWORD)pFileBuffer + foa_OriginalFirstThunk);
		DWORD foa_FirstThunk = RvaToFileOffset(pFileBuffer, pTempImportTable->FirstThunk);
		PDWORD pFirstThunk = (PDWORD)((DWORD)pFileBuffer + foa_FirstThunk);
		printf("dll名字:%s\n", name);
		printf("===================================================OriginalFirstThunk表\n");
		for (int j = 0; *pOriginalFirstThunk != 0; j++, pOriginalFirstThunk++) {

			if (*pOriginalFirstThunk & 0x80000000 == 0x80000000) {
				printf("第%d个函数的序号为:%#x\n", j + 1, *pOriginalFirstThunk & 0x0fff);
			}
			else {
				DWORD foa_funtionName = RvaToFileOffset(pFileBuffer, *pOriginalFirstThunk);
				PIMAGE_IMPORT_BY_NAME pFuntionName = (PIMAGE_IMPORT_BY_NAME)((DWORD)pFileBuffer + foa_funtionName);
				printf("第%d个函数的名字为:%s\n", j + 1, pFuntionName->Name);
			}
		}
		printf("===================================================FirstThunk表\n");
		for (int k = 0; *pFirstThunk != 0; k++, pFirstThunk++) {
			if (*pFirstThunk & 0x80000000 == 0x80000000) {
				printf("第%d个函数的序号为:%#x\n", k + 1, *pFirstThunk & 0x0fff);
			}
			else {
				DWORD foa_funtionName = RvaToFileOffset(pFileBuffer, *pFirstThunk);
				PIMAGE_IMPORT_BY_NAME pFuntionName = (PIMAGE_IMPORT_BY_NAME)((DWORD)pFileBuffer + foa_funtionName);
				printf("第%d个函数的名字为:%s\n", k + 1, pFuntionName->Name);
			}
		}
	}
	return i;
}

void operate_importTable(void) {
	PVOID pFileBuffer = NULL;
	//PVOID pNewFileBuffer = NULL;
	char file_path[] = "C:\\software__\\IPMSG2007.exe";
	//char file_path[] = "C:\\WINDOWS\\system32\\notepad.exe";
	//char file_path[] = "C:\\My_works\\nixiang_220120\\dyDll\\Debug\\dyDll.dll";
	//char write_file_path[] = "C:\\My_works\\IPMSG2007.exe";
	// exe->filebuffer
	DWORD ret1 = ReadPEFile(file_path, &pFileBuffer);
	if (!ret1) {
		printf("打开文件失败/n");
		return;
	}
	//打印导入表
	DWORD ret2 = print_importTable(pFileBuffer);
	if (!ret2) {
		printf("打印导入表失败/n");
		return;
	}
	else {
		printf("共有%d个dll\n", ret2);
	}

	/*//存盘
	bool ret4 = MemeryTOFile(pNewFileBuffer,size_NewFileBuffer, write_file_path);
	if(ret4){
		printf("写入到新文件成功！");
	}*/
	free(pFileBuffer);
	return;
}
//*****************************************************************************0207__p55导入表注入
DWORD add_importTable(IN PVOID pFileBuffer, IN PVOID* ppNewFileBuffer, IN LPSTR dllName) {
	// 初始化PE头部结构体
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	PIMAGE_IMPORT_DESCRIPTOR pImportTable = NULL;
	
	PVOID pTempNewFileBuffer = NULL;
	//判断exe->内存是否成功
	if (!pFileBuffer)
	{
		printf("读取到内存的pfilebuffer失败！\n");
		return 0;
	}

	// 判断是否是可执行文件
	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)  // IMAGE_DOS_SIGNATURE是4字节，将pFileBuffer强制类型转换为4字节指针类型（PWORD）
	{
		printf("不含MZ标志！\n");
		return 0;
	}
	//强制结构体类型转换pDosHeader
	pDosHeader = PIMAGE_DOS_HEADER(pFileBuffer);
	//判断是否含有PE标志       
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("不是有效的PE标志！\n");
		return 0;
	}
	// 强制结构体类型转换
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	pDataDirectory = (PIMAGE_DATA_DIRECTORY)pOptionHeader->DataDirectory;
	if (!(pDataDirectory + 1)->VirtualAddress) {
		printf("导入表不存在\n");
		return 0;
	}
	//使指针指向导入表
	DWORD Foa_importTable = RvaToFileOffset(pFileBuffer, (pDataDirectory + 1)->VirtualAddress);
	pImportTable = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)pFileBuffer + Foa_importTable);
	PIMAGE_IMPORT_DESCRIPTOR pTempImportTable = pImportTable;
	//判断(pDataDirectory + 1)->Size是否有效
	if (!(pDataDirectory + 1)->Size) {
		printf("(pDataDirectory + 1)->Size无效，需计算导入表大小\n");
		return 0;
	}
	size_t size_ImportTable = (pDataDirectory + 1)->Size;  //结束标志为sizeof(IMAGE_IMPORT_DESCRIPTOR)大小的全0
	size_t num_ImportTable = size_ImportTable/sizeof(IMAGE_IMPORT_DESCRIPTOR) - 1;
	size_t num_NewImportTable = num_ImportTable + 1;
	/*int i = 0;
	while(*pTempImportTable){
		i++;
		pTempImportTable++;
	}
	size_t num_ImportTable = i;
	size_t num_NewImportTable = num_ImportTable + 1;
	size_t size_ImportTable = sizeof(IMAGE_IMPORT_DESCRIPTOR) * (num_ImportTable + 1);  //结束标志为sizeof(IMAGE_IMPORT_DESCRIPTOR)大小的全0*/
	//一般情况导入表没有可以真假新表的空间，故不判断直接将旧表迁移到新节来建立新导入表
	//建立新节
	size_t size_NewImportTable = size_ImportTable + sizeof(IMAGE_IMPORT_DESCRIPTOR);
	DWORD foa_NewSection = addSection_returnFoa(pFileBuffer, &pTempNewFileBuffer, size_NewImportTable + 0x30);
	if (!foa_NewSection) {
		printf("建立新节失败/n");
		return 0;
	}
	//获得新节的指针
	PVOID pNewSection = (PVOID)((DWORD)pTempNewFileBuffer + foa_NewSection);
	//将导入表复制到新节
	memcpy(pNewSection, pImportTable, size_ImportTable);
	//修正目录项
	DWORD rva_NewSection = FoaToRva(pTempNewFileBuffer, foa_NewSection);
	(pDataDirectory + 1)->VirtualAddress = rva_NewSection;
	//建立新dll的int表、iat表、函数名称表
	PIMAGE_THUNK_DATA pNewINT = (PIMAGE_THUNK_DATA)((DWORD)pNewSection + sizeof(IMAGE_IMPORT_DESCRIPTOR) * (num_NewImportTable + 1));  //+1是结束标志
	PIMAGE_THUNK_DATA pNewIAT = pNewINT + 2;		//新dll只有1个函数
	PIMAGE_IMPORT_BY_NAME pNewNameTable = (PIMAGE_IMPORT_BY_NAME)(pNewIAT + 2);
	pNewNameTable->Hint = 0;
	size_t len_dllName = strlen(dllName) + 1;
	memcpy(&(pNewNameTable->Name), dllName, len_dllName);		//将函数名填入函数名称表
	DWORD rva_NewNameTable = FoaToRva(pTempNewFileBuffer, ((DWORD)pNewNameTable - (DWORD)pTempNewFileBuffer));
	pNewINT->u1.AddressOfData = (_IMAGE_IMPORT_BY_NAME *)rva_NewNameTable;
	pNewIAT->u1.AddressOfData = (_IMAGE_IMPORT_BY_NAME *)rva_NewNameTable;

	//修改新导入表参数
	PIMAGE_IMPORT_DESCRIPTOR pNewImportTable = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)pNewSection + sizeof(IMAGE_IMPORT_DESCRIPTOR) * num_ImportTable);
	DWORD rva_NewINT = FoaToRva(pTempNewFileBuffer, ((DWORD)pNewINT - (DWORD)pTempNewFileBuffer));
	DWORD rva_NewIAT = FoaToRva(pTempNewFileBuffer, ((DWORD)pNewIAT - (DWORD)pTempNewFileBuffer));
	pNewImportTable->OriginalFirstThunk = rva_NewINT;
	pNewImportTable->FirstThunk = rva_NewIAT;
	pNewImportTable->Name = FoaToRva(pTempNewFileBuffer, ((DWORD)&(pNewNameTable->Name) - (DWORD)pTempNewFileBuffer));
	pNewImportTable->TimeDateStamp = 0;

	*ppNewFileBuffer = pTempNewFileBuffer;
	pTempNewFileBuffer = NULL;
	return 1;
}




void operate_AddImportTable(void) {
	PVOID pFileBuffer = NULL;
	PVOID pNewFileBuffer = NULL;
	char file_path[] = "C:\\software__\\IPMSG2007.exe";
	//char file_path[] = "C:\\WINDOWS\\system32\\notepad.exe";
	char dll_path[] = "C:\\My_works\\InjectDll\\Debug\\InjectDll.dll";
	char write_file_path[] = "C:\\My_works\\add_IPMSG2007.exe";
	char dllname[] = "inj.dll";
	// exe->filebuffer
	DWORD ret1 = ReadPEFile(file_path, &pFileBuffer);
	if (!ret1) {
		printf("打开文件失败/n");
		return;
	}
	//新增导入表
	DWORD size_NewFileBuffer = add_importTable(pFileBuffer, &pNewFileBuffer,dllname);
	if (!size_NewFileBuffer) {
		printf("新增导入表失败/n");
		return;
	}
	else {
		printf("新文件大小:%#x\n", size_NewFileBuffer);
	}

	//存盘
	bool ret4 = MemeryTOFile(pNewFileBuffer, size_NewFileBuffer, write_file_path);
	if (ret4) {
		printf("写入到新文件成功！");
	}
	free(pFileBuffer);
	free(pNewFileBuffer);
	return;
}

//*****************************************************************************
//rva转foa，失败返回0，成功返回foa
DWORD RvaToFoa(IN PVOID pFileBuffer,OUT DWORD dwRva){
	// 初始化PE头部结构体
	PIMAGE_DOS_HEADER pDosHeader = NULL;	
	PIMAGE_NT_HEADERS pNTHeader = NULL;	
	PIMAGE_FILE_HEADER pPEHeader = NULL;	
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	
	// 判断pFileBuffer是否有效
	if(!pFileBuffer)
	{
		printf("(RVA->FOA阶段)pFileBuffer指针无效\n");
		return 0;
	}
	//判断是不是exe文件
	if(*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(RVA->FOA阶段)不含MZ标志！\n");
		return 0;
	}
	// 强制结构体类型转换
	pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	if(*((PDWORD)((DWORD)pFileBuffer+pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(RVA->FOA阶段)不是有效的PE标志！\n");	
		return 0;
	}
	// 强制结构体类型转换
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pFileBuffer+pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader+4); // 这里必须强制类型转换
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader+IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader+pPEHeader->SizeOfOptionalHeader);
	PIMAGE_SECTION_HEADER tempSection = pSectionHeader;
	if(dwRva <= pOptionHeader->SizeOfHeaders){
		return dwRva;
	}else{
		for(int i=0;i<pPEHeader->NumberOfSections;i++,tempSection++){
			if(dwRva >= tempSection->VirtualAddress && dwRva <= tempSection->VirtualAddress + tempSection->Misc.VirtualSize){
				return dwRva - tempSection->VirtualAddress + tempSection->PointerToRawData;
			}
		}
	}
	printf("rva->foa失败\n");
	return 0;

}

//*****************************************************************************打印资源表中所有资源，失败返回0
DWORD print_resourceTable(IN PVOID pFileBuffer){
	// 初始化PE头部结构体
	PIMAGE_DOS_HEADER pDosHeader = NULL;	
	PIMAGE_NT_HEADERS pNTHeader = NULL;	
	PIMAGE_FILE_HEADER pPEHeader = NULL;	
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY DataDirectory = NULL;
	PIMAGE_RESOURCE_DIRECTORY pResourceDirectory = NULL;
	
	// 判断pImageBuffer是否有效
	if(!pFileBuffer)
	{
		printf("(打印阶段)pFileBuffer指针无效\n");
		return 0;
	}
	//判断是不是exe文件
	if(*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(打印阶段)不含MZ标志！\n");
		return 0;
	}
	pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	if(*((PDWORD)((DWORD)pDosHeader + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(打印阶段)不是有效的PE标志！\n");	
		return 0;
	}
	// 强制结构体类型转换
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader+pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader+4); // 这里必须强制类型转换
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER)((DWORD)pPEHeader+IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader+pPEHeader->SizeOfOptionalHeader);
	DWORD rva_ResourceDirectory = pOptionHeader->DataDirectory[2].VirtualAddress;
	pResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)((DWORD)pDosHeader + RvaToFoa(pDosHeader,rva_ResourceDirectory));
	//打印资源表内容
	PIMAGE_RESOURCE_DIRECTORY pTempResourceDirectory_1 = pResourceDirectory;
	PIMAGE_RESOURCE_DIRECTORY pTempResourceDirectory_2 = NULL;
	PIMAGE_RESOURCE_DIRECTORY pTempResourceDirectory_3 = NULL;
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pTempResourceDirectoryEntry_1 = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pTempResourceDirectory_1 + 1);
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pTempResourceDirectoryEntry_2 = NULL;
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pTempResourceDirectoryEntry_3 = NULL;
	PIMAGE_RESOURCE_DIR_STRING_U pNameStringDir = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	for(int i=0;i<pTempResourceDirectory_1->NumberOfIdEntries + pTempResourceDirectory_1->NumberOfNamedEntries;i++,pTempResourceDirectoryEntry_1++){
		printf("*********************************类型%d\n",i+1);
		//打印类型名
		printf("---第一层参数:\n");
		if(pTempResourceDirectoryEntry_1->NameIsString){			//第一第二层其实可以不判断最高位(必为1)
			pNameStringDir = (PIMAGE_RESOURCE_DIR_STRING_U)pTempResourceDirectoryEntry_1->NameOffset;
			printf("类型(名字):%*s\n",pNameStringDir->Length,pNameStringDir->NameString);
		}else{
			printf("类型(编号):%#x\n",pTempResourceDirectoryEntry_1->NameOffset);
		}
		//进入第2层
		pTempResourceDirectory_2 = (PIMAGE_RESOURCE_DIRECTORY)(pTempResourceDirectoryEntry_1->OffsetToDirectory + (DWORD)pResourceDirectory);
		pTempResourceDirectoryEntry_2 = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pTempResourceDirectory_2 + 1);
		for(int j=0;j<pTempResourceDirectory_2->NumberOfIdEntries + pTempResourceDirectory_2->NumberOfNamedEntries;j++,pTempResourceDirectoryEntry_2++){
			//这里可打印第二层参数
			//进入第3层
			pTempResourceDirectory_3 = (PIMAGE_RESOURCE_DIRECTORY)(pTempResourceDirectoryEntry_2->OffsetToDirectory + (DWORD)pResourceDirectory);
			pTempResourceDirectoryEntry_3 = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pTempResourceDirectory_3 + 1);
			//打印第三层参数
			printf("---第三层参数(第%d个):\n",j+1);
			printf("总数量:%d\n",pTempResourceDirectory_3->NumberOfIdEntries + pTempResourceDirectory_3->NumberOfNamedEntries);
			for(int j=0;j<pTempResourceDirectory_3->NumberOfIdEntries + pTempResourceDirectory_3->NumberOfNamedEntries;j++,pTempResourceDirectoryEntry_3++){
				if(pTempResourceDirectoryEntry_3->NameIsString){			
					pNameStringDir = (PIMAGE_RESOURCE_DIR_STRING_U)pTempResourceDirectoryEntry_3->NameOffset;
					printf("代码页(名字):%*s\n",pNameStringDir->Length,pNameStringDir->NameString);
				}else{
					printf("代码页(编号):%#x\n",pTempResourceDirectoryEntry_3->NameOffset);
				}
				if(pTempResourceDirectoryEntry_3->DataIsDirectory){
					printf("有第四层\n");
					return 0;
				}else{
					pDataDirectory = (PIMAGE_DATA_DIRECTORY)(pTempResourceDirectoryEntry_3->OffsetToData + (DWORD)pResourceDirectory);
					printf("资源数据RVA:%#x  SIZE:%#x\n",pDataDirectory->VirtualAddress,pDataDirectory->Size);
				}
			}
		}
	}
	return 1;
}
//*****************************************************************************
// int main()
// {
// 	//operate_pe();
// 	//rva_foa_transf();
// 	//add_code();
// 	//expandOrMerge_Section();
// 	//operate_exprotTable();
// 	//operate_relocationTable();
// 	//operate_moveTable();
// 	//operate_importTable();
// 	operate_importTable();
// 	//operate_AddImportTable();
// 	getchar();
// 	return 0;
// }
