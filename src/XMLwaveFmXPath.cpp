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

#include "StringTokenizer.h"
#include <vector>
#include <string>
#include <sstream>
#include <iterator>

static int
outputXPathObjIntoWave(xmlDoc *doc, xmlXPathObjectPtr xpathObj, char* options){
	int err = 0;
	int numNodes = 0;
	CountInt j = 0, i=0, k=0, numTokens=0;
	Handle transfer = NULL;

	xmlChar* xmloutputBuf = NULL;
	string data;
	vector<string> *tokenizedOutput;
	vector<PSInt> *tokenSizes;
	size_t *szTotalTokens;
	PSInt maxNumTokens = 0;

	IndexInt *pTableOffset;
	char *pTextData;
	size_t totalBytes = 0;
	Handle textDataH = NULL;
	stringstream ss;
	
	
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
	const char *waveName = "M_xmlcontent";
	CountInt dimensionSizes[MAX_DIMENSIONS + 1];
	memset(dimensionSizes, 0, sizeof(dimensionSizes));
	
	/* Wave for output of the node names associated with content */
	waveHndl NODEoutputWav;
	const char *NODEwaveName = "W_xmlcontentnodes";
	CountInt NODEdimensionSizes[MAX_DIMENSIONS+1];
	CountInt NODEindices[MAX_DIMENSIONS+1];
	memset(NODEindices, 0, sizeof(NODEindices));
	memset(NODEdimensionSizes, 0, sizeof(NODEdimensionSizes));

		
	//make the nodenames textwave
	NODEdimensionSizes[0] = numNodes;
	if(err = MDMakeWave(&NODEoutputWav, NODEwaveName, NULL, NODEdimensionSizes, TEXT_WAVE_TYPE, 1))
		goto done;	

	/* Handle to transfer the string to the wave */
	transfer = WMNewHandle(0);
	if(transfer == NULL){
		err = NOMEM; goto done;
	}
	
	/* Associated Node Names are put in rows of W_xmlcontentnodes */
	for(j = 0; j < numNodes; ++j) {
		/* first put out the nodenames because that is easy */
		NODEindices[0] = j;		
		xmloutputBuf = xmlGetNodePath(xpathObj->nodesetval->nodeTab[j]);
		data.assign((const char*) xmloutputBuf, xmlStrlen(xmloutputBuf) * sizeof(xmlChar));
				
		if(xmloutputBuf != NULL){
			xmlFree(xmloutputBuf);
			xmloutputBuf = NULL;
		}
		
		if(err = PutCStringInHandle(data.c_str(), transfer))
			goto done;		
		if(err = MDSetTextWavePointValue(NODEoutputWav, NODEindices, transfer))
			goto done;
	}
	

	/* Now fill up the output waves */
	/* Content from separate nodes is put in a separate column of M_xmlcontent */
	tokenizedOutput = new (nothrow) vector<string>[numNodes];
	if(!tokenizedOutput){
		err = NOMEM;
		goto done;
	}
	tokenSizes = new (nothrow) vector<PSInt>[numNodes];
	if(!tokenSizes){
		err = NOMEM;
		goto done;
	}
	szTotalTokens = new (nothrow) size_t[numNodes];
	if(!szTotalTokens){
		err = NOMEM;
		goto done;
	}
	
	//tokenize all the data
	for(j = 0 ; j < numNodes ; ++j){
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
				
		if(xmloutputBuf != NULL){
			xmlFree(xmloutputBuf);
			xmloutputBuf = NULL;
		}
		Tokenize((const unsigned char*) data.data(), data.size(), tokenizedOutput[j], tokenSizes[j], &szTotalTokens[j], delimiter, strlen(delimiter));
		if(tokenizedOutput[j].size() > maxNumTokens)
			maxNumTokens = tokenizedOutput[j].size();
		
		totalBytes += szTotalTokens[j];
	}
	
	dimensionSizes[0] = (CountInt) maxNumTokens;
	dimensionSizes[1] = (CountInt) numNodes;
	dimensionSizes[2] = 0;

	//make the output textwave	
	if(err = MDMakeWave(&outputWav, waveName, NULL, dimensionSizes, TEXT_WAVE_TYPE, 1))
		goto done;
		
	if(err = GetTextWaveData(outputWav, 2, &textDataH))
		goto done;
	
	err = WMSetHandleSize(textDataH, totalBytes + (maxNumTokens * numNodes + 1) * sizeof(PSInt));
	if(err != 0)
		goto done;
	
	/*
	 copy the text data into the output wave
	 */
	pTextData = *textDataH + (maxNumTokens * numNodes + 1) * sizeof(PSInt);
	//first concatenate all the data
	for(j = 0 ; j < numNodes ; ++j)
		std::copy(tokenizedOutput[j].begin(), tokenizedOutput[j].end(), ostream_iterator<string>(ss));
	//then copy it into the handle
	memcpy(pTextData, ss.str().data(), totalBytes);

	/*
	 then do the offsets
	 */
	pTableOffset = (PSInt*)*textDataH;					// Pointer to table of offsets if mode==1 or mode==2
	for(j = 0, i = 0, k = 0 ; j < numNodes ; j++){
		numTokens = tokenizedOutput[j].size();
		for(i = 0 ; i < maxNumTokens ; i++){
			if(i < numTokens){
				pTableOffset[k + 1] = pTableOffset[k] + tokenSizes[j].at(i);
			} else {
				pTableOffset[k + 1] = pTableOffset[k];
			}
			k++;
		}
	}
	
	if(err = SetTextWaveData(outputWav, 2, textDataH))
		goto done;
			
	WaveHandleModified(outputWav);
	
done:
	if(tokenizedOutput)
		delete[] tokenizedOutput;
	if(tokenSizes)
		delete[] tokenSizes;
	if(szTotalTokens)
		delete[] szTotalTokens;
	if(textDataH)
		WMDisposeHandle(textDataH);
	if(xmloutputBuf != NULL)
		xmlFree(xmloutputBuf);
	if (transfer != NULL)
		WMDisposeHandle(transfer);
		
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
	
	xPath.append(*p->xPath, WMGetHandleSize(p->xPath));
	ns.append(*p->ns, WMGetHandleSize(p->ns));
	options.append(*p->options, WMGetHandleSize(p->options));

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

	WMDisposeHandle(p->xPath);
	WMDisposeHandle(p->options);
	WMDisposeHandle(p->ns);
	
	return err;	
}
