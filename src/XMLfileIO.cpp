/*
 *  XMLfileIO.cpp
 *  XMLutils
 *
 *  Created by andrew on 15/11/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "XMLutils.h"

//a utility that converts Mac paths to UNIX paths
#ifdef _MACINTOSH_
int ConvertPath(const char * inPath, char * outPath, int outPathMaxLen) {
	
	CFStringRef inStr = CFStringCreateWithCString(kCFAllocatorDefault, inPath ,kCFStringEncodingMacRoman);
	if (inStr == NULL)
		return -1;
	CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, inStr, kCFURLHFSPathStyle,0);
	CFStringRef outStr = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
	if (!CFStringGetCString(outStr, outPath, outPathMaxLen, kCFURLPOSIXPathStyle))
		return -1;
	CFRelease(outStr);
	CFRelease(url);
	CFRelease(inStr);
	return 0;
} 
#endif

int
XMLopenFile(XMLopenFileStruct *p){
	int err = 0;
	int err2;
	XOP_FILE_REF fileRef = NULL;
	char fullFilePath [MAX_PATH_LEN+1], nativePath[MAX_PATH_LEN+1];
	char unixpath[MAX_PATH_LEN+1];
	
	const char* macdelim = ":";
	char* isMAC = NULL;
	
	igorXMLfile openfile;
	xmlDoc *doc = NULL;
	//	xmlValidCtxt *ctxt = NULL;
	extern std::map<long,igorXMLfile> allXMLfiles;	
	FILE *fp = NULL;
	
	
	if(err = GetCStringFromHandle(p->fullFilePath,fullFilePath,MAX_PATH_LEN))
		goto done;
	//get native filesystem filepath
	if (err = GetNativePath(fullFilePath,nativePath))
		goto done;
	
	//if the file doesn't point to anything return an error, this is so programmatic aborts are less likely to happen
	//	if(FullPathPointsToFile(nativePath) == 0){
	//		err = 0;
	//		p->retval = -2;
	//		XOPNotice("XMLopenfile: File(path) to open doesn't exist\r");
	//		goto done;
	//	}
	
	//convert native Mac path to UNIX
#ifdef _MACINTOSH_
	//see if its a MAC path by seeing if there is the Mac delimiter : in there
	if((isMAC = strstr(nativePath,macdelim)) && (err = HFSToPosixPath(nativePath,unixpath,0)))
		goto done;
	if(isMAC)
		strcpy(nativePath, unixpath);
#endif
	
	fp = fopen(nativePath,"r");
	if( fp ) {
		// exists
		fclose(fp);
	} else {
		err = 0;
		p->retval = -2;
		XOPNotice("XMLopenfile: File(path) to open doesn't exist, or file can't be opened\r");
		goto done;
		// doesnt exist, at least not for reading
	}
	
	/* Load XML document */
    doc = xmlParseFile(nativePath);
    if (doc == NULL) {
		err = 0;
		p->retval = -1;
		XOPNotice("XMLopenfile: XML file was not parseable\r");
		goto done;
    }
	
	//validate the document
	//	ctxt = xmlNewValidCtxt();
	//	if (ctxt == NULL) {
	//		err = XML_PARSERCTXT_ERROR;
	//		goto done;
	//	}
	//	if(xmlValidateDocument(ctxt,doc) != 1){
	//		err = XMLDOC_NOTVALIDATED;
	//		goto done;
	//	}
	//	if(xmlValidateDocumentFinal(ctxt,doc) != 1){
	//		err = XMLDOC_NOTVALIDATED;
	//		goto done;
	//	}
	
	
	if(err = XOPOpenFile(nativePath,0,&fileRef)){
		XOPNotice("XMLopenfile: Couldn't open file\r");
		goto done;
	}	
	
	strcpy(openfile.fileNameStr,nativePath);
	openfile.fileRef = fileRef;
	openfile.doc = doc;
	allXMLfiles[fileno(fileRef)] = openfile;
	p->retval = fileno(fileRef);
	
done:
	if(err)
		if(doc!= NULL)
			xmlFreeDoc(doc);
	if(err){
		if(fileRef != NULL)
			err2 = XOPCloseFile(fileRef);
	}
	if(err)
		p->retval = -2;
	
	//	if(ctxt != NULL)
	//		xmlFreeValidCtxt(ctxt);
	if(p->fullFilePath)
		DisposeHandle(p->fullFilePath);
	return err;
}


