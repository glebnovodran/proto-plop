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
			EvalError err = ctx.get_error();
			if (err != EvalError::NONE) {
				ctx.print_error();
			}
			blk.reset();
		}
	}
	ctx.print_vars();

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

void Value::set_none() {
	kind = Kind::NON;
	val.num = 0;
}
bool Value::is_none() const {
	return kind == Kind::NON;
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
	mErrCode = EvalError::NONE;
}

void ExecContext::reset() {
	if (mpStrs) {
		cxStrStore::destroy(mpStrs);
		mpStrs = nullptr;
	}
	VarMap::destroy(mpVarMap);
	mVarCnt = 0;
	mErrCode = EvalError::NONE;
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

int ExecContext::add_var(const char* pName) {
	int id = -1;

	if (pName && mpVarMap) {
		if (mVarCnt < CODE_VAR_MAX) {
			const char* pVarName = add_str(pName);
			if (pVarName) {
				pVarName = mpVarMap->add(pVarName, mVarCnt);
				if (pVarName) {
					id = mVarCnt;
					mpVarNames[id] = pVarName;
					mVarVals[id].set_none();
					++mVarCnt;
				}
			}
		}
	}

	return id;
}

int ExecContext::find_var(const char* pName) const {
	int id = -1;
	if (pName && mpVarMap) {
		int foundId = -1;
		bool found = mpVarMap->get(pName, &foundId);
		if (found) {
			id = foundId;
		}
	}
	return id;
}

Value* ExecContext::var_val(int id) {
	Value* pVal = nullptr;
	if ((id >= 0) && (id < CODE_VAR_MAX)) {
		pVal = &mVarVals[id];
	}
	return pVal;
}

void ExecContext::print_vars() {
	nxCore::dbg_msg(FMT_BOLD "%d" FMT_OFF " variables\n", mVarCnt);
	for (uint32_t i = 0; i < mVarCnt; ++i) {
		const char* pVarName = mpVarNames[i];
		int varId = find_var(pVarName);
		nxCore::dbg_msg(FMT_BOLD "[%d]" FMT_OFF FMT_B_BLUE " %s" FMT_OFF ": ", varId, pVarName);
		Value* pVal = var_val(varId);
		if (pVal) {
			if (pVal->is_str()) {
				nxCore::dbg_msg(FMT_B_YELLOW "\"%s\"" FMT_OFF, pVal->val.pStr);
			} else if (pVal->is_num()) {
				nxCore::dbg_msg("%f", pVal->val.num);
			} else {
				nxCore::dbg_msg("--");
			}
		} else {

		}
		nxCore::dbg_msg("\n");
	}
}

void ExecContext::set_error(const EvalError errCode) {
	mErrCode = errCode;
}

EvalError ExecContext::get_error() const { return mErrCode; }

void ExecContext::print_error() const {
	nxCore::dbg_msg(FMT_BOLD FMT_RED "ERROR: " FMT_OFF);
	switch(mErrCode) {
		case EvalError::BAD_DEFVAR:
			nxCore::dbg_msg("Invalid variable definition clause.\n");
			break;
		case EvalError::VAR_SYM:
			nxCore::dbg_msg("SYM type expected for a variable name.\n");
			break;
		case EvalError::VAR_CTX_ADD:
			nxCore::dbg_msg("Error adding variable to the context.\n");
			break;
		case EvalError::BAD_OPERAND_COUNT:
			nxCore::dbg_msg("A numeric operand expected.\n");
			break;
		case EvalError::BAD_OPERAND_TYPE_NUM:
			nxCore::dbg_msg("A numeric value expected.\n");
			break;
		case EvalError::BAD_OPERAND_TYPE_SYM:
			nxCore::dbg_msg("A symbol expected.\n");
			break;
		case EvalError::BAD_OPERAND_TYPE_STR:
			nxCore::dbg_msg("A string value expected.\n");
			break;

		case EvalError::NONE:
		default:
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

Value NumOpInfo::apply(const Value& valA, const Value& valB) {
	Value val;
	val.set_none();

	if (valA.is_num() && valB.is_num()) {
		if (func) {
			val = func(valA, valB);
		}
	}

	return val;
}

static Value numop_add(const Value& valA, const Value& valB) {
	Value val;
	val.set_num(valA.val.num + valB.val.num);
	return val;
}

static Value numop_sub(const Value& valA, const Value& valB) {
	Value val;
	val.set_num(valA.val.num - valB.val.num);
	return val;
}

static Value numop_mul(const Value& valA, const Value& valB) {
	Value val;
	val.set_num(valA.val.num * valB.val.num);
	return val;
}

static Value numop_div(const Value& valA, const Value& valB) {
	Value val;
	val.set_num(valA.val.num / valB.val.num);
	return val;
}

static struct {
	const char* pName;
	NumOpInfo opInfo;
} s_numOp_tbl[] = {
	{ "+", { numop_add, 0.0 } },
	{ "-", { numop_sub, 0.0 } },
	{ "*", { numop_mul, 1.0 } },
	{ "/", { numop_div, 1.0 } },
};

static bool check_numop(const char* pSym, NumOpInfo* pInfo) {
	size_t numops = XD_ARY_LEN(s_numOp_tbl);
	bool res = false;
	for (size_t i = 0; i < numops; ++i) {
		if (nxCore::str_eq(s_numOp_tbl[i].pName, pSym)) {
			*pInfo = s_numOp_tbl[i].opInfo;
			res = true;
			break;
		}
	}
	return res;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeBlock::reset() {
	mpListStack->reset();
	mListCnt = 0;
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
	CodeList* pLst = mListCnt >= ListStack::CODE_LST_MAX ? nullptr : &mpLists[mListCnt++];
	return pLst;
}

bool CodeBlock::operator()(const cxLexer::Token& tok) {
	CodeItem item;
	item.set_none();

	CodeList* pTopLst = mpListStack->top();

	if (tok.is_punctuation()) {
		if (tok.id == cxXqcLexer::TokId::TOK_SEMICOLON) return false;
		if (tok.id == cxXqcLexer::TokId::TOK_LPAREN) {
			CodeList* pNewLst = new_list();
			if (pNewLst == nullptr) return false;
			mpListStack->push(pNewLst);
			item.set_list(pNewLst);
		} else if (tok.id == cxXqcLexer::TokId::TOK_RPAREN) {
			mpListStack->pop();
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
	for (int i = 0; i < ListStack::CODE_LST_MAX; ++i) {
		mpLists[i].reset();
	}
	mListCnt = 0;
	mpListStack->reset();
	if (line.valid()) {
		cxLexer lexer;
		lexer.set_text(line.pText, line.textSize);
		lexer.scan(*this);
	}
}

Value CodeBlock::eval_sub(CodeList* pLst, const uint32_t org, const uint32_t slice) {
	NumOpInfo numOpInfo;
	Value val;

	val.set_none();

	if (!pLst || mCtx.get_error() != EvalError::NONE) return val;

	uint32_t cnt = pLst->count();
	if (slice > 0) {
		cnt = org + slice;
	}
	if (cnt == 0) return val;
	CodeItem* pLstItems = pLst->get_items();
	for (uint32_t i = org; i < cnt; ++i) {
		CodeItem* pItem = &pLstItems[i];
		if (pItem->is_list()) {
			val = eval_sub(pItem->val.pLst);
		} else if (pItem->is_sym()) {
			if (nxCore::str_eq(pItem->val.sym, "defvar")) {
				if (i + 1 < cnt) {
					CodeItem* pVarNameItem = pItem + 1;
					if (pVarNameItem->is_sym()) {
						const char* pVarName = pVarNameItem->val.sym;
						int varId = mCtx.add_var(pVarName);
						if (varId >= 0) {
							if (i + 2 < cnt) {
								val = eval_sub(pLst, 2);
								i += 2;
								Value* pVarVal = mCtx.var_val(varId);
								if (pVarVal) {
									*pVarVal = val;
								}
							}
						} else {
							mCtx.set_error(EvalError::VAR_CTX_ADD);
						}
					} else {
						mCtx.set_error(EvalError::VAR_SYM);
					}
				} else {
					mCtx.set_error(EvalError::BAD_DEFVAR);
				}
			} else if (nxCore::str_eq(pItem->val.sym, "set")) {
				// ...
			} else if (check_numop(pItem->val.sym, &numOpInfo)) {
				Value valA;
				Value valB;
				if (i + 2 == cnt) {
					valA.set_num(numOpInfo.unaryVal);
					valB = eval_sub(pLst, 1, 1);
					val = numOpInfo.apply(valA, valB);
					i += 1;
				} else if (i + 2 < cnt) {
					Value valA = eval_sub(pLst, 1, 1);
					Value valB = eval_sub(pLst, 2, 1);
					val = numOpInfo.apply(valA, valB);
					i += 2;
				} else {
					mCtx.set_error(EvalError::BAD_OPERAND_COUNT);
				}
			} else { // variable name
				Value* pVal = mCtx.var_val(mCtx.find_var(pItem->val.sym));
				if (pVal) {
					val = *pVal;
				} else {
					mCtx.set_error(EvalError::VAR_NOT_FOUND);
				}
			}
		} else if (pItem->is_num()) {
				val.set_num(pItem->val.num);
		} else if (pItem->is_str()) {
			val.set_str(pItem->val.pStr);
		}
	}
	return val;
}

void CodeBlock::eval() {
	mCtx.set_error(EvalError::NONE);
	eval_sub(&mpLists[0]);
}

void CodeBlock::print_sub(const CodeList* pLst, int lvl) const {
	CodeItem* pItems = pLst->get_items();
	uint32_t sz = pLst->count();
	for (uint32_t i = 0; i < sz; ++i) {
		const CodeItem& item = pItems[i];
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
	if (mListCnt == 0) return;
	nxCore::dbg_msg("# lists: %d\n", mListCnt);
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
	size_t sz = nxCalc::clamp(nxCore::str_len(pStr), size_t(0), Value::SYM_MAX_LEN);
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

void CodeList::append(const CodeItem& itm) {
	if (mpItems == nullptr) {
		size_t sz = mChunkSize * sizeof(CodeItem);
		mpItems = reinterpret_cast<CodeItem*>(nxCore::mem_alloc(sz));
		mCapacity = mChunkSize;
	}
	if (mCount >= mCapacity) {
		size_t newSz = (mCapacity + mChunkSize)*sizeof(CodeItem);
		mpItems = reinterpret_cast<CodeItem*>(nxCore::mem_realloc(mpItems, newSz));
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

