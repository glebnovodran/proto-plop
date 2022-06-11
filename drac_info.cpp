/* SPDX-License-Identifier: MIT */
/* SPDX-FileCopyrightText: 2022 Glib Novodran <novodran@gmail.com> */

#include <crosscore.hpp>

struct sxPlopData : sxData {
	static const uint32_t KIND = XD_FOURCC('P', 'L', 'O', 'P');

	uint32_t mHeadTag;
	uint32_t mBlockNum;
	uint32_t mBodyOffs;
	uint32_t mBlks[1];

	uint32_t* get_block_code(const uint32_t bkid) const {
		return reinterpret_cast<uint32_t*>(XD_INCR_PTR(this, mBlks[bkid]));
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

	void dump_info(FILE* pOut) {
		nxCore::dbg_msg("Dumping %s", pOut);
		::fprintf(pOut, "Total %d nodes\n", mNodeNum);
		NodeInfo* pNodes = get_node_top();
		for(uint32_t i = 0; i < mNodeNum; ++i) {
			::fprintf(pOut, "____________________________________\n");
			NodeInfo* pNode = &pNodes[i];
			::fprintf(pOut, "Node %d id='%s'\n\n", i, get_str(pNode->mId));

			sxPlopData* pPlop = get_plop_data(pNode->mBefore);
			::fprintf(pOut, "Before plop id:[%d], nblk: %d\n", pNode->mBefore,  pPlop ? pPlop->mBlockNum : 0);

			pPlop = get_plop_data(pNode->mAfter);
			::fprintf(pOut, "After plop id:[%d], nblk: %d\n", pNode->mAfter, pPlop ? pPlop->mBlockNum : 0);

			::fprintf(pOut, "Player says:%s\n", pNode->mPlSay >= 0 ? get_str(pNode->mPlSay) : "[MUTE]");
			::fprintf(pOut, "Character says:%s\n", pNode->mSay >= 0 ? get_str(pNode->mSay) : "[MUTE]");
		}
	}

	void dump_info(const char* pOutPath) {
		FILE* pOut = nxSys::fopen_w_txt(pOutPath);
		if (!pOut) {
			return;
		}
		dump_info(pOut);
		::fclose(pOut);
	}
};

int main(int argc, char* argv[]) {
	nxApp::init_params(argc, argv);

	const char* pPath = nxApp::get_arg(0);
	const char* pOutPath = nxApp::get_opt("out");
	sxData* pData = nxData::load(pPath);
	if (pData) {
		sxDrama* pDrama = pData->as<sxDrama>();
		pDrama->dump_info(pOutPath ? pOutPath : "drama_dump.txt");
	}

	nxApp::reset();
	return 0;
}