/*
XMLutils - an XOP designed to work with XML files
*/

#include "XMLutils.h"

std::map<long,igorXMLfile> allXMLfiles;

static XOPIORecResult
RegisterFunction()
{
	int funcIndex;

	funcIndex = (int) GetXOPItem(0);			// Which function invoked ?
	switch (funcIndex) {
		case 0:							
			return((XOPIORecResult)XMLelemlist);	// This function is called using the direct method.
			break;
		case 1:
			return((XOPIORecResult)XMLstrFmXPath);
			break;
		case 2:
			return((XOPIORecResult)XMLWaveFmXPath);
			break;
		case 3:
			return((XOPIORecResult)XMLsetNodeStr);
			break;
		case 4:
			return((XOPIORecResult)XMLlistAttr);
			break;
		case 5:
			return((XOPIORecResult)XMLsetAttr);
			break;
		case 6:
			return ((XOPIORecResult)XMLopenFile);
			break;
		case 7:
			return ((XOPIORecResult)XMLcloseFile);
			break;
		case 8:
			return ((XOPIORecResult)XMLSAVEFILE);
			break;
		case 9:
			return ((XOPIORecResult)XMLcreateFile);
			break;
		case 10:
			return ((XOPIORecResult)XMLaddNode);
			break;
		case 11:
			return ((XOPIORecResult)XMLdelNode);
			break;
		case 12:
			return ((XOPIORecResult)XMLlistXPath);
			break;
		case 13:
			return ((XOPIORecResult)XMLdocDump);
			break;
		case 14:
			return ((XOPIORecResult)XMLschemaValidate);
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
	XOPIORecResult result = 0;

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
HOST_IMPORT int main(IORecHandle ioRecHandle)
{	
	XOPInit(ioRecHandle);							// Do standard XOP initialization.
	SetXOPEntry(XOPEntry);							// Set entry point for future calls.
	
	if (igorVersion < 600){
		SetXOPResult(REQUIRES_IGOR_600);
		return EXIT_FAILURE;
	}
	else
		SetXOPResult(0L);
	
	return EXIT_SUCCESS;
}

#ifdef WINIGOR
double roundf(double val){
	double retval;
	if(val > 0){
	if(val - floor(val) < 0.5){
		retval = floor(val);
	} else {
		retval = ceil(val);
	}
	} else {
	if(val - floor(val) <= 0.5){
		retval = floor(val);
	} else {
		retval = ceil(val);
	}
}
	return retval;
}
#endif


