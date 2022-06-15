/* SPDX-License-Identifier: MIT */
/* SPDX-FileCopyrightText: 2022 Glib Novodran <novodran@gmail.com> */
#include <crosscore.hpp>

#include "plop.hpp"

#if defined(PLOP_OP) 
#	undef PLOP_OP
#endif

#define PLOP_OP(SYM, ID) #SYM ,

static const char* s_opNames[] = { 
#		include "plop_op.inc"
};

const uint32_t PlopData::KIND = XD_FOURCC('P', 'L', 'O', 'P');

void PlopData::disasm(FILE* pOut) {
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
