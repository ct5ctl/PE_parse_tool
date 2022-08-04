#include "stdafx.h"
#include "DIALOG_Main.h"


// exe->filebuffer  ����ֵΪ���������ļ���С
DWORD ReadPEFile(IN LPSTR lpszFile, OUT LPVOID* ppFileBuffer) {
	FILE* pFile = NULL;  // �ļ�ָ��
	DWORD file_size = 0;
	LPVOID pTempFilebuffer = NULL;

	// ���ļ�
	pFile = fopen(lpszFile, "rb");  // ������µ�ָ�룬��Ҫ�����ж�
	if (!pFile)
	{
		printf("��exe�ļ�ʧ�ܣ�\n");//�������ʧ�ܾ�Ҫ�ر��ļ����ͷŶ�̬�ڴ桢ָ��ָ��NULL
		return 0;
	}
	// ��ȡ�ļ���С
	fseek(pFile, 0, SEEK_END);
	file_size = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);
	// ����ռ�
	pTempFilebuffer = malloc(file_size);  // ������µ�ָ�룬��Ҫ�����ж�
	if (!pTempFilebuffer)
	{
		printf("����ռ�ʧ�ܣ�\n");//�������ʧ�ܾ�Ҫ�ر��ļ����ͷŶ�̬�ڴ桢ָ��ָ��NULL
		fclose(pFile);
		return 0;
	}
	// �����ݶ�ȡ���ڴ���
	size_t n = fread(pTempFilebuffer, file_size, 1, pFile);
	if (!n)
	{
		printf("���ݶ�ȡ���ڴ���ʧ�ܣ�\n"); //�������ʧ�ܾ�Ҫ�ر��ļ����ͷŶ�̬�ڴ桢ָ��ָ��NULL
		fclose(pFile);
		free(pTempFilebuffer);
		return 0;
	}
	// �ر��ļ�(�Ѿ���ȡ���ڴ���)
	*ppFileBuffer = pTempFilebuffer;
	pTempFilebuffer = NULL;
	fclose(pFile);
	return file_size;
}

