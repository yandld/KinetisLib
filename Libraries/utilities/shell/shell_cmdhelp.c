
#include "shell.h"
<<<<<<< HEAD
#include "shell_cmdhelp.h"
=======

extern cmd_tbl_t* gpCmdTable[SHELL_MAX_FUNCTION_NUM];
>>>>>>> 0e64fd27aa62f8dfd5d6470ef5a690453e265aca

int DoHelp(int argc, char *const argv[])
{
    uint8_t i;
<<<<<<< HEAD
    cmd_tbl_t **cmdtpt = SHELL_get_cmd_tbl();
=======
>>>>>>> 0e64fd27aa62f8dfd5d6470ef5a690453e265aca
    if(argc == 1)
    {
        for(i = 0; i < SHELL_MAX_FUNCTION_NUM; i++)
        {
<<<<<<< HEAD
            if((cmdtpt[i] != NULL) && (cmdtpt[i]->usage != NULL))
            {
                SHELL_printf("%-*s(%d)- %s\r\n", 8, cmdtpt[i]->name, i, cmdtpt[i]->usage);
=======
            if((gpCmdTable[i] != NULL) && (gpCmdTable[i]->usage != NULL))
            {
                SHELL_printf("%-*s(%d)- %s\r\n", 8, gpCmdTable[i]->name, i, gpCmdTable[i]->usage);
>>>>>>> 0e64fd27aa62f8dfd5d6470ef5a690453e265aca
            }
				}
    }
		if(argc == 2)
		{
        for(i = 0; i < SHELL_MAX_FUNCTION_NUM; i++)
				{
<<<<<<< HEAD
					if((cmdtpt[i] != NULL))
					{
             if(!strcmp(argv[1], cmdtpt[i]->name))
						 {
							 if(cmdtpt[i]->help != NULL)
							 {
								 SHELL_printf("%-*s- %s\r\n", 8, cmdtpt[i]->name, cmdtpt[i]->help);
=======
					if((gpCmdTable[i] != NULL))
					{
             if(!strcmp(argv[1], gpCmdTable[i]->name))
						 {
							 if(gpCmdTable[i]->help != NULL)
							 {
								 SHELL_printf("%-*s- %s\r\n", 8, gpCmdTable[i]->name, gpCmdTable[i]->help);
>>>>>>> 0e64fd27aa62f8dfd5d6470ef5a690453e265aca
								 return 0;
							 }
							 else
							 {
								 SHELL_printf ("- No additional help available.\r\n");
							 }
						 }
					}
				}
				if(i == SHELL_MAX_FUNCTION_NUM)
				{
            SHELL_printf("- No Command available.\r\n");
				}
		}
		return 0;
}

int DoHelpComplete(int argc, char * const argv[], char last_char, int maxv, char *cmdv[])
{
    uint8_t str_len;
    uint8_t found = 0;
    uint8_t i;
    str_len = strlen(argv[argc-1]);
<<<<<<< HEAD
    cmd_tbl_t **cmdtpt = SHELL_get_cmd_tbl();
=======
>>>>>>> 0e64fd27aa62f8dfd5d6470ef5a690453e265aca
    switch(argc)
    {
        case 1:
            break;
        case 2:
            for(i = 0; i < SHELL_MAX_FUNCTION_NUM; i++)
            {
<<<<<<< HEAD
                if(cmdtpt[i] != NULL)
                {
                    if(!strncmp(argv[argc-1], cmdtpt[i]->name, str_len))
                    {
                        cmdv[found] = cmdtpt[i]->name;
=======
                if(gpCmdTable[i] != NULL)
                {
                    if(!strncmp(argv[argc-1], gpCmdTable[i]->name, str_len))
                    {
                        cmdv[found] = gpCmdTable[i]->name;
>>>>>>> 0e64fd27aa62f8dfd5d6470ef5a690453e265aca
                        found++;
                    }
                }
            }
            break;
        default:
            break;
    }
    return found;
}


const cmd_tbl_t CommandFun_Help = 
{
    .name = "help",
    .maxargs = 5,
    .repeatable = 1,
    .cmd = DoHelp,
    .usage = "print command description/usage",
    .complete = DoHelpComplete,
    .help = "\r\n"
            "	- print brief description of all commands\r\n"
            "help <command>\r\n"
            "	- print detailed usage of 'command'\r\n"
};