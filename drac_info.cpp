/* SPDX-License-Identifier: MIT */
/* SPDX-FileCopyrightText: 2022 Glib Novodran <novodran@gmail.com> */

#include <crosscore.hpp>
#include "plop.hpp"


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
				XD_SPRINTF(XD_SPRINTF_BUF(buf, sizeof(buf)), "before_%d.plop", i);
				dump_plop_info(pOut, pPlop, pNode->mBefore, savePlops ? buf : nullptr);

				XD_SPRINTF(XD_SPRINTF_BUF(buf, sizeof(buf)), "before_%d.dpl", i);
				pPlop->disasm(buf);
			} else {
				::fprintf(pOut, "[NONE]\n");
			}

			::fprintf(pOut, "[Player says]: %s\n", pNode->mPlSay >= 0 ? get_str(pNode->mPlSay) : "[NONE]");
			::fprintf(pOut, "[Character says]: %s\n", pNode->mSay >= 0 ? get_str(pNode->mSay) : "[NONE]");

			::fprintf(pOut, "[After]: ");

			pPlop = get_plop_data(pNode->mAfter);
			if (pPlop) {
				XD_SPRINTF(XD_SPRINTF_BUF(buf, sizeof(buf)), "after_%d.plop", i);
				dump_plop_info(pOut, pPlop, pNode->mAfter, savePlops ? buf : nullptr);

				XD_SPRINTF(XD_SPRINTF_BUF(buf, sizeof(buf)), "after_%d.dpl", i);
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