/*
XMLutils - an XOP designed to work with XML files
*/

#include "XMLutils.h"

std::map<long,igorXMLfile> allXMLfiles;

#ifdef _MACINTOSH_
HOST_IMPORT int main(IORecHandle ioRecHandle);
#endif	
#ifdef _WINDOWS_
HOST_IMPORT void main(IORecHandle ioRecHandle);
#endif

static long
RegisterFunction()
{
	int funcIndex;

	funcIndex = GetXOPItem(0);			// Which function invoked ?
	switch (funcIndex) {
		case 0:							
			return((long)XMLelemlist);	// This function is called using the direct method.
			break;
		case 1:
			return((long)XMLstrFmXPath);
			break;
		case 2:
			return((long)XMLWaveFmXPath);
			break;
		case 3:
			return((long)XMLsetNodeStr);
			break;
		case 4:
			return((long)XMLlistAttr);
			break;
		case 5:
			return((long)XMLsetAttr);
			break;
		case 6:
			return ((long)XMLopenFile);
			break;
		case 7:
			return ((long)XMLcloseFile);
			break;
		case 8:
			return ((long)XMLSAVEFILE);
			break;
		case 9:
			return ((long)XMLcreateFile);
			break;
		case 10:
			return ((long)XMLaddNode);
			break;
		case 11:
			return ((long)XMLdelNode);
			break;
		case 12:
			return ((long)XMLlistXPath);
			break;
		case 13:
			return ((long)XMLdocDump);
			break;
	}
	return NIL;
}

/*	XOPEntry()

	This is the entry point from the host application to the XOP for all
	messages after the INIT message.
*/
static void
XOPEntry(void)
{	
	long result = 0;
	XMLcloseFileStruct p;
	p.fileID = -1;
	p.toSave = 0;

	switch (GetXOPMessage()) {
		case NEW:
			xmlInitParser();
			result = XMLcloseFile(&p);
			allXMLfiles.clear();
			break;
		case CLEANUP:
			result = XMLcloseFile(&p);
			allXMLfiles.clear();
			xmlCleanupParser();
			break;
		case FUNCADDRS:
			result = RegisterFunction();	// This tells Igor the address of our function.
			break;
	}
	SetXOPResult(result);
}

/*	main(ioRecHandle)

	This is the initial entry point at which the host application calls XOP.
	The message sent by the host must be INIT.
	main() does any necessary initialization and then sets the XOPEntry field of the
	ioRecHandle to the address to be called for future messages.
*/
#ifdef _MACINTOSH_
HOST_IMPORT int main(IORecHandle ioRecHandle)
#endif	
#ifdef _WINDOWS_
HOST_IMPORT void main(IORecHandle ioRecHandle)
#endif
{	
	XOPInit(ioRecHandle);							// Do standard XOP initialization.
	SetXOPEntry(XOPEntry);							// Set entry point for future calls.
	
	if (igorVersion < 504)
		SetXOPResult(REQUIRES_IGOR_504);
	else
		SetXOPResult(0L);
}

#ifdef _WINDOWS_
double roundf(double val){
	double retval;
	if(val>0){
	if(val-floor(val) < 0.5){
		retval = floor(val);
	} else {
		retval = ceil(val);
	}
	} else {
	if(val-floor(val) <= 0.5){
		retval = floor(val);
	} else {
		retval = ceil(val);
	}
}
	return retval;
}
#endif


