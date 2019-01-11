#include "stdshit.h"
#include "iniFile.h"
enum { IniType_DynX = 0x30 };

// stdshit candidates
TMPL(T) std::make_unsigned_t<T> usn(T val) { return val; }
TMPL2(T, ... U) ALWAYS_INLINE bool is_nulspc_or(const T& t, 
	const U&... u) { return (usn(t) <= ' ')|| is_one_of(t,u...); }
static inline bool isSpc(char ch) { return inRng(ch, 1, ' '); }
static inline bool isNSpc(char ch) { return inRng(ch, 0, ' '); }
	
// line parsing code
cstr __stdcall getLine(char*& str) {
	char* curPos = str; SCOPE_EXIT(str = curPos);
	char *lineBase = str; while(!is_one_of(*curPos, '\0',
		'\r', '\n')) curPos++; char* lineEnd = curPos;
	if(*curPos == '\r') curPos++; if(*curPos == '\n') curPos++;
	return {lineBase, lineEnd}; }
cstr REGCALL(2) removeCrap(cstr str) {
	auto ptr = str.ptr(); while(ptr.chk()
		&&(usn(ptr.l()) <= ' ')) ptr.ld();
	return {ptr.data, ptr.end()}; }
DEF_RETPAIR(skipCrap_t, char*, pos, char, ch);
skipCrap_t __stdcall skipCrap(char* curPos) { char
	ch; for(; ch = *curPos, inRng(ch, 1, ' ');
		curPos++); return {curPos, ch}; }

DEF_RETPAIR(strchr_t, char*, end, char*, base);
strchr_t __stdcall strchr(cstr str, char ch) { auto
	ptr = str.ptr(); for(;ptr.chk(); ptr.data++) { if(
	ptr.f() == ch) return {ptr.data, str}; } return{}; }
strchr_t __stdcall strrchr(cstr str, char ch) { auto
	ptr = str.ptr(); while(ptr.chk()) { if(ptr.ld()
		== ch) return {ptr.end(), str}; } return{}; }
	
	
#define INIFSERH_(x) if(notErr && ((x) < 0)) notErr = 0; return notErr;

bool IniFile_Save::fmt(cch* fmt, size_t var) { 
	INIFSERH_( fprintf(fp, fmt, var)); }
bool IniFile_Save::fmtf(cch* fmt, void* var) { 
	INIFSERH_( fprintf(fp, fmt, RF(var))); }
bool IniFile_Save::fmtd(cch* fmt, void* var) { 
	INIFSERH_( fprintf(fp, fmt, RD(var))); }
bool IniFile_Save::create(cch* file) {
	fp = fopen(file, "w"); return (notErr = fp); }
bool IniFile_Save::close() { if(fclose_ref(fp))
	notErr = 0; return notErr; }
bool IniFile_Save::comment(cch* str) { INIFSERH_(
	fprintf(fp, getfmt("\n; %s", 1), str)); }
bool IniFile_Save::block(cch* name) { INIFSERH_(
	fprintf(fp, getfmt("\n\n[%s]", 2), name)); }
bool IniFile_Save::data(cch* name, cch* value) {
	INIFSERH_(fprintf(fp, getfmt("\n%s=%s", 1), name, value)); }
bool IniFile_Save::begin(cch* name) { state = 1; 
	INIFSERH_(fprintf(fp, getfmt("\n%s=", 1), name)); }
bool IniFile_Save::next(cch* value, char ch) { if(state) {
	state = 0; ch = 0; } char buff[2]; *(WORD*)buff=ch;
	INIFSERH_(fprintf(fp, "%s%s", buff, value)); }
bool IniFile_Save::wrstr(cch* name, cch* value) { 
	INI_ENCSTR_(value); return data(name, encStr); }
bool IniFile_Save::nxstr(cch* value, char delim) {
	INI_ENCSTR_(value); return next(encStr, delim); }
cch* IniFile_Save::getfmt(cch* str, int val) {
	if(!ftell(fp)) str += val; return str; }

#define INI_SFB_(bs,fn,rt) curPos = hint; DO_AGAIN: for(;curPos < endPos; \
	curPos++) if(fn(curPos->name, name)) { rt } if(curPos > hint) { \
	ARGFIX(bs); curPos =  bs; endPos = hint; goto DO_AGAIN; }

