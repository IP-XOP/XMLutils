/*
 *  XMLutils.h
 *  XMLutils
 *
 *  Created by andrew on 6/11/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */
#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h
#include "error.h"

#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>
#include <libxml/xPath.h>
#include <libxml/xpathInternals.h>
  
/*
in XMLelementlist.cpp
*/
#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct XMLelemlistStruct {
	Handle fileNameStr;
}XMLelemlistStruct, *XMLelemlistStructPtr;
#include "XOPStructureAlignmentReset.h"
int XMLelemlist(XMLelemlistStructPtr p);

/* in XMLevaluateXPathExpression.cpp */
xmlXPathObject* execute_xpath_expression(xmlDoc* doc, const xmlChar* xpathExpr, const xmlChar* nsList,  int* err);

/*
in XMLstrFmXPath.cpp
*/
#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct XMLstrFmXpathStruct {
	Handle options;					//an options string
	Handle ns;						//a namespace to register
	Handle xPath;					//the xpath
	Handle fileNameStr;				//the filename to load
	Handle returnString;			//the string containing the content
}XMLstrFmXpathStruct, *XMLstrFmXpathStructPtr;
#include "XOPStructureAlignmentReset.h"
int XMLstrFmXPath(XMLstrFmXpathStructPtr p);

/*
in XMLparsedWaveFromXPath.cpp
*/ 
#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct XMLWaveXPathStruct {
	Handle options;					//an options string
	Handle ns;						//a namespace to register
	Handle xPath;					//the xpath
	Handle fileNameStr;				//the filename to load
	DOUBLE retval;					//retval
}XMLWaveXPathStruct, *XMLWaveXPathStructPtr;
#include "XOPStructureAlignmentReset.h"
int XMLWaveFmXPath(XMLWaveXPathStructPtr p);