int
XMLcloseFile(XMLcloseFileStruct *p){
	int err = 0;
	long fileID;
	XOP_FILE_REF tmpfileref;
	extern std::map<long,igorXMLfile> allXMLfiles;
	std::map<long,igorXMLfile>::iterator allXMLfiles_iter;
	igorXMLfile tmp;
	
	fileID = (long)(roundf(p->fileID));
	
	xmlIndentTreeOutput = 1;
	
	switch(fileID){
		case -1:
			for(allXMLfiles_iter = allXMLfiles.begin() ; allXMLfiles_iter != allXMLfiles.end() ; allXMLfiles_iter ++){
				tmp = (*allXMLfiles_iter).second;
				
				if(p->toSave){
					rewind(allXMLfiles[fileID].fileRef);
					//					if(xmlDocFormatDump(tmp.fileRef,tmp.doc,1) == -1){
					//						err = XML_COULDNT_SAVE;
					//						goto done;
					//					}
					
					if(err = XOPCloseFile(tmp.fileRef))
						goto done;
					
					if(xmlSaveFormatFileEnc(tmp.fileNameStr , tmp.doc , NULL , 1) == -1){
						err = XML_COULDNT_SAVE;
						goto done;
					}
					if(tmp.doc)
						xmlFreeDoc((tmp.doc));
					allXMLfiles.erase(allXMLfiles_iter->first);
				} else {
					if(tmp.doc)
						xmlFreeDoc((tmp.doc));
					err = XOPCloseFile(tmp.fileRef);
					allXMLfiles.erase(allXMLfiles_iter->first);
				}
			}
			break;
		default:
			if(allXMLfiles.find(fileID) != allXMLfiles.end()){
				if(p->toSave){
					tmp = allXMLfiles[fileID];
					
					rewind(allXMLfiles[fileID].fileRef);
					//					if(xmlDocFormatDump(tmp.fileRef,tmp.doc,0) == -1){
					//						err = XML_COULDNT_SAVE;
					//						goto done;
					//					}
					
					if(err = XOPCloseFile((allXMLfiles[fileID].fileRef)))
						goto done;
					if(xmlSaveFormatFileEnc(tmp.fileNameStr , tmp.doc , NULL , 1) == -1){
						err = XML_COULDNT_SAVE;
						goto done;
					}
					xmlFreeDoc( (allXMLfiles[fileID].doc) );
					allXMLfiles.erase(fileID);
				} else {
					xmlFreeDoc((allXMLfiles[fileID].doc));
					tmpfileref = allXMLfiles[fileID].fileRef;
					if(err = XOPCloseFile(tmpfileref));
					allXMLfiles.erase(fileID);
				}
			}
	}
done:
	(err == 0)? (p->retval = 0):(p->retval = -1);
	
	return err;
}

//in CAPS because there is an identical command fm libxml
int
XMLSAVEFILE(XMLfileSaveStruct *p){
	int err = 0;
	extern std::map<long,igorXMLfile> allXMLfiles;
	long fileID = -1;
	xmlDoc *doc = NULL;
	
	char* fileName;
	
	fileID = (long)roundf(p->fileID);	
	if((allXMLfiles.find(fileID) == allXMLfiles.end())){
		XOPNotice("XMLsavefile: fileID doesn't exist\r");
		err = FILEID_DOESNT_EXIST;
		goto done;
	} else {
		doc = (allXMLfiles[p->fileID].doc);
		fileName = (char*) allXMLfiles[p->fileID].fileNameStr;
	}
	
	xmlIndentTreeOutput = 1;
	
	rewind(allXMLfiles[fileID].fileRef);
	//	if(xmlDocFormatDump(allXMLfiles[fileID].fileRef,allXMLfiles[fileID].doc,1) == -1){
	//		err = XML_COULDNT_SAVE;
	//		goto done;
	//	}
	
	if(xmlSaveFormatFileEnc(allXMLfiles[fileID].fileNameStr , doc , NULL , 1) == -1){
		//	if(xmlSaveFile(allXMLfiles[p->fileID].fileNameStr , doc) == -1){
		err = XML_COULDNT_SAVE;
		goto done;
	}
	
done:
	(err == 0)? (p->retval = 0):(p->retval = -1);
	return err;
}