char* IniFile_Load::Block::
	getValue(cch* name, int& hint_)
{
	// initialize state
	int iHint = hint_; if(iHint >= nLines) {
		VARFIX(iHint); iHint -= nLines; }
	Line* endPos = lines+nLines;
	Line* hint = lines+iHint;
	
	// search for block
	Line* curPos = hint; INI_SFB_(lines, !strcmp,
		hint_= (curPos-lines)+1; return curPos->value;);
	return NULL;
}

IniFile_Load::Block* IniFile_Load::findBlock(cch* name)
{
	// initialize state
	Block* curPos = blocks; if(!name[0]) {RET: 
		lineHint = 0; return (blockHint = curPos); }
	Block* endPos = curPos+nBlocks; Block* hint = blockHint; 
	if(!hint || (++hint == endPos)) hint = curPos+1;
	
	// search for block
	INI_SFB_(blocks, !strcmp, goto RET; );
	curPos = NULL; goto RET;
}

IniFile_Load::Block* IniFile_Load::findBlock(cch* prefix, int n)  {	
	return findBlock(Xstrfmt("%s%d", prefix, n)); }
IniFile_Load::Block* IniFile_Load::findBlock(cch* prefix, cch* name)  {	
	return findBlock(Xstrfmt("%s%s", prefix, name)); }
char* IniFile_Load::getValue(cch* name) {
	if(blockHint == NULL) return NULL;
	return blockHint->getValue(name, lineHint); }
cstr IniFile_Load::dupValue(cch* name) { char* str = getValue
	(name); if(!str) return {}; return xstrdup(str); }

int IniFile_Load::load(cch* fileName)
{
	HANDLE hFile = createFile(fileName, GENERIC_READ, FILE_SHARE_READ,
		0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE) return -1;
	SCOPE_EXIT(CloseHandle(hFile));
	return load(hFile);
}

void IniFile_Load::free(void)
{
	for(auto& block : *this) {
		::free(block.lines); }
	blocks = 0; nBlocks = 0;
	free_ref(blocks);
	free_ref(fileData);
}


int IniFile_Load::load(HANDLE hFile)
{
	// read the file from disk
	{ this->free();
	DWORD fileSize = GetFileSize(hFile, NULL);
	fileData = malloc(fileSize+1);
	if(fileData == NULL) return -1;
	fileData[fileSize] = '\0';
	if(!ReadFile(hFile, fileData, 
		fileSize,	&fileSize, NULL)) {
		free_ref(fileData);	return -1; }
	}
	
	// create the unamed block
	this->lineHint = 0;
	Block* block = &xNextAlloc(blocks, nBlocks);
	*block = {(char*)"", 0, 0};
	SCOPE_EXIT(blockHint = blocks);
		
	// parse the file
	int curLine = 0; int badLine = 0;
	char* curPos = fileData;
	while(1) { curLine++; cstr str = removeCrap(
		getLine(curPos)); char ch = str[0]; if(!ch) 
		break; if(is_nulspc_or(ch, ';')) continue;
		
		if(ch == '[') {	auto end = strrchr(str, ']');
			if(!end) goto BAD_LINE; *end = '\0';
			block = &xNextAlloc(blocks, nBlocks);
			*block = {end.base+1, 0, 0};
		} else { *str.end() = '\0';
			auto equ = strchr(str, '=');
			if(!equ) goto BAD_LINE;	*equ = '\0';
			xNextAlloc(block->lines,block->
				nLines) = {equ.base,  equ+1};
		}
		if(0) { BAD_LINE: if(!badLine)
			badLine = curLine; }
	}
	return badLine;
}

char* ini_encStr(cch* str)
{
	// get length
	bool reqEnc = false;
	if(!str) str = (char*)"";
	ei(!str[0]) reqEnc = true;
	cch* curPos = str; int extra = 2; 
	char ch = RDI(curPos);
	if(ch) { if(isNSpc(ch)) reqEnc = true;
		do { if(ch == '"') extra++; if(is_one_of(
			ch, '"', ',', ';')) { reqEnc = true; }
		} while(ch = RDI(curPos)); 
		if(isNSpc(curPos[-2])) reqEnc = true;
	}
	
	// encode string
	if(reqEnc) { char* buff = xMalloc(extra+(curPos-str));
		*buff = '"'; char* dst = buff+1; for(; ch = RDI(str);
			({WRI(dst, ch); if(ch == '"') WRI(dst, ch);}));
		RW(dst) = '"'; return buff; } return (char*)str;
}


