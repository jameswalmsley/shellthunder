#ifndef _SHELLTHUNDER_H_
#define _SHELLTHUNDER_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	@brief		A special macro to cast a member of a structure out to the containing structure
 *
 *	@ptr:       the pointer to the member.
 * 	@type:   	the type of the container struct this is embedded in.
 * 	@member:    the name of the member within the struct.
 *	@t_member:	the type of the member -- linux uses typeof() but most compilers dont have this.
 *
 *	@citation:	Linux kernel source-code.
 */
#define st_container_of(ptr, type, member, t_member) ((type *) (((char *) ((t_member *) (ptr))) - offsetof(type,member)))

struct st_command {
	char *name;
	char *description;
	int (*fn)	(int argc, char **argv, void *pParam);
};

struct st_shell {
	char *prompt;
};

struct st_shell *st_create	(const char *prompt);
int st_add					(struct st_shell *shell, struct st_command *command, void *pParam);
int st_exec					(struct st_shell *shell, const char *command_line);
int st_terminal				(struct st_shell *shell);

#ifdef __cplusplus
}
#endif


#endif
