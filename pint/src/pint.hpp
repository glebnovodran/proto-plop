namespace Pint {

struct CodeItem;
struct CodeList;

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
	ExecContext() {
		mpStrs = cxStrStore::create();
	}

	~ExecContext() {
		cxStrStore::destroy(mpStrs);
	}

	char* add_str(const char* pStr);

	void print_vars() const;
};

class CodeBlock : public cxLexer::TokenFunc {
protected:
	ExecContext& mCtx;
public:
	CodeBlock(ExecContext& ctx) : mCtx(ctx) {}

	virtual bool operator()(const cxLexer::Token& tok);

	void parse(const SrcCode::Line& line);
	
	void eval();

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

struct CodeList {
	uint32_t num;
	uint32_t reserved;
	CodeItem pItem[1];
};

void interp(const char* pSrcPath);

} // Pint