// filebuffer -> imagebuffer
DWORD CopyFileBufferToImageBuffer(IN LPVOID pFileBuffer, OUT LPVOID* ppImageBuffer) {
	// ��ʼ��PEͷ���ṹ��
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	// ��ʼ��IMAGE_BUFFERָ��(temparay)
	LPVOID pTempImagebuffer = NULL;
	//�ж�exe->�ڴ��Ƿ�ɹ�
	if (!pFileBuffer)
	{
		printf("(2pimagebuffer�׶�)��ȡ���ڴ��pfilebuffer��Ч��\n");
		return 0;
	}
	// �ж��Ƿ��ǿ�ִ���ļ�
	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)  // IMAGE_DOS_SIGNATURE��4�ֽڣ���pFileBufferǿ������ת��Ϊ4�ֽ�ָ�����ͣ�PWORD��
	{
		printf("(2pimagebuffer�׶�)����MZ��־������exe�ļ���\n");
		return 0;
	}
	//ǿ�ƽṹ������ת��pDosHeader
	pDosHeader = PIMAGE_DOS_HEADER(pFileBuffer);
	//�ж��Ƿ���PE��־       
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE) // ע��ָ��ļӷ��ǣ�ȥ��һ��*���������ӡ�����ת��ΪDWORD�����ټӼ���
	{																			  //��Ӻ�ĺ� ǿ������ת��Ϊ4�ֽ�ָ�����ͣ�PWORD�� IMAGE_NT_SIGNATURE 4BYTES
		printf("(2pimagebuffer�׶�)������Ч��PE��־��\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pFileBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);

	// ���䶯̬�ڴ�
	pTempImagebuffer = malloc(pOptionHeader->SizeOfImage);
	if (!pTempImagebuffer)
	{
		printf("���䶯̬�ڴ�ʧ�ܣ�\n");
		free(pTempImagebuffer);
		return 0;
	}
	//��ʼ����������ڴ�ռ�
	memset(pTempImagebuffer, 0, pOptionHeader->SizeOfImage);
	//����ͷ�ͽڱ�imagebuffer
	memcpy(pTempImagebuffer, pFileBuffer, pOptionHeader->SizeOfHeaders);
	//�����ڰ����뷽ʽ���Ƶ�ImageBuffer
	PIMAGE_SECTION_HEADER sectionTemp = pSectionHeader;
	for (int i = 0; i < pPEHeader->NumberOfSections; i++, sectionTemp++) {
		memcpy((PIMAGE_SECTION_HEADER)((DWORD)pTempImagebuffer + sectionTemp->VirtualAddress), (PIMAGE_SECTION_HEADER)((DWORD)pFileBuffer + sectionTemp->PointerToRawData), sectionTemp->SizeOfRawData);
	}			//����ֻҪ��ָ����У����ù�ת����ʲô���͵�ָ�루ֻ��λ�ã�

	*ppImageBuffer = pTempImagebuffer;
	pTempImagebuffer = NULL;    //��дҲ�У���Ϊ�Ǿֲ�����
	return pOptionHeader->SizeOfImage;
}

//imagebuffer->newbuffer
DWORD CopyImageBufferToNewBuffer(IN LPVOID pImageBuffer, OUT LPVOID* ppNewBuffer) {
	// ��ʼ��PEͷ���ṹ��
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;

	// ��ʼ��NEW_BUFFERָ��(temparay)
	LPVOID pTempNewbuffer = NULL;

	// �ж�pImageBuffer�Ƿ���Ч
	if (!pImageBuffer)
	{
		printf("(2pnewbuffer�׶�)��ȡ���ڴ��pimagebuffer��Ч��\n");
		return 0;
	}
	//�ж��ǲ���exe�ļ�
	if (*((PWORD)pImageBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(2pnewbuffer�׶�)����MZ��־������exe�ļ���\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pDosHeader = (PIMAGE_DOS_HEADER)pImageBuffer;
	if (*((PDWORD)((DWORD)pImageBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(2pnewbuffer�׶�)������Ч��PE��־��\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pImageBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // �������ǿ������ת��
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);

	//��ȡnew_buffer�Ĵ�С
	int new_buffer_size = pOptionHeader->SizeOfHeaders;
	for (DWORD i = 0; i < pPEHeader->NumberOfSections; i++)
	{
		new_buffer_size += pSectionHeader[i].SizeOfRawData;  // pSectionHeader[i]��һ�ּӷ�
	}
	// �����ڴ棨newbuffer��
	pTempNewbuffer = malloc(new_buffer_size);
	if (!pTempNewbuffer)
	{
		printf("(2pnewbuffer�׶�)����Newbufferʧ�ܣ�\n");
		return 0;
	}
	memset(pTempNewbuffer, 0, new_buffer_size);
	// ����ͷ��
	memcpy(pTempNewbuffer, pDosHeader, pOptionHeader->SizeOfHeaders);
	// ѭ����������
	PIMAGE_SECTION_HEADER pTempSectionHeader = pSectionHeader;
	for (DWORD j = 0; j < pPEHeader->NumberOfSections; j++, pTempSectionHeader++)
	{	//PointerToRawData�������ļ��е�ƫ��,VirtualAddress�������ڴ��е�ƫ�Ƶ�ַ,SizeOfRawData�����ļ��ж����ĳߴ�
		memcpy((PDWORD)((DWORD)pTempNewbuffer + pTempSectionHeader->PointerToRawData), (PDWORD)((DWORD)pImageBuffer + pTempSectionHeader->VirtualAddress), pTempSectionHeader->SizeOfRawData);
	}
	//��������
	*ppNewBuffer = pTempNewbuffer; //�ݴ�����ݴ����������ͷ�
	pTempNewbuffer = NULL;
	return new_buffer_size;  // ���ؼ���õ��ķ����ڴ�Ĵ�С
}


//newbuffer->����
bool MemeryTOFile(IN LPVOID pMemBuffer, IN size_t size, OUT LPSTR lpszFile) {
	FILE* pFile = fopen(lpszFile, "wb");
	if (!pFile) {
		printf("�������ļ�ʧ��/n");
		return false;
	}
	fwrite(pMemBuffer, size, 1, pFile);
	return true;
}

//���ڴ�ƫ��ת��Ϊ�ļ�ƫ��
DWORD RvaToFileOffset(IN LPVOID pFileBuffer, IN DWORD dwRva) {
	// ��ʼ��PEͷ���ṹ��
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;

	// �ж�pFileBuffer�Ƿ���Ч
	if (!pFileBuffer)
	{
		printf("pFileBufferָ����Ч\n");
		return 0;
	}
	//�ж��ǲ���exe�ļ�
	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(RVA->FOA�׶�)����MZ��־������exe�ļ���\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(RVA->FOA�׶�)������Ч��PE��־��\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pFileBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // �������ǿ������ת��
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
	printf("rva->foaʧ��\n");
	return 0;
}

//���ڴ�ƫ��ת��Ϊ�ļ�ƫ��
DWORD FoaToRva(IN PVOID pFileBuffer, IN DWORD Foa) {
	// ��ʼ��PEͷ���ṹ��
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;

	// �ж�pFileBuffer�Ƿ���Ч
	if (!pFileBuffer)
	{
		printf("pFileBufferָ����Ч\n");
		return 0;
	}
	//�ж��ǲ���exe�ļ�
	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(FOA->RVA�׶�)����MZ��־������exe�ļ���\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(FOA->RVA�׶�)������Ч��PE��־��\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pFileBuffer + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // �������ǿ������ת��
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
	printf("foa->rvaʧ��\n");
	return 0;
}

//ִ�к���
void operate_pe()
{   // ��ʼ������
	PVOID pFileBuffer = NULL;
	PVOID pImageBuffer = NULL;
	PVOID pNewFileBuffer = NULL;
	DWORD NewFileBufferSize = 0;
	//char file_path[] = "D:\\Lib\\IPMSG2007.exe";
	char file_path[] = "C:\\Windows\\System32\\notepad.exe";
	char write_file_path[] = "C:\\My_works\\cp_notepad.exe";

	// exe->filebuffer
	DWORD ret1 = ReadPEFile(file_path, &pFileBuffer);  // &pFileBuffer(void**����) ���ݵ�ַ����ֵ���Խ����޸�
	printf("exe->filebuffer  ����ֵΪ���������ļ���С��%#x\n", ret1);
	// filebuffer -> imagebuffer
	DWORD ret2 = CopyFileBufferToImageBuffer(pFileBuffer, &pImageBuffer);
	printf("filebuffer -> imagebuffer����ֵΪ���������ļ���С��%#x\n", ret2);
	//imagebuffer->newbuffer
	DWORD FileSize = CopyImageBufferToNewBuffer(pImageBuffer, &pNewFileBuffer);
	printf("imagebuffer->newbuffer����ֵΪ���������ļ���С��%#x\n", FileSize);
	//newbuffer->����
	bool ret4 = MemeryTOFile(pNewFileBuffer, FileSize, write_file_path);
	if (ret4) {
		printf("д�뵽���ļ��ɹ���");
	}
	//�ͷ�����Ŀռ�
	free(pFileBuffer);
	free(pImageBuffer);
	free(pNewFileBuffer);
}

void rva_foa_transf(void) {
	DWORD Rva = 0x10000;
	// ��ʼ������
	PVOID pFileBuffer = NULL;
	PVOID pImageBuffer = NULL;
	PVOID pNewFileBuffer = NULL;
	DWORD NewFileBufferSize = 0;
	//char file_path[] = "D:\\Lib\\IPMSG2007.exe";
	char file_path[] = "C:\\Windows\\System32\\notepad.exe";
	char write_file_path[] = "C:\\My_works\\cp_notepad.exe";

	// exe->filebuffer
	DWORD ret = ReadPEFile(file_path, &pFileBuffer);  // &pFileBuffer(void**����) ���ݵ�ַ����ֵ���Խ����޸�
	printf("exe->filebuffer  ����ֵΪ���������ļ���С��%#x\n", ret);

	DWORD FOA = RvaToFileOffset(pFileBuffer, Rva);	//ֻ�贫pFileBuffer���ɣ�ӦΪ�������ڴ��е�ʲô״̬����ͷ���ͽڱ���ǲ���ģ���rva->foa��ת��ֻ�迴�ڱ��ƫ�Ƶ�ַ�����Ƿ������޹أ�
	printf("�ļ�ƫ�ƣ�%x\n", FOA);

	//�ͷ�����Ŀռ�
	free(pFileBuffer);
	free(pImageBuffer);
	free(pNewFileBuffer);
}

//*************************************************************************************0201__
#define size_shellCode 0x12
#define MessageBoxAdress 0x77D5050B

DWORD addCodeToCodeSection(PVOID pImageBuffer, BYTE* shellCode) {
	// ��ʼ��PEͷ���ṹ��
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;

	// �ж�pFileBuffer�Ƿ���Ч
	if (!pImageBuffer)
	{
		printf("(addCode�׶�)pImageBufferָ����Ч\n");
		return 0;
	}
	//�ж��ǲ���exe�ļ�
	if (*((PWORD)pImageBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(addCode�׶�)����MZ��־������exe�ļ���\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pDosHeader = (PIMAGE_DOS_HEADER)pImageBuffer;
	if (*((PDWORD)((DWORD)pDosHeader + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(addCode�׶�)������Ч��PE��־��\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // �������ǿ������ת��
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	//�жϴ���ڵĿհ����ռ��Ƿ��㹻
	if ((pSectionHeader->SizeOfRawData - pSectionHeader->Misc.VirtualSize) < size_shellCode) {
		printf("(addCode�׶�)����ڿհ������㣡\n");
		return 0;
	}
	//��Ӵ���
	char* codeBegin = (char*)(pSectionHeader->VirtualAddress + pSectionHeader->Misc.VirtualSize + (DWORD)pImageBuffer);
	memcpy(codeBegin, shellCode, size_shellCode);
	//������ת��ַ����
	DWORD callCode = 0;
	DWORD jmpCode = 0;
	callCode = MessageBoxAdress - ((DWORD)codeBegin + 0XD - (DWORD)pImageBuffer + pOptionHeader->ImageBase);
	//jmpCode = (pOptionHeader->ImageBase + pOptionHeader->AddressOfEntryPoint) - ((DWORD)codeBegin + 0X12 - (DWORD)pImageBuffer + pOptionHeader->ImageBase);
	jmpCode = (pOptionHeader->AddressOfEntryPoint) - ((DWORD)codeBegin + 0X12 - (DWORD)pImageBuffer);
	*(PDWORD)(codeBegin + 9) = callCode;   //������PDWORD �������ܰ�4���ֽ�д��
	*(PDWORD)(codeBegin + 0XE) = jmpCode;
	pOptionHeader->AddressOfEntryPoint = (DWORD)codeBegin - (DWORD)pImageBuffer;  //oep�������ƫ��
	return (DWORD)codeBegin - (DWORD)pImageBuffer + pOptionHeader->ImageBase;
}




void add_code(void) {
	BYTE shellCode[] = {
		0x6A,00,0x6A,00,0x6A,00,0x6A,00,
		0xE8,0x00,0x00,0x00,0x00,
		0xE9,0x00,0x00,0x00,0x00
	};

	// ��ʼ������
	PVOID pFileBuffer = NULL;
	PVOID pImageBuffer = NULL;
	PVOID pNewFileBuffer = NULL;
	DWORD NewFileBufferSize = 0;
	//char file_path[] = "D:\\Lib\\IPMSG2007.exe";
	char file_path[] = "C:\\Windows\\System32\\notepad.exe";
	char write_file_path[] = "C:\\My_works\\add_notepad.exe";

	// exe->filebuffer
	DWORD ret1 = ReadPEFile(file_path, &pFileBuffer);  // &pFileBuffer(void**����) ���ݵ�ַ����ֵ���Խ����޸�
	printf("exe->filebuffer  ����ֵΪ���������ļ���С��%#x\n", ret1);
	// filebuffer -> imagebuffer
	DWORD ret2 = CopyFileBufferToImageBuffer(pFileBuffer, &pImageBuffer);
	printf("filebuffer -> imagebuffer����ֵΪ���������ļ���С��%#x\n", ret2);
	//��Ӵ��뵽����ڿհ���
	DWORD place = addCodeToCodeSection(pImageBuffer, shellCode);
	printf("�´�����ڴ�������λ�ã�%x\n", place);
	//imagebuffer->newbuffer
	DWORD FileSize = CopyImageBufferToNewBuffer(pImageBuffer, &pNewFileBuffer);
	printf("imagebuffer->newbuffer����ֵΪ���������ļ���С��%#x\n", FileSize);
	//newbuffer->����
	bool ret4 = MemeryTOFile(pNewFileBuffer, FileSize, write_file_path);
	if (ret4) {
		printf("д�뵽���ļ��ɹ���");
	}
	//�ͷ�����Ŀռ�
	free(pFileBuffer);
	free(pImageBuffer);
	free(pNewFileBuffer);
}

//***************************************************************************************************************0203___P46

#define addSection_needSpace 0x50
#define Size_NewSection 0x1000
//pFrom��pTo���ֵȫ0����1����֮�򷵻�0
int isAllZero(PVOID pFrom, PVOID pTo) {
	while (pFrom < pTo) {
		if (*(PBYTE)pFrom != 0)return 0;
		pFrom = (PVOID)((DWORD)pFrom + 1);
	}
	return 1;
}


//�Ҷ���ֵ������valueΪ������ݴ�С��SectionAlignmentΪ�ڴ����ֵ������ʵ�ʶ���ֵ
DWORD Align(DWORD value, DWORD SectionAlignment) {
	for (int i = 1; value > SectionAlignment * i; i++);
	return i * SectionAlignment;
}

//�����ֵ
DWORD Max(DWORD A, DWORD B) {
	return A > B ? A : B;
}

//�����ڵ�ִ�к���
DWORD addSection(PVOID pImageBuffer, PVOID* ppNewImageBuffer) {
	size_t size_NewBuffer = 0;
	// ��ʼ��PEͷ���ṹ��
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;


	// �ж�pImageBuffer�Ƿ���Ч
	if (!pImageBuffer)
	{
		printf("(addSection�׶�)pImageBufferָ����Ч\n");
		return 0;
	}
	//�ж��ǲ���exe�ļ�
	if (*((PWORD)pImageBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(addSection�׶�)����MZ��־������exe�ļ���\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pDosHeader = (PIMAGE_DOS_HEADER)pImageBuffer;
	if (*((PDWORD)((DWORD)pDosHeader + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(addSection�׶�)������Ч��PE��־��\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // �������ǿ������ת��
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);

	PIMAGE_SECTION_HEADER pSectionTemp = pSectionHeader;
	//�ж����һ���ڱ���Ƿ��������ڱ�Ŀռ䣨0x50��
	for (int i = 0; i < pPEHeader->NumberOfSections; i++) {
		pSectionTemp++;
	}
	if ((pOptionHeader->SizeOfHeaders - ((DWORD)pSectionTemp + 0x28)) < addSection_needSpace || !isAllZero((PVOID)((DWORD)pSectionTemp + 0x28), (PVOID)((DWORD)pSectionTemp + 0x78))) {
		printf("����½ڱ�����ռ䲻�㣬����нڱ�ǰ�ƻ������\n");
		//moveHeaders()   exSection()
		return 0;
	}

	printf("����½ڱ�����ռ����\n");
	//����newImageBuffer�ռ�
	size_t SizeOfNewImageBuffer = pOptionHeader->SizeOfImage + Size_NewSection;
	PVOID pTempNewImageBuffer = malloc(SizeOfNewImageBuffer);
	if (!pTempNewImageBuffer) {
		printf("(addSection�׶�)����NewImageBuffer�ڴ�ռ�ʧ��\n");
		return 0;
	}
	memset(pTempNewImageBuffer, 0, pOptionHeader->SizeOfImage + 0x1000);
	memcpy(pTempNewImageBuffer, pImageBuffer, pOptionHeader->SizeOfImage);
	//����ɶ��½ڽ��и�ֵ

	// ��ͷ��ָ��ָ��NewImageBuffer�Ķ�Ӧλ��
	PIMAGE_DOS_HEADER NewpDosHeader = (PIMAGE_DOS_HEADER)pTempNewImageBuffer;
	PIMAGE_NT_HEADERS NewpNTHeader = (PIMAGE_NT_HEADERS)((DWORD)NewpDosHeader + NewpDosHeader->e_lfanew);
	PIMAGE_FILE_HEADER NewpPEHeader = (PIMAGE_FILE_HEADER)((DWORD)NewpNTHeader + 4);
	PIMAGE_OPTIONAL_HEADER NewpOptionHeader = (PIMAGE_OPTIONAL_HEADER)((DWORD)NewpPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	PIMAGE_SECTION_HEADER NewpSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)NewpOptionHeader + NewpPEHeader->SizeOfOptionalHeader);

	//��������½ڲ���
	//PIMAGE_SECTION_HEADER newSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)(pSectionTemp+1) - (DWORD)pImageBuffer + (DWORD)pTempNewImageBuffer);  //��ָ��ָ��newImageBuffer�������ڱ�λ��
	PIMAGE_SECTION_HEADER newSectionHeader = NewpSectionHeader;
	for (i = 0; i < NewpPEHeader->NumberOfSections; i++, newSectionHeader++);
	memcpy(newSectionHeader, NewpSectionHeader, 0x28);
	memset(newSectionHeader + 1, 0, 0x28);       //�涨���һ��֮��Ҫ��һ���ڱ��ȫ0


	//�޸�ͷ����Ϣ
	NewpOptionHeader->SizeOfImage = NewpOptionHeader->SizeOfImage + 0x1000;
	NewpPEHeader->NumberOfSections++;
	//�޸��½ڱ���Ϣ
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




//�����ڵ��⺯��
void add_section(void) {
	// ��ʼ������
	PVOID pFileBuffer = NULL;
	PVOID pImageBuffer = NULL;
	PVOID pNewImageBuffer = NULL;
	PVOID pNewFileBuffer = NULL;
	DWORD NewFileBufferSize = 0;
	//char file_path[] = "D:\\Lib\\IPMSG2007.exe";
	char file_path[] = "C:\\software__\\IPMSG2007.exe";
	char write_file_path[] = "C:\\software__\\addSection_IPMSG2007.exe";

	// exe->filebuffer
	DWORD ret1 = ReadPEFile(file_path, &pFileBuffer);  // &pFileBuffer(void**����) ���ݵ�ַ����ֵ���Խ����޸�
	printf("ԭ�ļ���С��%#x\n", ret1);
	// filebuffer -> imagebuffer
	DWORD ret2 = CopyFileBufferToImageBuffer(pFileBuffer, &pImageBuffer);
	printf("filebuffer -> imagebuffer����ֵΪ���������ļ���С��%#x\n", ret2);
	//����һ����
	DWORD ret3 = addSection(pImageBuffer, &pNewImageBuffer);
	printf("�����ں����ļ��������ڴ��С��%#x\n", ret3);
	//imagebuffer->newbuffer
	DWORD FileSize = CopyImageBufferToNewBuffer(pNewImageBuffer, &pNewFileBuffer);
	printf("imagebuffer->newbuffer����ֵΪ���������ļ���С��%#x\n", FileSize);
	//newbuffer->����
	bool ret4 = MemeryTOFile(pNewFileBuffer, FileSize, write_file_path);
	if (ret4) {
		printf("д�뵽���ļ��ɹ���");
	}
	//�ͷ�����Ŀռ�
	free(pFileBuffer);
	free(pNewFileBuffer);
	free(pNewImageBuffer);
	free(pImageBuffer);
}


//**************************************************************************0202__p47����ںϲ���

#define size_expandSection 0x1000

//����ں���
DWORD expandLastSection(IN PVOID pImageBuffer, OUT PVOID* pExpandImageBuffer) {
	// ��ʼ��PEͷ���ṹ��
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	// �ж�pImageBuffer�Ƿ���Ч
	if (!pImageBuffer)
	{
		printf("(addSection�׶�)pImageBufferָ����Ч\n");
		return 0;
	}
	//�ж��ǲ���exe�ļ�
	if (*((PWORD)pImageBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(addSection�׶�)����MZ��־������exe�ļ���\n");
		return 0;
	}
	pDosHeader = (PIMAGE_DOS_HEADER)pImageBuffer;
	if (*((PDWORD)((DWORD)pDosHeader + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(addSection�׶�)������Ч��PE��־��\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // �������ǿ������ת��
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);


	//�޸����һ���ڱ����
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
	//�����¿ռ�
	size_t Size_ExpandImageBuffer = pOptionHeader->SizeOfImage;
	PVOID pTempExpandImageBuffer = malloc(Size_ExpandImageBuffer);
	if (!pTempExpandImageBuffer) {
		printf("���������һ�ڽ׶Σ������ڴ�ռ�ʧ��\n");
		return 0;
	}
	memset(pTempExpandImageBuffer, 0, Size_ExpandImageBuffer);
	memcpy(pTempExpandImageBuffer, pImageBuffer, pOptionHeader->SizeOfImage - size_expandSection);
	//�����һ������������(Ҫ�ȸı�ͷָ���ַ)
	//memcpy((char*)(pNewLastSection->VirtualAddress + pNewLastSection->Misc.VirtualSize),pNewData,size_expandSection);


	*pExpandImageBuffer = pTempExpandImageBuffer;
	return 1;
}

//�ϲ��ں���
DWORD MergeAllSection(IN PVOID pImageBuffer, OUT PVOID* pNewImageBuffer) {
	// ��ʼ��PEͷ���ṹ��
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	// �ж�pImageBuffer�Ƿ���Ч
	if (!pImageBuffer)
	{
		printf("(addSection�׶�)pImageBufferָ����Ч\n");
		return 0;
	}
	//�ж��ǲ���exe�ļ�
	if (*((PWORD)pImageBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(addSection�׶�)����MZ��־������exe�ļ���\n");
		return 0;
	}
	pDosHeader = (PIMAGE_DOS_HEADER)pImageBuffer;
	if (*((PDWORD)((DWORD)pDosHeader + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(addSection�׶�)������Ч��PE��־��\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // �������ǿ������ת��
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	PIMAGE_SECTION_HEADER pTempSectionHeader = pSectionHeader;
	//�޸Ľڱ�1�Ĳ���
	PIMAGE_SECTION_HEADER pLastSectionHeader = pSectionHeader + pPEHeader->NumberOfSections - 1;
	DWORD charactor = 0;
	for (int i = 0; i < pPEHeader->NumberOfSections; i++, pTempSectionHeader++) {
		charactor |= pTempSectionHeader->Characteristics;
	}
	pSectionHeader->Characteristics = charactor;
	pSectionHeader->Misc.VirtualSize = pOptionHeader->SizeOfImage - pOptionHeader->SizeOfHeaders;
	//pSectionHeader->SizeOfRawData = pLastSectionHeader->PointerToRawData - pSectionHeader->PointerToRawData + pLastSectionHeader->SizeOfRawData;  ***��������  ��Ϊֻ��һ���ڲ�����������������
	pSectionHeader->SizeOfRawData = pOptionHeader->SizeOfImage - pOptionHeader->SizeOfHeaders;
	//��������ڱ�
	memset((char*)((DWORD)pSectionHeader + 0x28), 0, (pPEHeader->NumberOfSections - 1) * 0x28);
	//�޸�ͷ����Ӧ����
	pPEHeader->NumberOfSections = 1;

	*pNewImageBuffer = pImageBuffer;
	pImageBuffer = NULL;
	return 1;
}

//����ںϲ��ڵ��⺯��
void expandOrMerge_Section(void) {
	// ��ʼ������
	PVOID pFileBuffer = NULL;
	PVOID pImageBuffer = NULL;
	PVOID pNewImageBuffer = NULL;
	PVOID pNewFileBuffer = NULL;
	DWORD NewFileBufferSize = 0;
	//char file_path[] = "D:\\Lib\\IPMSG2007.exe";
	char file_path[] = "C:\\software__\\IPMSG2007.exe";
	char write_file_path[] = "C:\\software__\\addSection_IPMSG2007.exe";

	// exe->filebuffer
	DWORD ret1 = ReadPEFile(file_path, &pFileBuffer);  // &pFileBuffer(void**����) ���ݵ�ַ����ֵ���Խ����޸�
	printf("ԭ�ļ���С��%#x\n", ret1);
	// filebuffer -> imagebuffer
	DWORD ret2 = CopyFileBufferToImageBuffer(pFileBuffer, &pImageBuffer);
	printf("filebuffer -> imagebuffer����ֵΪ���������ļ���С��%#x\n", ret2);
	//����һ����
	/*DWORD ret3 = expandLastSection(pImageBuffer,&pNewImageBuffer);
	if(ret3)
		printf("expand succeed\n");*/
		//�ϲ����н�
	DWORD flag = MergeAllSection(pImageBuffer, &pNewImageBuffer);
	if (flag)printf("�ϲ��ڱ�ɹ�\n");
	//imagebuffer->newbuffer
	DWORD FileSize = CopyImageBufferToNewBuffer(pNewImageBuffer, &pNewFileBuffer);
	printf("imagebuffer->newbuffer����ֵΪ���������ļ���С��%#x\n", FileSize);
	//newbuffer->����
	bool ret4 = MemeryTOFile(pNewFileBuffer, FileSize, write_file_path);
	if (ret4) {
		printf("д�뵽���ļ��ɹ���");
	}
	//�ͷ�����Ŀռ�
	free(pFileBuffer);
	//free(pImageBuffer);
	free(pNewImageBuffer);
	free(pNewFileBuffer);

}
//*****************************************************************************0205__P49��ӡ������

//��ӡpe�ļ�������ʧ�ܷ���0���ɹ�����1
DWORD print_exportTable(IN PVOID pFileBuffer) {
	// ��ʼ��PEͷ���ṹ��
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	PIMAGE_EXPORT_DIRECTORY pExportDirectory = NULL;
	//�ж�exe->�ڴ��Ƿ�ɹ�
	if (!pFileBuffer)
	{
		printf("��ȡ���ڴ��pfilebufferʧ�ܣ�\n");
		return 0;
	}
	// �ж��Ƿ��ǿ�ִ���ļ�
	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)  // IMAGE_DOS_SIGNATURE��4�ֽڣ���pFileBufferǿ������ת��Ϊ4�ֽ�ָ�����ͣ�PWORD��
	{
		printf("����MZ��־������exe�ļ���\n");
		return 0;
	}
	//ǿ�ƽṹ������ת��pDosHeader
	pDosHeader = PIMAGE_DOS_HEADER(pFileBuffer);
	//�ж��Ƿ���PE��־       
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("������Ч��PE��־��\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	pDataDirectory = (PIMAGE_DATA_DIRECTORY)pOptionHeader->DataDirectory;
	//�жϵ������Ƿ����
	if (!pDataDirectory->VirtualAddress) {
		printf("����������\n");
		return 0;
	}
	DWORD Foa = RvaToFileOffset(pFileBuffer, pDataDirectory->VirtualAddress);
	pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((DWORD)pFileBuffer + Foa);
	//��ӡ����������                      
	printf("*****************************************************\n");
	printf("Name:%#x\n", pExportDirectory->Name);
	printf("Base:%d\n", pExportDirectory->Base);
	printf("NumberOfFunctions��%d\n", pExportDirectory->NumberOfFunctions);
	printf("NumberOfNames:%d\n", pExportDirectory->NumberOfNames);
	//��ӡ������ַ��
	printf("*****************************************************��ַ��\n");
	PDWORD pAddressTable = (PDWORD)(RvaToFileOffset(pFileBuffer, pExportDirectory->AddressOfFunctions) + (DWORD)pDosHeader);
	for (DWORD i = 0; i < pExportDirectory->NumberOfFunctions; i++) {
		printf("%d�ĵ�ַ:%#x\n", i, *(pAddressTable + i));
	}
	//��ӡ�������Ʊ�
	printf("*****************************************************���Ʊ�\n");
	DWORD foa_AddressOfNames = RvaToFileOffset(pFileBuffer, pExportDirectory->AddressOfNames);
	PDWORD pNameTable = (PDWORD)(foa_AddressOfNames + (DWORD)pDosHeader);
	LPSTR name = (LPSTR)pNameTable;
	for (i = 0; i < pExportDirectory->NumberOfNames; i++, pNameTable++) {
		name = (LPSTR)pNameTable;
		printf("%d������:%s\n", i, name);
	}
	//��ӡ������ű�
	printf("*****************************************************��ű�\n");
	PWORD pNameOrdinalsTable = (PWORD)(RvaToFileOffset(pFileBuffer, pExportDirectory->AddressOfNameOrdinals) + (DWORD)pDosHeader);
	for (i = 0; i < pExportDirectory->NumberOfNames; i++) {
		printf("%d�ĺ������:%#x\n", i, *(pNameOrdinalsTable + i));
	}
	return 1;
}

//ͨ���������ֻ�ú�����ַ
DWORD GetFunctionAddrByName(IN PVOID pFileBuffer, IN LPSTR pFunctionName) {
	// ��ʼ��PEͷ���ṹ��
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	PIMAGE_EXPORT_DIRECTORY pExportDirectory = NULL;
	//�ж�exe->�ڴ��Ƿ�ɹ�
	if (!pFileBuffer)
	{
		printf("��ȡ���ڴ��pfilebufferʧ�ܣ�\n");
		return 0;
	}
	// �ж��Ƿ��ǿ�ִ���ļ�
	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)  // IMAGE_DOS_SIGNATURE��4�ֽڣ���pFileBufferǿ������ת��Ϊ4�ֽ�ָ�����ͣ�PWORD��
	{
		printf("����MZ��־��\n");
		return 0;
	}
	//ǿ�ƽṹ������ת��pDosHeader
	pDosHeader = PIMAGE_DOS_HEADER(pFileBuffer);
	//�ж��Ƿ���PE��־       
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("������Ч��PE��־��\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	pDataDirectory = (PIMAGE_DATA_DIRECTORY)pOptionHeader->DataDirectory;
	//�жϵ������Ƿ����
	if (!pDataDirectory->VirtualAddress) {
		printf("����������\n");
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
	//���ҵ���Ӧ��������������ű�������ͬ�±��ֵ�����ʵ�ַ��
	if (index >= 0) {
		PWORD indexTemp = (PWORD)((DWORD)pFileBuffer + RvaToFileOffset(pFileBuffer, pExportDirectory->AddressOfNameOrdinals));
		PDWORD addressTemp = (PDWORD)((DWORD)pFileBuffer + RvaToFileOffset(pFileBuffer, pExportDirectory->AddressOfFunctions));
		indexTemp += index;
		addressTemp += (DWORD)*indexTemp;
		return *addressTemp;
	}
	printf("δ�ҵ�ͬ������\n");
	return 0;
}

//ͨ��������Ż�ú�����ַ
DWORD GetFunctionAddrByOrdinals(IN PVOID pFileBuffer, IN DWORD Ord) {
	// ��ʼ��PEͷ���ṹ��
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	PIMAGE_EXPORT_DIRECTORY pExportDirectory = NULL;
	//�ж�exe->�ڴ��Ƿ�ɹ�
	if (!pFileBuffer)
	{
		printf("��ȡ���ڴ��pfilebufferʧ�ܣ�\n");
		return 0;
	}
	// �ж��Ƿ��ǿ�ִ���ļ�
	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)  // IMAGE_DOS_SIGNATURE��4�ֽڣ���pFileBufferǿ������ת��Ϊ4�ֽ�ָ�����ͣ�PWORD��
	{
		printf("����MZ��־��\n");
		return 0;
	}
	//ǿ�ƽṹ������ת��pDosHeader
	pDosHeader = PIMAGE_DOS_HEADER(pFileBuffer);
	//�ж��Ƿ���PE��־       
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("������Ч��PE��־��\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	pDataDirectory = (PIMAGE_DATA_DIRECTORY)pOptionHeader->DataDirectory;
	//�жϵ������Ƿ����
	if (!pDataDirectory->VirtualAddress) {
		printf("����������\n");
		return 0;
	}
	DWORD Foa = RvaToFileOffset(pFileBuffer, pDataDirectory->VirtualAddress);
	pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((DWORD)pFileBuffer + Foa);
	DWORD index = Ord - pExportDirectory->Base;
	if (index >= pExportDirectory->NumberOfFunctions || index < 0) {
		printf("���������Ч\n");
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
	DWORD ret1 = ReadPEFile(file_path, &pFileBuffer);  // &pFileBuffer(void**����) ���ݵ�ַ����ֵ���Խ����޸�
	if (!ret1) {
		printf("���ļ�ʧ��/n");
		return;
	}
	//��ӡ������
	DWORD ret = print_exportTable(pFileBuffer);
	if (!ret) {
		printf("��ӡ������ʧ��/n");
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
//*****************************************************************************0205__p50�ض�λ��
//��ӡ�ض�λ����������
DWORD print_relocationTable(IN PVOID pFileBuffer) {
	// ��ʼ��PEͷ���ṹ��
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	PIMAGE_EXPORT_DIRECTORY pExportDirectory = NULL;
	//�ж�exe->�ڴ��Ƿ�ɹ�
	if (!pFileBuffer)
	{
		printf("��ȡ���ڴ��pfilebufferʧ�ܣ�\n");
		return 0;
	}
	// �ж��Ƿ��ǿ�ִ���ļ�
	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)  // IMAGE_DOS_SIGNATURE��4�ֽڣ���pFileBufferǿ������ת��Ϊ4�ֽ�ָ�����ͣ�PWORD��
	{
		printf("����MZ��־������exe�ļ���\n");
		return 0;
	}
	//ǿ�ƽṹ������ת��pDosHeader
	pDosHeader = PIMAGE_DOS_HEADER(pFileBuffer);
	//�ж��Ƿ���PE��־       
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("������Ч��PE��־��\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	pDataDirectory = (PIMAGE_DATA_DIRECTORY)pOptionHeader->DataDirectory;
	if (!(pDataDirectory + 5)->VirtualAddress) {
		printf("�ض�λ������\n");
		return 0;
	}
	//ʹָ��ָ���ض�λ��
	DWORD Foa_relocationTable = RvaToFileOffset(pFileBuffer, (pDataDirectory + 5)->VirtualAddress);
	PIMAGE_BASE_RELOCATION pRelocationTable = (PIMAGE_BASE_RELOCATION)((DWORD)pFileBuffer + Foa_relocationTable);
	//��ӡ�ض�λ������
	for (int i = 0; pRelocationTable->SizeOfBlock != 0 && pRelocationTable->VirtualAddress != 0; i++, pRelocationTable = (PIMAGE_BASE_RELOCATION)((DWORD)pRelocationTable + pRelocationTable->SizeOfBlock)) {
		printf("===================================================��%d��\n", i);
		printf("VirtualAddress:%#x\n", pRelocationTable->VirtualAddress);
		printf("SizeOfBlock:%#x\n", pRelocationTable->SizeOfBlock);
		DWORD num = (pRelocationTable->SizeOfBlock - 8) / 2;
		printf("��ǰ��ľ���������:%d\n", num);
		printf("��Ҫ�޸ĵ�������:\n");
		PWORD pData = (PWORD)((DWORD)pRelocationTable + 8);
		for (DWORD k = 0; k < num; k++, pData++) {
			if (*pData & 0xF000 == 0x3000) {
				printf("��%d��:%#x\n", k, *pData & 0x0fff);
			}
		}
	}
	return 1;
}

//�ض�λ���⺯��
void operate_relocationTable(void) {
	PVOID pFileBuffer = NULL;
	//char file_path[] = "C:\\software__\\IPMSG2007.exe";			
	//char file_path[] = "C:\\WINDOWS\\system32\\notepad.exe";	C:\My_works\nixiang_220120\dyDll\Debug
	char file_path[] = "C:\\My_works\\nixiang_220120\\dyDll\\Debug\\dyDll.dll";
	// exe->filebuffer
	DWORD ret1 = ReadPEFile(file_path, &pFileBuffer);
	if (!ret1) {
		printf("���ļ�ʧ��/n");
		return;
	}
	//��ӡ������
	DWORD ret = print_relocationTable(pFileBuffer);
	if (!ret) {
		printf("��ӡ�ض�λ��ʧ��/n");
		return;
	}

	free(pFileBuffer);
	return;
}

//*****************************************************************************0205__p51�ƶ��������ض�λ��
//��size�Ĵ�С����һ���ڣ��������½ڵ�FOA,��������ʧ�ܣ�����0
DWORD addSection_returnFoa(IN PVOID pFileBuffer, OUT PVOID* ppNewFileBuffer, IN size_t size) {
	size_t size_NewBuffer = 0;
	// ��ʼ��PEͷ���ṹ��
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;


	// �ж�pFileBuffer�Ƿ���Ч
	if (!pFileBuffer)
	{
		printf("(addSection�׶�)pImageBufferָ����Ч\n");
		return 0;
	}
	//�ж��ǲ���exe�ļ�
	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(addSection�׶�)����MZ��־��\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	if (*((PDWORD)((DWORD)pDosHeader + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(addSection�׶�)������Ч��PE��־��\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4); // �������ǿ������ת��
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);

	PIMAGE_SECTION_HEADER pSectionTemp = pSectionHeader;
	//�ж����һ���ڱ���Ƿ��������ڱ�Ŀռ䣨0x50��
	for (int i = 0; i < pPEHeader->NumberOfSections; i++) {
		pSectionTemp++;
	}
	if ((pOptionHeader->SizeOfHeaders - ((DWORD)pSectionTemp + 0x28)) < addSection_needSpace || !isAllZero((PVOID)((DWORD)pSectionTemp + 0x28), (PVOID)((DWORD)pSectionTemp + 0x78))) {
		printf("����½ڱ�����ռ䲻�㣬����нڱ�ǰ�ƻ������\n");
		//moveHeaders()   exSection()
		return 0;
	}

	printf("����½ڱ�����ռ����\n");
	//����newpFileBuffer�ռ�
	size_t size_FileBuffer = (pSectionHeader + pPEHeader->NumberOfSections - 1)->PointerToRawData + (pSectionHeader + pPEHeader->NumberOfSections - 1)->SizeOfRawData;
	size_NewBuffer = Align(size, pOptionHeader->SectionAlignment) + size_FileBuffer;
	PVOID pTempNewFileBuffer = malloc(size_NewBuffer);
	if (!pTempNewFileBuffer) {
		printf("(addSection�׶�)����NewFileBuffer�ڴ�ռ�ʧ��\n");
		return 0;
	}
	memset(pTempNewFileBuffer, 0, size_NewBuffer);
	memcpy(pTempNewFileBuffer, pFileBuffer, size_FileBuffer);
	//����ɶ��½ڽ��и�ֵ

	// ��ͷ��ָ��ָ��NewFileBuffer�Ķ�Ӧλ��
	PIMAGE_DOS_HEADER NewpDosHeader = (PIMAGE_DOS_HEADER)pTempNewFileBuffer;
	PIMAGE_NT_HEADERS NewpNTHeader = (PIMAGE_NT_HEADERS)((DWORD)NewpDosHeader + NewpDosHeader->e_lfanew);
	PIMAGE_FILE_HEADER NewpPEHeader = (PIMAGE_FILE_HEADER)((DWORD)NewpNTHeader + 4);
	PIMAGE_OPTIONAL_HEADER NewpOptionHeader = (PIMAGE_OPTIONAL_HEADER)((DWORD)NewpPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	PIMAGE_SECTION_HEADER NewpSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)NewpOptionHeader + NewpPEHeader->SizeOfOptionalHeader);

	//��������½ڲ���
	//PIMAGE_SECTION_HEADER newSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)(pSectionTemp+1) - (DWORD)pImageBuffer + (DWORD)pTempNewImageBuffer);  //��ָ��ָ��newImageBuffer�������ڱ�λ��
	PIMAGE_SECTION_HEADER newSectionHeader = NewpSectionHeader;
	newSectionHeader = newSectionHeader + NewpPEHeader->NumberOfSections;
	memcpy(newSectionHeader, NewpSectionHeader, 0x28);
	memset(newSectionHeader + 1, 0, 0x28);       //�涨���һ��֮��Ҫ��һ���ڱ��ȫ0


	//�޸�ͷ����Ϣ
	NewpOptionHeader->SizeOfImage = NewpOptionHeader->SizeOfImage + Align(size, pOptionHeader->SectionAlignment);
	NewpPEHeader->NumberOfSections++;
	//�޸��½ڱ���Ϣ
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

//�ƶ��ض�λ��,ʧ�ܷ���0���ɹ��������ļ���С
DWORD move_relocationTable(IN PVOID pFileBuffer, OUT PVOID* ppNewFileBuffer) {
	// ��ʼ��PEͷ���ṹ��
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	PIMAGE_BASE_RELOCATION pRelocationTable = NULL;

	PVOID pTempNewFileBuffer = NULL;
	//�ж�exe->�ڴ��Ƿ�ɹ�
	if (!pFileBuffer)
	{
		printf("��ȡ���ڴ��pfilebufferʧ�ܣ�\n");
		return 0;
	}

	// �ж��Ƿ��ǿ�ִ���ļ�
	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)  // IMAGE_DOS_SIGNATURE��4�ֽڣ���pFileBufferǿ������ת��Ϊ4�ֽ�ָ�����ͣ�PWORD��
	{
		printf("����MZ��־��\n");
		return 0;
	}
	//ǿ�ƽṹ������ת��pDosHeader
	pDosHeader = PIMAGE_DOS_HEADER(pFileBuffer);
	//�ж��Ƿ���PE��־       
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("������Ч��PE��־��\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	pDataDirectory = (PIMAGE_DATA_DIRECTORY)pOptionHeader->DataDirectory;
	if (!(pDataDirectory + 5)->VirtualAddress) {
		printf("�ض�λ������\n");
		return 0;
	}
	//ʹָ��ָ���ض�λ��
	DWORD Foa_relocationTable = RvaToFileOffset(pFileBuffer, (pDataDirectory + 5)->VirtualAddress);
	pRelocationTable = (PIMAGE_BASE_RELOCATION)((DWORD)pFileBuffer + Foa_relocationTable);
	PIMAGE_BASE_RELOCATION pTempRelocationTable = pRelocationTable;

	//�����ض�λ���С
	size_t size_RelocationTable = 0;
	for (; pTempRelocationTable->SizeOfBlock != 0 && pTempRelocationTable->VirtualAddress != 0; pTempRelocationTable = (PIMAGE_BASE_RELOCATION)((DWORD)pTempRelocationTable + pTempRelocationTable->SizeOfBlock)) {
		size_RelocationTable += pTempRelocationTable->SizeOfBlock;
	}
	size_RelocationTable += 8;		//�������ռ8�ֽ�
	//�����ڣ��������½ڵ�foa
	DWORD Foa_NewSection = addSection_returnFoa(pFileBuffer, &pTempNewFileBuffer, size_RelocationTable);
	PDWORD pNewSection = (PDWORD)((DWORD)pTempNewFileBuffer + Foa_NewSection);
	//�����ض�λ���½�
	//printf("%#x\n",size_RelocationTable);
	memcpy(pNewSection, pRelocationTable, size_RelocationTable);
	//�޸�opͷ��Ŀ¼���ַ
	DWORD RvaRelocationTable = 0;
	/*if(*((PWORD)pTempNewFileBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("%s\n",((PWORD)pTempNewFileBuffer));
		return 0;
	}*/
	RvaRelocationTable = FoaToRva(pTempNewFileBuffer, Foa_NewSection);		//ע���¾�Bufferָ���ʹ��---------------------------------
	if (!RvaRelocationTable) {
		printf("RelocationTable��VirtualAddress����\n");
		return 0;
	}
	(pDataDirectory + 5)->VirtualAddress = RvaRelocationTable;

	*ppNewFileBuffer = pTempNewFileBuffer;
	pTempNewFileBuffer = NULL;
	//�������ļ���С
	size_t sizeNewFileBuffer = Align(size_RelocationTable, pOptionHeader->SectionAlignment) + Foa_NewSection;
	return sizeNewFileBuffer;


}


//�ƶ��������ض�λ���⺯��
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
		printf("���ļ�ʧ��/n");
		return;
	}
	//�ƶ��ض�λ��
	DWORD size_NewFileBuffer = move_relocationTable(pFileBuffer, &pNewFileBuffer);
	if (!size_NewFileBuffer) {
		printf("�ƶ��ض�λ��ʧ��/n");
		return;
	}

	//����
	bool ret4 = MemeryTOFile(pNewFileBuffer, size_NewFileBuffer, write_file_path);
	if (ret4) {
		printf("д�뵽���ļ��ɹ���");
	}
	free(pFileBuffer);
	return;
}
//*****************************************************************************0206_____p52/53�����iat��
//��ӡ�����ʧ�ܷ���0���ɹ�����dll����
DWORD print_importTable(IN PVOID pFileBuffer) {
	// ��ʼ��PEͷ���ṹ��
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	PIMAGE_IMPORT_DESCRIPTOR pImportTable = NULL;

	//�ж�exe->�ڴ��Ƿ�ɹ�
	if (!pFileBuffer)
	{
		printf("��ȡ���ڴ��pfilebufferʧ�ܣ�\n");
		return 0;
	}

	// �ж��Ƿ��ǿ�ִ���ļ�
	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)  // IMAGE_DOS_SIGNATURE��4�ֽڣ���pFileBufferǿ������ת��Ϊ4�ֽ�ָ�����ͣ�PWORD��
	{
		printf("����MZ��־��\n");
		return 0;
	}
	//ǿ�ƽṹ������ת��pDosHeader
	pDosHeader = PIMAGE_DOS_HEADER(pFileBuffer);
	//�ж��Ƿ���PE��־       
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("������Ч��PE��־��\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	pDataDirectory = (PIMAGE_DATA_DIRECTORY)pOptionHeader->DataDirectory;
	if (!(pDataDirectory + 1)->VirtualAddress) {
		printf("���������\n");
		return 0;
	}
	//ʹָ��ָ�����
	DWORD Foa_importTable = RvaToFileOffset(pFileBuffer, (pDataDirectory + 1)->VirtualAddress);
	pImportTable = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)pFileBuffer + Foa_importTable);
	PIMAGE_IMPORT_DESCRIPTOR pTempImportTable = pImportTable;
	//�����е��õ�dll���б���
	for (int i = 0; !isAllZero((PVOID)pTempImportTable, (PVOID)((DWORD)pTempImportTable + sizeof(IMAGE_IMPORT_DESCRIPTOR) - 1)); pTempImportTable++, i++) {

		printf("***************************************************��%d��dll\n", i + 1);
		DWORD foa_name = RvaToFileOffset(pFileBuffer, pTempImportTable->Name);
		PDWORD name = (PDWORD)((DWORD)pFileBuffer + foa_name);
		DWORD foa_OriginalFirstThunk = RvaToFileOffset(pFileBuffer, pTempImportTable->OriginalFirstThunk);
		PDWORD pOriginalFirstThunk = (PDWORD)((DWORD)pFileBuffer + foa_OriginalFirstThunk);
		DWORD foa_FirstThunk = RvaToFileOffset(pFileBuffer, pTempImportTable->FirstThunk);
		PDWORD pFirstThunk = (PDWORD)((DWORD)pFileBuffer + foa_FirstThunk);
		printf("dll����:%s\n", name);
		printf("===================================================OriginalFirstThunk��\n");
		for (int j = 0; *pOriginalFirstThunk != 0; j++, pOriginalFirstThunk++) {

			if (*pOriginalFirstThunk & 0x80000000 == 0x80000000) {
				printf("��%d�����������Ϊ:%#x\n", j + 1, *pOriginalFirstThunk & 0x0fff);
			}
			else {
				DWORD foa_funtionName = RvaToFileOffset(pFileBuffer, *pOriginalFirstThunk);
				PIMAGE_IMPORT_BY_NAME pFuntionName = (PIMAGE_IMPORT_BY_NAME)((DWORD)pFileBuffer + foa_funtionName);
				printf("��%d������������Ϊ:%s\n", j + 1, pFuntionName->Name);
			}
		}
		printf("===================================================FirstThunk��\n");
		for (int k = 0; *pFirstThunk != 0; k++, pFirstThunk++) {
			if (*pFirstThunk & 0x80000000 == 0x80000000) {
				printf("��%d�����������Ϊ:%#x\n", k + 1, *pFirstThunk & 0x0fff);
			}
			else {
				DWORD foa_funtionName = RvaToFileOffset(pFileBuffer, *pFirstThunk);
				PIMAGE_IMPORT_BY_NAME pFuntionName = (PIMAGE_IMPORT_BY_NAME)((DWORD)pFileBuffer + foa_funtionName);
				printf("��%d������������Ϊ:%s\n", k + 1, pFuntionName->Name);
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
		printf("���ļ�ʧ��/n");
		return;
	}
	//��ӡ�����
	DWORD ret2 = print_importTable(pFileBuffer);
	if (!ret2) {
		printf("��ӡ�����ʧ��/n");
		return;
	}
	else {
		printf("����%d��dll\n", ret2);
	}

	/*//����
	bool ret4 = MemeryTOFile(pNewFileBuffer,size_NewFileBuffer, write_file_path);
	if(ret4){
		printf("д�뵽���ļ��ɹ���");
	}*/
	free(pFileBuffer);
	return;
}
//*****************************************************************************0207__p55�����ע��
DWORD add_importTable(IN PVOID pFileBuffer, IN PVOID* ppNewFileBuffer, IN LPSTR dllName) {
	// ��ʼ��PEͷ���ṹ��
	PIMAGE_DOS_HEADER pDosHeader = NULL;
	PIMAGE_NT_HEADERS pNTHeader = NULL;
	PIMAGE_FILE_HEADER pPEHeader = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	PIMAGE_IMPORT_DESCRIPTOR pImportTable = NULL;
	
	PVOID pTempNewFileBuffer = NULL;
	//�ж�exe->�ڴ��Ƿ�ɹ�
	if (!pFileBuffer)
	{
		printf("��ȡ���ڴ��pfilebufferʧ�ܣ�\n");
		return 0;
	}

	// �ж��Ƿ��ǿ�ִ���ļ�
	if (*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)  // IMAGE_DOS_SIGNATURE��4�ֽڣ���pFileBufferǿ������ת��Ϊ4�ֽ�ָ�����ͣ�PWORD��
	{
		printf("����MZ��־��\n");
		return 0;
	}
	//ǿ�ƽṹ������ת��pDosHeader
	pDosHeader = PIMAGE_DOS_HEADER(pFileBuffer);
	//�ж��Ƿ���PE��־       
	if (*((PDWORD)((DWORD)pFileBuffer + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("������Ч��PE��־��\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader + 4);
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPEHeader + IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader + pPEHeader->SizeOfOptionalHeader);
	pDataDirectory = (PIMAGE_DATA_DIRECTORY)pOptionHeader->DataDirectory;
	if (!(pDataDirectory + 1)->VirtualAddress) {
		printf("���������\n");
		return 0;
	}
	//ʹָ��ָ�����
	DWORD Foa_importTable = RvaToFileOffset(pFileBuffer, (pDataDirectory + 1)->VirtualAddress);
	pImportTable = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)pFileBuffer + Foa_importTable);
	PIMAGE_IMPORT_DESCRIPTOR pTempImportTable = pImportTable;
	//�ж�(pDataDirectory + 1)->Size�Ƿ���Ч
	if (!(pDataDirectory + 1)->Size) {
		printf("(pDataDirectory + 1)->Size��Ч������㵼����С\n");
		return 0;
	}
	size_t size_ImportTable = (pDataDirectory + 1)->Size;  //������־Ϊsizeof(IMAGE_IMPORT_DESCRIPTOR)��С��ȫ0
	size_t num_ImportTable = size_ImportTable/sizeof(IMAGE_IMPORT_DESCRIPTOR) - 1;
	size_t num_NewImportTable = num_ImportTable + 1;
	/*int i = 0;
	while(*pTempImportTable){
		i++;
		pTempImportTable++;
	}
	size_t num_ImportTable = i;
	size_t num_NewImportTable = num_ImportTable + 1;
	size_t size_ImportTable = sizeof(IMAGE_IMPORT_DESCRIPTOR) * (num_ImportTable + 1);  //������־Ϊsizeof(IMAGE_IMPORT_DESCRIPTOR)��С��ȫ0*/
	//һ����������û�п�������±�Ŀռ䣬�ʲ��ж�ֱ�ӽ��ɱ�Ǩ�Ƶ��½��������µ����
	//�����½�
	size_t size_NewImportTable = size_ImportTable + sizeof(IMAGE_IMPORT_DESCRIPTOR);
	DWORD foa_NewSection = addSection_returnFoa(pFileBuffer, &pTempNewFileBuffer, size_NewImportTable + 0x30);
	if (!foa_NewSection) {
		printf("�����½�ʧ��/n");
		return 0;
	}
	//����½ڵ�ָ��
	PVOID pNewSection = (PVOID)((DWORD)pTempNewFileBuffer + foa_NewSection);
	//��������Ƶ��½�
	memcpy(pNewSection, pImportTable, size_ImportTable);
	//����Ŀ¼��
	DWORD rva_NewSection = FoaToRva(pTempNewFileBuffer, foa_NewSection);
	(pDataDirectory + 1)->VirtualAddress = rva_NewSection;
	//������dll��int��iat���������Ʊ�
	PIMAGE_THUNK_DATA pNewINT = (PIMAGE_THUNK_DATA)((DWORD)pNewSection + sizeof(IMAGE_IMPORT_DESCRIPTOR) * (num_NewImportTable + 1));  //+1�ǽ�����־
	PIMAGE_THUNK_DATA pNewIAT = pNewINT + 2;		//��dllֻ��1������
	PIMAGE_IMPORT_BY_NAME pNewNameTable = (PIMAGE_IMPORT_BY_NAME)(pNewIAT + 2);
	pNewNameTable->Hint = 0;
	size_t len_dllName = strlen(dllName) + 1;
	memcpy(&(pNewNameTable->Name), dllName, len_dllName);		//�����������뺯�����Ʊ�
	DWORD rva_NewNameTable = FoaToRva(pTempNewFileBuffer, ((DWORD)pNewNameTable - (DWORD)pTempNewFileBuffer));
	pNewINT->u1.AddressOfData = (_IMAGE_IMPORT_BY_NAME *)rva_NewNameTable;
	pNewIAT->u1.AddressOfData = (_IMAGE_IMPORT_BY_NAME *)rva_NewNameTable;

	//�޸��µ�������
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
		printf("���ļ�ʧ��/n");
		return;
	}
	//���������
	DWORD size_NewFileBuffer = add_importTable(pFileBuffer, &pNewFileBuffer,dllname);
	if (!size_NewFileBuffer) {
		printf("���������ʧ��/n");
		return;
	}
	else {
		printf("���ļ���С:%#x\n", size_NewFileBuffer);
	}

	//����
	bool ret4 = MemeryTOFile(pNewFileBuffer, size_NewFileBuffer, write_file_path);
	if (ret4) {
		printf("д�뵽���ļ��ɹ���");
	}
	free(pFileBuffer);
	free(pNewFileBuffer);
	return;
}

//*****************************************************************************
//rvaתfoa��ʧ�ܷ���0���ɹ�����foa
DWORD RvaToFoa(IN PVOID pFileBuffer,OUT DWORD dwRva){
	// ��ʼ��PEͷ���ṹ��
	PIMAGE_DOS_HEADER pDosHeader = NULL;	
	PIMAGE_NT_HEADERS pNTHeader = NULL;	
	PIMAGE_FILE_HEADER pPEHeader = NULL;	
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	
	// �ж�pFileBuffer�Ƿ���Ч
	if(!pFileBuffer)
	{
		printf("(RVA->FOA�׶�)pFileBufferָ����Ч\n");
		return 0;
	}
	//�ж��ǲ���exe�ļ�
	if(*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(RVA->FOA�׶�)����MZ��־��\n");
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	if(*((PDWORD)((DWORD)pFileBuffer+pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(RVA->FOA�׶�)������Ч��PE��־��\n");	
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pFileBuffer+pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader+4); // �������ǿ������ת��
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
	printf("rva->foaʧ��\n");
	return 0;

}

//*****************************************************************************��ӡ��Դ����������Դ��ʧ�ܷ���0
DWORD print_resourceTable(IN PVOID pFileBuffer){
	// ��ʼ��PEͷ���ṹ��
	PIMAGE_DOS_HEADER pDosHeader = NULL;	
	PIMAGE_NT_HEADERS pNTHeader = NULL;	
	PIMAGE_FILE_HEADER pPEHeader = NULL;	
	PIMAGE_OPTIONAL_HEADER pOptionHeader = NULL;
	PIMAGE_SECTION_HEADER pSectionHeader = NULL;
	PIMAGE_DATA_DIRECTORY DataDirectory = NULL;
	PIMAGE_RESOURCE_DIRECTORY pResourceDirectory = NULL;
	
	// �ж�pImageBuffer�Ƿ���Ч
	if(!pFileBuffer)
	{
		printf("(��ӡ�׶�)pFileBufferָ����Ч\n");
		return 0;
	}
	//�ж��ǲ���exe�ļ�
	if(*((PWORD)pFileBuffer) != IMAGE_DOS_SIGNATURE)
	{
		printf("(��ӡ�׶�)����MZ��־��\n");
		return 0;
	}
	pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
	if(*((PDWORD)((DWORD)pDosHeader + pDosHeader->e_lfanew)) != IMAGE_NT_SIGNATURE)
	{
		printf("(��ӡ�׶�)������Ч��PE��־��\n");	
		return 0;
	}
	// ǿ�ƽṹ������ת��
	pNTHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader+pDosHeader->e_lfanew);
	pPEHeader = (PIMAGE_FILE_HEADER)((DWORD)pNTHeader+4); // �������ǿ������ת��
	pOptionHeader = (PIMAGE_OPTIONAL_HEADER)((DWORD)pPEHeader+IMAGE_SIZEOF_FILE_HEADER);
	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionHeader+pPEHeader->SizeOfOptionalHeader);
	DWORD rva_ResourceDirectory = pOptionHeader->DataDirectory[2].VirtualAddress;
	pResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)((DWORD)pDosHeader + RvaToFoa(pDosHeader,rva_ResourceDirectory));
	//��ӡ��Դ������
	PIMAGE_RESOURCE_DIRECTORY pTempResourceDirectory_1 = pResourceDirectory;
	PIMAGE_RESOURCE_DIRECTORY pTempResourceDirectory_2 = NULL;
	PIMAGE_RESOURCE_DIRECTORY pTempResourceDirectory_3 = NULL;
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pTempResourceDirectoryEntry_1 = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pTempResourceDirectory_1 + 1);
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pTempResourceDirectoryEntry_2 = NULL;
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pTempResourceDirectoryEntry_3 = NULL;
	PIMAGE_RESOURCE_DIR_STRING_U pNameStringDir = NULL;
	PIMAGE_DATA_DIRECTORY pDataDirectory = NULL;
	for(int i=0;i<pTempResourceDirectory_1->NumberOfIdEntries + pTempResourceDirectory_1->NumberOfNamedEntries;i++,pTempResourceDirectoryEntry_1++){
		printf("*********************************����%d\n",i+1);
		//��ӡ������
		printf("---��һ�����:\n");
		if(pTempResourceDirectoryEntry_1->NameIsString){			//��һ�ڶ�����ʵ���Բ��ж����λ(��Ϊ1)
			pNameStringDir = (PIMAGE_RESOURCE_DIR_STRING_U)pTempResourceDirectoryEntry_1->NameOffset;
			printf("����(����):%*s\n",pNameStringDir->Length,pNameStringDir->NameString);
		}else{
			printf("����(���):%#x\n",pTempResourceDirectoryEntry_1->NameOffset);
		}
		//�����2��
		pTempResourceDirectory_2 = (PIMAGE_RESOURCE_DIRECTORY)(pTempResourceDirectoryEntry_1->OffsetToDirectory + (DWORD)pResourceDirectory);
		pTempResourceDirectoryEntry_2 = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pTempResourceDirectory_2 + 1);
		for(int j=0;j<pTempResourceDirectory_2->NumberOfIdEntries + pTempResourceDirectory_2->NumberOfNamedEntries;j++,pTempResourceDirectoryEntry_2++){
			//����ɴ�ӡ�ڶ������
			//�����3��
			pTempResourceDirectory_3 = (PIMAGE_RESOURCE_DIRECTORY)(pTempResourceDirectoryEntry_2->OffsetToDirectory + (DWORD)pResourceDirectory);
			pTempResourceDirectoryEntry_3 = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pTempResourceDirectory_3 + 1);
			//��ӡ���������
			printf("---���������(��%d��):\n",j+1);
			printf("������:%d\n",pTempResourceDirectory_3->NumberOfIdEntries + pTempResourceDirectory_3->NumberOfNamedEntries);
			for(int j=0;j<pTempResourceDirectory_3->NumberOfIdEntries + pTempResourceDirectory_3->NumberOfNamedEntries;j++,pTempResourceDirectoryEntry_3++){
				if(pTempResourceDirectoryEntry_3->NameIsString){			
					pNameStringDir = (PIMAGE_RESOURCE_DIR_STRING_U)pTempResourceDirectoryEntry_3->NameOffset;
					printf("����ҳ(����):%*s\n",pNameStringDir->Length,pNameStringDir->NameString);
				}else{
					printf("����ҳ(���):%#x\n",pTempResourceDirectoryEntry_3->NameOffset);
				}
				if(pTempResourceDirectoryEntry_3->DataIsDirectory){
					printf("�е��Ĳ�\n");
					return 0;
				}else{
					pDataDirectory = (PIMAGE_DATA_DIRECTORY)(pTempResourceDirectoryEntry_3->OffsetToData + (DWORD)pResourceDirectory);
					printf("��Դ����RVA:%#x  SIZE:%#x\n",pDataDirectory->VirtualAddress,pDataDirectory->Size);
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
