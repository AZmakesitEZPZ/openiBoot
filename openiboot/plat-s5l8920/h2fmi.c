#include "h2fmi.h"
#include "hardware/h2fmi.h"
#include "timer.h"
#include "tasks.h"
#include "clock.h"
#include "util.h"

typedef struct _nand_chipid
{
	uint32_t chipID;
	uint32_t unk1;
} __attribute__((__packed__)) nand_chipid_t;

typedef struct _nand_smth_struct
{
	uint32_t unk1;
	uint32_t unk2;
	uint32_t symmetric_masks[];
} __attribute__((__packed__)) nand_smth_struct_t;

typedef struct _nand_chip_info
{
	nand_chipid_t chipID;
	uint16_t unk1;
	uint16_t unk2;
	uint16_t unk3;
	uint16_t unk4;
	uint16_t unk5;
	uint16_t unk6;
	uint32_t unk7;
	uint16_t unk8;
	uint16_t unk9;
} __attribute__((__packed__)) nand_chip_info_t;

typedef struct _nand_board_id
{
	uint32_t num_busses;
	uint32_t num_symmetric;
	nand_chipid_t chipID;
	uint8_t unk3;
	nand_chipid_t chipID2;
	uint8_t unk4;
} __attribute__((__packed__)) nand_board_id_t;

typedef struct _nand_board_info
{
	nand_board_id_t board_id;
	uint16_t unk1;
	uint16_t unk2;
} __attribute__((__packed__)) nand_board_info_t;

typedef struct _nand_timing_info
{
	nand_board_id_t board_id;
	uint8_t unk1;
	uint8_t unk2;
	uint8_t unk3;
	uint8_t unk4;
	uint8_t unk5;
	uint8_t unk6;
	uint8_t unk7;
	uint8_t unk8;
} __attribute__((__packed__)) nand_timing_info_t;

typedef struct _nand_info
{
	nand_chip_info_t *chip_info;
	nand_board_info_t *board_info;
	nand_timing_info_t *timing_info;
} nand_info_t;

static nand_chip_info_t nand_chip_info[] = {
	{ { 0x7294D7EC, 0, }, 0x1038, 0x80, 0x2000, 0x1B4, 0xC, 0, 8, 1, 0 },
	{ { 0x72D5DEEC, 0, }, 0x2070, 0x80, 0x2000, 0x1B4, 0xC, 0, 8, 2, 0 },
	{ { 0x29D5D7EC, 0, }, 0x2000, 0x80, 0x1000, 0xDA, 8, 0, 2, 2, 0 },
	{ { 0x2994D5EC, 0, }, 0x1000, 0x80, 0x1000, 0xDA, 8, 0, 2, 1, 0 },
	{ { 0xB614D5EC, 0, }, 0x1000, 0x80, 0x1000, 0x80, 4, 0, 2, 1, 0 },
	{ { 0xB655D7EC, 0, }, 0x2000, 0x80, 0x1000, 0x80, 4, 0, 2, 2, 0 },
	{ { 0xB614D5AD, 0, }, 0x1000, 0x80, 0x1000, 0x80, 4, 0, 3, 1, 0 },
	{ { 0x3294E798, 0, }, 0x1004, 0x80, 0x2000, 0x1C0, 0x10, 0, 1, 1, 0 },
	{ { 0xBA94D598, 0, }, 0x1000, 0x80, 0x1000, 0xDA, 8, 0, 1, 1, 0 },
	{ { 0xBA95D798, 0, }, 0x2000, 0x80, 0x1000, 0xDA, 8, 0, 1, 2, 0 },
	{ { 0x3294D798, 0, }, 0x1034, 0x80, 0x2000, 0x178, 8, 0, 1, 1, 0 },
	{ { 0x3295DE98, 0, }, 0x2068, 0x80, 0x2000, 0x178, 8, 0, 1, 2, 0 },
	{ { 0x3295EE98, 0, }, 0x2008, 0x80, 0x2000, 0x1C0, 0x18, 0, 1, 2, 0 },
	{ { 0x3E94D789, 0, }, 0x2000, 0x80, 0x1000, 0xDA, 0x10, 0, 5, 1, 0 },
	{ { 0x3ED5D789, 0, }, 0x2000, 0x80, 0x1000, 0xDA, 8, 0, 6, 2, 0 },
	{ { 0x3ED5D72C, 0, }, 0x2000, 0x80, 0x1000, 0xDA, 8, 0, 5, 2, 0 },
	{ { 0x3E94D72C, 0, }, 0x2000, 0x80, 0x1000, 0xDA, 0xC, 0, 7, 1, 0 },
	{ { 0x4604682C, 0, }, 0x1000, 0x100, 0x1000, 0xE0, 0xC, 0, 7, 1, 0 },
	{ { 0x3294D745, 0, }, 0x1000, 0x80, 0x2000, 0x178, 8, 0, 9, 1, 0 },
	{ { 0x3295DE45, 0, }, 0x2000, 0x80, 0x2000, 0x178, 8, 0, 9, 2, 0 },
	{ { 0x32944845, 0, }, 0x1000, 0x80, 0x2000, 0x1C0, 8, 0, 9, 1, 0 },
	{ { 0x32956845, 0, }, 0x2000, 0x80, 0x2000, 0x1C0, 8, 0, 9, 2, 0 },
};

