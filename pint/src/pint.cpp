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
			blk.reset();
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
ExecContext::ExecContext() {
	mpStrs = cxStrStore::create();
}

ExecContext::~ExecContext() {
	cxStrStore::destroy(mpStrs);
}

char* ExecContext::add_str(const char* pStr) {
	return mpStrs != nullptr? mpStrs->add(pStr) : nullptr;
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
	CodeList* pLst = &mpLists[mNumAllocList++];
	return pLst;
}

bool CodeBlock::operator()(const cxLexer::Token& tok) {
	CodeItem item;
	item.set_none();

	ListStack* pStack = get_stack();
	CodeList* pTopLst = pStack->top();

	if (tok.is_punctuation()) {
		if (tok.id == cxXqcLexer::TokId::TOK_SEMICOLON) return false;
		if (tok.id == cxXqcLexer::TokId::TOK_LPAREN) {
			CodeList* pNewLst = new_list();
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
		char* pStr = mCtx.add_str(reinterpret_cast<char*>(tok.val.p));
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
	CodeItem* pItems = pLst->get_items();
	uint32_t sz = pLst->size();
	for (uint32_t i = 0; i < sz; ++i) {
		const CodeItem& item = pItems[i];
		if (item.is_list()) {
			nxCore::dbg_msg("%*c- LST %p\n", lvl, ' ', item.val.pLst);
			print_sub(item.val.pLst, lvl+1);
		} else if (item.is_num()) {
			nxCore::dbg_msg("%*cNUM %f\n", lvl, ' ', item.val.num);
		} else if (item.is_sym()) {
			nxCore::dbg_msg("%*cSYM %s\n", lvl, ' ', item.val.sym);
		} else if (item.is_str()) {
			nxCore::dbg_msg("%*cSTR \"%s\"\n", lvl, ' ', item.val.pStr);
		}
	}
}

void CodeBlock::print() const {
	if (mNumAllocList == 0) return;
	nxCore::dbg_msg("# lists: %d\n", mNumAllocList);
	print_sub(mpLists, 1);
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
	if (mpItems) {
		size_t sz = mSize * sizeof(CodeItem);
		nxCore::mem_zero(mpItems, sz);
	} else {
		mSize = 0;
	}
	mNumItems = 0;
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
	if (mpItems == nullptr) {
		size_t sz = mChunkSize * sizeof(CodeItem);
		mpItems = reinterpret_cast<CodeItem*>(nxCore::mem_alloc(sz));
		mSize = mChunkSize;
	}
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

