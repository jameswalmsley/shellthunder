#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <shellthunder.h>

struct st_command_item {
	struct st_list_head 	item;
	struct st_command   *command;
	void *pParam;
};

struct shell_intern {
	struct st_shell 		base;
	struct st_list_head 	commands;
	uint32_t				print_prompt;
	char 		   	   		input_buffer[256];
	uint32_t 				input_pos;
};

static char *eatspace(char *input) {
	while(isspace((int)*input)) {
		input++;	// Eat up prefixed whitespace.
	}
	return input;
}

static char *replace_expressions(struct shell_intern *s, const char *input) {

	char *last_replace = malloc(strlen(input)+1);
	char *replaced = NULL;
	char *item = NULL;
	char *item_end = NULL;

	strcpy(last_replace, input);

	goto next_item;

	while(item) {
		ST_u32 input_len = strlen(last_replace);
		if(item[1] == '{') {
			item_end = strchr(item, '}');
		} else if(item[1] == '(') {
			item_end = strchr(item, ')');
		} else {
			item++;
			goto next_item;
		}

		ST_BOOL bAlloced = 0;
		char *reduced = item;
		ST_u32 reduced_len = strlen(reduced);
		ST_u32 orig_len = (item_end - item);
		ST_u32 diff = reduced_len - orig_len;

		replaced = malloc(input_len + diff + 1);
		if(!replaced) {
			if(last_replace) {
				free(last_replace);
			}
			if(bAlloced) {
				free(reduced);
			}
			return NULL;
		}

		char *start = last_replace;
		ST_u32 i;
		strncpy(replaced, start, (item - start));
		i = (item - start);

		strncpy(replaced + i, reduced, reduced_len);
		i += reduced_len;

		strcpy(replaced+i, item_end + 1);

		if(bAlloced) {
			free(reduced);
		}

		free(last_replace);
		last_replace = replaced;

	next_item:
		item = strchr(last_replace, '$');
	}

	return last_replace;
}

struct st_shell *st_create(const char *prompt) {
	struct shell_intern *shell = calloc(1, sizeof(*shell));
	if(!shell) {
		return NULL;
	}

	ST_LIST_INIT_HEAD(&shell->commands);

	asprintf(&shell->base.prompt, prompt);
	shell->print_prompt = 1;

	return &shell->base;
}

int st_add(struct st_shell *shell, struct st_command *command, void *pParam) {

	if(!shell) {
		return -1;
	}

	struct shell_intern *s = st_container_of(shell, struct shell_intern, base, struct st_shell);

	struct st_command_item *item = calloc(1, sizeof(*item));
	if(!item) {
		return -1;
	}

	item->command = command;
	item->pParam = pParam;

	st_list_add(&item->item, &s->commands);

	return 0;
}

int st_exec(struct st_shell *shell, const char *command_line) {

	if(!shell) {
		return -1;
	}

	struct shell_intern *s = st_container_of(shell, struct shell_intern, base, struct st_shell);

	ST_u32 ulArguments = 0;
	ST_u32 bIsArg = 0;
	ST_u32 bIgnoreSpace = 0;
	char *input;

	if(!command_line || command_line[0] == '\0') {
		return 0;
	}

	char *copy = replace_expressions(s, command_line);

	input = copy;
	input = eatspace(input);

	if(!strlen(input)) {
		free(copy);
		return 0;
	}

	bIsArg = 1;

	char *line = input;

	while(*input) {

		if(!bIgnoreSpace && *input == '"') {			// Allow arguments with spaces in them, is surrounded by quotes.
			bIgnoreSpace = 1;
			input++;					// skip the char.
			continue;
		} else {
			if(*input == '"') {
				bIgnoreSpace = 0;
				input++;
				continue;
			}
		}

		if(bIsArg) {
			if(!bIgnoreSpace && isspace((int)*input)) {
				ulArguments += 1;
				bIsArg = 0;
			}
		} else {
			if(bIgnoreSpace || !isspace((int)*input)) {
				bIsArg = 1;
			}
		}

		input++;
	}

	if(bIsArg) {
		ulArguments += 1;
	}

	char **pargs = malloc(sizeof(char *) * ulArguments);
	if(!pargs) {
		return -1;
	}

	input = eatspace(line);

	bIsArg = 0;

	ST_u32 i = 0;
	bIgnoreSpace = 0;

	while(*input) {
		if(!bIgnoreSpace && *input == '"') {
			bIgnoreSpace = 1;
			input++;
			continue;
		} else {
			if(*input == '"') {
				bIgnoreSpace = 0;
				if(bIsArg) {
					*input = '\0';
				}
				input++;
				continue;
			}
		}

		if(!bIsArg) {
			if(!isspace((int)*input)) {
				bIsArg = 1;
				pargs[i++] = input;
			}
		} else {
			if(!bIgnoreSpace && isspace((int)*input)) {
				*input = '\0';
				bIsArg = 0;
			}
		}

		input++;
	}

	int retval = -1;

	struct st_list_head *pos;
	st_list_for_each(pos, &s->commands) {
		struct st_command_item *item = (struct st_command_item *) pos;
		if(!strcmp(pargs[0], item->command->name)) {
			retval = item->command->fn(ulArguments, pargs, item->pParam);
			goto executed;
		}
	}

executed:
	free(pargs);
	free(copy);

	return retval;
}

int st_terminal(struct st_shell *shell) {

	if(!shell) {
		return -1;
	}

	struct shell_intern *s = st_container_of(shell, struct shell_intern, base, struct st_shell);

	while(1) {

		if(s->print_prompt) {
			fputs(s->base.prompt, stdout);
			s->print_prompt = 0;
		}

		int32_t c = fgetc(stdin);
		if(c >= 0) {
			if(c == '\r' || c == '\n') {
				//fputs("\n", stdout);
				s->input_buffer[s->input_pos] = '\0';
				shell_exec(shell, s->input_buffer);
				s->input_pos = 0;
				s->print_prompt = 1;
			} else if(c == '\b') {
				if(s->input_pos) {
					//fputc(c, stdout);
					//fputc(' ', stdout);
					//fputc(c, stdout);
					s->input_pos -= 1;
				}
			} else {
				//fputc(c, stdout);
				if(s->input_pos < 256-1) {
					s->input_buffer[s->input_pos++] = c;
				}
			}
		}
	}

	return 0;
}