static nand_board_info_t nand_board_info[] = {
	{ { 2, 1, { 0x7294D7EC, 0x0 }, 2, { 0x0, 0x0, }, 0, }, 1, 1, },
	{ { 2, 1, { 0x7294D7EC, 0x0 }, 4, { 0x0, 0x0, }, 0, }, 1, 1, },
	{ { 2, 2, { 0x7294D7EC, 0x0 }, 2, { 0x7294D7EC, 0x0, }, 2, }, 1, 1, },
	{ { 2, 1, { 0x29D5D7EC, 0x0 }, 4, { 0x0, 0x0, }, 0, }, 1, 1, },
	{ { 1, 1, { 0xB655D7EC, 0x0 }, 4, { 0x0, 0x0, }, 0, }, 1, 1, },
	{ { 2, 1, { 0x2994D5EC, 0x0 }, 4, { 0x0, 0x0, }, 0, }, 1, 1, },
	{ { 2, 1, { 0x72D5DEEC, 0x0 }, 4, { 0x0, 0x0, }, 0, }, 1, 1, },
	{ { 1, 1, { 0xB614D5EC, 0x0 }, 4, { 0x0, 0x0, }, 0, }, 1, 1, },
	{ { 1, 1, { 0xBA94D598, 0x0 }, 4, { 0x0, 0x0, }, 0, }, 1, 1, },
	{ { 2, 1, { 0x3294D798, 0x0 }, 2, { 0x0, 0x0, }, 0, }, 1, 1, },
	{ { 2, 1, { 0x3294D798, 0x0 }, 4, { 0x0, 0x0, }, 0, }, 1, 1, },
	{ { 2, 1, { 0x3295DE98, 0x0 }, 4, { 0x0, 0x0, }, 0, }, 1, 1, },
	{ { 2, 2, { 0x3295DE98, 0x0 }, 6, { 0x3295DE98, 0x0, }, 6, }, 1, 1, },
	{ { 2, 1, { 0x3294E798, 0x0 }, 4, { 0x0, 0x0, }, 0, }, 1, 1, },
	{ { 2, 1, { 0x3294E798, 0x0 }, 2, { 0x0, 0x0, }, 0, }, 1, 1, },
	{ { 2, 1, { 0x3295EE98, 0x0 }, 4, { 0x0, 0x0, }, 0, }, 1, 1, },
	{ { 1, 1, { 0xB614D5AD, 0x0 }, 4, { 0x0, 0x0, }, 0, }, 1, 1, },
	{ { 2, 1, { 0xB614D5AD, 0x0 }, 4, { 0x0, 0x0, }, 0, }, 1, 1, },
	{ { 2, 2, { 0xB614D5AD, 0x0 }, 4, { 0xB614D5AD, 0x0, }, 4, }, 1, 1, },
	{ { 2, 1, { 0x3E94D789, 0x0 }, 2, { 0x0, 0x0, }, 0, }, 1, 1, },
	{ { 1, 1, { 0x3ED5D789, 0x0 }, 2, { 0x0, 0x0, }, 0, }, 1, 1, },
	{ { 2, 1, { 0x3E94D72C, 0x0 }, 2, { 0x0, 0x0, }, 0, }, 1, 1, },
	{ { 2, 1, { 0x3E94D72C, 0x0 }, 4, { 0x0, 0x0, }, 0, }, 1, 1, },
	{ { 1, 1, { 0x3ED5D72C, 0x0 }, 2, { 0x0, 0x0, }, 0, }, 1, 1, },
	{ { 2, 1, { 0x3294D745, 0x0 }, 4, { 0x0, 0x0, }, 0, }, 1, 1, },
	{ { 2, 1, { 0x3295DE45, 0x0 }, 4, { 0x0, 0x0, }, 0, }, 1, 1, },
	{ { 2, 2, { 0xBA95D798, 0x0 }, 4, { 0xBA95D798, 0x0, }, 4, }, 1, 1, },
	{ { 2, 1, { 0x4604682C, 0x0 }, 2, { 0x0, 0x0, }, 0, }, 1, 1, },
	{ { 2, 1, { 0x4604682C, 0x0 }, 4, { 0x0, 0x0, }, 0, }, 1, 1, },
	{ { 2, 2, { 0x4604682C, 0x0 }, 4, { 0x4604682C, 0x0, }, 4, }, 1, 1, },
	{ { 2, 1, { 0x3294D745, 0x0 }, 4, { 0x0, 0x0, }, 0, }, 1, 1, },
	{ { 2, 1, { 0x32944845, 0x0 }, 4, { 0x0, 0x0, }, 0, }, 17, 21, },
	{ { 2, 1, { 0x32956845, 0x0 }, 4, { 0x0, 0x0, }, 0, }, 17, 21, },
};

