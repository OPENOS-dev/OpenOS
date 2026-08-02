// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2015 Google LLC
 */

#include "lib/nonspd_modules.h"

/* Common manufacturing IDs */
#define MFGID_HYNIX                      \
	{                                \
		.msb = 0xad, .lsb = 0x80 \
	}
#define MFGID_LONGSYS                    \
	{                                \
		.msb = 0xd6, .lsb = 0x8a \
	}
#define MFGID_MICRON                     \
	{                                \
		.msb = 0x2c, .lsb = 0x80 \
	}
#define MFGID_NANYA                      \
	{                                \
		.msb = 0x0b, .lsb = 0x83 \
	}
#define MFGID_SAMSUNG                    \
	{                                \
		.msb = 0xce, .lsb = 0x80 \
	}
#define MFGID_SANDISK                    \
	{                                \
		.msb = 0x45, .lsb = 0x80 \
	}

#define MFGID_CXMT                       \
	{                                \
		.msb = 0x91, .lsb = 0x8a \
	}

const struct nonspd_mem_info hynix_lpddr3_h9ccnnn8gtmlar_nud = {
	.dram_type = SPD_DRAM_TYPE_LPDDR3,
	.module_type.ddr3_type = DDR3_MODULE_TYPE_SO_DIMM,

	.module_size_mbits = 8192,
	.num_ranks = 1,
	.device_width = 32,

	.module_mfg_id = MFGID_HYNIX,
	.dram_mfg_id = MFGID_HYNIX,

	.part_num = "H9CCNNN8GTMLAR-NUD",
};

const struct nonspd_mem_info hynix_lpddr3_h9ccnnnbjtalar_nud = {
	.dram_type = SPD_DRAM_TYPE_LPDDR3,
	.module_type.ddr3_type = DDR3_MODULE_TYPE_SO_DIMM,

	.module_size_mbits = 16384,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_HYNIX,
	.dram_mfg_id = MFGID_HYNIX,

	.part_num = "H9CCNNNBJTALAR-NUD",
};

const struct nonspd_mem_info hynix_lpddr3_h9ccnnnbltblar_nud = {
	.dram_type = SPD_DRAM_TYPE_LPDDR3,
	.module_type.ddr3_type = DDR3_MODULE_TYPE_SO_DIMM,

	.module_size_mbits = 16384,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_HYNIX,
	.dram_mfg_id = MFGID_HYNIX,

	.part_num = "H9CCNNNBLTBLAR-NUD",
};

const struct nonspd_mem_info hynix_lpddr4x_h9hcnnnbkmmlxr_nee = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 16384,
	.num_ranks = 1,
	.device_width = 32,

	.module_mfg_id = MFGID_HYNIX,
	.dram_mfg_id = MFGID_HYNIX,

	.part_num = "H9HCNNNBKMMLXR-NEE",
};

const struct nonspd_mem_info hynix_lpddr4x_h9hcnnncpmalhr_nee = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 32768,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_HYNIX,
	.dram_mfg_id = MFGID_HYNIX,

	.part_num = "H9HCNNNCPMALHR-NEE",
};

const struct nonspd_mem_info hynix_lpddr4x_h54g46cyrbx267 = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 16384,
	.num_ranks = 1,
	.device_width = 32,

	.module_mfg_id = MFGID_HYNIX,
	.dram_mfg_id = MFGID_HYNIX,

	.part_num = "H54G46CYRBX267",
};

const struct nonspd_mem_info hynix_lpddr4x_h54g56cyrbx247 = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 32768,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_HYNIX,
	.dram_mfg_id = MFGID_HYNIX,

	.part_num = "H54G56CYRBX247",
};

const struct nonspd_mem_info hynix_lpddr4x_h54g56cyrbx247n = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 32768,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_HYNIX,
	.dram_mfg_id = MFGID_HYNIX,

	.part_num = "H54G56CYRBX247N",
};

