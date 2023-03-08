#include "crosscore.hpp"
#include "pint.hpp"

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
		}
	}

	src.reset();
	nxCore::bin_unload(pSrc);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SrcCode::Line::print() const {
	if (valid()) {
		for (size_t i = 0; i < textSize; ++i) {
			nxCore::dbg_msg("%c", pText[i]);
		}
		nxCore::dbg_msg("\n");
	} else {
		nxCore::dbg_msg("invalid\n");
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
	if (mpLineBuf == nullptr) {
		mpLineBuf = reinterpret_cast<char*>(nxCore::mem_alloc(mChunkSize));
		mLineBufSize = mChunkSize;
	}

	size_t lineStart = mCur;
	size_t skipped = 0;
	size_t linePos = 0;
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

	SrcCode::Line line = {mpLineBuf, linePos};
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

char* ExecContext::add_str(const char* pStr) {
	return mpStrs != nullptr? mpStrs->add(pStr) : nullptr;
}

void ExecContext::print_vars() const {
	// . . .
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CodeBlock::operator()(const cxLexer::Token& tok) {
	// . . .
	return true;
}

void CodeBlock::parse(const SrcCode::Line& line) {
	// . . .
}
	
void CodeBlock::eval() {
	// . . .
}

void CodeBlock::print() const {
	// . . .
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeItem::set_none() {
	kind = Kind::NON;
	val.num = 0;
}
bool CodeItem::is_none() const {
	return kind == Kind::NON;
}

void CodeItem::set_sym(const char* pStr) {
	kind = Kind::SYM;
	size_t sz = nxCalc::clamp(nxCore::str_len(pStr), size_t(0), CodeItem::SYM_MAX_LEN);
	nxCore::mem_copy(val.sym, pStr, sz);
	val.sym[sz] = '\x0';
}
bool CodeItem::is_sym() const {
	return kind == Kind::SYM;
}

void CodeItem::set_num(double num) {
	kind = Kind::NUM;
	val.num = num;
}
bool CodeItem::is_num() const {
	return kind == Kind::NUM;
}

void CodeItem::set_str(const char* pStr) {
	kind = Kind::STR;
	val.pStr = pStr;
}
bool CodeItem::is_str() const {
	return kind == Kind::STR;
}

void CodeItem::set_list(CodeList* pLst) {
	kind = Kind::LST;
	val.pLst = pLst;
}
bool CodeItem::is_list() const {
	return kind == Kind::LST;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeList::init() {
	if (mpItems == nullptr) {
		size_t sz = mChunkSize * sizeof(CodeItem);
		mpItems = reinterpret_cast<CodeItem*>(nxCore::mem_alloc(sz));
		nxCore::mem_zero(mpItems, sz);
		mNumItems = 0;
		mSize = mChunkSize;
	}
}

void CodeList::reset() {
	if (mpItems) {
		nxCore::mem_free(mpItems);
		mpItems = nullptr;
		mNumItems = 0;
		mSize = mChunkSize;
	}
}

bool CodeList::valid() const {
	return (mpItems != nullptr);
}

void CodeList::append(const CodeItem& itm) {
	if (mNumItems >= mSize) {
		size_t newSz = (mSize + mChunkSize)*sizeof(CodeItem);
		mpItems = reinterpret_cast<CodeItem*>(nxCore::mem_realloc(mpItems, newSz));
		mSize += mChunkSize;
	}
	mpItems[mNumItems++] = itm;
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