static nand_timing_info_t nand_timing_info[] = {
	{ { 2, 1, { 0x7294d7ec, 0x0, }, 2, { 0x0, 0x0, }, 0, }, 0x1e, 0xf, 0xa, 0x1e, 0xf, 0xa, 0x19, 0xf, },
	{ { 2, 1, { 0x7294d7ec, 0x0, }, 4, { 0x0, 0x0, }, 0, }, 0x1e, 0xf, 0xa, 0x1e, 0xf, 0xa, 0x19, 0xf, },
	{ { 2, 2, { 0x7294d7ec, 0x0, }, 2, { 0x7294d7ec, 0x0, }, 2, }, 0x1e, 0xf, 0xa, 0x1e, 0xf, 0xa, 0x19, 0xf, },
	{ { 2, 1, { 0x72d5deec, 0x0, }, 4, { 0x0, 0x0, }, 0, }, 0x1e, 0xf, 0xa, 0x1e, 0xf, 0xa, 0x14, 0xf, },
	{ { 2, 1, { 0x29d5d7ec, 0x0, }, 4, { 0x0, 0x0, }, 0, }, 0x1e, 0xf, 0xa, 0x1e, 0xf, 0xa, 0x14, 0xf, },
	{ { 2, 1, { 0x2994d5ec, 0x0, }, 4, { 0x0, 0x0, }, 0, }, 0x1e, 0xf, 0xa, 0x1e, 0xf, 0xa, 0x14, 0xf, },
	{ { 1, 1, { 0xb614d5ec, 0x0, }, 4, { 0x0, 0x0, }, 0, }, 0x19, 0xc, 0x5, 0x1e, 0x14, 0xa, 0x14, 0xf, },
	{ { 1, 1, { 0xb655d7ec, 0x0, }, 4, { 0x0, 0x0, }, 0, }, 0x2d, 0x19, 0xf, 0x32, 0x19, 0xf, 0x1e, 0xf, },
	{ { 1, 1, { 0xb614d5ad, 0x0, }, 4, { 0x0, 0x0, }, 0, }, 0x19, 0xc, 0xa, 0x19, 0xc, 0xa, 0x14, 0xf, },
	{ { 2, 1, { 0xb614d5ad, 0x0, }, 4, { 0x0, 0x0, }, 0, }, 0x19, 0xc, 0xa, 0x19, 0xc, 0xa, 0x14, 0xf, },
	{ { 2, 2, { 0xb614d5ad, 0x0, }, 4, { 0xb614d5ad, 0x0, }, 4, }, 0x19, 0xc, 0xa, 0x19, 0xc, 0xa, 0x14, 0xf, },
	{ { 2, 1, { 0x3294d798, 0x0, }, 2, { 0x0, 0x0, }, 0, }, 0x19, 0xc, 0xa, 0x19, 0xc, 0xa, 0x14, 0x19, },
	{ { 2, 1, { 0x3294d798, 0x0, }, 4, { 0x0, 0x0, }, 0, }, 0x19, 0xc, 0xa, 0x19, 0xc, 0xa, 0x14, 0x19, },
	{ { 1, 1, { 0xba94d598, 0x0, }, 4, { 0x0, 0x0, }, 0, }, 0x1e, 0xf, 0xa, 0x1e, 0xf, 0xf, 0x19, 0x1e, },
	{ { 2, 2, { 0xba95d798, 0x0, }, 4, { 0xba95d798, 0x0, }, 4, }, 0x1e, 0xf, 0xa, 0x1e, 0xf, 0xf, 0x19, 0x1e, },
	{ { 2, 1, { 0x3295de98, 0x0, }, 4, { 0x0, 0x0, }, 0, }, 0x19, 0xc, 0xa, 0x19, 0xc, 0xa, 0x14, 0x19, },
	{ { 2, 2, { 0x3295de98, 0x0, }, 6, { 0x3295de98, 0x0, }, 6, }, 0x19, 0xc, 0xa, 0x19, 0xc, 0xa, 0x14, 0x19, },
	{ { 2, 1, { 0x3294e798, 0x0, }, 4, { 0x0, 0x0, }, 0, }, 0x19, 0xc, 0xa, 0x19, 0xc, 0xa, 0x14, 0x19, },
	{ { 2, 1, { 0x3295ee98, 0x0, }, 4, { 0x0, 0x0, }, 0, }, 0x19, 0xc, 0xa, 0x19, 0xc, 0xa, 0x14, 0x19, },
	{ { 1, 1, { 0x3ed5d789, 0x0, }, 2, { 0x0, 0x0, }, 0, }, 0x19, 0xa, 0xf, 0x19, 0xa, 0xf, 0x14, 0xf, },
	{ { 2, 1, { 0x3e94d789, 0x0, }, 2, { 0x0, 0x0, }, 0, }, 0x14, 0xa, 0x7, 0x14, 0xa, 0x7, 0x10, 0xf, },
	{ { 1, 1, { 0x3ed5d72c, 0x0, }, 2, { 0x0, 0x0, }, 0, }, 0x19, 0xa, 0xf, 0x19, 0xa, 0xf, 0x14, 0xf, },
	{ { 2, 1, { 0x3e94d72c, 0x0, }, 4, { 0x0, 0x0, }, 0, }, 0x14, 0xa, 0x7, 0x14, 0xa, 0x7, 0x10, 0xf, },
	{ { 2, 1, { 0x3e94d72c, 0x0, }, 2, { 0x0, 0x0, }, 0, }, 0x14, 0xa, 0x7, 0x14, 0xa, 0x7, 0x10, 0xf, },
	{ { 2, 1, { 0x4604682c, 0x0, }, 2, { 0x0, 0x0, }, 0, }, 0x19, 0xc, 0xa, 0x19, 0xc, 0xa, 0x14, 0xf, },
	{ { 2, 1, { 0x4604682c, 0x0, }, 4, { 0x0, 0x0, }, 0, }, 0x19, 0xc, 0xa, 0x19, 0xc, 0xa, 0x14, 0xf, },
	{ { 2, 2, { 0x4604682c, 0x0, }, 4, { 0x4604682c, 0x0, }, 4, }, 0x19, 0xc, 0xa, 0x19, 0xc, 0xa, 0x14, 0xf, },
	{ { 2, 1, { 0x3294e798, 0x0, }, 2, { 0x0, 0x0, }, 0, }, 0x19, 0xc, 0xa, 0x19, 0xc, 0xa, 0x14, 0x19, },
	{ { 2, 1, { 0x3294d745, 0x0, }, 4, { 0x0, 0x0, }, 0, }, 0x19, 0xc, 0xa, 0x19, 0xc, 0xa, 0x14, 0x1e, },
	{ { 2, 1, { 0x3295de45, 0x0, }, 4, { 0x0, 0x0, }, 0, }, 0x19, 0xc, 0xa, 0x19, 0xc, 0xa, 0x14, 0x1e, },
	{ { 2, 1, { 0x32944845, 0x0, }, 4, { 0x0, 0x0, }, 0, }, 0x19, 0xc, 0xa, 0x19, 0xc, 0xa, 0x14, 0x19, },
	{ { 2, 1, { 0x32956845, 0x0, }, 4, { 0x0, 0x0, }, 0, }, 0x19, 0xc, 0xa, 0x19, 0xc, 0xa, 0x14, 0x19, },
};

