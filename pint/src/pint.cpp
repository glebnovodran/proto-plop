#include "crosscore.hpp"
#include "pint.hpp"

#if 1
#define FMT_ESC(_code_) "\x1B[" #_code_ "m"
#else
#define FMT_ESC(_code_) ""
#endif

#define FMT_BOLD FMT_ESC(1)
#define FMT_UNDER FMT_ESC(4)
#define FMT_BLACK FMT_ESC(30)
#define FMT_BLACK_BG FMT_ESC(40)
#define FMT_RED FMT_ESC(31)
#define FMT_RED_BG FMT_ESC(41)
#define FMT_GREEN FMT_ESC(32)
#define FMT_GREEN_BG FMT_ESC(42)
#define FMT_YELLOW FMT_ESC(33)
#define FMT_YELLOW_BG FMT_ESC(43)
#define FMT_BLUE FMT_ESC(34)
#define FMT_BLUE_BG FMT_ESC(44)
#define FMT_MAGENTA FMT_ESC(35)
#define FMT_MAGENTA_BG FMT_ESC(45)
#define FMT_CYAN FMT_ESC(36)
#define FMT_CYAN_BG FMT_ESC(46)
#define FMT_WHITE FMT_ESC(37)
#define FMT_WHITE_BG FMT_ESC(47)
#define FMT_GRAY FMT_ESC(90)
#define FMT_GRAY_BG FMT_ESC(100)
#define FMT_B_RED FMT_ESC(91)
#define FMT_B_RED_BG FMT_ESC(101)
#define FMT_B_GREEN FMT_ESC(92)
#define FMT_B_GREEN_BG FMT_ESC(102)
#define FMT_B_YELLOW FMT_ESC(93)
#define FMT_B_YELLOW_BG FMT_ESC(103)
#define FMT_B_BLUE FMT_ESC(94)
#define FMT_B_BLUE_BG FMT_ESC(104)
#define FMT_B_MAGENTA FMT_ESC(95)
#define FMT_B_MAGENTA_BG FMT_ESC(105)
#define FMT_B_CYAN FMT_ESC(96)
#define FMT_B_CYAN_BG FMT_ESC(106)
#define FMT_B_WHITE FMT_ESC(97)
#define FMT_B_WHITE_BG FMT_ESC(107)
#define FMT_OFF FMT_ESC(0)

