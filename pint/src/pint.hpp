namespace Pint {

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

	void reset() {
		if (mpLineBuf) {
			nxCore::mem_free(mpLineBuf);
			mpLineBuf = nullptr;
			mLineBufSize = 0;
			mCur = 0;
			mpSrc = nullptr;
			mSrcSize = 0;
		}
	}

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


void interp(const char* pSrcPath);

} // Pint
