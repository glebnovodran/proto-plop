namespace Pint {

struct CodeItem;
class CodeList;
struct ListStack;

class SrcCode {
protected:
	const char* mpSrc;
	size_t mSrcSize;
	size_t mChunkSize;

	char* mpLineBuf;
	size_t mLineBufSize;

	size_t mCur;
	size_t mLineNo;
public:
	struct Line {
		const char* pText;
		size_t textSize;
		size_t no;

		bool valid() const;

		void print() const;
	};

	SrcCode(const char* pSrc, size_t srcSize, const size_t cnkSize = 4096)
	:
	mpSrc(pSrc),
	mSrcSize(srcSize),
	mpLineBuf(nullptr),
	mLineBufSize(0),
	mCur(0),
	mLineNo(0)
	{
		mChunkSize = nxCalc::clamp(cnkSize, (size_t)(16), (size_t)(1024 * 1024));
	}

	void reset();

	bool eof() const;

	Line get_line();
};

struct Value {
	static const size_t SYM_MAX_LEN = 63;

	enum class Kind : uint32_t {
		NON = 0,
		NUM,
		STR
	};

	union {
		const char* pStr;
		double num;
	} val;

	Kind kind;

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
};

class ExecContext {
protected:
	typedef cxStrMap<int> VarMap;
	static const size_t CODE_VAR_MAX = 256;
	cxStrStore* mpStrs;
	VarMap* mpVarMap;
	Value mVarVals[CODE_VAR_MAX];
	const char* mpVarNames[CODE_VAR_MAX];
	uint32_t mVarCnt;
	EvalError mErrCode;
	bool mBreak;
public:

	ExecContext();

	~ExecContext();

	void init();
	void reset();

	char* add_str(const char* pStr);

	int add_var(const char* pName);
	int find_var(const char* pName) const;
	Value* var_val(int id);

	void print_vars();

	void set_break(const bool brk = true);
	bool should_break() const;

	void set_error(const EvalError errCode);
	EvalError get_error() const;
	void print_error() const;

	void set_mem_lock(sxLock* pLock);
};

class CodeBlock : public cxLexer::TokenFunc {
protected:
	ExecContext& mCtx;
	ListStack* mpListStack;
	CodeList* mpLists;
	uint32_t mListCnt;

	void print_sub(const CodeList* lst, int lvl = 0) const;

	bool eval_numeric_values(CodeList* pLst, const uint32_t org, const uint32_t slice, Value* pValues);

	Value eval_sub(CodeList* pLst, const uint32_t org = 0, const uint32_t slice = 0);

public:
	CodeBlock(ExecContext& ctx);

	~CodeBlock();

	CodeList* new_list();

	virtual bool operator()(const cxLexer::Token& tok);

	void parse(const SrcCode::Line& line);
	
	void eval();

	void reset();

	void print() const;
};

struct CodeItem {
	static const size_t SYM_MAX_LEN = 63;

	enum class Kind : uint32_t {
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

	Kind kind;

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

class CodeList {
protected:
	CodeItem* mpItems;
	uint32_t mChunkSize;
	uint32_t mCount;
	uint32_t mCapacity;
public:
	CodeList(const uint32_t chunkSize = 2)
	:
	mpItems(nullptr),
	mChunkSize(chunkSize),
	mCount(0)
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

void interp(const char* pSrcPath);

} // Pint
