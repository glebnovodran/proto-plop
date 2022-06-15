/* SPDX-License-Identifier: MIT */
/* SPDX-FileCopyrightText: 2022 Glib Novodran <novodran@gmail.com> */

struct PlopData : sxData {

	enum class Op : uint32_t {
		_BASE_ = 100,
		BEGIN  = _BASE_ + 0,
		END    = _BASE_ + 1,
		VAR    = _BASE_ + 2,
		SYM    = _BASE_ + 3,
		SET    = _BASE_ + 4,
		FVAL   = _BASE_ + 5,
		SVAL   = _BASE_ + 6,
		IF     = _BASE_ + 7,
		ADD    = _BASE_ + 8,
		SUB    = _BASE_ + 9,
		MUL    = _BASE_ + 10,
		DIV    = _BASE_ + 11,
		EQ     = _BASE_ + 12,
		NE     = _BASE_ + 13,
		LT     = _BASE_ + 14,
		GT     = _BASE_ + 15,
		LE     = _BASE_ + 16,
		GE     = _BASE_ + 17,
		NOT    = _BASE_ + 18,
		AND    = _BASE_ + 19,
		OR     = _BASE_ + 20,
		XOR    = _BASE_ + 21,
		MIN    = _BASE_ + 22,
		MAX    = _BASE_ + 23,
		CALL   = _BASE_ + 24,
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
