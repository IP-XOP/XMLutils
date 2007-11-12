/*
 *  error.h
 *  XMLutils
 *
 *  Created by andrew on 6/11/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

// Custom error codes
#define REQUIRES_IGOR_504				1 + FIRST_XOP_ERR
#define XMLDOC_PARSE_ERROR				2 + FIRST_XOP_ERR
#define XPATH_CONTEXT_CREATION_ERROR	3 + FIRST_XOP_ERR
#define FAILED_TO_REGISTER_NAMESPACE	4 + FIRST_XOP_ERR
#define UNABLE_TO_EVAL_XPATH_EXPR		5 + FIRST_XOP_ERR
#define NULL_STRING_HANDLE				6 + FIRST_XOP_ERR
#define XPATH_COMPILE_ERROR				7 + FIRST_XOP_ERR
#define XML_COULDNT_SAVE				8 + FIRST_XOP_ERR