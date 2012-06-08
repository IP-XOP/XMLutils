/*
 *  XMLparsedWaveFromXPath.cpp
 *  XMLutils
 *
 *  Created by andrew on 7/11/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

/*
 *  XMLstringFromXPath.cpp
 *  XMLutils
 *
 *  Created by andrew on 7/11/07.
 *	based on http://xmlsoft.org/examples/xpath1.c
 *
 */

#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h
#include "XMLutils.h"

#include "UTF8_multibyte_conv.h"
#include "StringTokenizer.h"
#include <vector>
#include <string>


static int
outputXPathObjIntoWave(xmlDoc *doc, xmlXPathObjectPtr xpathObj, char* options){
	int err = 0;
	int numNodes = 0;
	int j = 0;
	Handle transfer = NULL;

	xmlChar* xmloutputBuf = NULL;
	string data;
	vector<string> tokenizedOutput;
	
	//delimiter is going to be used in the string tokenizer
	//the default will be space.
	const char *delimiter = " ";
	
	//for know lets use the options string to pass in the delimiters
	if (strlen(options) > 0) {
		delimiter = options;
	}
	
	/* work out how many nodes in the nodeset from the Xpath object */
	numNodes = (xpathObj->nodesetval) ? xpathObj->nodesetval->nodeNr : 0;
	
	/* Wave for output of content*/
	waveHndl outputWav;
	char *waveName = "M_xmlcontent";
	int numDimensions;
	CountInt dimensionSizes[MAX_DIMENSIONS + 1];
	CountInt olddimensionSizes[MAX_DIMENSIONS + 1];
	CountInt indices[MAX_DIMENSIONS + 1];
	memset(indices, 0, sizeof(indices));
	memset(dimensionSizes, 0, sizeof(dimensionSizes));
	
	/* Wave for output of the node names associated with content */
	waveHndl NODEoutputWav;
	char *NODEwaveName = "W_xmlcontentnodes";
	CountInt NODEdimensionSizes[MAX_DIMENSIONS+1];
	CountInt NODEindices[MAX_DIMENSIONS+1];
	memset(NODEindices, 0, sizeof(NODEindices));
	memset(NODEdimensionSizes, 0, sizeof(NODEdimensionSizes));
	
	//make the output textwave	
	if(numNodes > 1)
		dimensionSizes[1] = (CountInt) numNodes;
	if(err = MDMakeWave(&outputWav, waveName, NULL, dimensionSizes, TEXT_WAVE_TYPE, 1))
		goto done;
		
	//make the nodenames textwave
	NODEdimensionSizes[0] = numNodes;
	if(err = MDMakeWave(&NODEoutputWav, NODEwaveName, NULL, NODEdimensionSizes, TEXT_WAVE_TYPE, 1))
		goto done;	

	/* Handle to transfer the string to the wave */
	transfer = NewHandle(0);
	if(transfer == NULL){
		err = NOMEM; goto done;
	}

	/* Now fill up the wave */
	/* Content from separate nodes is put in a separate column of M_xmlcontent */
	/* Associated Node Names are put in rows of W_xmlcontentnodes */
	for(j = 0; j < numNodes; ++j) {
		tokenizedOutput.clear();
		
		indices[1] = j;
		
		/* first put out the nodenames because that is easy */
		NODEindices[0] = j;

		xmloutputBuf = xmlGetNodePath(xpathObj->nodesetval->nodeTab[j]);
		
		data.assign((const char*) xmloutputBuf, xmlStrlen(xmloutputBuf) * sizeof(xmlChar));
		
		if(err = UTF8toSystemEncoding(data))
			goto done;
		
		if(xmloutputBuf != NULL){
			xmlFree(xmloutputBuf);
			xmloutputBuf = NULL;
		}
		
		if(err = PutCStringInHandle(data.c_str(), transfer))
			goto done;		
		if(err = MDSetTextWavePointValue(NODEoutputWav, NODEindices, transfer))
			goto done;
					
		/* get the current dimensions of the wave */
		if(err = MDGetWaveDimensions(outputWav, &numDimensions, olddimensionSizes))
			goto done;
		
		switch(xpathObj->nodesetval->nodeTab[j]->type){
			case(XML_ATTRIBUTE_NODE):
				xmloutputBuf =xmlNodeGetContent(xpathObj->nodesetval->nodeTab[j]->children);
				break;
			case(XML_TEXT_NODE):
				xmloutputBuf = xmlNodeGetContent(xpathObj->nodesetval->nodeTab[j]);
				break;
			default:
				xmloutputBuf = xmlNodeGetContent(xpathObj->nodesetval->nodeTab[j]);
				break;
		}

		data.assign((const char*) xmloutputBuf, xmlStrlen(xmloutputBuf) * sizeof(xmlChar));
		
		if(err = UTF8toSystemEncoding(data))
			goto done;
		
		if(xmloutputBuf != NULL){
			xmlFree(xmloutputBuf);
			xmloutputBuf = NULL;
		}
		
		Tokenize((const unsigned char*) data.data(), data.size(), tokenizedOutput, delimiter, strlen(delimiter));
		
		if(tokenizedOutput.size() > olddimensionSizes[0]){
			dimensionSizes[0] = tokenizedOutput.size();
			olddimensionSizes[0] = dimensionSizes[0];
			if(err = MDChangeWave2(outputWav, -1, dimensionSizes, 0))
				goto done;
		}
		
		indices[0] = 0;
		for( vector<string>::const_iterator iter = tokenizedOutput.begin();
				iter != tokenizedOutput.end();
					++iter ) {
			if(err = PutCStringInHandle((*iter).c_str(), transfer))
			   goto done;
			if(err = MDSetTextWavePointValue(outputWav, indices, transfer))
			   goto done;
			(indices[0])++;
		}		
				
    }
	
	WaveHandleModified(outputWav);
	
done:
	if(xmloutputBuf != NULL)
		xmlFree(xmloutputBuf);
	if (transfer != NULL)
		DisposeHandle(transfer);
		
	return err;
}

