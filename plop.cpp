/* SPDX-License-Identifier: MIT */
/* SPDX-FileCopyrightText: 2022 Glib Novodran <novodran@gmail.com> */

#include <crosscore.hpp>

#include "plop.hpp"


static const char* s_opNames[] = { 
#if defined(PLOP_OP) 
#	undef PLOP_OP
#endif
#define PLOP_OP(SYM, ID) #SYM ,
#include "plop_op.inc"
#undef PLOP_OP
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
			uint32_t strid = uint32_t(op) - uint32_t(Op::_BASE_);
			const char* pOpName = s_opNames[uint32_t(op) - uint32_t(Op::_BASE_)];
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
				case Op::NOP:
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
				case Op::LSET: {
						sid = pCode[i];
						uint32_t val = pCode[i + 1];
						::fprintf(pOut, " %d <%d>; %s\n", sid, val, get_str(sid));
						i += 2;
					}
					break;
				case Op::LGET: {
						sid = pCode[i];
						::fprintf(pOut, " %d; %s\n", sid, get_str(sid));
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
					}
					break;
				case Op::ADD:
				case Op::SUB:
				case Op::MUL:
				case Op::DIV:
				case Op::NEG:
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
				case Op::CALL:
				case Op::LIST: {
						uint32_t narg = pCode[i];
						::fprintf(pOut, " ( %d )\n", narg);
						++i;
					}
					break;

			}
		}

	}
}