int
XMLcreateFile(XMLcreateFileStruct *p){
	int err = 0;
	
	XOP_FILE_REF fileRef = NULL;
	char fullFilePath [MAX_PATH_LEN+1], nativePath[MAX_PATH_LEN+1],unixpath[MAX_PATH_LEN+1];
	char *macdelim = ":";
	char *isMAC = NULL;
	
	igorXMLfile openfile;
	xmlDoc *doc = NULL;
	xmlNode *root_element= NULL ;
	xmlNs *nspace = NULL;
	char *rootname   = NULL;
	char *ns = NULL;
	char *prefix = NULL;
	
	extern std::map<long,igorXMLfile> allXMLfiles;
	
	p->fileID = -1;
	
	if(p->rootelement == NULL || p->ns == NULL){
		err = NULL_STRING_HANDLE;
		goto done;
	}
	
	//allocate space for the C-strings.
	rootname = (char*)malloc((GetHandleSize(p->rootelement)+1)*sizeof(char));
	if(rootname == NULL){
		err = NOMEM;goto done;
	}
	ns = (char*)malloc((GetHandleSize(p->ns)+1)*sizeof(char));
	if(ns == NULL){
		err = NOMEM;goto done;
	}
	prefix = (char*)malloc((GetHandleSize(p->prefix)+1)*sizeof(char));
	if(prefix == NULL){
		err = NOMEM;goto done;
	}
	
	/* get all of the igor input strings into C-strings */
	if (err = GetCStringFromHandle(p->rootelement,rootname,GetHandleSize(p->rootelement)))
		goto done;
	if (err = GetCStringFromHandle(p->ns, ns, GetHandleSize(p->ns)))
		goto done;
	if (err = GetCStringFromHandle(p->prefix, prefix, GetHandleSize(p->prefix)))
		goto done;
	if(err = GetCStringFromHandle(p->fileName,fullFilePath,MAX_PATH_LEN))
		goto done;
	//get native filesystem filepath
	if (err = GetNativePath(fullFilePath,nativePath))
		goto done;
	
	//convert native Mac path to UNIX
#ifdef _MACINTOSH_
	//see if its a MAC path by seeing if there is the Mac delimiter : in there
	if((isMAC = strstr(nativePath,macdelim)) && (err = HFSToPosixPath(nativePath,unixpath,0)))
		goto done;
	if(isMAC)
		strcpy(nativePath, unixpath);
#endif
	
	//create a new XML document
	doc = xmlNewDoc(BAD_CAST "1.0");
	if(doc == NULL){
		err = COULDNT_CREATE_XMLDOC;
		goto done;
	}
	
	//check if the node name is invalid
	if(xmlValidateName(BAD_CAST rootname , 0) != 0){
		err = INVALID_NODE_NAME;
		goto done;
	}
	
	
	//create the root element
	root_element=xmlNewNode(NULL , BAD_CAST rootname);
	if(root_element == NULL){
		err = COULDNT_CREATE_NODE;
		goto done;
	}
	
	if(strlen(prefix)>0){
		nspace = xmlNewNs(root_element, BAD_CAST ns, BAD_CAST prefix );
		root_element->ns = nspace;
	} else {
		nspace = xmlNewNs(root_element, BAD_CAST ns, NULL );
	}
	
	root_element = xmlDocSetRootElement(doc,root_element);
	root_element = xmlDocGetRootElement(doc);
	if(root_element == NULL){
		err = COULDNT_CREATE_NODE;
		goto done;
	}
	
	if(err = XOPOpenFile(nativePath,1,&fileRef))
		goto done;
	
	strcpy(openfile.fileNameStr,nativePath);
	openfile.doc = doc;
	openfile.fileRef = fileRef;
	
	allXMLfiles[fileno(fileRef)] = openfile;
	p->fileID = fileno(fileRef);
	
done:		
	if(err){
		p->fileID = -1;
		if(doc != NULL)
			xmlFreeDoc(doc);
		if(nspace != NULL)
			xmlFreeNs(nspace);
	}
	if(rootname != NULL)
		free(rootname);
	if(ns != NULL)
		free(ns);
	if(prefix != NULL)
		free(prefix);
	DisposeHandle(p->prefix);
	DisposeHandle(p->fileName);
	DisposeHandle(p->rootelement);
	DisposeHandle(p->ns);
	return err;
}