static h2fmi_struct_t fmi0 = {
	.bus_num = 0,
	.base_address = H2FMI0_BASE,
	.clock_gate = H2FMI0_CLOCK_GATE,
};

static h2fmi_struct_t fmi1 = {
	.bus_num = 1,
	.base_address = H2FMI1_BASE,
	.clock_gate = H2FMI1_CLOCK_GATE,
};

static int compare_board_ids(nand_board_id_t *a, nand_board_id_t *b)
{
	return memcmp(a, b, sizeof(nand_board_id_t)) == 0;
}

static int count_bits(uint32_t _val)
{
	int ret = 0;

	while(_val > 0)
	{
		if(_val & 1)
			ret++;
		
		_val >>= 1;
	}

	return ret;
}

static int h2fmi_wait_for_done(h2fmi_struct_t *_fmi, uint32_t _reg, uint32_t _mask, uint32_t _val)
{
	uint64_t startTime = timer_get_system_microtime();
	while((GET_REG(_reg) & _mask) != _val)
	{
		task_yield();

		if(has_elapsed(startTime, 10000))
		{
			bufferPrintf("h2mi: timout on 0x%08x failed.\n", _reg);
			return -1;
		}
	}

	return 0;
}

static int h2fmi_read(h2fmi_struct_t *_fmi, void *_dest, int _amt)
{
	char *dest = _dest;
	uint32_t oldVal = GET_REG(H2FMI_UNKREG1(_fmi));
	SET_REG(H2FMI_UNKREG1(_fmi), 0);
	SET_REG(H2FMI_UNKREG8(_fmi), 0);
	
	int i;
	for(i = 0; i < _amt; i++)
	{
		SET_REG(H2FMI_UNKREG5(_fmi), 0x50);
		udelay(1);

		dest[i] = GET_REG(H2FMI_DATA(_fmi));

		SET_REG(H2FMI_UNKREG5(_fmi), 0);
	}

	SET_REG(H2FMI_UNKREG1(_fmi), oldVal);

	return 0;
}

