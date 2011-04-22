// This won't compile. But you get the feeling for what it does. :]

uint32_t yaFTL_inited = 0;

typedef struct
{
  uint32_t buffer;
  uint32_t endOfBuffer;
  uint32_t size;
  uint32_t numAllocs;
  uint32_t numRebases;
  uint32_t paddingsSize;
  uint32_t state;
} WMR_zone_t;

void WMR_BufZone_Init(WMR_zone_t *_zone)
{
	_zone->buffer = 0;
	_zone->endOfBuffer = 0;
	_zone->size = 0;
	_zone->numAllocs = 0;
	_zone->numRebases = 0;
	_zone->paddingsSize = 0;
	_zone->state = 1;
}

// returns the sub-buffer offset
uint32_t WMR_Buf_Alloc_ForDMA(WMR_zone_t *_zone, uint32_t size)
{
	uint32_t oldSize;
	uint32_t oldSizeRounded;
	uint32_t numAllocs;

	if (_zone->state != 1)
		system_panic("WMR_Buf_Alloc_ForDMA: bad state\r\n");

	oldSize = _zone->size;
	oldSizeRounded = ROUNDUP(oldSize, 64);
	_zone->paddingsSize = _zone->paddingsSize - oldSize + oldSizeRounded;
	numAllocs = _zone->numAllocs;
	_zone->size = size + oldSizeRounded;
	_zone->numAllocs = numAllocs + 1;

	return oldSizeRounded;
}

void WMR_BufZone_FinishedAllocs(WMR_zone_t *_zone)
{
	uint32_t size_rounded_up;
	void* buff;

	if (_zone->state != 1)
		system_panic("WMR_BufZone_FinishedAllocs: bad state\r\n");

	size_rounded_up = (_zone->size + 63) & 0xFFFFFFC0;// Round up to 64
	_zone->size = size_rounded_up;

	buff = wmr_malloc(size_rounded_up);

	if(!buff)
		system_panic("WMR_BufZone_FinishedAllocs: No buffer.\r\n");

	_zone->buffer = buff;
	_zone->endOfBuffer = (buff + _zone->size);
	_zone->state = 2;
}

void WMR_BufZone_Rebase(WMR_zone_t *_zone, uint32_t* ppBuffer)
{
	*ppBuffer = *ppBuffer + _zone->buffer;
	_zone->numRebases++;
}

void WMR_BufZone_FinishedRebases(WMR_zone_t *_zone)
{
	if (_zone->numAllocs != _zone->numRebases)
		system_panic("WMR_BufZone_FinishedRebases: _zone->numAllocs != _zone->numRebases\r\n");
}

typedef struct {
	uint32_t unkn0;
	uint16_t unkn1;
	uint16_t unkn2;
	uint16_t unkn3;
	uint8_t unkn4;
	uint8_t unkn5;
} UNKNBUFFERSTRUCT;

