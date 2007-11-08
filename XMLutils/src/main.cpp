/*
XMLutils - an XOP designed to work with XML files
*/

#include "XMLutils.h"


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
		case 0:							// y = Abeles(w,x) (curve fitting function).
			return((long)XMLelementlist);	// This function is called using the direct method.
			break;
		case 1:
			return((long)XMLstringFromXPath);
			break;
		case 2:
			return((long)XMLparsedWaveFromXPath);
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

	switch (GetXOPMessage()) {
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


