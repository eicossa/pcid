#include "encodedecode.h"

FUNCTION_RETURN encode_pc_id(PcIdentifier identifier1, PcIdentifier identifier2, PcSignature pc_identifier_out) 
{
	// TODO base62 encoding, now uses base64
	PcIdentifier concat_identifiers[2];
	char* b64_data = NULL;
	int b64_size = 0;
	size_t concatIdentifiersSize = sizeof(PcIdentifier) * 2;
	//concat_identifiers = (PcIdentifier *) malloc(concatIdentifiersSize);
	memcpy(&concat_identifiers[0], identifier1, sizeof(PcIdentifier));
	memcpy(&concat_identifiers[1], identifier2, sizeof(PcIdentifier));
	// two identifiers are simply concatenated and base64'ed
	b64_data = base64(concat_identifiers, concatIdentifiersSize, &b64_size);
	// ----
	if (b64_size > sizeof(PcSignature)) {
		free(b64_data);
		return FUNC_RET_BUFFER_TOO_SMALL;
	}
	
	sprintf(pc_identifier_out, "%.4s-%.4s-%.4s-%.4s", &b64_data[0],	&b64_data[4], &b64_data[8], &b64_data[12]);
	//printf("encode_pc_id() - \n");
	//free(concat_identifiers);
	free(b64_data);
	return FUNC_RET_OK;
}


/**
 * Extract the two pc identifiers from the user provided code.
 * @param identifier1_out
 * @param identifier2_out
 * @param str_code: the code in the string format XXXX-XXXX-XXXX-XXXX
 * @return
 */
FUNCTION_RETURN decode_pc_id(PcIdentifier identifier1_out,
		                     PcIdentifier identifier2_out, 
							 PcSignature pc_signature_in) 
{
	//TODO base62 encoding, now uses base64

	unsigned char * concat_identifiers = NULL;
	char base64ids[17];
	int identifiers_size;

	sscanf(pc_signature_in, "%4s-%4s-%4s-%4s", &base64ids[0], &base64ids[4], &base64ids[8], &base64ids[12]);
	concat_identifiers = unbase64(base64ids, 16, &identifiers_size);
	if (identifiers_size > sizeof(PcIdentifier) * 2) {
		free(concat_identifiers);
		return FUNC_RET_BUFFER_TOO_SMALL;
	}
	memcpy(identifier1_out, concat_identifiers,                        sizeof(PcIdentifier));
	memcpy(identifier2_out, concat_identifiers + sizeof(PcIdentifier), sizeof(PcIdentifier));
	free(concat_identifiers);
	return FUNC_RET_OK;
}
