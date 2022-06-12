/* SPDX-License-Identifier: MIT */
/* SPDX-FileCopyrightText: 2022 Glib Novodran <novodran@gmail.com> */

#include <crosscore.hpp>

struct sxPlopData : sxData {
	static const uint32_t KIND = XD_FOURCC('P', 'L', 'O', 'P');
	struct BlockEntry {
		uint32_t mOffs;
		uint32_t mLen;
	};
	uint32_t mHeadTag;
	uint32_t mBlockNum;
	uint32_t mBodyOffs;
	BlockEntry mBlks[1];

	uint32_t* get_block_code(const uint32_t bkid) const {
		uint32_t offs = mBlks[bkid].mOffs;
		uint8_t* pCodeBytes = XD_INCR_PTR(this, offs);
		return reinterpret_cast<uint32_t*>(pCodeBytes);
	}

	size_t get_plop_size() const {
		uint32_t* pFirst = get_block_code(0);
		uint32_t len0 = mBlks[0].mLen;
		uint32_t* pLast = get_block_code(mBlockNum - 1);
		size_t len = mBlks[mBlockNum - 1].mLen;
		uint8_t* pEnd = XD_INCR_PTR(pLast, len * sizeof(uint32_t));

		ptrdiff_t diff = pEnd - reinterpret_cast<const uint8_t*>(this);
		return diff;
	}

	void save(FILE* pOut) {
		size_t sz = get_plop_size();
		::fwrite(reinterpret_cast<void*>(this), sz, 1, pOut);
	}

	void save (const char* pOutPath) {
		FILE* pOut = nxSys::fopen_w_bin(pOutPath);
		if (!pOut) {
			return;
		}
		save(pOut);
		::fclose(pOut);
	}
};

struct sxDrama : sxData {

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

	sxPlopData* get_plop_data(const int32_t plopId) const {
		return (plopId < mPlopNum) && (plopId >= 0) ? reinterpret_cast<sxPlopData*>(XD_INCR_PTR(this, mPlopCat[plopId])) : nullptr;
	}

	void dump_plop_info(FILE* pOut, sxPlopData* pPlop, const uint32_t id, const char* pBinName) {
		if (pPlop) {
			::fprintf(pOut, "[ plop id:[%d] ; nblk: %d ]\n", id,  pPlop ? pPlop->mBlockNum : 0);
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

			sxPlopData* pPlop = get_plop_data(pNode->mBefore);
			if (pPlop) {
#if defined(_MSC_VER)
				::sprintf_s(buf, "before_%d.plop", i);
#else
				::sprintf(buf, "before_%d.plop", i);
#endif
				dump_plop_info(pOut, pPlop, pNode->mBefore, savePlops ? buf : nullptr);
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
		sxDrama* pDrama = pData->as<sxDrama>();
		pDrama->dump_info(pOutPath ? pOutPath : "drama_dump.txt", savePlops);
	}

	nxApp::reset();
	return 0;
}