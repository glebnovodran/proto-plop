namespace Pint {

class SrcCode {
protected:
	const char* mpSrc;
	size_t mSrcSize;
	size_t mChunkSize;

public:
	struct Line {
		const char* pText;
		size_t textSize;

		bool valid() const { return pText != nullptr && textSize > 0;}
	};

	SrcCode(const char* pSrc, size_t srcSize, const size_t cnkSize = 4096)
	:
	mpSrc(pSrc),
	mSrcSize(srcSize)
	{
		mChunkSize = nxCalc::clamp(cnkSize, (size_t)(16), (size_t)(1024 * 1024));
	}

	bool eof() const;
	Line get_line();
};

void interp(const char* pSrcPath);

} // Pint