typedef struct {
	WMR_zone_t zone;
	WMR_zone_t segment_info_temp;
	uint16_t unkFactor_0x1; // 38
	uint16_t unkn3A_0x800; // 3A
	uint32_t unknCalculatedValue0; // 3C
	uint32_t unknCalculatedValue1; // 40
	uint32_t total_pages_ftl; // 44
	uint16_t unknCalculatedValue3; // 48
	uint32_t* ftl2_buffer_x;
	uint32_t* ftl2_buffer_x2000;
	uint32_t unk74_4;
	uint32_t unk78_counter;
	uint32_t unk7C_byteMask;
	uint32_t unk80_3;
	uint32_t* unk84_buffer;
	uint32_t* unk88_buffer;
	uint32_t* unk8c_buffer;
	WMR_zone_t ftl_buffer2;
	uint32_t unkAC_2;
	uint32_t unkB0_1;
	uint32_t unkB4_buffer_0x8;
	uint32_t unkB8_buffer_0x8;
	uint8_t unkStruct_ftl[0x20]; // BC
	uint32_t* indexPageBuf; // EC
	UNKNBUFFERSTRUCT* unknBuffer4_ftl; // F0
	UNKNBUFFERSTRUCT* unknBuffer2_ftl; // F4
	uint32_t* pageBuffer2;
	uint32_t* buffer3;
	uint32_t* buffer4;
	uint32_t* buffer5;
	uint32_t* buffer6;
	uint32_t* buffer7;
	uint32_t* buffer8;
	uint32_t* buffer9;
	uint32_t* buffer10;
	uint32_t* buffer11;
	uint32_t* buffer12;
	uint32_t* buffer13;
	uint32_t* buffer14;
	uint32_t* buffer15;
	uint32_t* buffer16;
	uint32_t* buffer17;
	uint32_t* pageBuffer0;
	uint32_t* pageBuffer1;
	uint32_t unk140_n1;
	uint32_t* buffer19;
	uint32_t* unknBuffer3_ftl; // 148
	uint32_t* buffer20;
	uint32_t* buffer18;
	uint32_t unkn_pageOffset;
	uint32_t unknCalculatedValue4;
	uint32_t unknCalculatedValue5;
	uint32_t unknCalculatedValue6;
	uint32_t unknCalculatedValue7;
	uint32_t unknCalculatedValue8;
	uint32_t unknCalculatedValue9;
	uint32_t unk170_n1;
	uint16_t unkn_vflcxt69a;
	uint16_t unkn_vflcxt69c;
	uint16_t unkn_vflcxt69e;
	uint16_t unkn_1;
	uint32_t unk17C_0x0;
	uint32_t unk180_n1;
	uint32_t unk184_0xA;
	uint8_t unk188_0x63;
} YAFTL_INFO;
YAFTL_INFO yaftl_info;

typedef struct {
	uint16_t pages_per_block_total_banks;
	uint16_t usable_blocks_per_bank;
	uint16_t bytes_per_page_ftl;
	uint16_t meta_struct_size_0xC;
	uint16_t total_banks;
	uint32_t total_usable_pages;
} NAND_GEOMETRY_FTL;
NAND_GEOMETRY_FTL nand_geometry_ftl;

uint32_t BTOC_Init() {
	yaftl_info.unk74_4 = 4;
	yaftl_info.unk78_counter = 0;
	yaftl_info.unkAC_2 = 2;
	yaftl_info.unkB0_1 = 1;

	yaftl_info.unk8c_buffer_0x10 = wmr_malloc(0x10);
	yaftl_info.unk88_buffer_0x10 = wmr_malloc(yaftl_info.unk74_4 << 2);
	yaftl_info.unk84_buffer_0x10 = wmr_malloc(yaftl_info.unk74_4 << 2);
	yaftl_info.unkB8_buffer_0x8 = wmr_malloc(yaftl_info.unkAC_2 << 2);
	yaftl_info.unkB4_buffer_0x8 = wmr_malloc(yaftl_info.unkAC_2 << 2);

	if(!yaftl_info.unk8c_buffer_0x10 || !yaftl_info.unk8c_buffer_0x10 || !yaftl_info.unk84_buffer_0x10 || !yaftl_info.unkB8_buffer_0x8 || !yaftl_info.unkB4_buffer_0x8)
		return -1;

	uint32_t i;
	for(i = 0; i < yaftl_info.unk74_4; i++) {
		yaftl_info.unk88_buffer_0x10[i] = -1;
	}

	for(i = 0; i < yaftl_info.unkAC_2; i++) {
		yaftl_info.unkB4_buffer_0x8[i] = -1;
		yaftl_info.unkB8_buffer_0x8[i] = wmr_malloc(nand_geometry_ftl.pages_per_block_total_banks<<2);
		memset(yaftl_info.unkB8_buffer_0x8[i], 0xFF, nand_geometry_ftl.pages_per_block_total_banks<<2);
	}

	yaftl_info.unk7C_byteMask = (1 << yaftl_info.unk74_4) - 1;
	yaftl_info.unk80_3 = 3;

	WMR_BufZone_Init(&yaftl_info.ftl_buffer2);

	for(i = 0; i < yaftl_info.unk74_4; i++) {
		yaftl_info.unk84_buffer_0x10[i] = WMR_Buf_Alloc_ForDMA(&yaftl_info.ftl_buffer2, nand_geometry_ftl.bytes_per_page_ftl * unknFactor_0x1);
	}

	if(!WMR_BufZone_FinishedAllocs(&yaftl_info.ftl_buffer2))
		return -1;

	for(i = 0; i < yaftl_info.unk74_4; i++) {
		WMR_BufZone_Rebase(&yaftl_info.ftl_buffer2, &yaftl_info.unk84_buffer_0x10[i]);
	}

	WMR_BufZone_FinishedRebases(&yaftl_info.ftl_buffer2);

	return 0;
}

