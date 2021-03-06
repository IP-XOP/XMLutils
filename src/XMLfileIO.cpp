/*
 *  XMLfileIO.cpp
 *  XMLutils
 *
 *  Created by andrew on 15/11/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "XMLutils.h"
#include <string>
using namespace std;

/*	VC 2015 warns about using fileno and says to use _fileno.
	But Xcode 9 does not recognize _fileno. This ifdef works around that.
*/
#ifdef MACIGOR
	#define FILENO fileno
#else
	#define FILENO _fileno
#endif


int
XMLopenFile(XMLopenFileStruct *p){
	int err = 0;
	int err2;
	XOP_FILE_REF fileRef = NULL;
	char fullFilePath [MAX_PATH_LEN+1], nativePath[MAX_PATH_LEN+1];
	
	const char* macdelim = ":";
	char* isMAC = NULL;
	
	igorXMLfile openfile;
	xmlDoc *doc = NULL;
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
#ifdef MACIGOR
	//see if its a MAC path by seeing if there is the Mac delimiter : in there
	char unixpath[MAX_PATH_LEN+1];
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
    doc = xmlReadFile(nativePath, NULL, XML_PARSE_NOENT);
    if (doc == NULL) {
		err = 0;
		p->retval = -1;
		XOPNotice("XMLopenfile: XML file was not parseable\r");
		goto done;
    }
	
	
	if(err = XOPOpenFile(nativePath,0,&fileRef)){
		XOPNotice("XMLopenfile: Couldn't open file\r");
		goto done;
	}	
	
	strcpy(openfile.fileNameStr,nativePath);
	openfile.fileRef = fileRef;
	openfile.doc = doc;
	allXMLfiles[FILENO(fileRef)] = openfile;
	p->retval = FILENO(fileRef);
	
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
	
	if(p->fullFilePath)
		WMDisposeHandle(p->fullFilePath);
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
				} else {
					if(tmp.doc)
						xmlFreeDoc((tmp.doc));
					err = XOPCloseFile(tmp.fileRef);
				}
			}
            allXMLfiles.clear();
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
					err = XOPCloseFile(tmpfileref);
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
		doc = (allXMLfiles[(long) p->fileID].doc);
		fileName = (char*) allXMLfiles[(long) p->fileID].fileNameStr;
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
	char fullFilePath [MAX_PATH_LEN + 1], nativePath[MAX_PATH_LEN + 1];
	const char *macdelim = ":";
	char *isMAC = NULL;
	
	igorXMLfile openfile;
	xmlDoc *doc = NULL;
	xmlNode *root_element= NULL ;
	xmlNs *nspace = NULL;
	string rootname, ns, prefix;
		
	extern std::map<long,igorXMLfile> allXMLfiles;
	
	p->fileID = -1;
	
	if(p->rootelement == NULL || p->ns == NULL){
		err = NULL_STRING_HANDLE;
		goto done;
	}
	
	//allocate space for the C-strings.
	rootname.append(*p->rootelement, WMGetHandleSize(p->rootelement));
	ns.append(*p->ns, WMGetHandleSize(p->ns));
	prefix.append(*p->prefix, WMGetHandleSize(p->prefix));
		
	if(err = GetCStringFromHandle(p->fileName, fullFilePath, MAX_PATH_LEN))
		goto done;
	//get native filesystem filepath
	if (err = GetNativePath(fullFilePath, nativePath))
		goto done;
	
	//convert native Mac path to UNIX
#ifdef MACIGOR
	//see if its a MAC path by seeing if there is the Mac delimiter : in there
	char unixpath[MAX_PATH_LEN + 1];
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
	if(xmlValidateName(BAD_CAST rootname.c_str() , 0) != 0){
		err = INVALID_NODE_NAME;
		goto done;
	}
	
	//create the root element
	root_element = xmlNewNode(NULL , BAD_CAST rootname.c_str());
	if(root_element == NULL){
		err = COULDNT_CREATE_NODE;
		goto done;
	}
	
	if(prefix.size() && strlen((const char*) prefix.c_str()) > 0){
		nspace = xmlNewNs(root_element, BAD_CAST ns.c_str(), BAD_CAST prefix.c_str() );
		root_element->ns = nspace;
	} else if (ns.size() && strlen((const char*) ns.c_str())) {
		nspace = xmlNewNs(root_element, BAD_CAST ns.c_str(), NULL );
	}
	
	root_element = xmlDocSetRootElement(doc, root_element);
	root_element = xmlDocGetRootElement(doc);
	if(root_element == NULL){
		err = COULDNT_CREATE_NODE;
		goto done;
	}
	
	if(err = XOPOpenFile(nativePath, 1, &fileRef))
		goto done;
	
	strcpy(openfile.fileNameStr, nativePath);
	openfile.doc = doc;
	openfile.fileRef = fileRef;
	
	allXMLfiles[FILENO(fileRef)] = openfile;
	p->fileID = FILENO(fileRef);
	
done:		
	if(err){
		p->fileID = -1;
        // following two lines commented out because nspace should
        // be freed by freeing the document.
//        if(nspace != NULL)
//            xmlFreeNs(nspace);
        if(doc != NULL)
			xmlFreeDoc(doc);

	}
	WMDisposeHandle(p->prefix);
	WMDisposeHandle(p->fileName);
	WMDisposeHandle(p->rootelement);
	WMDisposeHandle(p->ns);
	return err;
}