const struct nonspd_mem_info hynix_lpddr4x_h9hcnnncpmmlxr_nee = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 32768,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_HYNIX,
	.dram_mfg_id = MFGID_HYNIX,

	.part_num = "H9HCNNNCPMMLXR-NEE",
};

const struct nonspd_mem_info hynix_lpddr4x_h9hcnnnfammlxr_nee = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 65536,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_HYNIX,
	.dram_mfg_id = MFGID_HYNIX,

	.part_num = "H9HCNNNFAMMLXR-NEE",
};

const struct nonspd_mem_info hynix_lpddr4x_h54g68cyrbx248 = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 65536,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_HYNIX,
	.dram_mfg_id = MFGID_HYNIX,

	.part_num = "H54G68CYRBX248",
};

const struct nonspd_mem_info hynix_lpddr4x_h54g68cyrbx248n = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 65536,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_HYNIX,
	.dram_mfg_id = MFGID_HYNIX,

	.part_num = "H54G68CYRBX248N",
};

const struct nonspd_mem_info nanya_lpddr3_nt6cl512t32am_h0 = {
	.dram_type = SPD_DRAM_TYPE_LPDDR3,
	.module_type.ddr3_type = DDR3_MODULE_TYPE_SO_DIMM,

	.module_size_mbits = 16384,
	.num_ranks = 2,
	.device_width = 32,
	.module_mfg_id = MFGID_NANYA,
	.dram_mfg_id = MFGID_NANYA,

	.part_num = "NT6CL512T32AM-H0",
};

const struct nonspd_mem_info samsung_lpddr3_k4e6e304ee_egce = {
	.dram_type = SPD_DRAM_TYPE_LPDDR3,
	.module_type.ddr3_type = DDR3_MODULE_TYPE_SO_DIMM,

	.module_size_mbits = 16384,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_SAMSUNG,
	.dram_mfg_id = MFGID_SAMSUNG,

	.part_num = "K4E6E304EE-EGCE",
};

const struct nonspd_mem_info samsung_lpddr3_k4e6e304eb_egcf = {
	.dram_type = SPD_DRAM_TYPE_LPDDR3,
	.module_type.ddr3_type = DDR3_MODULE_TYPE_SO_DIMM,

	.module_size_mbits = 16384,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_SAMSUNG,
	.dram_mfg_id = MFGID_SAMSUNG,

	.part_num = "K4E6E304EB-EGCF",
};

const struct nonspd_mem_info samsung_lpddr3_k4e6e304ec_egcg = {
	.dram_type = SPD_DRAM_TYPE_LPDDR3,
	.module_type.ddr3_type = DDR3_MODULE_TYPE_SO_DIMM,

	.module_size_mbits = 16384,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_SAMSUNG,
	.dram_mfg_id = MFGID_SAMSUNG,

	.part_num = "K4E6E304EC-EGCG",
};

const struct nonspd_mem_info samsung_lpddr3_k4e8e304ee_egce = {
	.dram_type = SPD_DRAM_TYPE_LPDDR3,
	.module_type.ddr3_type = DDR3_MODULE_TYPE_SO_DIMM,

	.module_size_mbits = 8192,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_SAMSUNG,
	.dram_mfg_id = MFGID_SAMSUNG,

	.part_num = "K4E8E304EE-EGCE",
};

const struct nonspd_mem_info samsung_lpddr3_k4e8e324eb_egcf = {
	.dram_type = SPD_DRAM_TYPE_LPDDR3,
	.module_type.ddr3_type = DDR3_MODULE_TYPE_SO_DIMM,

	.module_size_mbits = 8192,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_SAMSUNG,
	.dram_mfg_id = MFGID_SAMSUNG,

	.part_num = "K4E8E324EB-EGCF",
};

