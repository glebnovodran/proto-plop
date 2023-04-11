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

#if defined(PINT_DBG)
#define PINT_DBG_MSG(...) nxCore::dbg_msg(__VA_ARGS__)
#else
#define PINT_DBG_MSG(...)
#endif

namespace Pint {

typedef Value (*NumOpFunc)(const Value& valA, const Value& valB);

struct NumOpInfo {
	NumOpFunc func;
	double unaryVal;

	Value apply(const Value& valA, const Value& valB);
};

void interp(const char* pSrc, size_t srcSize, ExecContext* pCtx, FuncLibrary* pFuncLib) {
	if (pSrc && pCtx) {
		SrcCode src(pSrc, srcSize, 32);
		CodeBlock blk(*pCtx, pFuncLib);

		while (!(src.eof() || pCtx->should_break())) {
			SrcCode::Line line = src.get_line();
			line.print();
			if (line.valid()) {
				blk.parse(line);
				blk.print();
				blk.eval();

				blk.reset();
			}
		}
		//src.restart();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SrcCode::Line::print() const {
	PINT_DBG_MSG(FMT_BOLD "line %d: " FMT_OFF, no);
	if (valid()) {
		PINT_DBG_MSG(FMT_GREEN);
		for (size_t i = 0; i < textSize; ++i) {
			PINT_DBG_MSG("%c", pText[i]);
		}
		PINT_DBG_MSG(FMT_OFF "\n");
	} else {
		PINT_DBG_MSG(FMT_RED "invalid\n" FMT_OFF);
	}
}

bool SrcCode::Line::valid() const {
	return pText != nullptr && textSize > 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
SrcCode::SrcCode(const char* pSrc, size_t srcSize, const size_t cnkSize)
	:
	mpSrc(pSrc),
	mSrcSize(srcSize),
	mSrcLoc(0),
	mLineLoc(0),
	mLineNo(0)
	{}

bool SrcCode::eof() const {
	return mSrcLoc >= mSrcSize;
}

void SrcCode::restart() {
	mSrcLoc = 0;
}

SrcCode::Line SrcCode::get_line() {
	SrcCode::Line line = {0};

	mLineLoc = mSrcLoc;
	if (mpSrc) {
		if (!eof()) {
			++mLineNo;
		}
		while (!eof()) {
			if (mpSrc[mSrcLoc] == '\r') {
				++mSrcLoc;
				if (eof()) {
					break;
				}
				if (mpSrc[mSrcLoc] != '\n') {
					break;
				}
			}
			if (mpSrc[mSrcLoc] == '\n') {
				++mSrcLoc;
				break;
			}
			++mSrcLoc;
			++line.textSize;
		}
	}
	if (line.textSize > 0) {
		line.pText = &mpSrc[mLineLoc];
	}
	line.no = mLineNo;
	return line;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Value::set_none() {
	type = Type::NON;
	val.num = 0;
}
bool Value::is_none() const {
	return type == Type::NON;
}

void Value::set_num(double num) {
	type = Type::NUM;
	val.num = num;
}
bool Value::is_num() const {
	return type == Type::NUM;
}

void Value::set_str(const char* pStr) {
	type = Type::STR;
	val.pStr = pStr;
}
bool Value::is_str() const {
	return type == Type::STR;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

Value df_sin(ExecContext& ctx, const uint32_t nargs, Value* pArgs) {
	Value res;
	res.set_num(mth_sin(pArgs[0].val.num));
	return res;
}
static const FuncDef s_df_sin_desc = {
	"sin", df_sin, 1, Value::Type::NUM, {Value::Type::NUM}
};

Value df_cos(ExecContext& ctx, const uint32_t nargs, Value* pArgs) {
	Value res;
	res.set_num(mth_cos(pArgs[0].val.num));
	return res;
}
static const FuncDef s_df_cos_desc = {
	"cos", df_cos, 1, Value::Type::NUM, {Value::Type::NUM}
};

Value df_abs(ExecContext& ctx, const uint32_t nargs, Value* pArgs) {
	Value res;
	res.set_num(mth_fabs(pArgs[0].val.num));
	return res;
}
static const FuncDef s_df_abs_desc = {
	"abs", df_abs, 1, Value::Type::NUM, {Value::Type::NUM}
};

Value df_not(ExecContext& ctx, const uint32_t nargs, Value* pArgs) {
	Value res;
	res.set_num(double(!pArgs[0].val.num));
	return res;
}
static const FuncDef s_df_not_desc = {
	"not", df_not, 1, Value::Type::NUM, {Value::Type::NUM}
};

Value glb_rng_next(ExecContext& ctx, const uint32_t nargs, Value* pArgs) {
	Value res;
	uint64_t rnd = nxCore::rng_next();
	rnd &= 0xffffffff;
	res.set_num(double(rnd));
	return res;
}
static const FuncDef s_df_rng_next_desc = {
	"glb_rng_next", glb_rng_next, 0, Value::Type::NUM, {Value::Type::NUM}
};

Value glb_rng_01(ExecContext& ctx, const uint32_t nargs, Value* pArgs) {
	Value res;
	float rnd = nxCore::rng_f01();
	res.set_num(double(rnd));
	return res;
}
static const FuncDef s_df_rng_01_desc = {
	"glb_rng_01", glb_rng_01, 0, Value::Type::NUM, {Value::Type::NUM}
};

static const FuncDef s_defFuncDesc[] = {
	s_df_sin_desc, s_df_cos_desc, s_df_abs_desc, s_df_not_desc, s_df_rng_next_desc, s_df_rng_01_desc
};


FuncLibrary::FuncLibrary() : mpFuncMap(nullptr) {}

FuncLibrary::~FuncLibrary() {
	reset();
}

void FuncLibrary::init() {
	if (mpFuncMap == nullptr) {
		mpFuncMap = FuncMap::create();
		register_func(s_defFuncDesc, XD_ARY_LEN(s_defFuncDesc));
	}
}

void FuncLibrary::reset() {
	if (mpFuncMap) {
		FuncMap::destroy(mpFuncMap);
		mpFuncMap = nullptr;
	}
}

bool FuncLibrary::register_func(const FuncDef* pFuncDef, const uint32_t nfunc) {
	bool res = false;
	if (pFuncDef) {
		if (mpFuncMap == nullptr) {
			mpFuncMap = FuncMap::create();
		}
		for (uint32_t i = 0; i < nfunc; ++i) {
			res = register_func(pFuncDef[i]);
			if (!res) {
				PINT_DBG_MSG(FMT_BOLD FMT_RED "ERROR: " FMT_OFF " cannot register function %s", pFuncDef[i].pName);
				break;
			}
		}
	}
	return res;
}

bool FuncLibrary::register_func(const FuncDef& def) {
	const char* pKey = mpFuncMap->put(def.pName, def);
	return pKey != nullptr;
}

bool FuncLibrary::find(const char* pName, FuncDef* pDef) {
	return mpFuncMap->get(pName, pDef);
}

bool FuncLibrary::check_func_args(const FuncDef& def, const uint32_t nargs, const Value* pArgs) {
	bool res = true;
	if (nargs >= def.nargs) {
		for (uint32_t i = 0; i < def.nargs; ++i) {
			if (pArgs[i].type != def.argTypes[i]) {
				res = false;
				break;
			}
		}
	}
	return res;
}

FuncLibrary* FuncLibrary::create_default() {
	FuncLibrary* pFuncMapper = nxCore::tMem<FuncLibrary>::alloc();
	pFuncMapper->register_func(s_defFuncDesc, XD_ARY_LEN(s_defFuncDesc));
	return pFuncMapper;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

ExecContext::ExecContext() :
	mpStrs(nullptr),
	mpVarMap(nullptr),
	mpBinding(nullptr),
	mVarCnt(0),
	mErrCode(EvalError::NONE),
	mBreak(false) {}

ExecContext::~ExecContext() {
	reset();
}

void ExecContext::init(void* pBinding) {
	mpStrs = nullptr;
	mpBinding = pBinding;
	mVarCnt = 0;
	mErrCode = EvalError::NONE;
	mBreak = false;

	mpVarMap = VarMap::create();
}

void ExecContext::reset() {
	if (mpStrs) {
		cxStrStore::destroy(mpStrs);
		mpStrs = nullptr;
	}
	if (mpVarMap) {
		VarMap::destroy(mpVarMap);
		mpVarMap = nullptr;
	}

	mVarCnt = 0;
	mErrCode = EvalError::NONE;
	mBreak = false;
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
	PINT_DBG_MSG(FMT_BOLD "%d" FMT_OFF " variables\n", mVarCnt);
	for (uint32_t i = 0; i < mVarCnt; ++i) {
		const char* pVarName = mpVarNames[i];
		int varId = find_var(pVarName);
		PINT_DBG_MSG(FMT_BOLD "[%d]" FMT_OFF FMT_B_BLUE " %s" FMT_OFF ": ", varId, pVarName);
		Value* pVal = var_val(varId);
		if (pVal) {
			if (pVal->is_str()) {
				PINT_DBG_MSG(FMT_B_YELLOW "\"%s\"" FMT_OFF, pVal->val.pStr);
			} else if (pVal->is_num()) {
				PINT_DBG_MSG("%f", pVal->val.num);
			} else {
				PINT_DBG_MSG("--");
			}
		} else {

		}
		PINT_DBG_MSG("\n");
	}
}

void ExecContext::set_error(const EvalError errCode) {
	mErrCode = errCode;
	if (errCode != EvalError::NONE) {
		set_break(true);
	}
}

EvalError ExecContext::get_error() const { return mErrCode; }

void ExecContext::print_error() const {
	PINT_DBG_MSG(FMT_BOLD FMT_RED "ERROR: " FMT_OFF);
	switch(mErrCode) {
		case EvalError::BAD_VAR_CLAUSE:
			PINT_DBG_MSG("Invalid variable definition clause.\n");
			break;
		case EvalError::VAR_SYM:
			PINT_DBG_MSG("SYM type expected for a variable name.\n");
			break;
		case EvalError::VAR_CTX_ADD:
			PINT_DBG_MSG("Error adding variable to the context.\n");
			break;
		case EvalError::BAD_OPERAND_COUNT:
			PINT_DBG_MSG("A numeric operand expected.\n");
			break;
		case EvalError::BAD_OPERAND_TYPE_NUM:
			PINT_DBG_MSG("A numeric value expected.\n");
			break;
		case EvalError::BAD_OPERAND_TYPE_SYM:
			PINT_DBG_MSG("A symbol expected.\n");
			break;
		case EvalError::BAD_OPERAND_TYPE_STR:
			PINT_DBG_MSG("A string value expected.\n");
			break;
		case EvalError::VAR_NOT_FOUND:
			PINT_DBG_MSG("Variable not found.\n");
			break;
		case EvalError::BAD_IF_CLAUSE:
			PINT_DBG_MSG("Missing condition expression in 'if'.\n");
			break;
		case EvalError::BAD_FUNC_ARGS:
			PINT_DBG_MSG("Bad argument number or arguments types for a function call.\n");
			break;
		case EvalError::NONE:
		default:
			break;
	}
}

void ExecContext::set_break(const bool brk) {
	mBreak = brk;
}

bool ExecContext::should_break() const {
	return mBreak;
}

void ExecContext::set_mem_lock(sxLock* pLock) {
	mpVarMap->set_mem_lock(pLock);
}

void ExecContext::set_local_binding(void* pBinding) {
	mpBinding = pBinding;
}

void* ExecContext::get_local_binding() {
	return mpBinding;
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

static Value numop_logand(const Value& valA, const Value& valB) {
	Value val;
	val.set_num(uint64_t(valA.val.num) & uint64_t(valB.val.num));
	return val;
}

static Value numop_logxor(const Value& valA, const Value& valB) {
	Value val;
	val.set_num(uint64_t(valA.val.num) ^ uint64_t(valB.val.num));
	return val;
}

static Value numop_logior(const Value& valA, const Value& valB) {
	Value val;
	val.set_num(uint64_t(valA.val.num) | uint64_t(valB.val.num));
	return val;
}

static Value numop_eq(const Value& valA, const Value& valB) {
	Value val;
	val.set_num(double(valA.val.num == valB.val.num));
	return val;
}

static Value numop_ne(const Value& valA, const Value& valB) {
	Value val;
	val.set_num(double(valA.val.num != valB.val.num));
	return val;
}

static Value numop_gt(const Value& valA, const Value& valB) {
	Value val;
	val.set_num(double(valA.val.num > valB.val.num));
	return val;
}

static Value numop_ge(const Value& valA, const Value& valB) {
	Value val;
	val.set_num(double(valA.val.num >= valB.val.num));
	return val;
}

static Value numop_lt(const Value& valA, const Value& valB) {
	Value val;
	val.set_num(double(valA.val.num < valB.val.num));
	return val;
}

static Value numop_le(const Value& valA, const Value& valB) {
	Value val;
	val.set_num(double(valA.val.num <= valB.val.num));
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
	{ "logand", { numop_logand, 1.0 } },
	{ "logxor", { numop_logxor, 0.0 } },
	{ "logior", { numop_logior, 0.0 } },
	{ "=", { numop_eq, 0.0 } },
	{ "/=", { numop_ne, 0.0 } },
	{ ">", { numop_gt, 0.0 } },
	{ ">=", { numop_ge, 0.0 } },
	{ "<", { numop_lt, 0.0 } },
	{ "<=", { numop_le, 0.0 } },
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

CodeBlock::CodeBlock(ExecContext& ctx, FuncLibrary* pFuncLib) :
	mCtx(ctx),
	mpFuncLib(pFuncLib)
{
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
	FuncDef funcDef;
	for (uint32_t i = org; i < cnt; ++i) {
		CodeItem* pItem = &pLstItems[i];
		if (pItem->is_list()) {
			val = eval_sub(pItem->val.pLst);
		} else if (pItem->is_sym()) {
			if (nxCore::str_eq(pItem->val.sym, "if")) {
				if (i + 1 < cnt) {
					Value condVal = eval_sub(pLst, 1, 1);

					if (!!condVal.val.num) {
						if (i + 2 < cnt) {
							val = eval_sub(pLst, 2, 1);
						} else {
							mCtx.set_error(EvalError::BAD_IF_CLAUSE);
						}
					} else {
						if (i + 3 < cnt) {
							val = eval_sub(pLst, 3, 1);
						}
					}
					i = cnt;
				} else {
					mCtx.set_error(EvalError::BAD_IF_CLAUSE);
				}
			} else if (nxCore::str_eq(pItem->val.sym, "break")) {
				mCtx.set_break();
				i = cnt;
			} else if (nxCore::str_eq(pItem->val.sym, "defvar")) {
				if (i + 1 < cnt) {
					CodeItem* pVarNameItem = pItem + 1;
					if (pVarNameItem->is_sym()) {
						const char* pVarName = pVarNameItem->val.sym;
						int varId = mCtx.add_var(pVarName);
						if (varId >= 0) {
							Value* pVarVal = mCtx.var_val(varId);
							if (i + 2 < cnt) {
								val = eval_sub(pLst, 2, 1);
								i += 2;
								if (pVarVal) {
									*pVarVal = val;
								}
							} else {
								++i;
								pVarVal->set_none();
							}
						} else {
							mCtx.set_error(EvalError::VAR_CTX_ADD);
						}
					} else {
						mCtx.set_error(EvalError::VAR_SYM);
					}
				} else {
					mCtx.set_error(EvalError::BAD_VAR_CLAUSE);
				}
			} else if (nxCore::str_eq(pItem->val.sym, "set")) {
				if (i + 1 < cnt) {
					CodeItem* pVarNameItem = pItem + 1;
					const char* pVarName = pVarNameItem->val.sym;
					Value* pVal = mCtx.var_val(mCtx.find_var(pVarName));
					if (pVal) {
						if (i + 2 < cnt) {
							val = eval_sub(pLst, 2, 1);
							*pVal = val;
							i += 2;
						}
					} else {
						mCtx.set_error(EvalError::VAR_NOT_FOUND);
					}
				}
			} else if (nxCore::str_eq(pItem->val.sym, "eq")) {
				if (i + 2 < cnt) {
					Value valA, valB;
					valA = eval_sub(pLst, 1, 1);
					valB = eval_sub(pLst, 2, 1);
					i += 2;
					if (valA.is_str() && valB.is_str()) {
						val.set_num(double(nxCore::str_eq(valA.val.pStr, valB.val.pStr)));
						i = cnt;
					} else {
						mCtx.set_error(EvalError::BAD_OPERAND_TYPE_STR);
					}
				} else {
					mCtx.set_error(EvalError::BAD_OPERAND_COUNT);
				}
			} else if (nxCore::str_eq(pItem->val.sym, "ne")) {
				if (i + 2 < cnt) {
					Value valA, valB;
					valA = eval_sub(pLst, 1, 1);
					valB = eval_sub(pLst, 2, 1);
					i += 2;
					if (valA.is_str() && valB.is_str()) {
						val.set_num(double(!nxCore::str_eq(valA.val.pStr, valB.val.pStr)));
						i = cnt;
					} else {
						mCtx.set_error(EvalError::BAD_OPERAND_TYPE_STR);
					}
				} else {
					mCtx.set_error(EvalError::BAD_OPERAND_COUNT);
				}

			} else if (check_numop(pItem->val.sym, &numOpInfo)) {
				Value valA;
				Value valB;
				if (i + 2 > cnt) {
					mCtx.set_error(EvalError::BAD_OPERAND_COUNT);
				} else if (i + 2 == cnt) {
					valA.set_num(numOpInfo.unaryVal);
					valB = eval_sub(pLst, 1, 1);
					val = numOpInfo.apply(valA, valB);

					++i;
				} else {
					val = eval_sub(pLst, 1, 1);

					for (uint32_t j = 2; j < cnt; ++j) {
						valA = val;
						valB = eval_sub(pLst, j, 1);
						val = numOpInfo.apply(valA, valB);
					}
					i = cnt;
				}
			} else if (mpFuncLib && mpFuncLib->find(pItem->val.sym, &funcDef)) {
				uint32_t n = cnt - i - 1;
				Value args[FuncDef::MAX_ARGS];
				uint32_t nargs = nxCalc::min(n, FuncDef::MAX_ARGS);

				for(uint32_t j = 0; j < nargs; ++j) {
					args[j] = eval_sub(pLst, i + j + 1, 1);
					PINT_DBG_MSG("Arg %d : %f\n", j, args[j].val.num);
				}

				i += n;

				if (mpFuncLib->check_func_args(funcDef, nargs, args)) {
					val = (*funcDef.func)(mCtx, nargs, args);
				} else {
					mCtx.set_error(EvalError::BAD_FUNC_ARGS);
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

		if (mCtx.get_error() != EvalError::NONE) {
			i = cnt;
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
			PINT_DBG_MSG("%*c" FMT_B_BLUE "- LST" FMT_OFF " %p\n", lvl, ' ', item.val.pLst);
			print_sub(item.val.pLst, lvl+1);
		} else if (item.is_num()) {
			PINT_DBG_MSG("%*c" FMT_B_GREEN "NUM" FMT_OFF " %f\n", lvl, ' ', item.val.num);
		} else if (item.is_sym()) {
			PINT_DBG_MSG("%*c" FMT_B_GREEN "SYM" FMT_OFF " %s\n", lvl, ' ', item.val.sym);
		} else if (item.is_str()) {
			PINT_DBG_MSG("%*c" FMT_B_GREEN "STR" FMT_B_YELLOW " \"%s\"" FMT_OFF "\n", lvl, ' ', item.val.pStr);
		}
	}
}

void CodeBlock::print() const {
	if (mListCnt == 0) return;
	PINT_DBG_MSG("# lists: %d\n", mListCnt);
	print_sub(mpLists, 1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeItem::set_none() {
	type = Type::NON;
	val.num = 0;
}
bool CodeItem::is_none() const {
	return type == Type::NON;
}

void CodeItem::set_sym(const char* pStr) {
	type = Type::SYM;
	size_t sz = nxCalc::clamp(nxCore::str_len(pStr), size_t(0), Value::SYM_MAX_LEN);
	nxCore::mem_copy(val.sym, pStr, sz);
	val.sym[sz] = '\x0';
}
bool CodeItem::is_sym() const {
	return type == Type::SYM;
}

void CodeItem::set_num(double num) {
	type = Type::NUM;
	val.num = num;
}
bool CodeItem::is_num() const {
	return type == Type::NUM;
}

void CodeItem::set_str(const char* pStr) {
	type = Type::STR;
	val.pStr = pStr;
}
bool CodeItem::is_str() const {
	return type == Type::STR;
}

void CodeItem::set_list(CodeList* pLst) {
	type = Type::LST;
	val.pLst = pLst;
}
bool CodeItem::is_list() const {
	return type == Type::LST;
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

CodeItem* CodeList::get_items() const {
	return mpItems;
}

uint32_t CodeList::count() const {
	return mCount;
}

uint32_t CodeList::capacity() const {
	return mCapacity;
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

