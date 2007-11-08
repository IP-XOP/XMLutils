#include "XOPStandardHeaders.r"

resource 'vers' (1) {						/* XOP version info */
	0x01, 0x10, final, 0x00, 0,				/* version bytes and country integer */
	"1.10",
	"1.10, © 1996-2004 WaveMetrics, Inc., all rights reserved."
};

resource 'vers' (2) {						/* Igor version info */
	0x05, 0x04, release, 0x00, 0,			/* version bytes and country integer */
	"5.04",
	"(for Igor Pro 5.04 or later)"
};

resource 'STR#' (1100) {					/* custom error messages */
	{
	//[1]
	"XMLutils requires Igor Pro 5.04 or later.",
	//[2]
	"Error while parsing XML document.",
	//[3]
	"Unable to create new XPath context.",
	//[4]
	"Failed to register namespaces list.",
	//[5]
	"Unable to evaluate xpath expression.",
	//[6]
	"One of the input strings is NULL.",
	//[7]
	"Your Xpath expression did not compile.",
	}
};

resource 'STR#' (1101) {					// Misc strings for XOP.
	{
		"-1",								// This item is no longer supported by the Carbon XOP Toolkit.
		"No Menu Item",						// This item is no longer supported by the Carbon XOP Toolkit.
		"XMLutils Help",					// Name of XOP's help file.
	}
};

resource 'XOPI' (1100) {
	XOP_VERSION,							// XOP protocol version.
	DEV_SYS_CODE,							// Development system information.
	0,										// Obsolete - set to zero.
	0,										// Obsolete - set to zero.
	XOP_TOOLKIT_VERSION,					// XOP Toolkit version.
};

resource 'XOPF' (1100) {
	{
		"XMLelementlist",					// Function name.
		F_EXP | F_EXTERNAL,				// Function category,
		NT_FP64,						// Return value type.
		{									
		HSTRING_TYPE,	
		},
		"XMLstringFromXPath",					// Function name.
		F_EXP | F_EXTERNAL,				// Function category,
		HSTRING_TYPE,						// Return value type.
		{									
		HSTRING_TYPE,					//filename
		HSTRING_TYPE,					//xpath
		HSTRING_TYPE,					//namespaces
		HSTRING_TYPE,					//options
		},
		"XMLparsedWaveFromXPath",					// Function name.
		F_EXP | F_EXTERNAL,				// Function category,
		NT_FP64,						// Return value type.
		{									
		HSTRING_TYPE,					//filename
		HSTRING_TYPE,					//xpath
		HSTRING_TYPE,					//namespaces
		HSTRING_TYPE,					//options
		},			
	}
};
