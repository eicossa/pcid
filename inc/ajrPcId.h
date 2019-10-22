#include "dtypes.h"
#include "diskID.h"
#include "ntwkId.h"

// FUNCTION_RETURN encode_pc_id(PcIdentifier identifier1, PcIdentifier identifier2, PcSignature pc_identifier_out); 
// FUNCTION_RETURN decode_pc_id(PcIdentifier identifier1_out, PcIdentifier identifier2_out, PcSignature pc_signature_in);
IDENTIFICATION_STRATEGY strategy_from_pc_id(PcIdentifier identifier);
FUNCTION_RETURN generate_user_pc_signature(PcSignature identifier_out, IDENTIFICATION_STRATEGY strategy); 

FUNCTION_RETURN generate_pc_id(PcIdentifier * identifiers, unsigned int * array_size, IDENTIFICATION_STRATEGY strategy);
FUNCTION_RETURN generate_default_pc_id(PcIdentifier * identifiers, unsigned int * num_identifiers); 
