#include "hardware/arm.h"
#include "arm.h"
#include "openiboot-asmhelpers.h"

int arm_setup() {
#ifndef CONFIG_IPHONE_4G
	CleanAndInvalidateCPUDataCache();
	ClearCPUInstructionCache();
#endif

	WriteControlRegisterConfigData(ReadControlRegisterConfigData() & ~(ARM11_Control_INSTRUCTIONCACHE));	// Disable instruction cache
	WriteControlRegisterConfigData(ReadControlRegisterConfigData() & ~(ARM11_Control_DATACACHE));		// Disable data cache

#ifndef CONFIG_IPHONE_4G
	GiveFullAccessCP10CP11();
	EnableVFP();

	// Map the peripheral port of size 128 MB to 0x38000000
	WritePeripheralPortMemoryRemapRegister(PeripheralPort | ARM11_PeripheralPortSize128MB);

	InvalidateCPUDataCache();
	ClearCPUInstructionCache();
#endif

	WriteControlRegisterConfigData(ReadControlRegisterConfigData() | ARM11_Control_INSTRUCTIONCACHE);	// Enable instruction cache
	WriteControlRegisterConfigData(ReadControlRegisterConfigData() | ARM11_Control_DATACACHE);		// Enable data cache

#ifndef CONFIG_IPHONE_4G
	WriteControlRegisterConfigData((ReadControlRegisterConfigData()
		& ~(ARM11_Control_STRICTALIGNMENTCHECKING))				// Disable strict alignment fault checking
		| ARM11_Control_UNALIGNEDDATAACCESS);					// Enable unaligned data access operations
#else
	WriteControlRegisterConfigData((ReadControlRegisterConfigData()
		& ~(ARM11_Control_STRICTALIGNMENTCHECKING)));				// Disable strict alignment fault checking
#endif


	WriteControlRegisterConfigData(ReadControlRegisterConfigData() | ARM11_Control_BRANCHPREDICTION); 	// Enable branch prediction

#ifndef CONFIG_IPHONE_4G
	// Enable return stack, dynamic branch prediction, static branch prediction
	WriteAuxiliaryControlRegister(ReadAuxiliaryControlRegister()
		| ARM11_AuxControl_RETURNSTACK
		| ARM11_AuxControl_DYNAMICBRANCHPREDICTION
		| ARM11_AuxControl_STATICBRANCHPREDICTION);
#endif
	return 0;
}

void arm_disable_caches() {
	CleanAndInvalidateCPUDataCache();
	ClearCPUInstructionCache();
	WriteControlRegisterConfigData(ReadControlRegisterConfigData() & ~ARM11_Control_INSTRUCTIONCACHE);	// Disable instruction cache
	WriteControlRegisterConfigData(ReadControlRegisterConfigData() & ~ARM11_Control_DATACACHE);		// Disable data cache
}
