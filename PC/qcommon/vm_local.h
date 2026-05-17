#include "../qcommon/qcommon.h"

#ifndef QDECL
#if defined(_MSC_VER)
#define QDECL __cdecl
#else
#define QDECL
#endif
#endif

struct vm_s {
    // DO NOT MOVE OR CHANGE THESE WITHOUT CHANGING THE VM_OFFSET_* DEFINES
    // USED BY THE ASM CODE
    int			programStack;		// the vm may be recursively entered
    int			(*systemCall)( int *parms );

	//------------------------------------
   
    char		name[MAX_QPATH];

	// for dynamic linked modules
	void		*dllHandle;
	int			(QDECL *entryPoint)( int callNum, ... );

	// for interpreted modules
	qboolean	currentlyInterpreting;

	qboolean	compiled;
	byte		*codeBase;
	int			codeLength;

	int			*instructionPointers;
	int			instructionPointersLength;

	byte		*dataBase;
	int			dataMask;

	int			stackBottom;		// if programStack < stackBottom, error

	int			numSymbols;
	struct vmSymbol_s	*symbols;

	int			callLevel;			// for debug indenting
	int			breakFunction;		// increment breakCount on function entry to this
	int			breakCount;

// fqpath member added 7/20/02 by T.Ray
	char		fqpath[MAX_QPATH+1] ;
};