static void h2fmi_device_reset(h2fmi_struct_t *_fmi)
{
	clock_gate_reset(_fmi->clock_gate);

	SET_REG(H2FMI_UNKREG2(_fmi), 1);
	SET_REG(H2FMI_UNKREG1(_fmi), _fmi->field_38);
}

static void h2fmi_device_reset_814(h2fmi_struct_t *_fmi)
{
	SET_REG(H2FMI_UNKREG3(_fmi), 0);
}

static void h2fmi_enable_chip(h2fmi_struct_t *_fmi, uint8_t _chip)
{
	h2fmi_struct_t *chipFMI = (_chip & 0x8) ? &fmi1: &fmi0;
	if(_fmi->bus_num == 0 && (_fmi->field_C & 0xFF00))
	{
		h2fmi_struct_t *fmi = (_fmi == chipFMI) ? &fmi1: _fmi;
		SET_REG(H2FMI_CHIP_MASK(fmi), 0);		
	}

	uint32_t reg = H2FMI_CHIP_MASK(chipFMI);
	SET_REG(reg, GET_REG(reg) | (1 << (_chip & 0x7)));
}

static void h2fmi_disable_chip(uint8_t _chip)
{
	h2fmi_struct_t *fmi = (_chip & 0x8) ? &fmi1: &fmi0;
	uint32_t maskReg = H2FMI_CHIP_MASK(fmi);
	SET_REG(maskReg, GET_REG(maskReg) &~ (1 << (_chip & 0x7)));
}

static int h2fmi_init_bus(h2fmi_struct_t *_fmi)
{
	clock_gate_switch(_fmi->clock_gate, ON);

	_fmi->bitmap = 0;
	_fmi->num_chips = 0;
	
	_fmi->field_38 = 0xFFFF;
	SET_REG(H2FMI_UNKREG1(_fmi), _fmi->field_38);

	_fmi->field_180 = 0xFFFFFFFF;

	h2fmi_device_reset(_fmi);
	h2fmi_device_reset_814(_fmi);

	return 0;
}

static int h2fmi_nand_reset(h2fmi_struct_t *_fmi, uint8_t _chip)
{
	h2fmi_enable_chip(_fmi, _chip);
	
	SET_REG(H2FMI_UNKREG4(_fmi), 0xFF);
	SET_REG(H2FMI_UNKREG5(_fmi), 1);

	int ret = h2fmi_wait_for_done(_fmi, H2FMI_UNKREG6(_fmi), 1, 1);
	h2fmi_disable_chip(_chip);

	return ret;
}

