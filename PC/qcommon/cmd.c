/* Generated source skeleton from xSYM metadata. */
#ifndef __UNIMPLEMENTED__
#define __UNIMPLEMENTED__() do { } while (0)
#endif

#include "../qcommon/qcommon.h"
#include <stdlib.h>
#include <string.h>

#define	MAX_CMD_BUFFER	16384

/* Original path: /Volumes/BigCheese/ Source/AspyrP4/CoD/Source/qcommon/cmd.c */
/* No exact function-to-file mapping was present for this file in the decoded xSYM tables. */
static int cmd_argc; /* cmd_argc */
static char * cmd_argv[512]; /* cmd_argv */
cmd_t		cmd_text;
byte		cmd_text_buf[MAX_CMD_BUFFER];

int Cmd_Argc(void)
{
    return cmd_argc;
}

char * Cmd_Argv(int arg)
{
    if (cmd_argc > arg) {
        return cmd_argv[arg];
    }
    return "";
}

void Cbuf_AddText(const char *text)
{
    int length;

    length = strlen(text);
    if (cmd_text.cursize + length >= cmd_text.maxsize) {
        Com_Printf("Cbuf_AddText: overflow\n");
        return;
    }

    memcpy(cmd_text.data + cmd_text.cursize, text, length);
    cmd_text.cursize += length;
}

void Cbuf_Init(void)
{
	cmd_text.data = cmd_text_buf;
	cmd_text.maxsize = MAX_CMD_BUFFER;
	cmd_text.cursize = 0;
}
extern void Z_FreeInternal(void *ptr);
static cmd_function_t *cmd_functions; /* cmd_functions */

void Cmd_AddCommand(const char *cmdName, xcommand_t function)
{
    cmd_function_t *cmd;

    for (cmd = cmd_functions; cmd; cmd = (cmd_function_t *)cmd->next) {
        if (!strcmp(cmdName, cmd->name)) {
            cmd->function = function;
            return;
        }
    }

    cmd = (cmd_function_t *)malloc(sizeof(cmd_function_t));
    if (!cmd) {
        return;
    }
    cmd->name = (char *)malloc(strlen(cmdName) + 1);
    if (!cmd->name) {
        free(cmd);
        return;
    }
    strcpy(cmd->name, cmdName);
    cmd->function = function;
    cmd->autoCompleteDir = NULL;
    cmd->autoCompleteExt = NULL;
    cmd->next = (int)cmd_functions;
    cmd_functions = cmd;
}
void Cmd_RemoveCommand(const char *cmdName)
{
    cmd_function_t *cmd;
    cmd_function_t **back;

    back = &cmd_functions;
    for (cmd = cmd_functions; cmd; cmd = (cmd_function_t *)cmd->next) {
        if (!strcmp(cmdName, cmd->name)) {
            *back = (cmd_function_t *)cmd->next;
            if (cmd->name) {
                Z_FreeInternal(cmd->name);
            }
            Z_FreeInternal(cmd);
            return;
        }
        back = (cmd_function_t **)&cmd->next;
    }
}

void Cmd_TokenizeString(const char *text)
{
    static char tokenized_buffer[MAX_CMD_BUFFER];
    
    // Clear previous argv
    for (int i = 0; i < cmd_argc; i++) {
        cmd_argv[i] = NULL;
    }
    cmd_argc = 0;
    
    if (!text) return;
    
    strncpy(tokenized_buffer, text, sizeof(tokenized_buffer) - 1);
    tokenized_buffer[sizeof(tokenized_buffer) - 1] = '\0';
    
    char *p = tokenized_buffer;
    while (*p && cmd_argc < 512) {
        // Skip leading whitespace
        while (*p && *p <= ' ') {
            p++;
        }
        if (!*p) break;
        
        // Check for quoted token
        if (*p == '"') {
            p++;
            cmd_argv[cmd_argc++] = p;
            while (*p && *p != '"') {
                p++;
            }
            if (*p == '"') {
                *p++ = '\0';
            }
        } else {
            cmd_argv[cmd_argc++] = p;
            while (*p && *p > ' ') {
                p++;
            }
            if (*p) {
                *p++ = '\0';
            }
        }
    }
}

extern cvar_t *Cvar_FindVar( const char *var_name );
extern cvar_t *Cvar_Set2( const char *var_name, const char *value, qboolean force);

void Cmd_ExecuteString(const char *text) {
    if (!text || !text[0]) return;
    
    Cmd_TokenizeString(text);
    if (cmd_argc == 0) return;
    
    const char *cmd_name = cmd_argv[0];
    
    // 1. Search registered commands in cmd_functions linked list
    cmd_function_t *cmd;
    for (cmd = cmd_functions; cmd; cmd = (cmd_function_t *)cmd->next) {
        if (!Q_stricmp(cmd_name, cmd->name)) {
            if (cmd->function) {
                cmd->function();
            }
            return;
        }
    }
    
    // 2. Search registered CVars
    cvar_t *cvar = Cvar_FindVar(cmd_name);
    if (cvar) {
        if (cmd_argc == 1) {
            Com_Printf("\"%s\" is: \"%s^7\" default: \"%s^7\"\n", cvar->name, cvar->string, cvar->resetString ? cvar->resetString : "");
        } else {
            Cvar_Set2(cmd_name, cmd_argv[1], qfalse);
            Com_Printf("\"%s\" set to \"%s\"\n", cvar->name, cmd_argv[1]);
        }
        return;
    }
    
    // 3. Fallback print for unknown command
    Com_Printf("Unknown command \"%s\"\n", cmd_name);
}

void Cbuf_Execute(void) {
    if (cmd_text.cursize == 0) return;
    
    cmd_text.data[cmd_text.cursize] = '\0';
    
    static char exec_buf[MAX_CMD_BUFFER];
    memcpy(exec_buf, cmd_text.data, cmd_text.cursize + 1);
    
    cmd_text.cursize = 0;
    
    char *p = exec_buf;
    while (*p) {
        char line[512];
        int len = 0;
        while (*p && *p != '\n' && *p != '\r' && *p != ';' && len < 511) {
            line[len++] = *p++;
        }
        line[len] = '\0';
        
        while (*p && (*p == '\n' || *p == '\r' || *p == ';')) {
            p++;
        }
        
        if (len > 0) {
            Cmd_ExecuteString(line);
        }
    }
}