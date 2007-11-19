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
#include <libxml/xmlstring.h>
#include <libxml/xpathInternals.h>
#include <map>

/* A structure to hold all the file IO information */
typedef struct igorXMLfile {
	XOP_FILE_REF fileRef;
	char fileNameStr[MAX_PATH_LEN + 1];
	xmlDoc *doc;
}igorXMLfile, *igorXMLfilePtr;

#ifdef _WINDOWS_
	double roundf(double val);
#endif

/*
in XMLelementlist.cpp
*/
#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct XMLelemlistStruct {
	DOUBLE fileID;
	DOUBLE retval;
}XMLelemlistStruct, *XMLelemlistStructPtr;
#include "XOPStructureAlignmentReset.h"
int XMLelemlist(XMLelemlistStructPtr p);

/* in XMLevaluateXPathExpression.cpp */
xmlXPathObject* execute_xpath_expression(xmlDoc* doc, xmlChar* xpathExpr, xmlChar* nsList,  int* err);

/*
in XMLstrFmXPath.cpp
*/
#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct XMLstrFmXpathStruct {
	Handle options;					//an options string
	Handle ns;						//a namespace to register
	Handle xPath;					//the xpath
	DOUBLE fileID;					//the fileID to load
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
	DOUBLE fileID;				//the filename to load
	DOUBLE retval;					//retval
}XMLWaveXPathStruct, *XMLWaveXPathStructPtr;
#include "XOPStructureAlignmentReset.h"
int XMLWaveFmXPath(XMLWaveXPathStructPtr);

/* in XMLsetNodeStr */
#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct XMLsetNodeStrStruct {
	Handle content;					//an options string
	Handle ns;						//a namespace to register
	Handle xPath;					//the xpath
	DOUBLE fileID;					//fileID
	DOUBLE retval;					//retval
}XMLsetNodeStrStruct, *XMLsetNodeStrStructPtr;
#include "XOPStructureAlignmentReset.h"
int XMLsetNodeStr(XMLsetNodeStrStructPtr p);

/* in XMLlistAttr */
#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct XMLlistAttrStruct {
	Handle ns;						//a namespace to register
	Handle xPath;					//the xpath
	DOUBLE fileID;					//fileID
	DOUBLE returnval;					//retval
}XMLlistAttrStruct, *XMLlistAttrStructPtr;
#include "XOPStructureAlignmentReset.h"
int XMLlistAttr(XMLlistAttrStructPtr p);

/* in XMLsetAttr */
#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct XMLsetAttrStruct {
	Handle val;
	Handle attribute;
	Handle ns;						//a namespace to register
	Handle xPath;					//the xpath
	DOUBLE fileID;					//fileID
	DOUBLE returnval;					//retval
}XMLsetAttrStruct, *XMLsetAttrStructPtr;
#include "XOPStructureAlignmentReset.h"
int XMLsetAttr(XMLsetAttrStructPtr p);

/* in XMLfileIO.cpp */
#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct XMLopenFileStruct {
	Handle fullFilePath;
	DOUBLE retval;
}XMLopenFileStruct, *XMLopenFileStructPtr;
#include "XOPStructureAlignmentReset.h"
int XMLopenFile(XMLopenFileStruct *p);

#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct XMLcloseFileStruct {
	DOUBLE toSave;
	DOUBLE fileID;
	DOUBLE retval;
}XMLcloseFileStruct, *XMLcloseFileStructPtr;
#include "XOPStructureAlignmentReset.h"
int XMLcloseFile(XMLcloseFileStruct *p);

#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct XMLfileSaveStruct {
	DOUBLE fileID;
	DOUBLE retval;
}XMLfileSaveStruct, *XMLfileSaveStructPtr;
#include "XOPStructureAlignmentReset.h"
int XMLSAVEFILE(XMLfileSaveStruct *p);

#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct XMLcreateFileStruct {
	Handle ns;
	Handle rootelement;
	Handle fileName;
	DOUBLE fileID;
}XMLcreateFileStruct, *XMLcreateFileStructPtr;
#include "XOPStructureAlignmentReset.h"
int XMLcreateFile(XMLcreateFileStruct *p);

/* in XMLchangeNodeStructure.cpp */
#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct XMLaddNodeStruct {
	DOUBLE nodeType;
	Handle nodeName;
	Handle ns;
	Handle xPath;
	DOUBLE fileID;
	DOUBLE retval;
}XMLaddNodeStruct, *XMLaddNodeStructPtr;
#include "XOPStructureAlignmentReset.h"
int XMLaddNode(XMLaddNodeStruct*);

#include "XOPStructureAlignmentTwoByte.h"	// All structures passed to Igor are two-byte aligned.
typedef struct XMLdelNodeStruct {
	Handle ns;
	Handle xPath;
	DOUBLE fileID;
	DOUBLE retval;
}XMLdelNodeStruct, *XMLdelNodeStructPtr;
#include "XOPStructureAlignmentReset.h"
int XMLdelNode(XMLdelNodeStruct*);