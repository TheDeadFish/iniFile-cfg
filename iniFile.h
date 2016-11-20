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
	
	
	
	
	// struct writing
	/*void writeFields(Ini_FieldInfo* fi, void* obj);
	void writeFields(Ini_FieldInfo* fi,
		void* obj, int objSize, int objCount);
	void block(cch* name, Ini_FieldInfo* fi);
	void block(cch* name, Ini_FieldInfo* fi,
		void* obj, int objSize, int objCount);*/
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
	
	/* field reading
	void readField(cch* name, int type, int , void* obj);
	void readField(cch* name, Ini_FieldInfo* fi, void* obj);
	
	
	
	
	
	void readField(cch* name, Ini_FieldInfo* fi, 
		void* obj, int objSize, int objCount);
		
		
		
		
	void readFields(Ini_FieldInfo* fi, void* obj);
	void readBlock(cch* name, Ini_FieldInfo* fi, void* obj);
	void readBlock(cch* name, Ini_FieldInfo* fi, 
		void* obj, int objSize, int objCount);
	*/
	
	
		
		
	
		
	
	
	
	
	
	
	
	
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


enum { IniType_None, IniType_Bool, IniType_Byte, IniType_Word,
	IniType_Int, IniType_Hex, IniType_Float, IniType_Str, 
	IniType_FStr, IniType_Uct, IniType_Blk, IniType_Dyn = 0x10 };

#define INI_DEFUCT(name, type, ...) typedef type TMPNAME(cfgType);  \
	const Ini_FieldInfo name[] = {{0,sizeof(type),0}, __VA_ARGS__ {0,0,0}};
#define INI_DEF0_(n,m,n2) {n, m, offsetof(TMPNAME(cfgType), n2)},
#define INI_DEF1_(n,t,t2,c,d) INI_DEF0_(#n, (MCAT(IniType_,t) \
	|MCAT(IniType_,t2)|(c<<8)|(u8(d<<6))), n)
#define INI_DS(n,t) INI_DEF1_(n,t,None,0,0)
#define INI_DF(n,t,c) INI_DEF1_(n,t,None,c,0)
#define INI_DS8(n,t) INI_DEF1_(n,t,None,0,-2)
#define INI_DF8(n,t,c) INI_DEF1_(n,t,None,c,-2)
#define INI_DSN(n,t) INI_DEF1_(n,t,None,0,-1)
#define INI_DFN(n,t,c) INI_DEF1_(n,t,None,c,-1)



#define INI_DFB(n,c,s) INI_DEF1_(n,Blk,None,c,0) {(char*)s,0,0},
#define INI_DVU(n,l,m,s) INI_DEF1_(n,Uct,Dyn,0,0) INI_DEF0_((char*)s, m, l)
	
struct Ini_FieldInfo
{
	cch* name; WORD vmax_, offset;
	byte type() const { return vmax_&31; }
	byte type2() const { return type() & ~IniType_Dyn; }	
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
	
	
		
	
	
	
	
	
	
	
	
	
	
	
	
	//bool writeBlock(IniFile_Save* ini,
//		cch* name, void* obj);
//	bool writeBlock(IniFile_Save* ini, 
	//	cch* name, void* obj, int count);
};

#endif