int
XMLWaveFmXPath(XMLWaveXPathStructPtr p){
	//the error code
	int err = 0;
	
	extern std::map<long,igorXMLfile> allXMLfiles;
	xmlXPathObjectPtr xpathObj = NULL; 
	xmlDocPtr doc = NULL;

	//the fileID, Xpath handle,namespace handle,options handle
	long fileID = -1;
	string xPath, ns, options;
	
	/* check if any of the argument string handles are null */
	if(p->xPath == NULL || p->ns == NULL || p->options == NULL){
		err = NULL_STRING_HANDLE;
		goto done;
	}
	
	xPath.append(*p->xPath, GetHandleSize(p->xPath));
	ns.append(*p->ns, GetHandleSize(p->ns));
	options.append(*p->options, GetHandleSize(p->options));
	
	if(err = SystemEncodingToUTF8(xPath))
		goto done;
	if(err = SystemEncodingToUTF8(ns))
		goto done;

	fileID = (long)roundf(p->fileID);	
	if((allXMLfiles.find(fileID) == allXMLfiles.end())){
		XOPNotice("XMLwavefmXPath: fileID isn't valid\r");
		err = FILEID_DOESNT_EXIST;
		goto done;
	} else {
		doc = (allXMLfiles[fileID].doc);
	}
 
	//execute Xpath expression
	//for some reason the xpathObj doesn't like being passed as a pointer argument, therefore return it as a result.
	xpathObj = execute_xpath_expression(doc, (xmlChar*) xPath.c_str(), (xmlChar*) ns.c_str(), &err); 
	if(err)
		goto done;

	if(err = outputXPathObjIntoWave(doc, xpathObj, (char*) options.c_str()))
		goto done;
		
done:
	(err == 0)? (p->retval = 0):(p->retval = -1);
		
	if(err == FILEID_DOESNT_EXIST ||
		err == XPATH_CONTEXT_CREATION_ERROR ||
		 err == FAILED_TO_REGISTER_NAMESPACE ||
		  err == XPATH_COMPILE_ERROR ||
		   err == UNABLE_TO_EVAL_XPATH_EXPR){
		err = 0;
	}
	if(xpathObj != NULL)
		xmlXPathFreeObject(xpathObj); 

	DisposeHandle(p->xPath);
	DisposeHandle(p->options);
	DisposeHandle(p->ns);
	
	return err;	
}