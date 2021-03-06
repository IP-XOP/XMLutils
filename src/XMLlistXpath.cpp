/*
 *  XMLlistXpath.cpp
 *  XMLutils
 *
 *  Created by andrew on 10/12/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "XMLutils.h"
#include <string>
using namespace std;

static int
fill_xpath_list(xmlNodeSet *nodesetval)
{
	int err = 0;
	xmlChar *path = NULL;
	
	int ii = 0;

	//textwave to put element list in
	waveHndl textWav = NULL;
	const char *textWavName = "M_listXPath";
	int overwrite = 1;		//wave will always be overwritten
	int type = TEXT_WAVE_TYPE;				//Xpaths will be text wave
	CountInt dimensionSizes[MAX_DIMENSIONS+1];
	CountInt indices[MAX_DIMENSIONS+1];
	int numDimensions = 0;
	int size = 0;
	string data;
	Handle pathName = NULL;

	memset(indices, 0, sizeof(indices));
	memset(dimensionSizes, 0, sizeof(dimensionSizes));
	pathName = WMNewHandle(0);
	if(pathName == NULL) {
		err = NOMEM;
		goto done;
	}
	
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
				
			data.assign((const char*) path, sizeof(xmlChar)* xmlStrlen(path));
			
			if(path != NULL){
				xmlFree(path);
				path = NULL;
			}
		
			if(err = PutCStringInHandle((char*)data.c_str(), pathName))
				goto done;
		
			indices[0] = dimensionSizes[0] - 1;
			indices[1] = 0;
			if(err = MDSetTextWavePointValue(textWav, indices, pathName))
				goto done;

			err = WMSetHandleSize(pathName, 0);
			if(err != 0)
				goto done;
					
			if(nodesetval->nodeTab[ii]->ns != NULL && nodesetval->nodeTab[ii]->ns->href != NULL){
				if(nodesetval->nodeTab[ii]->ns->prefix != NULL && xmlStrlen(nodesetval->nodeTab[ii]->ns->prefix) > 0){
					data.assign((const char*) nodesetval->nodeTab[ii]->ns->prefix, sizeof(xmlChar) * strlen((char*)nodesetval->nodeTab[ii]->ns->prefix));
					
					if(err = WMPtrAndHand((void*) data.data(), pathName, data.size()))
						goto done;
					if(err = WMPtrAndHand((char*)"=", pathName, sizeof(char)))
						goto done;
				}
				
				data.assign((const char*) nodesetval->nodeTab[ii]->ns->href, sizeof(xmlChar) * xmlStrlen(nodesetval->nodeTab[ii]->ns->href));
				
				if(err = WMPtrAndHand((void*) data.data(), pathName, data.size()))
					goto done;					
				indices[1] = 1;
				if(err = MDSetTextWavePointValue(textWav, indices, pathName))
					goto done;
			}
		data.assign((const char*) nodesetval->nodeTab[ii]->name, sizeof(xmlChar) * xmlStrlen(nodesetval->nodeTab[ii]->name));
		
			if(err = PutCStringInHandle((char*) data.c_str(), pathName))
				goto done;
			indices[1] = 2;
			if(err = MDSetTextWavePointValue(textWav, indices, pathName))
				goto done;

		}

done:
if(path != NULL)
	xmlFree(path);
if(pathName != NULL)
	WMDisposeHandle(pathName);	

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
	string xPath, ns;

	//the filename handle, Xpath handle,namespace handle,options handle	
	if(p->xPath == NULL || p->ns == NULL){
		err = NULL_STRING_HANDLE;
		goto done;
	}
	
	xPath.append(*p->xPath, WMGetHandleSize(p->xPath));
	ns.append(*p->ns, WMGetHandleSize(p->ns));
			  
	fileID = (long)roundf(p->fileID);	
	if((allXMLfiles.find(fileID) == allXMLfiles.end())){
		XOPNotice("XMLlistXpath: FileID isn't valid\r");
		err = FILEID_DOESNT_EXIST;
		goto done;
	} else {
		doc = (allXMLfiles[fileID].doc);
	}
	
	//execute Xpath expression
	xpathObj = execute_xpath_expression(doc, BAD_CAST xPath.c_str(), BAD_CAST ns.c_str(), &err);
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
		WMDisposeHandle(p->xPath);
	if(p->ns)
		WMDisposeHandle(p->ns);
		
	return err;	
}
