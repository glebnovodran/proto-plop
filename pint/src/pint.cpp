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
	int i = 0;
	while (!src.eof()) {
		SrcCode::Line line = src.get_line();
		nxCore::dbg_msg("Line %d, size : %d\n", i, line.textSize);
		line.print();
		++i;
	}
	//
	// . . .
	//
	src.reset();
	nxCore::bin_unload(pSrc);
}

SrcCode::Line SrcCode::get_line() {
	size_t skipped = 0;
	if (mpLineBuf == nullptr) {
		mpLineBuf = reinterpret_cast<char*>(nxCore::mem_alloc(mChunkSize));
		mLineBufSize = mChunkSize;
	}

	size_t lineStart = mCur;

	for (size_t cur = mCur, i = 0; cur < mSrcSize; ++cur) {
		if (cur >= mLineBufSize) {
			mpLineBuf = reinterpret_cast<char*>(nxCore::mem_realloc(mpLineBuf, mLineBufSize + mChunkSize));
			mLineBufSize += mChunkSize;
		}
		char ch = mpSrc[cur];
		if (ch == '\r') {
			skipped++;
		} else {
			mpLineBuf[i++] = ch;
		}
		mCur++;
		if (ch == '\n') {
			break;
		}
	}

	SrcCode::Line line = {mpLineBuf, (mCur - skipped) - lineStart};
	return line;
}

} // Pint

