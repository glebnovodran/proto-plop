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

static Pint::Value glb_plr_kind(Pint::ExecContext& ctx, const uint32_t nargs, Pint::Value* pArgs) {
	Pint::Value res;
	res.set_num(1.0);
	int id = ctx.find_var("plr_kind");
	if (id >= 0) {
		Pint::Value* pVal = ctx.var_val(id);
		pVal->set_num(42.0);
	}
	nxCore::dbg_msg("Calling a global function.\n");
	return res;
}

static Pint::Value glb_plr_kind2(Pint::ExecContext& ctx, const uint32_t nargs, Pint::Value* pArgs) {
	Pint::Value res;
	res.set_num(1.0);
	int id = ctx.find_var("plr_kind");
	if (id >= 0) {
		Pint::Value* pVal = ctx.var_val(id);
		pVal->set_num(42.0);
	}
	nxCore::dbg_msg("Calling a global function kind2.\n");
	return res;
}

static const Pint::FuncDef s_glb_plr_kind_desc = {
	"glb_plr_kind", glb_plr_kind, 0, Pint::Value::Type::NUM, {}
};

static const Pint::FuncDef s_glb_plr_kind2_desc = {
	"glb_plr_kind", glb_plr_kind2, 0, Pint::Value::Type::NUM, {}
};

int main(int argc, char* argv[]) {
	nxApp::init_params(argc, argv);
	init_sys();

	if (nxApp::get_args_count() < 1) {
		nxCore::dbg_msg("pint_test <src_path>\n");
	} else {
		const char* pSrcPath = nxApp::get_arg(0);
		if (pSrcPath) {
			size_t srcSize = 0;
			char* pSrc = (char*)nxCore::raw_bin_load(pSrcPath, &srcSize);
			if (pSrc) {
				Pint::ExecContext ctx;
				Pint::FuncLibrary funcLib;

				ctx.init();
				funcLib.init();

				funcLib.register_func(s_glb_plr_kind_desc);
				funcLib.register_func(s_glb_plr_kind2_desc);

				nxCore::rng_seed(1);

				Pint::interp(pSrc, srcSize, &ctx, &funcLib);

				Pint::EvalError err = ctx.get_error();
				if (err != Pint::EvalError::NONE) {
					ctx.print_error();
				}

				ctx.print_vars();
				ctx.reset();

				nxCore::bin_unload(pSrc);
			} else {
				nxCore::dbg_msg("Unable to load \"%s\"\n", pSrcPath);
			}
		}
	}

	nxApp::reset();
	nxCore::mem_dbg();
	reset_sys();

	return 0;
}

