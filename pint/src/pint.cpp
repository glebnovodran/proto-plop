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
		nxCore::dbg_msg("\nLine %d, size : %d\n", i, line.textSize);
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
	if (mpLineBuf == nullptr) {
		mpLineBuf = reinterpret_cast<char*>(nxCore::mem_alloc(mChunkSize));
		mLineBufSize = mChunkSize;
	}

	size_t lineStart = mCur;
	size_t skipped = 0;
	size_t linePos = 0;
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

	SrcCode::Line line = {mpLineBuf, linePos};
	return line;
}

} // Pint