const struct nonspd_mem_info samsung_lpddr3_k4e6e304ed_egcg = {
	.dram_type = SPD_DRAM_TYPE_LPDDR3,
	.module_type.ddr3_type = DDR3_MODULE_TYPE_SO_DIMM,

	.module_size_mbits = 16384,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_SAMSUNG,
	.dram_mfg_id = MFGID_SAMSUNG,

	.part_num = "K4E6E304ED-EGCG",
};

const struct nonspd_mem_info micron_lpddr3_mt52l256m32d1pf_107wtb = {
	.dram_type = SPD_DRAM_TYPE_LPDDR3,
	.module_type.ddr3_type = DDR3_MODULE_TYPE_SO_DIMM,

	.module_size_mbits = 8192,
	.num_ranks = 1,
	.device_width = 32,

	.module_mfg_id = MFGID_MICRON,
	.dram_mfg_id = MFGID_MICRON,

	.part_num = "MT52L256M32D1PF-107WT:B",
};

const struct nonspd_mem_info micron_lpddr3_mt52l256m32d1pf_10 = {
	.dram_type = SPD_DRAM_TYPE_LPDDR3,
	.module_type.ddr3_type = DDR3_MODULE_TYPE_SO_DIMM,

	.module_size_mbits = 2048 * 8,
	.num_ranks = 1,
	.device_width = 64,

	.module_mfg_id = MFGID_MICRON,
	.dram_mfg_id = MFGID_MICRON,

	.part_num = "MT52L256M32D1PF-10",
};

const struct nonspd_mem_info micron_lpddr3_mt52l512m32d2pf_107wtb = {
	.dram_type = SPD_DRAM_TYPE_LPDDR3,
	.module_type.ddr3_type = DDR3_MODULE_TYPE_SO_DIMM,

	.module_size_mbits = 16384,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_MICRON,
	.dram_mfg_id = MFGID_MICRON,

	.part_num = "MT52L512M32D2PF-107WT:B",
};

const struct nonspd_mem_info micron_lpddr3_mt52l512m32d2pf_10 = {
	.dram_type = SPD_DRAM_TYPE_LPDDR3,
	.module_type.ddr3_type = DDR3_MODULE_TYPE_SO_DIMM,

	.module_size_mbits = 4096 * 8,
	.num_ranks = 2,
	.device_width = 64,

	.module_mfg_id = MFGID_MICRON,
	.dram_mfg_id = MFGID_MICRON,

	.part_num = "MT52L512M32D2PF-10",
};

const struct nonspd_mem_info micron_lpddr4x_mt53e1g32d4nq_046wte = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 32768,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_MICRON,
	.dram_mfg_id = MFGID_MICRON,

	.part_num = "MT53E1G32D4NQ-46WT:E",
};

const struct nonspd_mem_info micron_lpddr4x_mt53e1g32d2np_046wta = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 32768,
	.num_ranks = 1,
	.device_width = 32,

	.module_mfg_id = MFGID_MICRON,
	.dram_mfg_id = MFGID_MICRON,

	.part_num = "MT53E1G32D2NP-46WT:A",
};

const struct nonspd_mem_info micron_lpddr4x_mt53e1g32d2np_046wtb = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 32768,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_MICRON,
	.dram_mfg_id = MFGID_MICRON,

	.part_num = "MT53E1G32D2NP-46WT:B",
};

const struct nonspd_mem_info foresee_lpddr4x_feprf6432_58a1930 = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 32768,
	.num_ranks = 1,
	.device_width = 32,

	.module_mfg_id = MFGID_LONGSYS,
	.dram_mfg_id = MFGID_MICRON,

	.part_num = "FEPRF6432-58A1930",
};

const struct nonspd_mem_info foresee_lpddr4x_flxc2004g_n1 = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 32768,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_LONGSYS,
	.dram_mfg_id = MFGID_MICRON,

	.part_num = "FLXC2004G-N1",
};

