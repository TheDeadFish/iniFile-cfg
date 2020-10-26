#ifndef _INIFILE_H_
#define _INIFILE_H_



struct IniFile_Save
{	
	FILE* fp; bool notErr; char state;
	bool create(cch* file); bool close(void);
	bool comment(cch* str); bool block(cch* name);
	bool data(cch* name, cch* value);
	bool wrstr(cch* name, cch* value);
	bool begin(cch* name);	
	bool next(cch* value, char delim = ',');
	bool nxstr(cch* value, char delim = ',');
	cch* getfmt(cch* str, int ofs);
	
	bool fmt(cch* fmt, size_t var);
	bool fmtf(cch* fmt, void* var);
	bool fmtd(cch* fmt, void* var);
};

struct IniFile_Load 
{
	struct Line { char* name; char* value; };
	struct Block { char* name; Line* lines; int nLines;
		DEF_BEGINEND(Line, lines, nLines); 
		char* getValue(cch* name, int& hint); };
	Block* blocks; int nBlocks;
	DEF_BEGINEND(Block, blocks, nBlocks);
	char* fileData; 
	
	// block searching
	Block* blockHint; int lineHint;
	Block* findBlock(cch* name);
	Block* findBlock(cch* prefix, int n);
	Block* findBlock(cch* prefix, cch* name);
	char* getValue(cch* name);
	cstr dupValue(cch* name);
	cstr dupValue2(cch* name);
	
	// construction
	IniFile_Load() : fileData(0),
		blocks(0), nBlocks(0) {}
	~IniFile_Load() { free(); }
	void free(void);
	int load(cch* fileName); 
	int load(HANDLE hFile);
};

// high level parsing
__stdcall cstr ini_getStr(char*& curPos_);
__stdcall void ini_uctEnd(char*& curPos_);
#define INI_ENCSTR_(str) char* encStr = ini_encStr(\
	str); SCOPE_EXIT(if(encStr != str) free(encStr));
REGCALL(2) cstr ini_dupStr(cstr str);
__stdcall char* ini_cpyStr(cstr src, char* dst);
__stdcall char* ini_encStr(cch* str);

// 
enum { IniType_None, IniType_Bool, IniType_Byte, IniType_Word,
	IniType_Int, IniType_Hex, IniType_Float, IniType_Double,
	IniType_Str, IniType_FStr, IniType_Uct, IniType_Blk, 
	IniType_Dyn = 0x10, IniType_Dyn2 = 0x20 };
enum { IniDef_0 = 0, IniDef_1 = 64, 
	IniDef_N = 192, IniDef_8 = 128 };

#define INI_DEFUCT(name, type, ...)   \
	const Ini_FieldInfo name[] = {{0,sizeof(type),0}, __VA_ARGS__ {0,0,0}};
#define INI_DEF0_(n,m,n2) {n, u16(m), offsetof(INI_TYPE, n2)},
#define INI_DEF1_(n,t,t2,c,d) INI_DEF0_(#n, (MCAT(IniType_,t) \
	|MCAT(IniType_,t2)|(c<<8)|MCAT(IniDef_,d)), n)

// basic fixed types
#define INI_DS_(d,n,t) INI_DEF1_(n,t,None,0,d)
#define INI_DF_(d,n,t,c) INI_DEF1_(n,t,None,c,d)
#define INI_DFB(n,c,s) INI_DEF1_(n,Blk,None,c,0) {(char*)s,0,0},
#define INI_DFU(n,c,s) INI_DEF1_(n,Uct,None,c,0) {(char*)s,0,0},

// basic dynamic types
#define INI_DV_(d,n,t,l,m) INI_DEF1_(n,t,Dyn,0,d) INI_DEF0_(0, m, l)
#define INI_DVB(n,l,m,s) INI_DEF1_(n,Blk,Dyn,0,0) INI_DEF0_((char*)s, m, l)
#define INI_DVU(n,l,m,s) INI_DEF1_(n,Uct,Dyn,0,0) INI_DEF0_((char*)s, m, l)
#define INI_DV2(d,n,t,l,m) INI_DEF1_(n,t,Dyn2,0,d) INI_DEF0_(0, m, l)
#define INI_DV2B(n,l,m,s) INI_DEF1_(n,Blk,Dyn2,0,0) INI_DEF0_((char*)s, m, l)
#define INI_DV2U(n,l,m,s) INI_DEF1_(n,Uct,Dyn2,0,0) INI_DEF0_((char*)s, m, l)

// derived types
#define INI_DS(n,t) INI_DS_(0,n,t)
#define INI_DF(n,t,c) INI_DF_(0,n,t,c)
#define INI_DV(n,t,l,m) INI_DV_(0,n,t,l,m)
#define INI_DS1(n,t) INI_DS_(1,n,t)
#define INI_DS8(n,t) INI_DS_(8,n,t)
#define INI_DF8(n,t,c) INI_DF_(8,n,t,c)
#define INI_DSN(n,t) INI_DS_(N,n,t)
#define INI_DFN(n,t,c) INI_DF_(N,n,t,c)
	
struct Ini_FieldInfo
{
	cch* name; WORD vmax_, offset;
	byte type() const { return vmax_&63; }
	byte type2() const { return vmax_&15; }
	byte count() const { return vmax_>>8; }
	int def() const { return ((s8)vmax_) >> 6; }
	
	uint vmax(void) const { return this[1].vmax_; }
	void* fld(void* o) const { return o + offset; }
	Ini_FieldInfo* fi(void) const{ return (Ini_FieldInfo*)this[1].name; }
	uint& vcount(void* o) const { return *(uint*)this[1].fld(o); }
	int size(void) const; Ini_FieldInfo* next() const { return 
		(Ini_FieldInfo*) this + ((type() >= IniType_Uct) ? 2 : 1); }
		
	int getDef(int size) const;
	DEF_RETPAIR(countSizeObj_t, int, count, int, size );
	countSizeObj_t countSizeObj(void*& obj) const;
	countSizeObj_t countSizeObj2(void*& obj) const;
	
	void writeField(IniFile_Save* ini, void* obj) const;
	void writeBlock(IniFile_Save* ini, cch* name, void* obj) const;
	void readField(char*& str, void* obj) const;
	void readBlock(IniFile_Load* ini,  cch* name, void* obj) const;
};

#endif
