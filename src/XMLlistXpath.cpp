/*
 *  XMLlistXpath.cpp
 *  XMLutils
 *
 *  Created by andrew on 10/12/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "XMLutils.h"
#ifndef HAVE_MEMUTILS
#include "memutils.h"
#endif
#include "UTF8_multibyte_conv.h"

static int
fill_xpath_list(xmlNodeSet *nodesetval)
{
	int err = 0;
	xmlChar *path = NULL;
	
	int ii = 0;

	//textwave to put element list in
	waveHndl textWav = NULL;
	char *textWavName = "M_listXPath";
	int overwrite = 1;		//wave will always be overwritten
	int type = TEXT_WAVE_TYPE;				//Xpaths will be text wave
	long dimensionSizes[MAX_DIMENSIONS+1];
	long numDimensions = 0;
	long indices[MAX_DIMENSIONS+1];
	int size = 0;
	MemoryStruct data;
	Handle pathName = NULL;

	memset(indices, 0, sizeof(indices));
	memset(dimensionSizes, 0, sizeof(dimensionSizes));
	pathName = NewHandle(0);
	if(err = MemError())
		goto done;
	
	//now 2D
	dimensionSizes[1] = 3;
	
	//need to make a textwave to contain the elements
	if(err = MDMakeWave(&textWav, textWavName, NULL, dimensionSizes, type, overwrite))
		goto done;

	size = (nodesetval) ? nodesetval->nodeNr : 0;
 
	//XPathNames in 1st column
	//namespace  in 2nd column
	//nodeNames in 3rd column
					
    for (ii = 0; ii < size ; ii+=1) {
	
			path = ARJNxmlGetNodePath(nodesetval->nodeTab[ii]);
		
			if(err = MDGetWaveDimensions(textWav, &numDimensions, dimensionSizes))
				return err;
		
			dimensionSizes[0] = dimensionSizes[0] + 1; 
			dimensionSizes[1] = 3;
			
			if(err = MDChangeWave(textWav, -1, dimensionSizes))
				goto done;
				
			if(data.reset(path, sizeof(xmlChar), xmlStrlen(path)) == -1){
				err = NOMEM;
				goto done;
			}
			if(err = UTF8toSystemEncoding(&data))
				goto done;
			if(path != NULL){
				xmlFree(path);
				path = NULL;
			}
		
			if(err = PutCStringInHandle((char*)data.getData(), pathName))
				goto done;
		
			indices[0] = dimensionSizes[0] - 1;
			indices[1] = 0;
			if(err = MDSetTextWavePointValue(textWav, indices, pathName))
				goto done;

			SetHandleSize(pathName , 0);
			if(MemError())
				goto done;
					
			if(nodesetval->nodeTab[ii]->ns != NULL && nodesetval->nodeTab[ii]->ns->href != NULL){
				if(nodesetval->nodeTab[ii]->ns->prefix != NULL && xmlStrlen(nodesetval->nodeTab[ii]->ns->prefix) > 0){
					if(data.reset((void*) nodesetval->nodeTab[ii]->ns->prefix, sizeof(xmlChar), strlen((char*)nodesetval->nodeTab[ii]->ns->prefix)) == -1){
						err = NOMEM;
						goto done;
					}
					if(err = UTF8toSystemEncoding(&data))
						goto done;
					
					if(err = PtrAndHand((void*) data.getData(), pathName, sizeof(char) * strlen((char*)data.getData())))
						goto done;
					if(err = PtrAndHand((char*)"=", pathName, sizeof(char)))
						goto done;
				}
				if(data.reset((void*)nodesetval->nodeTab[ii]->ns->href, sizeof(xmlChar), xmlStrlen(nodesetval->nodeTab[ii]->ns->href)) == -1){
					err = NOMEM;
					goto done;
				}
				if(err = UTF8toSystemEncoding(&data))
					goto done;
				
				if(err = PtrAndHand((void*) data.getData(), pathName, sizeof(char) * strlen((char*)data.getData())))
					goto done;					
				indices[1] = 1;
				if(err = MDSetTextWavePointValue(textWav, indices, pathName))
					goto done;
			}
			
			if(data.reset((void*) nodesetval->nodeTab[ii]->name, sizeof(xmlChar), xmlStrlen(nodesetval->nodeTab[ii]->name)) == -1){
				err = NOMEM;
				goto done;
			}
			if(err = UTF8toSystemEncoding(&data))
				goto done;
		
			if(err = PutCStringInHandle((char*) data.getData(), pathName))
				goto done;
			indices[1] = 2;
			if(err = MDSetTextWavePointValue(textWav, indices, pathName))
				goto done;

		}

done:
if(path != NULL)
	xmlFree(path);
if(pathName != NULL)
	DisposeHandle(pathName);	

return err;
}

int
XMLlistXPath(XMLlistXpathStructPtr p){
	//the error code
	int err = 0;
	
	extern std::map<long,igorXMLfile> allXMLfiles;
	long fileID = -1;
	xmlXPathObject *xpathObj = NULL; 
	xmlDoc *doc = NULL;
	MemoryStruct xPath, ns;

	//the filename handle, Xpath handle,namespace handle,options handle	
	if(p->xPath == NULL || p->ns == NULL){
		err = NULL_STRING_HANDLE;
		goto done;
	}
	
	if(xPath.append(*p->xPath, GetHandleSize(p->xPath)) == -1){
		err = NOMEM;
		goto done;
	}
    if(ns.append(*p->ns, GetHandleSize(p->ns)) == -1){
		err = NOMEM;
		goto done;
	}
	
	if(err = SystemEncodingToUTF8(&xPath))
		goto done;
	if(err = SystemEncodingToUTF8(&ns))
		goto done;
			  
	fileID = (long)roundf(p->fileID);	
	if((allXMLfiles.find(fileID) == allXMLfiles.end())){
		XOPNotice("XMLlistXpath: FileID isn't valid\r");
		err = FILEID_DOESNT_EXIST;
		goto done;
	} else {
		doc = (allXMLfiles[fileID].doc);
	}
	
	//execute Xpath expression
	xpathObj = execute_xpath_expression(doc, BAD_CAST xPath.getData(), BAD_CAST ns.getData(), &err);
	if(err)
		goto done;
	//and print it out to a handle
	if(err = fill_xpath_list(xpathObj->nodesetval))
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
	if(p->xPath != NULL)
		DisposeHandle(p->xPath);
	if(p->ns)
		DisposeHandle(p->ns);
		
	return err;	
}