const struct nonspd_mem_info micron_lpddr4x_mt53e2g32d4nq_046wta = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 65536,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_MICRON,
	.dram_mfg_id = MFGID_MICRON,

	.part_num = "MT53E2G32D4NQ-46WT:A",
};

const struct nonspd_mem_info micron_lpddr4x_mt53e2g32d4nq_046wtc = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 65536,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_MICRON,
	.dram_mfg_id = MFGID_MICRON,

	.part_num = "MT53E2G32D4NQ-46WT:C",
};

const struct nonspd_mem_info micron_lpddr4x_mt53e512m32d1np_046wtb = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 16384,
	.num_ranks = 1,
	.device_width = 32,

	.module_mfg_id = MFGID_MICRON,
	.dram_mfg_id = MFGID_MICRON,

	.part_num = "MT53E512M32D1NP-046WT:B",
};

const struct nonspd_mem_info micron_lpddr4x_mt53e512m32d2np_046 = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 16384,
	.num_ranks = 1,
	.device_width = 32,

	.module_mfg_id = MFGID_MICRON,
	.dram_mfg_id = MFGID_MICRON,

	.part_num = "MT53E512M32D2NP-046",
};

const struct nonspd_mem_info micron_lpddr4x_mt53d1g32d4dt_046wtd = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 32768,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_MICRON,
	.dram_mfg_id = MFGID_MICRON,

	.part_num = "MT53D1G32D4DT-46WT:D",
};

const struct nonspd_mem_info micron_lpddr4x_mt29vzzzad8dqksl = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 32768,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_MICRON,
	.dram_mfg_id = MFGID_MICRON,

	.part_num = "MT29VZZZAD8DQKSL",
};

const struct nonspd_mem_info micron_lpddr4x_mt29vzzzad8gqfsl_046 = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 32768,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_MICRON,
	.dram_mfg_id = MFGID_MICRON,

	.part_num = "MT29VZZZAD8GQFSL-046",
};

const struct nonspd_mem_info micron_lpddr4x_mt29vzzzbd9dqkpr_046 = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 32768,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_MICRON,
	.dram_mfg_id = MFGID_MICRON,

	.part_num = "MT29VZZZBD9DQKPR-046",
};

const struct nonspd_mem_info micron_lpddr4x_mt29vzzzad9gqfsm_046 = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 32768,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_MICRON,
	.dram_mfg_id = MFGID_MICRON,

	.part_num = "MT29VZZZAD9GQFSM-046",
};

const struct nonspd_mem_info micron_lpddr4x_mt29vzzzcd9gqkpr_046 = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 32768,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_MICRON,
	.dram_mfg_id = MFGID_MICRON,

	.part_num = "MT29VZZZCD9GQKPR-046",
};

const struct nonspd_mem_info samsung_lpddr4x_kmdh6001da_b422 = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 32768,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_SAMSUNG,
	.dram_mfg_id = MFGID_SAMSUNG,

	.part_num = "KMDH6001DA-B422",
};

const struct nonspd_mem_info samsung_lpddr4x_kmdp6001da_b425 = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 32768,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_SAMSUNG,
	.dram_mfg_id = MFGID_SAMSUNG,

	.part_num = "KMDP6001DA-B425",
};

const struct nonspd_mem_info samsung_lpddr4x_kmdv6001da_b620 = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 32768,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_SAMSUNG,
	.dram_mfg_id = MFGID_SAMSUNG,

	.part_num = "KMDV6001DA-B620",
};

const struct nonspd_mem_info samsung_lpddr4x_k4u6e3s4aa_mgcr = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 16384,
	.num_ranks = 1,
	.device_width = 32,

	.module_mfg_id = MFGID_SAMSUNG,
	.dram_mfg_id = MFGID_SAMSUNG,

	.part_num = "K4U6E3S4AA-MGCR",
};