static int h2fmi_nand_reset_all(h2fmi_struct_t *_fmi)
{
	uint8_t i;
	for(i = 0; i < H2FMI_CHIP_COUNT; i++)
	{
		int ret = h2fmi_nand_reset(_fmi, i);
		if(ret != 0)
			return ret;
	}

	return 0;
}

static int h2fmi_read_chipid(h2fmi_struct_t *_fmi, uint8_t _chip, void *_buffer, uint8_t _unk)
{
	h2fmi_enable_chip(_fmi, _chip);

	SET_REG(H2FMI_UNKREG4(_fmi), 0x90);
	SET_REG(H2FMI_UNKREG9(_fmi), _unk);
	SET_REG(H2FMI_UNKREG10(_fmi), 0);
	SET_REG(H2FMI_UNKREG5(_fmi), 9);

	int ret = h2fmi_wait_for_done(_fmi, H2FMI_UNKREG6(_fmi), 9, 9);
	if(ret == 0)
		ret = h2fmi_read(_fmi, _buffer, H2FMI_CHIPID_LENGTH);

	h2fmi_disable_chip(_chip);
	return ret;
}

static int h2fmi_check_chipid(h2fmi_struct_t *_fmi, char *_id, char *_base, int _unk)
{
	char *ptr = _id;

	_fmi->num_chips = 0;
	_fmi->bitmap = 0;

	uint8_t i;
	for(i = 0; i < H2FMI_CHIP_COUNT; i++)
	{
		if(!memcmp(ptr, _base, 6))
		{
			bufferPrintf("fmi: Found chip ID %2x %2x %2x %2x %2x %2x on fmi%d:ce%d.\n",
					ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5],
					_fmi->bus_num, i);

			_fmi->bitmap |= (1 << i);
			_fmi->num_chips++;
		}
		else if(memcmp(ptr, "\xff\xff\xff\xff\xff\xff", 6) && memcmp(ptr, "\0\0\0\0\0\0", 6))
			bufferPrintf("fmi: Ignoring mismatched chip with ID %2x %2x %2x %2x %2x %2x on fmi%d:ce%d.\n",
					ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5],
					_fmi->bus_num, i);

		ptr += H2FMI_CHIPID_LENGTH;
	}

	return 0;
}

static int h2fmi_reset_and_read_chipids(h2fmi_struct_t *_fmi, void *_buffer, uint8_t _unk)
{
	char *buffer = _buffer;

	_fmi->field_8 = 0;
	_fmi->field_C = 0;

	int ret = h2fmi_nand_reset_all(_fmi);
	if(ret != 0)
		return ret;
	
	uint8_t i;
	for(i = 0; i < H2FMI_CHIP_COUNT; i++)
	{
		ret = h2fmi_read_chipid(_fmi, i, buffer, _unk);
		if(ret != 0)
			return ret;

		buffer += H2FMI_CHIPID_LENGTH;
	}
	
	return ret;
}

