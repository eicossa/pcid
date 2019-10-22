#include <iostream>
#include "private-key.h"
#include "dtypes.h"

int main(int argc, const char** argv) 
{
    PcSignature str_code;
    FUNCTION_RETURN genok = generate_user_pc_signature(str_code, DEFAULT);
    printf("PC SIGNATURE : %s \n", str_code);
}