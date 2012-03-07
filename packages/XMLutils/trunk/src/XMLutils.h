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

#define LIBXML_STATIC

/**
*TO DO:
*Get rid of this bodge in the XPath when libxml2 is compileable again
*/
#ifdef WINIGOR
#define snprintf _snprintf
#endif
xmlChar* ARJNxmlGetNodePath(xmlNodePtr node);


/* A structure to hold all the file IO information */
typedef struct igorXMLfile {
	XOP_FILE_REF fileRef;
	char fileNameStr[MAX_PATH_LEN + 1];
	xmlDoc *doc;
}igorXMLfile, *igorXMLfilePtr;

#ifdef WINIGOR
	double roundf(double val);
#endif

/*
in XMLelementlist.cpp
*/
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
typedef struct XMLelemlistStruct {
	double fileID;
	double retval;
}XMLelemlistStruct, *XMLelemlistStructPtr;
#pragma pack()
int XMLelemlist(XMLelemlistStructPtr p);

/* in XMLevaluateXPathExpression.cpp */
xmlXPathObject* execute_xpath_expression(xmlDoc* doc, xmlChar* xpathExpr, xmlChar* nsList,  int* err);

/*
in XMLstrFmXPath.cpp
*/
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
typedef struct XMLstrFmXpathStruct {
	Handle options;					//an options string
	Handle ns;						//a namespace to register
	Handle xPath;					//the xpath
	double fileID;					//the fileID to load
	Handle returnString;			//the string containing the content
}XMLstrFmXpathStruct, *XMLstrFmXpathStructPtr;
#pragma pack()
int XMLstrFmXPath(XMLstrFmXpathStructPtr p);

/*
in XMLparsedWaveFromXPath.cpp
*/ 

#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
typedef struct XMLWaveXPathStruct {
	Handle options;					//an options string
	Handle ns;						//a namespace to register
	Handle xPath;					//the xpath
	double fileID;				//the filename to load
	double retval;					//retval
}XMLWaveXPathStruct, *XMLWaveXPathStructPtr;
#pragma pack()
int XMLWaveFmXPath(XMLWaveXPathStructPtr);

/* in XMLsetNodeStr */
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
typedef struct XMLsetNodeStrStruct {
	Handle content;					//an options string
	Handle ns;						//a namespace to register
	Handle xPath;					//the xpath
	double fileID;					//fileID
	double retval;					//retval
}XMLsetNodeStrStruct, *XMLsetNodeStrStructPtr;
#pragma pack()
int XMLsetNodeStr(XMLsetNodeStrStructPtr p);

/* in XMLlistAttr */
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
typedef struct XMLlistAttrStruct {
	Handle ns;						//a namespace to register
	Handle xPath;					//the xpath
	double fileID;					//fileID
	double retval;					//retval
}XMLlistAttrStruct, *XMLlistAttrStructPtr;
#pragma pack()
int XMLlistAttr(XMLlistAttrStructPtr p);

/* in XMLsetAttr */
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
typedef struct XMLsetAttrStruct {
	Handle val;
	Handle attribute;
	Handle ns;						//a namespace to register
	Handle xPath;					//the xpath
	double	fileID;					//fileID
	double retval;					//retval
}XMLsetAttrStruct, *XMLsetAttrStructPtr;
#pragma pack()
int XMLsetAttr(XMLsetAttrStructPtr p);

/* in XMLfileIO.cpp */
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
typedef struct XMLopenFileStruct {
	Handle fullFilePath;
	double retval;
}XMLopenFileStruct, *XMLopenFileStructPtr;
#pragma pack()
int XMLopenFile(XMLopenFileStruct *p);

#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
typedef struct XMLcloseFileStruct {
	double toSave;
	double fileID;
	double retval;
}XMLcloseFileStruct, *XMLcloseFileStructPtr;
#pragma pack()
int XMLcloseFile(XMLcloseFileStruct *p);

#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
typedef struct XMLfileSaveStruct {
	double fileID;
	double retval;
}XMLfileSaveStruct, *XMLfileSaveStructPtr;
#pragma pack()
int XMLSAVEFILE(XMLfileSaveStruct *p);

#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
typedef struct XMLcreateFileStruct {
	Handle prefix;
	Handle ns;
	Handle rootelement;
	Handle fileName;
	double fileID;
}XMLcreateFileStruct, *XMLcreateFileStructPtr;
#pragma pack()
int XMLcreateFile(XMLcreateFileStruct *p);

/* in XMLchangeNodeStructure.cpp */
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
typedef struct XMLaddNodeStruct {
	double nodeType;
	Handle content;
	Handle nodeName;
	Handle ns;
	Handle xPath;
	double fileID;
	double retval;
}XMLaddNodeStruct, *XMLaddNodeStructPtr;
#pragma pack()
int XMLaddNode(XMLaddNodeStruct *);

#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
typedef struct XMLdelNodeStruct {
	Handle ns;
	Handle xPath;
	double fileID;
	double retval;
}XMLdelNodeStruct, *XMLdelNodeStructPtr;
#pragma pack()
int XMLdelNode(XMLdelNodeStruct *);

/*
in XMLlistXPath.cpp
*/
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
typedef struct XMLlistXpathStruct {
	Handle ns;						//a namespace to register
	Handle xPath;					//the xpath
	double fileID;					//the fileID to load
	double retval;					//return value
}XMLlistXpathStruct, *XMLlistXpathStructPtr;
#pragma pack()
int XMLlistXPath(XMLlistXpathStructPtr p);

/*
in XMLdumpDoc.cpp
*/
#pragma pack(2)	// All structures passed to Igor are two-byte aligned.
typedef struct XMLdocDumpStruct {
	double fileID;
	double retval;
}XMLdocDumpStruct, *XMLdocDumpStructPtr;
#pragma pack()
int XMLdocDump(XMLdocDumpStruct *p);

/*
 in XMLschemaValidate.cpp
 */
#pragma pack(2)	
typedef struct XMLschemaValidateStruct {
	Handle schemaFileName;
	double fileID;
	double retval;
}XMLschemaValidateStruct, *XMLschemaValidateStructPtr;
#pragma pack()
int XMLschemaValidate(XMLschemaValidateStruct *p);
