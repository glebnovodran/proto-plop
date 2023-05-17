namespace Pint {

struct CodeItem;
class CodeList;
struct ListStack;
class ExecContext;

class SrcCode {
protected:
	const char* mpSrc;
	size_t mSrcSize;

	size_t mSrcLoc;
	size_t mLineLoc;
	size_t mLineNo;

public:
	struct Line {
		const char* pText;
		size_t textSize;
		size_t no;

		bool valid() const;

		void print() const;
	};

	SrcCode(const char* pSrc, size_t srcSize, const size_t cnkSize = 4096);

	~SrcCode() {}

	void restart();

	bool eof() const;

	Line get_line();
};

struct Value {
	static const size_t SYM_MAX_LEN = 63;

	enum class Type : uint32_t {
		NON = 0,
		NUM,
		STR
	};

	union {
		const char* pStr;
		double num;
	} val;

	Type type;

	void set_none();
	bool is_none() const;

	void set_num(double num);
	bool is_num() const;

	void set_str(const char* pStr);
	bool is_str() const;
};

enum class EvalError : int32_t {
	NONE = 0,
	BAD_VAR_CLAUSE = 1,           // bad defvar/set clause structure
	VAR_SYM = 2,                 // variable name should be a symbol
	VAR_CTX_ADD = 3,             // can't add variable to the exec context
	BAD_OPERAND_COUNT = 4,       // invalid operand type in calculation
	BAD_OPERAND_TYPE_NUM = 5,    // invalid operand type: NUM expected
	BAD_OPERAND_TYPE_SYM = 6,    // invalid operand type : SYM expected
	BAD_OPERAND_TYPE_STR = 7,    // invalid operand type : STR expected
	VAR_NOT_FOUND = 8,           // variable not found
	BAD_IF_CLAUSE = 9,           // missing condition expression in if
	BAD_FUNC_ARGS = 10,          // Bad argument number or arguments types for a function call
};

typedef Value (*Func)(ExecContext& ctx, const uint32_t nargs, Value* pArgs);

struct FuncDef {
	static const uint32_t MAX_ARGS = 10;
	const char* pName;
	Func func;
	uint32_t nargs;
	Value::Type resultType;
	Value::Type argTypes[MAX_ARGS];
};

class FuncLibrary {
protected:
	typedef cxStrMap<FuncDef> FuncMap;

	FuncMap* mpFuncMap;
public:
	FuncLibrary();
	~FuncLibrary();

	void init();
	void reset();
	bool register_func(const FuncDef* pFuncDef, const uint32_t nfunc);
	bool register_func(const FuncDef& def);

	bool find(const char* pName, FuncDef* pDef);
	bool check_func_args(const FuncDef& def, const uint32_t nargs, const Value* pArgs);

	static FuncLibrary* create_default();
};

class ExecContext {
protected:
	typedef cxStrMap<int> VarMap;
	static const size_t CODE_VAR_MAX = 256;

	cxStrStore* mpStrs;
	VarMap* mpVarMap;
	void* mpBinding;
	Value mVarVals[CODE_VAR_MAX];
	const char* mpVarNames[CODE_VAR_MAX];
	size_t mVarCnt;
	EvalError mErrCode;
	bool mBreak;
public:

	ExecContext();
	~ExecContext();

	void init(void* pBinding = nullptr);
	void reset();

	char* add_str(const char* pStr);

	int add_var(const char* pName);
	int find_var(const char* pName) const;
	Value* var_val(int id);

	void clear_vars();
	void print_vars();

	void set_break(const bool brk = true);
	bool should_break() const;

	void set_error(const EvalError errCode);
	EvalError get_error() const;
	void print_error() const;

	void set_local_binding(void* pBinding);
	void* get_local_binding();
};

struct CodeItem {
	static const size_t SYM_MAX_LEN = 63;

	enum class Type : uint32_t {
		NON = 0,
		SYM,
		NUM,
		STR,
		LST
	};

	union {
		char sym[SYM_MAX_LEN+1];
		const char* pStr;
		CodeList* pLst;
		double num;
	} val;

	Type type;

	void set_none();
	bool is_none() const;

	void set_sym(const char* pStr);
	bool is_sym() const;

	void set_num(double num);
	bool is_num() const;

	void set_str(const char* pStr);
	bool is_str() const;

	void set_list(CodeList* pLst);
	bool is_list() const;
};

#if !defined(PINT_CL_CHUNK_SZ)
	#define PINT_CL_CHUNK_SZ 16
#endif

class CodeList {
protected:
	CodeItem* mpItems;
	uint32_t mChunkSize;
	uint32_t mCount;
	uint32_t mCapacity;
public:
	CodeList(const uint32_t chunkSize = PINT_CL_CHUNK_SZ) // ifndef macro
	:
	mpItems(nullptr),
	mChunkSize(chunkSize),
	mCount(0),
	mCapacity(0)
	{
		init();
	}

	~CodeList() {
		reset();
	}

	void init();

	void reset();

	bool valid() const;

	void append(const CodeItem& itm);

	CodeItem* get_items() const;

	uint32_t count() const;
	uint32_t capacity() const;
};

struct ListStack {
	static const int CODE_LST_MAX = 256;

	CodeList* lists[CODE_LST_MAX];
	int ptr;

	void reset() { ptr = 0; }

	CodeList* top();

	void push(CodeList* pLst);

	CodeList* pop();
};

class CodeBlock : public cxLexer::TokenFunc {
protected:
	ExecContext& mCtx;
	FuncLibrary* mpFuncLib;
	ListStack mListStack;
	CodeList mLists[ListStack::CODE_LST_MAX];
	uint32_t mListCnt;

	void print_sub(const CodeList* lst, int lvl = 0) const;

	bool eval_numeric_values(CodeList* pLst, const uint32_t org, const uint32_t slice, Value* pValues);

	Value eval_sub(CodeList* pLst, const uint32_t org = 0, const uint32_t slice = 0);

public:
	CodeBlock(ExecContext& ctx, FuncLibrary* pFuncLib = nullptr);

	~CodeBlock();

	CodeList* new_list();

	virtual bool operator()(const cxLexer::Token& tok);

	void parse(const SrcCode::Line& line);

	void eval();

	void init();

	void reset();

	void print() const;
};

void interp(const char* pSrc, size_t srcSize, ExecContext* pCtx, FuncLibrary* pFuncLib);

void set_mem_lock(sxLock* pLock);

} // Pint