const struct nonspd_mem_info samsung_lpddr4x_k4u6e3s4ab_mgcl = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 16384,
	.num_ranks = 1,
	.device_width = 32,

	.module_mfg_id = MFGID_SAMSUNG,
	.dram_mfg_id = MFGID_SAMSUNG,

	.part_num = "K4U6E3S4AB-MGCL",
};

const struct nonspd_mem_info samsung_lpddr4x_k4ube3d4aa_mgcl = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 32768,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_SAMSUNG,
	.dram_mfg_id = MFGID_SAMSUNG,

	.part_num = "K4UBE3D4AA-MGCL",
};

const struct nonspd_mem_info samsung_lpddr4x_k4ube3d4ab_mgcl = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 32768,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_SAMSUNG,
	.dram_mfg_id = MFGID_SAMSUNG,

	.part_num = "K4UBE3D4AB-MGCL",
};

const struct nonspd_mem_info samsung_lpddr4x_k4ube3d4aa_mgcr = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 32768,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_SAMSUNG,
	.dram_mfg_id = MFGID_SAMSUNG,

	.part_num = "K4UBE3D4AA-MGCR",
};

const struct nonspd_mem_info samsung_lpddr4x_k4uce3d4aa_mgcl = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 65536,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_SAMSUNG,
	.dram_mfg_id = MFGID_SAMSUNG,

	.part_num = "K4UCE3D4AA-MGCL",
};

const struct nonspd_mem_info samsung_lpddr4x_k4uce3q4aa_mgcr = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 65536,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_SAMSUNG,
	.dram_mfg_id = MFGID_SAMSUNG,

	.part_num = "K4UCE3Q4AA-MGCR",
};

const struct nonspd_mem_info samsung_lpddr4x_k4uce3q4ab_mgcl = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 65536,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_SAMSUNG,
	.dram_mfg_id = MFGID_SAMSUNG,

	.part_num = "K4UCE3Q4AB-MGCL",
};

const struct nonspd_mem_info sandisk_lpddr4x_sdada4cr_128g = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 32768,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_SANDISK,
	.dram_mfg_id = MFGID_SANDISK,

	.part_num = "SDADA4CR-128G",
};

const struct nonspd_mem_info cxmt_lpddr4x_cxdb5ccbm_ml_a = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 32768,
	.num_ranks = 2,
	.device_width = 32,

	.module_mfg_id = MFGID_CXMT,
	.dram_mfg_id = MFGID_CXMT,

	.part_num = "CXDB5CCBM-ML-A",
};

const struct nonspd_mem_info nanya_lpddr4x_nt6ap1024f32bl_j1 = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 32768,
	.num_ranks = 2,
	.device_width = 32,
	.module_mfg_id = MFGID_NANYA,
	.dram_mfg_id = MFGID_NANYA,

	.part_num = "NT6AP1024F32BL-J1",
};

const struct nonspd_mem_info rayson_lpddr4x_rs1g32lr4d2bnr_46bt = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 32768,
	.num_ranks = 1,
	.device_width = 16,
	.module_mfg_id = MFGID_CXMT,
	.dram_mfg_id = MFGID_CXMT,

	.part_num = "RS1G32LR4D2BNR-46BT",
};

const struct nonspd_mem_info rayson_lpddr4x_rs2g32lr4d4bnr_46bt = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 65536,
	.num_ranks = 2,
	.device_width = 16,
	.module_mfg_id = MFGID_CXMT,
	.dram_mfg_id = MFGID_CXMT,

	.part_num = "RS2G32LR4D4BNR-46BT",
};

const struct nonspd_mem_info rayson_lpddr4x_rs2g32lv4d4bdt_46bt = {
	.dram_type = SPD_DRAM_TYPE_LPDDR4X,

	.module_size_mbits = 65536,
	.num_ranks = 2,
	.device_width = 32,
	.module_mfg_id = MFGID_MICRON,
	.dram_mfg_id = MFGID_MICRON,

	.part_num = "RS2G32LV4D4BDT-46BT",
};