__stdcall void ini_uctEnd(char*& curPos_)
{
	if((curPos_)&&(curPos_[0] == ';')) curPos_++;
}

cstr ini_getStr(char*& curPos_)
{
	// skip leading whitespace
	if(!curPos_) { return {(char*)"", 0}; }
	GET_RETPAIR(char* curPos, char ch, skipCrap(curPos_));
	SCOPE_EXIT(curPos_ = curPos); /*if(!ch) { return {0,0}; }*/
	
	// skip string
	char *basePos = curPos, *endPos;
	for(;; ch = *curPos) { endPos = curPos;	if(!ch||
		(ch==';')) break; curPos++; if(ch == ',') break;
		if(ch=='"'){ while(ch = *curPos) { curPos++; 
			if(ch == '"') { if(*curPos != '"')
				break; curPos++; } }}
	}
	
	// trim trailing whitespace
	if(endPos > basePos) { while(usn(
		endPos[-1]) <= ' ') endPos--; }
	return {basePos, endPos};	
}

cstr REGCALL(2) ini_dupStr(cstr str)
{
	if(!str.slen) return {0,0};
	char* buff = xMalloc(str.slen+1);
	return {buff, ini_cpyStr(str, buff) };
}


char* ini_cpyStr(cstr src, char* dst)
{
	auto ptr = src.ptr(); bool inQuote = false;
	for(char ch; ptr.chk();){ if((ch=ptr.fi()) == '"') {
		if(!inQuote) { inQuote = 1; continue; }
		ei(ptr.f() != '"') { inQuote = 0; continue; }
		 ptr.data++; } WRI(dst, ch); } 
	WRI(dst, 0); return dst;
}

void Ini_FieldInfo::writeBlock(IniFile_Save* ini,
	cch* name, void* obj) const
{
	ini->block(name); auto* curPos = this+1;
	for(;curPos->type(); curPos = curPos->next()) {
		if(curPos->type2() != IniType_Blk) { ini->begin(
			curPos->name); curPos->writeField(ini, obj);
		} else { void* fld = obj; GET_RETPAIR(int count,
			int size, curPos->countSizeObj(fld)); 
			if(size) { cch* fmt = count ? "%s.%s:%d" : "%s.%s";
			int i = 0; do { curPos->fi()->writeBlock(ini, Xstrfmt(fmt, 
			name, curPos->name, i), fld); PTRADD(fld, size); 
			} while(++i < count); }
		}
	}
}

Ini_FieldInfo::countSizeObj_t Ini_FieldInfo
	::countSizeObj(void*& obj) const
{
	int size = this->size(); void* fld = this->fld(obj);
	int count = this->count(); if(type() & IniType_DynX) { 
	count = vcount(obj); if(count <= 0) return {0,0}; 
	fld = *(void**)fld; } obj = fld; return {count, size};
}

Ini_FieldInfo::countSizeObj_t Ini_FieldInfo
	::countSizeObj2(void*& obj) const
{
	int size = this->size(); void* fld = this->fld(obj);
	int count = this->count(); if(type() & IniType_DynX) {
		if(vcount(obj) >= vmax()) vcount(obj) = -1;
		count = vcount(obj); if(count <= 0) return {0,0}; 
		fld = *(void**)fld = xcalloc(count*size);
	} obj = fld; return {count, size};
}

void Ini_FieldInfo::writeField(IniFile_Save* ini, void* obj) const
{
	GET_RETPAIR(int count, int size,
		countSizeObj(obj)); if(!size) return;
	goto LOOP_START; while(--count > 0) { ini->fmt(
		"%c", (type2() == IniType_Uct) ? ';' : ','); 
		PTRADD(obj, size); LOOP_START:
	switch(type2()) { case IniType_Bool: 
		ini->fmt(RB(obj) ? "true" : "false", 0); break;
	case IniType_Int: ini->fmt("%d", RI(obj)); break;
	case IniType_Word: ini->fmt("%d", RW(obj)); break;
	case IniType_Byte: ini->fmt("%d", RB(obj)); break;
	case IniType_Float: ini->fmtf("%g", obj); break;
	case IniType_Double: ini->fmtd("%g", obj); break;
	case IniType_Hex: ini->fmt("%08X", RI(obj)); break;
	case IniType_Str: { INI_ENCSTR_(*(cch**)obj);
		ini->fmt("%s", (size_t)encStr); break; }
	case IniType_FStr: { INI_ENCSTR_((cch*)obj);
		ini->fmt("%s", (size_t)encStr); break; }
	case IniType_Uct: { auto* curPos = fi()+1;  while(1) {
		curPos->writeField(ini, obj); curPos = curPos->next();
		if(!curPos->type()) break; ini->fmt("%c", ','); }}
		break; default: UNREACH; }
	}
}

