#include "dtypes.h"

// typedef enum  {
// 	FUNC_RET_OK, FUNC_RET_NOT_AVAIL, FUNC_RET_ERROR, FUNC_RET_BUFFER_TOO_SMALL
// } FUNCTION_RETURN;

// typedef unsigned char PcIdentifier[6];
// typedef char PcSignature[21];

FUNCTION_RETURN encode_pc_id(PcIdentifier identifier1, PcIdentifier identifier2, PcSignature pc_identifier_out);
FUNCTION_RETURN decode_pc_id(PcIdentifier identifier1_out, PcIdentifier identifier2_out, PcSignature pc_signature_in); 