namespace Pint {

void interp(const char* pSrcPath) {
	if (!pSrcPath) return;
	size_t srcSize = 0;
	char* pSrc = (char*)nxCore::raw_bin_load(pSrcPath, &srcSize);
	if (!pSrc) {
		nxCore::dbg_msg("Pint::interp: unable to load \"%s\"\n", pSrcPath);
		return;
	}

	SrcCode src(pSrc, srcSize, 32);
	ExecContext ctx;
	CodeBlock blk(ctx);

	while (!src.eof()) {
		SrcCode::Line line = src.get_line();
		line.print();
		if (line.valid()) {
			blk.parse(line);
			blk.print();
			blk.eval();
			blk.reset();
		}
	}

	src.reset();
	nxCore::bin_unload(pSrc);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SrcCode::Line::print() const {
	nxCore::dbg_msg(FMT_BOLD "line %d: " FMT_OFF, no);
	if (valid()) {
		nxCore::dbg_msg(FMT_GREEN);
		for (size_t i = 0; i < textSize; ++i) {
			nxCore::dbg_msg("%c", pText[i]);
		}
		//nxCore::dbg_msg("\n");
		nxCore::dbg_msg(FMT_OFF "\n");
	} else {
		nxCore::dbg_msg(FMT_RED "invalid\n" FMT_OFF);
	}
}

bool SrcCode::Line::valid() const {
	return pText != nullptr && textSize > 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SrcCode::eof() const {
	return mCur >= mSrcSize;
}

SrcCode::Line SrcCode::get_line() {
	SrcCode::Line line = {0};
	if (mpSrc == nullptr) return line;

	if (mpLineBuf == nullptr) {
		mpLineBuf = reinterpret_cast<char*>(nxCore::mem_alloc(mChunkSize));
		mLineBufSize = mChunkSize;
	}

	size_t lineStart = mCur;
	size_t skipped = 0;
	size_t linePos = 0;

	if (!eof()) {
		++mLineNo;
	}

	while (!eof()) {
		linePos = (mCur - skipped) - lineStart;
		if (linePos >= mLineBufSize) {
			mpLineBuf = reinterpret_cast<char*>(nxCore::mem_realloc(mpLineBuf, mLineBufSize + mChunkSize));
			mLineBufSize += mChunkSize;
		}

		char ch = mpSrc[mCur];
		if (ch == '\r') {
			skipped++;
		} else {
			mpLineBuf[linePos++] = ch;
		}
		mCur++;
		if (ch == '\n') {
			break;
		}
	}

	line.pText = mpLineBuf;
	line.textSize = linePos;
	line.no = mLineNo;

	return line;
}

void SrcCode::reset() {
	if (mpLineBuf) {
		nxCore::mem_free(mpLineBuf);
		mpLineBuf = nullptr;
		mLineBufSize = 0;
		mCur = 0;
		mpSrc = nullptr;
		mSrcSize = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
ExecContext::ExecContext() {
	init();
}

ExecContext::~ExecContext() {
	reset();
}

void ExecContext::init() {
	mpStrs = nullptr;
	mpVarMap = VarMap::create();
	mVarCnt = 0;
}

void ExecContext::reset() {
	if (mpStrs) {
		cxStrStore::destroy(mpStrs);
		mpStrs = nullptr;
	}
	VarMap::destroy(mpVarMap);
	mVarCnt = 0;
}

char* ExecContext::add_str(const char* pStr) {
	char* pStored = nullptr;
	if (pStr) {
		if (mpStrs == nullptr) {
			mpStrs = cxStrStore::create();
		}
		if (mpStrs) {
			pStored = mpStrs->add(pStr);
		}
	}
	return pStored;
}

void ExecContext::print_vars() const {
	// . . .
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CodeBlock::reset() {
	mpListStack->reset();
	mNumAllocList = 0;
}

CodeBlock::CodeBlock(ExecContext& ctx) : mCtx(ctx) {
	mpListStack = nxCore::tMem<ListStack>::alloc(1, "pint:stack");
	mpLists = nxCore::tMem<CodeList>::alloc(ListStack::CODE_LST_MAX, "pint:codelists");
	reset();
}

CodeBlock::~CodeBlock() {
	nxCore::tMem<ListStack>::free(mpListStack);
	nxCore::tMem<CodeList>::free(mpLists, ListStack::CODE_LST_MAX);
}

CodeList* CodeBlock::new_list() {
	CodeList* pLst = mNumAllocList >= ListStack::CODE_LST_MAX ? nullptr : &mpLists[mNumAllocList++];
	return pLst;
}

bool CodeBlock::operator()(const cxLexer::Token& tok) {
	Value item;
	item.set_none();

	ListStack* pStack = get_stack();
	CodeList* pTopLst = pStack->top();

	if (tok.is_punctuation()) {
		if (tok.id == cxXqcLexer::TokId::TOK_SEMICOLON) return false;
		if (tok.id == cxXqcLexer::TokId::TOK_LPAREN) {
			CodeList* pNewLst = new_list();
			if (pNewLst == nullptr) return false;
			pStack->push(pNewLst);
			item.set_list(pNewLst);
		} else if (tok.id == cxXqcLexer::TokId::TOK_RPAREN) {
			pStack->pop();
		} else {
			item.set_sym(tok.val.c);
		}
	} else if (tok.is_symbol()) {
		item.set_sym(reinterpret_cast<char*>(tok.val.p));
	} else if (tok.id == cxXqcLexer::TokId::TOK_FLOAT) {
		item.set_num(tok.val.f);
	} else if (tok.id == cxXqcLexer::TokId::TOK_INT) {
		item.set_num(tok.val.i);
	} else if ((tok.id == cxXqcLexer::TokId::TOK_QSTR) || (tok.id == cxXqcLexer::TokId::TOK_SQSTR)) {
		const char* pStr = mCtx.add_str(reinterpret_cast<const char*>(tok.val.p));
		item.set_str(pStr);
	}

	if (!item.is_none()) {
		if (pTopLst) {
			pTopLst->append(item);
		}
	}
	return true;
}

void CodeBlock::parse(const SrcCode::Line& line) {
	cxLexer lexer;
	lexer.set_text(line.pText, line.textSize);
	lexer.scan(*this);
}
	
void CodeBlock::eval() {
	// . . .
	get_stack()->reset();
}

void CodeBlock::print_sub(const CodeList* pLst, int lvl) const {
	Value* pItems = pLst->get_items();
	uint32_t sz = pLst->count();
	for (uint32_t i = 0; i < sz; ++i) {
		const Value& item = pItems[i];
		if (item.is_list()) {
			nxCore::dbg_msg("%*c" FMT_B_BLUE "- LST" FMT_OFF " %p\n", lvl, ' ', item.val.pLst);
			print_sub(item.val.pLst, lvl+1);
		} else if (item.is_num()) {
			nxCore::dbg_msg("%*c" FMT_B_GREEN "NUM" FMT_OFF " %f\n", lvl, ' ', item.val.num);
		} else if (item.is_sym()) {
			nxCore::dbg_msg("%*c" FMT_B_GREEN "SYM" FMT_OFF " %s\n", lvl, ' ', item.val.sym);
		} else if (item.is_str()) {
			nxCore::dbg_msg("%*c" FMT_B_GREEN "STR" FMT_B_YELLOW " \"%s\"" FMT_OFF "\n", lvl, ' ', item.val.pStr);
		}
	}
}

void CodeBlock::print() const {
	if (mNumAllocList == 0) return;
	nxCore::dbg_msg("# lists: %d\n", mNumAllocList);
	print_sub(mpLists, 1);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Value::set_none() {
	kind = Kind::NON;
	val.num = 0;
}
bool Value::is_none() const {
	return kind == Kind::NON;
}

void Value::set_sym(const char* pStr) {
	kind = Kind::SYM;
	size_t sz = nxCalc::clamp(nxCore::str_len(pStr), size_t(0), Value::SYM_MAX_LEN);
	nxCore::mem_copy(val.sym, pStr, sz);
	val.sym[sz] = '\x0';
}
bool Value::is_sym() const {
	return kind == Kind::SYM;
}

void Value::set_num(double num) {
	kind = Kind::NUM;
	val.num = num;
}
bool Value::is_num() const {
	return kind == Kind::NUM;
}

void Value::set_str(const char* pStr) {
	kind = Kind::STR;
	val.pStr = pStr;
}
bool Value::is_str() const {
	return kind == Kind::STR;
}

void Value::set_list(CodeList* pLst) {
	kind = Kind::LST;
	val.pLst = pLst;
}
bool Value::is_list() const {
	return kind == Kind::LST;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CodeList::init() {
	if (mpItems) {
		size_t sz = mCapacity * sizeof(Value);
		nxCore::mem_zero(mpItems, sz);
	} else {
		mCapacity = 0;
	}
	mCount = 0;
}

void CodeList::reset() {
	if (mpItems) {
		nxCore::mem_free(mpItems);
		mpItems = nullptr;
		mCount = 0;
		mCapacity = mChunkSize;
	}
}

bool CodeList::valid() const {
	return (mpItems != nullptr);
}

void CodeList::append(const Value& itm) {
	if (mpItems == nullptr) {
		size_t sz = mChunkSize * sizeof(Value);
		mpItems = reinterpret_cast<Value*>(nxCore::mem_alloc(sz));
		mCapacity = mChunkSize;
	}
	if (mCount >= mCapacity) {
		size_t newSz = (mCapacity + mChunkSize)*sizeof(Value);
		mpItems = reinterpret_cast<Value*>(nxCore::mem_realloc(mpItems, newSz));
		mCapacity += mChunkSize;
	}
	mpItems[mCount++] = itm;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

CodeList* ListStack::top() {
	return ptr - 1 < 0 ? nullptr : lists[ptr - 1];
}

void ListStack::push(CodeList* pLst) {
	if (ptr < CODE_LST_MAX) {
		lists[ptr++] = pLst;
	}
}

CodeList* ListStack::pop() {
	CodeList* pList = top();
	if (pList) {
		ptr--;
	}
	return pList;
}

} // Pint

