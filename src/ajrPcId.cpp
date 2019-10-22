#include "ajrPcId.h"
#include "encodedecode.h"

FUNCTION_RETURN generate_default_pc_id(PcIdentifier * identifiers, unsigned int * num_identifiers) 
{

	//printf("---------------------------------------------------------\n");
	//printf("generate_default_pc_id() : Called with num_identifiers = %d \n", *num_identifiers);
	size_t adapter_num, disk_num;
	FUNCTION_RETURN result_adapterInfos, result_diskinfos, function_return;
	unsigned int caller_identifiers, i, j, k, array_index;
	DiskInfo      *diskInfoPtr;
	OsAdapterInfo *adapterInfoPtr;

	if (identifiers == NULL || *num_identifiers == 0) {
		result_adapterInfos = getAdapterInfos(NULL, &adapter_num);
		//printf("generate_default_pc_id() : Num of adapters %d \n", adapter_num);
		if (result_adapterInfos != FUNC_RET_OK) {
			//printf("generate_default_pc_id() :: within result_adapterInfos != FUNC_RET_OK \n");
			return generate_disk_pc_id(identifiers, num_identifiers, false);
		}

		result_diskinfos = getDiskInfos(NULL, &disk_num);
		//printf("generate_default_pc_id() : Num of disks : %d \n", disk_num);
		if (result_diskinfos != FUNC_RET_OK) {
			//printf("generate_default_pc_id() :: within result_diskinfos != FUNC_RET_OK \n");
			return generate_ethernet_pc_id(identifiers, num_identifiers, true);
		}

		*num_identifiers = disk_num * adapter_num;
		function_return  = FUNC_RET_OK;

	} else {

		//printf("generate_default_pc_id() :: within adapterInfoPtr \n");
		adapterInfoPtr = (OsAdapterInfo*) malloc((*num_identifiers) * sizeof(OsAdapterInfo));
		adapter_num = *num_identifiers;
		result_adapterInfos = getAdapterInfos(adapterInfoPtr, &adapter_num);

		if (result_adapterInfos != FUNC_RET_OK && result_adapterInfos != FUNC_RET_BUFFER_TOO_SMALL) {
			//printf("generate_default_pc_id() :: within result_adapterInfos != FUNC_RET_OK != FUNC_RET_BUFFER_TOO_SMALL");	
			free(adapterInfoPtr);
			return generate_disk_pc_id(identifiers, num_identifiers, false);
		}

		diskInfoPtr      = (DiskInfo*) malloc((*num_identifiers) * sizeof(DiskInfo));
		disk_num         = *num_identifiers;
		result_diskinfos = getDiskInfos(diskInfoPtr, &disk_num);

		if (result_diskinfos != FUNC_RET_OK	&& result_diskinfos != FUNC_RET_BUFFER_TOO_SMALL) {
			//printf("generate_default_pc_id() :: within result_diskinfos != FUNC_RET_OK != FUNC_RET_BUFFER_TOO_SMALL");				
			free(diskInfoPtr);
			free(adapterInfoPtr);
			return generate_ethernet_pc_id(identifiers, num_identifiers, true);
		}

		function_return = FUNC_RET_OK;

		caller_identifiers = *num_identifiers;
		//printf("NumDisks : %d | NumAdapters : %d \n", disk_num, adapter_num);


		for (i = 0; i < disk_num; i++) {
			for (j = 0; j < adapter_num; j++) {
				array_index = i * adapter_num + j;
				if (array_index >= caller_identifiers) {
					function_return = FUNC_RET_BUFFER_TOO_SMALL;
					//sweet memories...
					goto end;
				}
				for (k = 0; k < 6; k++){
					//printf("generate_default_pc_id() with k = %d disk_sn : %c \n", k,diskInfoPtr[i].disk_sn[k+2]);
					
					identifiers[array_index][k] = diskInfoPtr[i].disk_sn[k + 2]  ^ adapterInfoPtr[j].mac_address[k + 2];
					
				}
				// printf("Mixing %s with %s (%03d.%03d.%03d.%03d): identifiers[%d] : %02x%02x%02x%02x%02x%02x \n", 
				// 	diskInfoPtr[i].device, 
				// 	adapterInfoPtr[j].description, 
				// 	adapterInfoPtr[j].ipv4_address[0],adapterInfoPtr[j].ipv4_address[1],adapterInfoPtr[j].ipv4_address[2],adapterInfoPtr[j].ipv4_address[3],
				// 	array_index,
				// 	identifiers[array_index][0],identifiers[array_index][1],identifiers[array_index][2],identifiers[array_index][3],
				// 	identifiers[array_index][4],identifiers[array_index][5]);	


				// printf("identifiers[%d] = %x%x%x%x%x%x ^ %x%x%x%x%x%x = %x%x%x%x%x%x \n", 
				// 											array_index, 
				// 											diskInfoPtr[i].disk_sn[2],
				// 											diskInfoPtr[i].disk_sn[3],
				// 											diskInfoPtr[i].disk_sn[4],
				// 											diskInfoPtr[i].disk_sn[5],
				// 											diskInfoPtr[i].disk_sn[6],
				// 											diskInfoPtr[i].disk_sn[7],
				// 											adapterInfoPtr[j].mac_address[2],
				// 											adapterInfoPtr[j].mac_address[3],
				// 											adapterInfoPtr[j].mac_address[4],
				// 											adapterInfoPtr[j].mac_address[5],
				// 											adapterInfoPtr[j].mac_address[6],
				// 											adapterInfoPtr[j].mac_address[7],
				//                                             identifiers[array_index][0], 
				// 											identifiers[array_index][1],
				// 											identifiers[array_index][2],
				// 											identifiers[array_index][4],
				// 											identifiers[array_index][5]);

			}
		}
end:
#ifdef _MSC_VER
        *num_identifiers = min(*num_identifiers, adapter_num * disk_num);
#else
        *num_identifiers = cmin(*num_identifiers, adapter_num * disk_num);
#endif
		// printf("num_identifiers : %u \n", *num_identifiers);
		free(diskInfoPtr);
		free(adapterInfoPtr);
	}
	// printf("---------------------------------------------------------\n");
	return function_return;
}





