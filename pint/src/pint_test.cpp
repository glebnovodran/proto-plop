#include "crosscore.hpp"
#include "pint.hpp"

static void dbgmsg_impl(const char* pMsg) {
	::fprintf(stderr, "%s", pMsg);
	::fflush(stderr);
}

static void init_sys() {
	sxSysIfc sysIfc;
	nxCore::mem_zero(&sysIfc, sizeof(sysIfc));
	sysIfc.fn_dbgmsg = dbgmsg_impl;
	nxSys::init(&sysIfc);
}

static void reset_sys() {
}

int main(int argc, char* argv[]) {
	nxApp::init_params(argc, argv);
	init_sys();

	if (nxApp::get_args_count() < 1) {
		nxCore::dbg_msg("pint_test <src_path>\n");
	} else {
		const char* pSrcPath = nxApp::get_arg(0);
		Pint::interp(pSrcPath);
	}

	nxApp::reset();
	nxCore::mem_dbg();
	reset_sys();

	return 0;
}

