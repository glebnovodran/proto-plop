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

		bool valid() const { return pText != nullptr && textSize > 0;}

		void print() {
			if (valid()) {
				for (size_t i = 0; i < textSize; ++i) {
					nxCore::dbg_msg("%c", pText[i]);
				}
				nxCore::dbg_msg("\n");
			} else {
				nxCore::dbg_msg("invalid\n");
			}
		}
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

	bool eof() const { return mCur >= mSrcSize; }
	Line get_line();
};

void interp(const char* pSrcPath);

} // Pint
