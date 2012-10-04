/**
 * @file
 *
 * @date 02.02.09
 * @author Alexey Fomin
 */

#include <embox/unit.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <util/array.h>
#include <cmd/cmdline.h>
#include <framework/cmd/api.h>
#include <err.h>

#include "console/console.h"


// XXX just for now -- Eldar
EMBOX_UNIT(shell_start, shell_stop);

static void exec_callback(CONSOLE_CALLBACK *cb, CONSOLE *console, char *cmdline) {
	const struct cmd *cmd;
	int code;
	/* In the worst case cmdline looks like "x x x x x x". */
	char *argv[(CMDLINE_MAX_LENGTH + 1) / 2];
	int argc = 0;

	if (0 == (argc = cmdline_tokenize(cmdline, argv))) {
		/* Only spaces were entered */
		return;
	}

	if (NULL == (cmd = cmd_lookup(argv[0]))) {
		printf("%s: Command not found\n", argv[0]);
		return;
	}

	if (0 != (code = cmd_exec(cmd, argc, argv))) {
		printf("%s: Command returned with code %d: %s\n",
				cmd_name(cmd), code, strerror(-code));
	}
}

/**
 * Guesses command using its beginning
 *
 * -- Eldar
 */
static void guess_callback(CONSOLE_CALLBACK *cb, CONSOLE *console,
		const char* line, const int max_proposals, int *proposals_len,
		const char *proposals[], int *offset, int *common) {
	const struct cmd *cmd = NULL;
	int cursor = strlen(line);
	int start = cursor, i;
	char ch;

	while (start > 0 && isalpha(line[start - 1])) {
		start--;
	}
	line += start;

	*offset = cursor - start;
	*proposals_len = 0;

	cmd_foreach(cmd) {
		if (0 == strncmp(cmd_name(cmd), line, *offset)) {
			proposals[(*proposals_len)++] = cmd_name(cmd);
		}
	}

	*common = 0;
	if (*proposals_len == 0) {
		return;
	}
	while (1) {
		if ((ch = proposals[0][*offset + *common]) == '\0') {
			return;
		}
		for (i = 1; i < *proposals_len; ++i) {
			if (ch != proposals[i][*offset + *common]) {
				return;
			}
		}
		(*common)++;
	}
}
#if 0
static const char *script_commands[] = {
	#include <start_script.inc>
};

static void shell_start_script(CONSOLE *console, CONSOLE_CALLBACK *callback) {
	char buf[CMDLINE_MAX_LENGTH + 1];
	const char *command;

	array_foreach(command, script_commands, ARRAY_SIZE(script_commands)) {
		strncpy(buf, command, sizeof(buf));
		printf("> %s \n", buf);
		exec_callback(callback, console, buf);
	}
}
#endif

static CONSOLE console[1];

extern void run_start_script(void);

static int shell_start(void) {

	static CONSOLE_CALLBACK callback[1];

	callback->exec = exec_callback;
	callback->guess = guess_callback;
	if (console_init(console, callback) == NULL) {
		LOG_ERROR("Failed to create a console");
		return -1;
	}
#if 0
	if (ARRAY_SIZE(script_commands)) {
		printf("\nStarting script...\n");
		shell_start_script(console, callback);
	}
#endif
	//run_start_script();

	//printf("\n%s", OPTION_STRING_GET(welcome_msg));
	//console_start(console, prompt);
	return 0;
}

void shell_run(void) {
	static const char* prompt = OPTION_STRING_GET(prompt);

	printf("\n%s", OPTION_STRING_GET(welcome_msg));
	console_start(console, prompt);
}

static int shell_stop(void) {
	console_stop(console);
	return 0;
}
