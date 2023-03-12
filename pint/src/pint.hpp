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
public:
	struct Line {
		const char* pText;
		size_t textSize;

		bool valid() const;

		void print() const;
	};

	SrcCode(const char* pSrc, size_t srcSize, const size_t cnkSize = 4096)
	:
	mpSrc(pSrc),
	mSrcSize(srcSize),
	mpLineBuf(nullptr),
	mLineBufSize(0),
	mCur(0)
	{
		mChunkSize = nxCalc::clamp(cnkSize, (size_t)(16), (size_t)(1024 * 1024));
	}

	void reset();

	bool eof() const;

	Line get_line();
};

class ExecContext {
protected:
	cxStrStore* mpStrs;
public:
	ExecContext();

	~ExecContext();

	void reset();

	char* add_str(const char* pStr);

	void print_vars() const;
};

class CodeBlock : public cxLexer::TokenFunc {
protected:
	ExecContext& mCtx;
	ListStack* mpListStack;
	CodeList* mpLists;
	uint32_t mNumAllocList;

	void print_sub(const CodeList* lst, int lvl = 0) const;

public:
	CodeBlock(ExecContext& ctx);

	~CodeBlock();

	ListStack* get_stack() const { return mpListStack; }

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
	uint32_t mNumItems;
	uint32_t mCapacity;
public:
	CodeList(const uint32_t chunkSize = 2)
	:
	mpItems(nullptr),
	mChunkSize(chunkSize),
	mNumItems(0)
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

	CodeItem* get_items() const { return mpItems; }

	uint32_t size() const { return mNumItems;}
	uint32_t capacity() const { return mCapacity; }
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