int Ini_FieldInfo::size(void) const
{
	switch(type() & ~IniType_DynX) {
	case IniType_Byte: case IniType_Bool: return 1;
	case IniType_Word: return 2; case IniType_Int: 
	case IniType_Float: case IniType_Hex: return 4; 	
	case IniType_Uct: case IniType_Blk: return fi()->vmax_;
	case IniType_Str: return sizeof(char*); default: UNREACH; }
}

void Ini_FieldInfo::readField(char*& str_, void* obj) const
{
	if(type() & IniType_Dyn2) { char ch = 
		type2() == IniType_Uct ? ';' : ',';
		int count = 0; if(str_ && str_[0]) {
		count++; for(int i = 0; str_[i]; i++)
			if(str_[i] == ch) count++; }
		vcount(obj) = count;
	}

	GET_RETPAIR(int count, int size,
		countSizeObj2(obj)); if(!size) return;
	int defVal = this->def(); if(defVal == -2) {
		defVal <<= (size*8)-2; }

	do { if(type2() == IniType_Uct) { auto* curPos = fi()+1;
		while(1) {  if(!curPos->type()) break; curPos->
			readField(str_, obj); curPos = curPos->next(); }
		ini_uctEnd(str_); } else {
		cstr str = ini_getStr(str_); 
		switch(type2()) {
	case IniType_Bool: { if(strScmp(str, "true")) defVal = 1;
		ei(strScmp(str, "false")) defVal = 0; RB(obj) = defVal; break; }
	case IniType_Byte: { char* endPtr; int tmp = strtol(str, &endPtr, 10);
		if(str.data == endPtr) tmp = defVal; RB(obj) = tmp; break; }
	case IniType_Word: { char* endPtr; int tmp = strtol(str, &endPtr, 10);
		if(str.data == endPtr) tmp = defVal; RW(obj) = tmp; break; }
	case IniType_Int: { char* endPtr; int tmp = strtol(str, &endPtr, 10); 
		if(str.data == endPtr) tmp = defVal; RI(obj) = tmp; break; }
	case IniType_Hex: { char* endPtr; int tmp = strtoul(str, &endPtr, 16);
		if(str.data == endPtr) tmp = defVal; RI(obj) = tmp; break; }
	case IniType_Float: case IniType_Double: { char* endPtr; double tmp = 
		strtod(str, &endPtr); if(str.data == endPtr) { tmp = defVal; 
		if(defVal==2) tmp = INFINITY; ei(defVal==3) tmp = -INFINITY; }
		if(type2() == IniType_Double) RD(obj) = tmp; else RF(obj) = tmp; break;}
	case IniType_Str: { char* tmp = ini_dupStr(str); if(!tmp && 
		!def()) tmp = xstrdup(""); *(char**)obj = tmp; break; }
	}} PTRADD(obj, size); } while(--count > 0);
}


void Ini_FieldInfo::readBlock(
	IniFile_Load* ini, cch* name, void* obj) const
{
	ini->findBlock(name); auto* curPos = this+1;
	for(;curPos->type(); curPos = curPos->next()) {
		if(curPos->type2() != IniType_Blk) { 
			char* str = ini->getValue(curPos->name);
			curPos->readField(str, obj);
		} else { void* fld = obj; GET_RETPAIR(int count,
			int size, curPos->countSizeObj2(fld)); 
			if(size) { cch* fmt = count ? "%s.%s:%d" : "%s.%s";
			int i = 0; do { curPos->fi()->readBlock(ini, Xstrfmt(fmt, 
			name, curPos->name, i), fld); PTRADD(fld, size); 
			} while(++i < count); }
		}
	}
}