static nand_info_t *h2fmi_nand_find_info(char *_id, h2fmi_struct_t **_busses, int _count)
{
	static nand_smth_struct_t nand_smth = { 0xA010101, 0x30306, { 0xF0F, 0, 0 } };

	uint8_t chipID[H2FMI_CHIPID_LENGTH];
	uint32_t bitmap = 0;
	int found = 0, i;
	nand_board_id_t board_id;
	memset(&board_id, 0, sizeof(board_id));

	board_id.num_busses = _count;

	for(i = 0; i < H2FMI_CHIP_COUNT; i++)
	{
		if(_busses[i] != NULL)
		{
			if(found > 0)
			{
				if(memcmp(chipID, &_id[i*H2FMI_CHIPID_LENGTH], H2FMI_CHIPID_LENGTH) != 0)
				{
					uint8_t *a = (uint8_t*)&board_id.chipID;
					uint8_t *b = (uint8_t*)&_id[i*H2FMI_CHIPID_LENGTH];

					bufferPrintf("fmi: ChipIDs do not match.\n");
					bufferPrintf("fmi: %02x %02x %02x %02x %02x %02x\n", a[0], a[1], a[2], a[3], a[4], a[5]);
					bufferPrintf("fmi: %02x %02x %02x %02x %02x %02x\n", b[0], b[1], b[2], b[3], b[4], b[5]);
					return NULL;
				}
			}
			else
				memcpy(chipID, &_id[i*H2FMI_CHIPID_LENGTH], H2FMI_CHIPID_LENGTH);

			found++;
			bitmap |= (1 << i);
		}
	}

	i = 0;
	uint32_t mask = nand_smth.symmetric_masks[i];
	uint32_t bit_count = count_bits(bitmap & mask);
	while(mask)
	{
		uint32_t bits = bitmap & mask;
		if(bits)
		{
			if(count_bits(bits) != bit_count)
			{
				bufferPrintf("fmi: Chip IDs not symmetric.\n");
				return NULL;
			}

			board_id.num_symmetric++;
		}

		i++;
		mask = nand_smth.symmetric_masks[i];
	}

	nand_info_t *info = malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	for(i = 0; i < array_size(nand_chip_info); i++)
	{
		nand_chip_info_t *ci = &nand_chip_info[i];

		if(memcmp(chipID, &ci->chipID, sizeof(uint32_t)) == 0)
			info->chip_info = ci;
	}

	if(!info->chip_info)
	{
		bufferPrintf("fmi: Unsupported chip.\n");
		return NULL;
	}

	nand_chipid_t *chipIDPtr = &board_id.chipID;
	uint8_t *unkPtr = &board_id.unk3;
	for(i = 0; i < board_id.num_symmetric; i++)
	{
		memcpy(chipIDPtr, chipID, sizeof(uint32_t));

		*unkPtr = found / board_id.num_symmetric;

		chipIDPtr = (nand_chipid_t*)(((uint8_t*)chipIDPtr) + sizeof(*chipIDPtr) + sizeof(*unkPtr));
		unkPtr += sizeof(*chipIDPtr) + sizeof(*unkPtr);
	}

	bufferPrintf("fmi: NAND board ID: (%d, %d, 0x%x, 0x%x, %d, 0x%x, 0x%x, %d).\n",
			board_id.num_busses, board_id.num_symmetric, board_id.chipID.chipID, board_id.chipID.unk1,
			board_id.unk3, board_id.chipID2.chipID, board_id.chipID2.unk1, board_id.unk4);

	for(i = 0; i < array_size(nand_board_info); i++)
	{
		nand_board_info_t *bi = &nand_board_info[i];
		if(compare_board_ids(&board_id, &bi->board_id))
			info->board_info = bi;
	}

	if(!info->board_info)
	{
		bufferPrintf("fmi: No support for board.\n");
		return NULL;
	}

	for(i = 0; i < array_size(nand_timing_info); i++)
	{
		nand_timing_info_t *ti = &nand_timing_info[i];
		if(compare_board_ids(&board_id, &ti->board_id))
			info->timing_info = ti;
	}

	if(!info->timing_info)
	{
		bufferPrintf("fmi: Failed to find timing info for board.\n");
		return NULL;
	}

	return info;
}

void h2fmi_init()
{
	h2fmi_init_bus(&fmi0);
	h2fmi_init_bus(&fmi1);

	char *buff1 = malloc(H2FMI_CHIPID_LENGTH*H2FMI_CHIP_COUNT*2);
	char *buff2 = buff1 + (H2FMI_CHIPID_LENGTH*H2FMI_CHIP_COUNT);
	h2fmi_reset_and_read_chipids(&fmi0, buff1, 0);
	h2fmi_reset_and_read_chipids(&fmi1, buff2, 0);

	h2fmi_check_chipid(&fmi0, buff1, buff1, 0);
	h2fmi_check_chipid(&fmi1, buff2, buff1, 0);

	h2fmi_struct_t *busses[H2FMI_CHIP_COUNT];
	memset(busses, 0, sizeof(busses));

	uint32_t i;
	for(i = 0; i < H2FMI_CHIP_COUNT; i++)
	{
		if(fmi1.bitmap & (1 << i))
		{
			memcpy(buff1 + (i*H2FMI_CHIPID_LENGTH), 
					buff2 + (i*H2FMI_CHIPID_LENGTH),
					H2FMI_CHIPID_LENGTH);
			busses[i] = &fmi1;
		}
		else if(fmi0.bitmap & (1 << i))
			busses[i] = &fmi0;
	}

	nand_info_t *info = h2fmi_nand_find_info(buff1, busses, (fmi0.num_chips > 0) + (fmi1.num_chips > 0));

	if(info)
		free(info);

	free(buff1);
}
MODULE_INIT(h2fmi_init);
