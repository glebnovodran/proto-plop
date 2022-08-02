/* SPDX-License-Identifier: MIT */
/* SPDX-FileCopyrightText: 2022 Glib Novodran <novodran@gmail.com> */

#include <crosscore.hpp>
#include "plot_prog.hpp"

int main(int argc, char* argv[]) {
	nxApp::init_params(argc, argv);

	const char* pPath = nxApp::get_arg(0);
	const char* pOutPath = nxApp::get_opt("out");
	pOutPath = pOutPath ? pOutPath : "./out.dis";

	sxData* pData = nxData::load(pPath);
	if (pData) {
		PlopData* pPlopData = pData->as<PlopData>();
		pPlopData->disasm(pOutPath);
	}

	nxApp::reset();
	return 0;
}