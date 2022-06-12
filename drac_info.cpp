/* SPDX-License-Identifier: MIT */
/* SPDX-FileCopyrightText: 2022 Glib Novodran <novodran@gmail.com> */

#include <crosscore.hpp>


static const char* s_opNames[] = { "BEGIN", "END", "VAR", "SYM", "SET", "FVAL", "SVAL", "IF", "+", "-", "*", "/",
									"=", "!=", "<", ">", "<=", ">=", "NOT", "AND", "OR", "XOR", "MIN", "MAX", "CALL" };

struct PlopData : sxData {
	static const uint32_t KIND = XD_FOURCC('P', 'L', 'O', 'P');

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

	void disasm(FILE* pOut) {
		char buf[256];
		for (uint32_t bkid = 0, lvl = 0; bkid < mBlkNum; ++bkid) {
			uint32_t* pCode = get_block_code(bkid);
			uint32_t len = mBlks[bkid].mLen;
			uint32_t sid = -1;

			for (uint32_t i = 0; i < len;) {
				Op op = Op(pCode[i]);
				::fprintf(pOut, "%3d:", i);
				lvl = (op == Op::END) && (lvl > 0) ? lvl - 1 : lvl;
				for (uint32_t l = 0; l < lvl; ++l) { ::fprintf(pOut, "\t"); }

				::fprintf(pOut, "%s ", s_opNames[uint32_t(op) - uint32_t(Op::_BASE_)]);
				++i;

				switch (op) {
					case Op::BEGIN: {
							uint32_t eloc = pCode[i];
							::fprintf(pOut, " < %d >\n", eloc);
							++i;
							++lvl;
						}
						break;
					case Op::END:
						::fprintf(pOut, "\n");
						break;
					case Op::VAR:
					case Op::SYM:
					case Op::SET: {
							sid = pCode[i];
							::fprintf(pOut, " %d ; %s\n", sid, get_str(sid));
							++i;
						}
						break;
					case Op::FVAL: {
							float fval = nxCore::f32_set_bits(pCode[i]);
							::fprintf(pOut, " %f\n", fval);
							++i;
						}
						break;
					case Op::SVAL: {
							sid = pCode[i];
							::fprintf(pOut, " %d ; %s\n", sid, get_str(sid));
							++i;
						}
						break;
					case Op::IF: {
							uint32_t yes = pCode[i];
							uint32_t no = pCode[i + 1];
							::fprintf(pOut, "(%d, %d)\n", yes, no);
							i += 2;
							//++lvl;
						}
						break;
					case Op::ADD:
					case Op::SUB:
					case Op::MUL: 
					case Op::DIV: 
					case Op::EQ : 
					case Op::NE : 
					case Op::LT : 
					case Op::GT : 
					case Op::LE : 
					case Op::GE : 
					case Op::NOT: 
					case Op::AND: 
					case Op::OR : 
					case Op::XOR: 
					case Op::MIN: 
					case Op::MAX: 
					case Op::CALL: {
							uint32_t narg = pCode[i];
							::fprintf(pOut, " ( %d )\n", narg);
							++i;
						}
						break;

				}
			}

		}
	}

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
};

struct Drama : sxData {

	static const uint32_t KIND = XD_FOURCC('D', 'R', 'A', 'C');

	uint32_t mHeadTag;
	uint32_t mNodeNum;
	uint32_t mPlopNum;
	uint32_t mNodesOffs;
	uint32_t mPlopCat[1];

	struct NodeInfo {
		int32_t mId;
		int32_t mBefore;
		int32_t mAfter;
		int32_t mPlSay;
		int32_t mSay;
	};

	NodeInfo* get_node_top() const {
		return mNodesOffs ? reinterpret_cast<NodeInfo*>(XD_INCR_PTR(this, mNodesOffs)) : nullptr;
	}

	PlopData* get_plop_data(const int32_t plopId) const {
		return (plopId < mPlopNum) && (plopId >= 0) ? reinterpret_cast<PlopData*>(XD_INCR_PTR(this, mPlopCat[plopId])) : nullptr;
	}

	void dump_plop_info(FILE* pOut, PlopData* pPlop, const uint32_t id, const char* pBinName) {
		if (pPlop) {
			::fprintf(pOut, "[ plop id:[%d] ; nblk: %d ]\n", id,  pPlop ? pPlop->mBlkNum : 0);
			if (pBinName) {
				pPlop->save(pBinName);
			}
		}
	}

	void dump_info(FILE* pOut, const bool savePlops) {
		char buf[32] = {};
		nxCore::dbg_msg("Dumping %s", pOut);
		::fprintf(pOut, "Total %d nodes\n", mNodeNum);
		NodeInfo* pNodes = get_node_top();
		for(uint32_t i = 0; i < mNodeNum; ++i) {
			::fprintf(pOut, "____________________________________\n");
			NodeInfo* pNode = &pNodes[i];
			::fprintf(pOut, "Node %d id='%s'\n\n", i, get_str(pNode->mId));
			::fprintf(pOut, "[Before]: ");

			PlopData* pPlop = get_plop_data(pNode->mBefore);
			if (pPlop) {
#if defined(_MSC_VER)
				::sprintf_s(buf, "before_%d.plop", i);
#else
				::sprintf(buf, "before_%d.plop", i);
#endif
				dump_plop_info(pOut, pPlop, pNode->mBefore, savePlops ? buf : nullptr);

#if defined(_MSC_VER)
				::sprintf_s(buf, "before_%d.dpl", i);
#else
				::sprintf(buf, "before_%d.dpl", i);
#endif
				pPlop->disasm(buf);
			} else {
				::fprintf(pOut, "[NONE]\n");
			}

			::fprintf(pOut, "[Player says]: %s\n", pNode->mPlSay >= 0 ? get_str(pNode->mPlSay) : "[NONE]");
			::fprintf(pOut, "[Character says]: %s\n", pNode->mSay >= 0 ? get_str(pNode->mSay) : "[NONE]");

			::fprintf(pOut, "[After]: ");

			pPlop = get_plop_data(pNode->mAfter);
			if (pPlop) {
#if defined(_MSC_VER)
				::sprintf_s(buf, "after_%d.plop", i);
#else
				::sprintf(buf, "after_%d.plop", i);
#endif
				dump_plop_info(pOut, pPlop, pNode->mAfter, savePlops ? buf : nullptr);

#if defined(_MSC_VER)
				::sprintf_s(buf, "after_%d.dpl", i);
#else
				::sprintf(buf, "after_%d.dpl", i);
#endif
				pPlop->disasm(buf);
			}

		}
	}

	void dump_info(const char* pOutPath, bool savePlop) {
		FILE* pOut = nxSys::fopen_w_txt(pOutPath);
		if (!pOut) {
			return;
		}
		dump_info(pOut, savePlop);
		::fclose(pOut);
	}
};

int main(int argc, char* argv[]) {
	nxApp::init_params(argc, argv);

	const char* pPath = nxApp::get_arg(0);
	const char* pOutPath = nxApp::get_opt("out");
	bool savePlops = nxApp::get_bool_opt("saveplop");
	sxData* pData = nxData::load(pPath);
	if (pData) {
		Drama* pDrama = pData->as<Drama>();
		pDrama->dump_info(pOutPath ? pOutPath : "drama_dump.txt", savePlops);
	}

	nxApp::reset();
	return 0;
}