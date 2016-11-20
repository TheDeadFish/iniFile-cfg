#include <stdshit.h>
#include "iniFile.h"
const char progName[] = "test";

struct TestStruct 
{
	// project
	char* name;
	char* data;
	char* desc;
	int nParagraph;
	
	// dynamic struct
	struct Paragraph {
		char* head;
		char* body; };
	Paragraph* paragraph;
	
	// fixed array tests
	s8 array16x8[16];
	s16 array16x16[16];
	s32 array16x32[16];
	u32 array3xHx[3];
	float array16xFl[16];
	char* array16xSt[16];
	s8 defIntMin[16];
	s8 defNegOne[16];
	
	// dynamic array tests
	int arrayLen;
	s8* arrayNx8;
	s16* arrayNx16;
	s32* arrayNx32;
	u32* arrayNxHx;
	float *arrayNxFl;
	char** arrayNxSt;
	
	// structure tests
	struct Test {
		int arrayLen;
		char** str; };
	Test array4xNxSt[4];
};

INI_DEFUCT(paragraphTab, TestStruct::Paragraph,
	INI_DS(head, Str) INI_DS(body, Str)
);


INI_DEFUCT(testStructTab, TestStruct,
	INI_DS(name, Str) INI_DS(data, Str)
	INI_DS(desc, Str) INI_DS(nParagraph, Int)
	INI_DVB(paragraph, nParagraph, -1, paragraphTab)
);

INI_DEFUCT(TestStructTestTab, TestStruct::Test,
	INI_DS(arrayLen, Int)
	INI_DV(str, Str, arrayLen, -1)
);


INI_DEFUCT(testStructTab2, TestStruct,
	INI_DF(array16x8, Byte, 16)
	INI_DF(array16x16, Word, 16)
	INI_DF(array16x32, Int, 16)
	INI_DF(array3xHx, Hex, 3)
	INI_DF(array16xFl, Float, 16)
	INI_DF(array16xSt, Str, 16)
	
	INI_DF8(defIntMin, Byte, 16)
	INI_DFN(defNegOne, Byte, 16)
	
	INI_DS(arrayLen, Int)
	INI_DV(arrayNx8, Byte, arrayLen, -1)
	INI_DV(arrayNx16, Int, arrayLen, -1)
	INI_DV(arrayNx32, Int, arrayLen, -1)
	INI_DV(arrayNxHx, Hex, arrayLen, -1)
	INI_DV(arrayNxFl, Float, arrayLen, -1)
	INI_DV(arrayNxSt, Str, arrayLen, -1)
	
	INI_DFU(array4xNxSt, 4, TestStructTestTab)
);



TestStruct ts;


int main()
{
	// load the ini file
	{IniFile_Load ifl;
	int x = ifl.load("test.ini");
	if(x) fatalError("bad ini file");
	testStructTab->readBlock(&ifl, "project", &ts);
	testStructTab2->readBlock(&ifl, "test", &ts);
	
	}
	
	// save the ini file
	{ IniFile_Save ifs;
	ifs.create("out.ini");
	testStructTab->writeBlock(&ifs, "project", &ts);
	testStructTab2->writeBlock(&ifs, "test", &ts);
	}
}