uint32_t BTOC_Alloc(uint32_t _arg0, uint32_t _arg1) {
	uint32_t inited = 0x7FFFFFFF;
	yaftl_info.unkB0_1 = (yaftl_info.unkB0_1 + 1) % yaftl_info.unkAC_2;
	yaftl_info.unkB4_buffer_0x8[yaftl_info.unkB0_1] = _arg0;

	memset(yaftl_info.unkB8_buffer_0x8[yaftl_info.unkB0_1], 0xFF, nand_geometry_ftl.pages_per_block_total_banks<<2);

	yaftl_info.unk78_counter++;

	uint32_t found;
	uint32_t i;
	for(i = 0; i < yaftl_info.unk74_4; i++) {
		if((yaftl_info.unk80_3 & 1<<i) && (yaftl_info.unk7C_byteMask & 1<<i)) {
			if(inited > yaftl_info.unk8c_buffer_0x10[i]) {
				found = i;
				inited = yaftl_info.unk8c_buffer_0x10[i];
			}
	}

	if(!found)
		system_panic("BTOC_Alloc: Couldn't allocate a BTOC.\r\n");

	yaftl_info.unk7C_byteMask &= (~(1<<i));
	yaftl_info.unk88_buffer_0x10[i] = _arg0;
	yaftl_info.unk8c_buffer_0x10[i] = yaftl_info.unk78_counter;
	return yaftl_info.unk84_buffer_0x10[i];
}

void YAFTL_Init() {
	if(yaFTL_inited)
		system_panic("Oh shit\r\n");

	memset(yaftl_info, 0, sizeof(yaftl_info)); // 0x358

	yaftl_info.unkn140_n1 = -1;
	yaftl_info.unkn170_n1 = -1;
	yaftl_info.unkn184_0xA = 0xA;
	yaftl_info.unkn188_0x63 = 0x63;

	memcpy(pVSVFL_fn_table, VSVFL_fn_table, 0x2C);

	nand_geometry_ftl.pages_per_block_total_banks = vfl_device_get_info(_dev, diPagesPerBlockTotalBanks);
	nand_geometry_ftl.usable_blocks_per_bank = vfl_device_get_info(_dev, diSomeThingFromVFLCXT);
	nand_geometry_ftl.bytes_per_page_ftl = vfl_device_get_info(_dev, diUnkn140x2000);
	nand_geometry_ftl.meta_struct_size_0xC = vfl_device_get_info(_dev, diMetaBytes0xC) * vfl_device_get_info(_dev, diUnkn20_1);
	nand_geometry_ftl.total_banks_ftl = vfl_device_get_info(_dev, diTotalBanks);
	nand_geometry_ftl.total_usable_pages = nand_geometry_ftl.pages_per_block_total_banks * nand_geometry_ftl.usable_blocks_per_bank;

	if(nand_geometry_ftl.meta_struct_size_0xC != 0xC)
		system_panic("MetaStructSize not equal 0xC!\r\n");

	uint32_t i = 1;
	while(((i * nand_geometry_ftl.bytes_per_page_ftl) - (i << 2)) < ((nand_geometry_ftl.pages_per_block_total_banks - i) << 2)) { i++; };
	yaftl_info.unkFactor_0x1 = i;
	yaftl_info.unkn3A_0x800 = nand_geometry_ftl.bytes_per_page_ftl >> 2;
	WMR_BufZone_Init(&yaftl_info.zone);
	yaftl_info.pageBuffer0 = WMR_Buf_Alloc_ForDMA(&yaftl_info.zone, nand_geometry_ftl.bytes_per_page_ftl);
	yaftl_info.pageBuffer1 = WMR_Buf_Alloc_ForDMA(&yaftl_info.zone, nand_geometry_ftl.bytes_per_page_ftl);
	yaftl_info.pageBuffer2 = WMR_Buf_Alloc_ForDMA(&yaftl_info.zone, nand_geometry_ftl.bytes_per_page_ftl * yaftl_info.unkFactor_0x1);
	yaftl_info.buffer3 = WMR_Buf_Alloc_ForDMA(&yaftl_info.zone, 0xC);
	yaftl_info.buffer4 = WMR_Buf_Alloc_ForDMA(&yaftl_info.zone, 0xC);
	yaftl_info.buffer5 = WMR_Buf_Alloc_ForDMA(&yaftl_info.zone, 0xC);
	yaftl_info.buffer6 = WMR_Buf_Alloc_ForDMA(&yaftl_info.zone, 0xC);
	yaftl_info.buffer7 = WMR_Buf_Alloc_ForDMA(&yaftl_info.zone, 0xC);
	yaftl_info.buffer8 = WMR_Buf_Alloc_ForDMA(&yaftl_info.zone, 0xC);
	yaftl_info.buffer9 = WMR_Buf_Alloc_ForDMA(&yaftl_info.zone, 0xC);
	yaftl_info.buffer10 = WMR_Buf_Alloc_ForDMA(&yaftl_info.zone, 0xC);
	yaftl_info.buffer11 = WMR_Buf_Alloc_ForDMA(&yaftl_info.zone, 0xC);
	yaftl_info.buffer12 = WMR_Buf_Alloc_ForDMA(&yaftl_info.zone, 0xC);
	yaftl_info.buffer13 = WMR_Buf_Alloc_ForDMA(&yaftl_info.zone, 0xC);
	yaftl_info.buffer14 = WMR_Buf_Alloc_ForDMA(&yaftl_info.zone, 0xC);
	yaftl_info.buffer15 = WMR_Buf_Alloc_ForDMA(&yaftl_info.zone, 0xC);
	yaftl_info.buffer16 = WMR_Buf_Alloc_ForDMA(&yaftl_info.zone, 0xC);
	yaftl_info.buffer17 = WMR_Buf_Alloc_ForDMA(&yaftl_info.zone, 0xC);
	yaftl_info.buffer18 = WMR_Buf_Alloc_ForDMA(&yaftl_info.zone, 0xC);
	yaftl_info.buffer19 = WMR_Buf_Alloc_ForDMA(&yaftl_info.zone, nand_geometry_ftl.total_banks_ftl * 0x180);
	yaftl_info.buffer20 = WMR_Buf_Alloc_ForDMA(&yaftl_info.zone, nand_geometry_ftl.pages_per_block_total_banks * 0xC);
	WMR_BufZone_FinishedAllocs(&yaftl_info.zone);

	WMR_BufZone_Rebase(&yaftl_info.zone, yaftl_info.pageBuffer0);
	WMR_BufZone_Rebase(&yaftl_info.zone, yaftl_info.pageBuffer1);
	WMR_BufZone_Rebase(&yaftl_info.zone, yaftl_info.pageBuffer2);
	WMR_BufZone_Rebase(&yaftl_info.zone, yaftl_info.buffer3);
	WMR_BufZone_Rebase(&yaftl_info.zone, yaftl_info.buffer4);
	WMR_BufZone_Rebase(&yaftl_info.zone, yaftl_info.buffer5);
	WMR_BufZone_Rebase(&yaftl_info.zone, yaftl_info.buffer6);
	WMR_BufZone_Rebase(&yaftl_info.zone, yaftl_info.buffer7);
	WMR_BufZone_Rebase(&yaftl_info.zone, yaftl_info.buffer8);
	WMR_BufZone_Rebase(&yaftl_info.zone, yaftl_info.buffer9);
	WMR_BufZone_Rebase(&yaftl_info.zone, yaftl_info.buffer10);
	WMR_BufZone_Rebase(&yaftl_info.zone, yaftl_info.buffer11);
	WMR_BufZone_Rebase(&yaftl_info.zone, yaftl_info.buffer12);
	WMR_BufZone_Rebase(&yaftl_info.zone, yaftl_info.buffer13);
	WMR_BufZone_Rebase(&yaftl_info.zone, yaftl_info.buffer14);
	WMR_BufZone_Rebase(&yaftl_info.zone, yaftl_info.buffer15);
	WMR_BufZone_Rebase(&yaftl_info.zone, yaftl_info.buffer16);
	WMR_BufZone_Rebase(&yaftl_info.zone, yaftl_info.buffer17);
	WMR_BufZone_Rebase(&yaftl_info.zone, yaftl_info.buffer18);
	WMR_BufZone_Rebase(&yaftl_info.zone, yaftl_info.buffer19);
	WMR_BufZone_Rebase(&yaftl_info.zone, yaftl_info.buffer20);
	WMR_BufZone_FinishedRebases(&yaftl_info.zone);

	yaftl_info.unknCalculatedValue0 = (((nand_geometry_ftl.pages_per_block_total_banks - yaftl_info.unkFactor_0x1) - (nand_geometry_ftl.usable_blocks_per_bank - 8)) / ((nand_geometry_ftl.pages_per_block_total_banks - yaftl_info.unkFactor_0x1) * yaftl_info.unkn3A_0x800) * 3);
	if(((nand_geometry_ftl.pages_per_block_total_banks - yaftl_info.unkFactor_0x1) - (nand_geometry_ftl.usable_blocks_per_bank - 8)) % ((nand_geometry_ftl.pages_per_block_total_banks - yaftl_info.unkFactor_0x1) * yaftl_info.unkn3A_0x800))
		yaftl_info.unknCalculatedValue0 = yaftl_info.unknCalculatedValue0+3;

	yaftl_info.unknCalculatedValue1 = nand_geometry_ftl.usable_blocks_per_bank - yaftl_info.unknCalculatedValue0 - 3;

	yaftl_info.total_pages_ftl = ((nand_geometry_ftl.pages_per_block_total_banks - yaftl_info.unkFactor_0x1) - (nand_geometry_ftl.usable_blocks_per_bank - 8)) - (nand_geometry_ftl.usable_blocks_per_bank * nand_geometry_ftl.pages_per_block_total_banks);

	yaftl_info.unknCalculatedValue3 = ((nand_geometry_ftl.pages_per_block_total_banks * nand_geometry_ftl.usable_blocks_per_bank) << 2) / nand_geometry_ftl.bytes_per_page_ftl;
	yaftl_info.indexPageBuf = wmr_malloc(yaftl_info.unknCalculatedValue3 << 3);
	if(!yaftl_info.indexPageBuf)
		system_panic("No buffer.\r\n");

	yaftl_info.unknBuffer2_ftl = wmr_malloc(nand_geometry_ftl.usable_blocks_per_bank * 0xC);
	if(!yaftl_info.unknBuffer2_ftl)
		system_panic("No buffer.\r\n");

	yaftl_info.unknBuffer3_ftl = wmr_malloc(nand_geometry_ftl.pages_per_block_total_banks << 2);
	if(!yaftl_info.unknBuffer3_ftl)
		system_panic("No buffer.\r\n");

	yaftl_info.unknCalculatedValue4 = (yaftl_info.unknCalculatedValue3 << 2) / nand_geometry_ftl.bytes_per_page_ftl;
	if((yaftl_info.unknCalculatedValue3 << 2) % nand_geometry_ftl.bytes_per_page_ftl)
		yaftl_info.unknCalculatedValue4++;

	yaftl_info.unknCalculatedValue5 = (yaftl_info.usable_blocks_per_bank / nand_geometry_ftl.bytes_per_page_ftl) & 0xFFFF;
	if((yaftl_info.usable_blocks_per_bank % nand_geometry_ftl.bytes_per_page_ftl) & 0xFFFF)
		yaftl_info.unknCalculatedValue5++;

	yaftl_info.unknCalculatedValue6 = (yaftl_info.usable_blocks_per_bank << 1) / nand_geometry_ftl.bytes_per_page_ftl;
	yaftl_info.unknCalculatedValue8 = yaftl_info.unknCalculatedValue6;
	yaftl_info.unknCalculatedValue9 = yaftl_info.unknCalculatedValue6;
	uint32_t unknv6mod = (yaftl_info.usable_blocks_per_bank << 1) % nand_geometry_ftl.bytes_per_page_ftl;
	if(unknv6mod) {
		yaftl_info.unknCalculatedValue6++;
		yaftl_info.unknCalculatedValue8++;
		yaftl_info.unknCalculatedValue9++;
	}

	yaftl_info.unknCalculatedValue7 =  (yaftl_info.usable_blocks_per_bank << 2) / nand_geometry_ftl.bytes_per_page_ftl;
	if((yaftl_info.usable_blocks_per_bank << 2) % nand_geometry_ftl.bytes_per_page_ftl)
		yaftl_info.unknCalculatedValue7++;

	yaftl_info.unkn_pageOffset = yaftl_info.unknCalculatedValue4 + yaftl_info.unknCalculatedValue5 + yaftl_info.unknCalculatedValue6 + yaftl_info.unknCalculatedValue7 + yaftl_info.unknCalculatedValue8 + yaftl_info.unknCalculatedValue9 + ((yaftl_info.unkFactor_0x1 << 1)+1);

	memset(yaftl_info.unkStruct_ftl, 0, 0x20);

	if(BTOC_Init())
		system_panic("Actually (and before) it should free something, but,.. hey, we don't fuck up, do we?\r\n");

	yaftl_info.ftl2_buffer_x = BTOC_Alloc(~2, 1);
	yaftl_info.ftl2_buffer_x2000 = BTOC_Alloc(~1, 0);

	yaFTL_inited = 1;

	return 0;
}

uint32_t YAFTL_Open(uint32_t* pagesAvailable, uint32_t* bytesPerPage, uint32_t signature_bit) {
	uint16_t vfl_bytes[3];
	uint32_t unkn1 = 1;

	memset(yaftl_info.indexPageBuf, 0xFF, yaft_info.unknCalculatedValue3<<3);
	memset(yaftl_info.unknBuffer2_ftl, 0xFF, nand_geometry_ftl.usable_blocks_per_bank * 0xC);

	uint32_t i;
	for (i = 0; i <= nand_geometry_ftl.usable_blocks_per_bank) {
		yaftl_info.unknBuffer2_ftl[i].unkn0 = 0;
		yaftl_info.unknBuffer2_ftl[i].unkn1 = 0;
		yaftl_info.unknBuffer2_ftl[i].unkn2 = 0;
		yaftl_info.unknBuffer2_ftl[i].unkn3 = 0;
		yaftl_info.unknBuffer2_ftl[i].unkn4 = 0;
		yaftl_info.unknBuffer2_ftl[i].unkn5 = 0;
	}

	memcpy(vfl_bytes, some_vsvfl_sub_to_get_most_recent_vflcxt_offset_0x69a(), sizeof(vfl_bytes));
	if (vfl_bytes[0] != 0xFFFF) { // !restoreMount
		// mark first 3 blocks as dead
		yaftl_info.unkn_vflcxt69a = vfl_bytes[0];
		yaftl_info.unkn_vflcxt69c = vfl_bytes[1];
		yaftl_info.unkn_vflcxt69e = vfl_bytes[2];

		yaftl_info.unknBuffer2_ftl[yaftl_info.unkn_vflcxt69a].unkn4 = 2;
		yaftl_info.unknBuffer2_ftl[yaftl_info.unkn_vflcxt69c].unkn4 = 2;
		yaftl_info.unknBuffer2_ftl[yaftl_info.unkn_vflcxt69e].unkn4 = 2;

		if(!yaftl_info.pageBuffer0) {
			system_panic("This can't happen. This shouldn't happen. Whatever, it's doing something else then.\r\n");
		else {
			// wtf has been done before
			yaftl_info.unknBuffer2_ftl[vfl_bytes[0]].unkn4 = 2;
			yaftl_info.unknBuffer2_ftl[vfl_bytes[1]].unkn4 = 2;
			yaftl_info.unknBuffer2_ftl[vfl_bytes[2]].unkn4 = 2;

			// find valid ftlcxt page
			uint32_t sth1 = 0;
			uint32_t sth2 = 0;
			for (i = 0; i < 3; i++) {
				if(!YAFTL_readPage(vfl_bytes[i] * yaftl_info.pages_per_block_total_banks, yaftl_info.pageBuffer0, buffer6, 0, 1, 0)) {
					if(yaftl_info.buffer6[1] != 0xFFFFFFFF && yaftl_info.buffer6[1] > sth1) {
						sth1 = yaftl_info.buffer6[1];
						sth2 = vfl_bytes[i];
					}
				}
			}
			uint32_t some_val;
			if(sth1) {
				yaftl_info.unknBuffer2_ftl[sth2] = 16;
				i = 0;
				while(1) {
					if(((uint16_t)yaftl_info.unkn_pageOffset)+i < nand_geometry_ftl.pages_per_block_total_banks
							&& !YAFTL_readPage(yaftl_info.pages_per_block_total_banks * sth2 + i), yaftl_info.pageBuffer0, buffer6, 0, 1, 0) {
						if(YAFTL_readPage(yaftl_info.pages_per_block_total_banks*sth2+yaftl_info.unkn_pageOffset+i, yaftl_info.pageBuffer0, buffer6, 0, 1, 0) == 1) {
							yaftl_info.unkn170_n1 = yaftl_info.pages_per_block_total_banks*sth2+i;
							if(_readCxtInfo(yaftl_info.pages_per_block_total_banks*sth2+i, pageBuffer0, 1, &unkn1))
								some_val = 5;
							else
								some_val = 0;
							break;
						}
					} else {
						_readCxtInfo(yaftl_info.pages_per_block_total_banks*sth2+(~yaftl_info.unkn_pageOffset)+i, yaftl_info.pageBuffer0, 0, &unkn1);
						some_val = 5;
					}
				}
			} else {
				yaftl_info.unknBuffer2_ftl[vfl_bytes[0]] = 16;
				some_val = 5;
			}

			if(unkn1 != 1) {
				bufferPrintf("YAFTL_Open: unsupported low-level format version.\r\n");
				return -1;
			}

			if(some_val || signature_bit) {
				bufferPrintf("Ach, fu, 0x80000001\r\n");
				yaftl_info.unk184_0xA *= 100;
			}

			yaftl_info.unknBuffer4_ftl = wmr_malloc(yaftl_info.unk184_0xA*0xC);
			if(!yaftl_info.unknBuffer4_ftl) {
				bufferPrintf("Ach, fu, 0x80000001\r\n");
				return -1;
			}

			WMR_BufZone_Init(&yaftl_info.segment_info_temp);
			for(i = 0; i < yaftl_info.unk184_0xA; i++) {
				yaftl_info.unknBuffer4_ftl.unkn0 = WMR_Buf_Alloc_ForDMA(&yaftl_info.segment_info_temp, nand_geometry_ftl.bytes_per_page_ftl);
			}
			if(!WMR_BufZone_FinishedAllocs(&yaftl_info.segment_info_temp))
				return -1;
			for(i = 0; i < yaftl_info.unk184_0xA; i++) {
				WMR_BufZone_Rebase(&yaftl_info.segment_info_temp, yaftl_info.unknBuffer4_ftl[i]);
				yaftl_info.unknBuffer4_ftl[i].unkn1 = -1;
				yaftl_info.unknBuffer4_ftl[i].unkn2 = 0;
				yaftl_info.unknBuffer4_ftl[i].unkn3 = 0;
				yaftl_info.unknBuffer4_ftl[i].unkn4 = -1;
				yaftl_info.unknBuffer4_ftl[i].unkn5 = -1;
				memset(yaftl_info.unknBuffer4_ftl[i].unkn0, 0xFF, nand_geometry_ftl.bytes_per_page_ftl);
			}
			WMR_BufZone_FinishedRebases(&yaftl_info.segment_info_temp);

			pagesAvailable = yaftl_info.total_pages_ftl*0.99;
			bytesPerPage = nand_geometry_ftl.bytes_per_page_ftl;
			yaftl_info.unkn17C_0x0 = 0;
			yaftl_info.unkn180_n1 = -1;
			for (i = 0; i <= nand_geometry_ftl.usable_blocks_per_bank; i++) {
				if(yaftl_info.unknBuffer2_ftl[i].unkn4 != 16 && yaftl_info.unknBuffer2_ftl[i].unkn4 != 2) {
					if(yaftl_info.unknBuffer2_ftl[i].unkn0 > yaftl_info.unkn17C_0x0)
						yaftl_info.unkn17C_0x0 = yaftl_info.unknBuffer2_ftl[i].unkn0;
					if(yaftl_info.unknBuffer2_ftl[i].unkn0 < yaftl_info.unkn180_n1)
						yaftl_info.unkn180_n1 = yaftl_info.unknBuffer2_ftl[i].unkn0;
				}
			}
			SetupFreeAndAllocd();
			yaftl_loop_thingy();
			return 0;
		}
	} else {
		// do restore
		// NO! Better not! Not yet...
	}
}
