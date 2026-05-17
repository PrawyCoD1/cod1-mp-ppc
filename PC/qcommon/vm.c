/* Generated source skeleton from xSYM metadata. */
#ifndef __UNIMPLEMENTED__
#define __UNIMPLEMENTED__() do { } while (0)
#endif
#include "../qcommon/qcommon.h"
#include <stdarg.h>
#include <stddef.h>

#ifndef QDECL
#if defined(_MSC_VER)
#define QDECL __cdecl
#else
#define QDECL
#endif
#endif

typedef struct vm_s {
	int (QDECL *entryPoint)( int callnum, ... );
	int compiled;
} vm_t;

extern vm_t *currentVM;
extern vm_t *lastVM;
extern int vm_debugLevel;
extern int VM_CallCompiled( vm_t *vm, int *args );
extern int VM_CallInterpreted( vm_t *vm, int *args );
/* Original path: /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/qcommon/vm.c */
/* No exact function-to-file mapping was present for this file in the decoded xSYM tables. */

int	QDECL VM_Call( vm_t *vm, int callnum, ... ) {
	vm_t	*oldVM;
	int		r;
	int i;
	int args[16];
	va_list ap;


	if ( !vm ) {
		Com_Error( ERR_FATAL, "VM_Call with NULL vm" );
	}

	oldVM = currentVM;
	currentVM = vm;
	lastVM = vm;

	if ( vm_debugLevel ) {
	  Com_Printf( "VM_Call( %i )\n", callnum );
	}

	// if we have a dll loaded, call it directly
	if ( vm->entryPoint ) {
		//rcg010207 -  see dissertation at top of VM_DllSyscall() in this file.
		va_start(ap, callnum);
		for (i = 0; i < sizeof (args) / sizeof (args[i]); i++) {
			args[i] = va_arg(ap, int);
		}
		va_end(ap);

		r = vm->entryPoint( callnum,  args[0],  args[1],  args[2], args[3],
                            args[4],  args[5],  args[6], args[7],
                            args[8],  args[9], args[10], args[11],
                            args[12], args[13], args[14], args[15]);
	} else if ( vm->compiled ) {
		r = VM_CallCompiled( vm, &callnum );
	} else {
		r = VM_CallInterpreted( vm, &callnum );
	}

	if ( oldVM != NULL ) // bk001220 - assert(currentVM!=NULL) for oldVM==NULL
	  currentVM = oldVM;
	return r;
}
