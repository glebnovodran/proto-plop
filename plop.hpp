/* SPDX-License-Identifier: MIT */
/* SPDX-FileCopyrightText: 2022 Glib Novodran <novodran@gmail.com> */

struct PlopData : sxData {

	enum class Op : uint32_t {
		_BASE_ = 100,
#if defined(PLOP_OP) 
#	undef PLOP_OP
#endif
#define PLOP_OP(SYM, ID) SYM = _BASE_ + ID ,
#include "plop_op.inc"
#undef PLOP_OP
	};

	struct BlockEntry {
		uint32_t mOffs;
		uint32_t mLen;
	};

	uint32_t mHeadTag;
	uint32_t mBlkNum;
	uint32_t mBodyOffs;
	BlockEntry mBlks[1];

	uint32_t* get_block_code(const uint32_t bkid) const {
		uint32_t offs = mBlks[bkid].mOffs;
		uint8_t* pCodeBytes = XD_INCR_PTR(this, offs);
		return reinterpret_cast<uint32_t*>(pCodeBytes);
	}

	size_t get_plop_size() const {
		uint32_t* pLast = get_block_code(mBlkNum - 1);
		size_t len = mBlks[mBlkNum - 1].mLen;
		uint8_t* pEnd = XD_INCR_PTR(pLast, len * sizeof(uint32_t));

		ptrdiff_t diff = pEnd - reinterpret_cast<const uint8_t*>(this);
		return diff;
	}

	void disasm(FILE* pOut);

	void disasm(const char* pOutPath) {
		FILE* pOut = nxSys::fopen_w_bin(pOutPath);
		if (!pOut) {
			return;
		}
		disasm(pOut);
		::fclose(pOut);
	}

	void save(FILE* pOut) {
		size_t sz = get_plop_size();
		::fwrite(reinterpret_cast<void*>(this), sz, 1, pOut);
	}

	void save(const char* pOutPath) {
		FILE* pOut = nxSys::fopen_w_bin(pOutPath);
		if (!pOut) {
			return;
		}
		save(pOut);
		::fclose(pOut);
	}

	static const uint32_t KIND;
};