IDENTIFICATION_STRATEGY strategy_from_pc_id(PcIdentifier identifier) 
{
	return (IDENTIFICATION_STRATEGY) (identifier[0] >> 5);
}


/**
 *
 * Calculates all the possible identifiers for the current machine, for the
 * given calculation strategy requested. Pc identifiers are more than one,
 * for instance a machine with more than one disk and one network interface has
 * usually multiple identifiers.
 *
 * First 4 bit of each pc identifier are reserved 
 * - 3 for the type of strategy used in calculation and 
 * - 1 for parity checks (not implemented here)
 *
 * @param identifiers
 * @param array_size
 * @param
 * @return
 */

FUNCTION_RETURN generate_pc_id(PcIdentifier * identifiers,
		                       unsigned int * array_size, 
							   IDENTIFICATION_STRATEGY strategy) 
{
	FUNCTION_RETURN result;
	unsigned int i, j;
	const unsigned int original_array_size = *array_size;
	unsigned char strategy_num;
	//printf("generate_pc_id() with IDENTIFICATION STRATEGY : %d \n", strategy);
	switch (strategy) {
	case DEFAULT:
		result = generate_default_pc_id(identifiers, array_size);
		break;
	case ETHERNET:
		result = generate_ethernet_pc_id(identifiers, array_size, true);
		break;
	case IP_ADDRESS:
		result = generate_ethernet_pc_id(identifiers, array_size, false);
		break;
	case DISK_NUM:
		result = generate_disk_pc_id(identifiers, array_size, false);
		break;
	case DISK_LABEL:
		result = generate_disk_pc_id(identifiers, array_size, true);
		break;
	default:
		return FUNC_RET_ERROR;
	}

	// printf("|--------------------------------------------------|\n");
	// printf("generate_pc_id() : Array_size - %d \n", *array_size);

	if (result == FUNC_RET_OK && identifiers != NULL) {
		strategy_num = strategy << 5;
		for (i = 0; i < *array_size; i++) {
			// printf("generate_pc_id() for %d - %02x.%02x.%02x.%02x.%02x.%02x \n", 
			// 		i, 
			// 		identifiers[i][0],identifiers[i][1],identifiers[i][2],
			// 		identifiers[i][3],identifiers[i][4],identifiers[i][5]);
			//encode strategy in the first three bits of the pc_identifier
			identifiers[i][0] = (identifiers[i][0] & 15) | strategy_num;
			//printf("about to try to print something here");
			//printf("identifiers[%d][%d] : %s ", i, 0, identifiers[i][0]);
		}
		//fill array if larger
		for (i = *array_size; i < original_array_size; i++) {
			identifiers[i][0] = STRATEGY_UNKNOWN << 5;
			for (j = 1; j < sizeof(PcIdentifier); j++) {
				identifiers[i][j] = 42; //padding
			}
		}
	}
	//printf("|----|\n\n\n\n");
	return result;
}



/* the main thang bras */
FUNCTION_RETURN generate_user_pc_signature(PcSignature identifier_out, IDENTIFICATION_STRATEGY strategy) 
{
	FUNCTION_RETURN result;
	PcIdentifier* identifiers;
	unsigned int req_buffer_size = 0;

	//strategy = ETHERNET;
	strategy = DISK_NUM;
	result = generate_pc_id(NULL, &req_buffer_size, strategy);
	if (result != FUNC_RET_OK) {
		return result;
	}
	if (req_buffer_size == 0) {
		return FUNC_RET_ERROR;
	}
	req_buffer_size = req_buffer_size < 2 ? 2 : req_buffer_size;
	identifiers = (PcIdentifier *) malloc(sizeof(PcIdentifier) * req_buffer_size);

	result = generate_pc_id(identifiers, &req_buffer_size, strategy);
	

	if (result != FUNC_RET_OK) {
		free(identifiers);
		return result;
	}
#ifdef __linux__
	//VALGRIND_CHECK_VALUE_IS_DEFINED(identifiers[0]);
	//VALGRIND_CHECK_VALUE_IS_DEFINED(identifiers[1]);
#endif
	// printf("identifiers[0] = %02x.%02x.%02x.%02x.%02x.%02x \n",
	// 		identifiers[0][0],identifiers[0][1],identifiers[0][2],
	// 		identifiers[0][3],identifiers[0][4],identifiers[0][5]);

	// printf("identifiers[1] = %02x.%02x.%02x.%02x.%02x.%02x \n",
	// 		identifiers[1][0],identifiers[1][1],identifiers[1][2],
	// 		identifiers[1][3],identifiers[1][4],identifiers[1][5]);

	result = encode_pc_id(identifiers[0], identifiers[1], identifier_out);

	// printf("identifier_out = %02x.%02x.%02x.%02x.%02x.%02x \n",
	// 		identifier_out[0],identifier_out[1],identifier_out[2],
	// 		identifier_out[3],identifier_out[4],identifier_out[5]);
	// printf("identifier_out : %s\n", identifier_out);
	printf("---------\n");

#ifdef __linux__
	//VALGRIND_CHECK_VALUE_IS_DEFINED(identifier_out);
#endif
	free(identifiers);
	return result;
}
