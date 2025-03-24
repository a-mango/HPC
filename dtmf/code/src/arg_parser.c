#include <argp.h>
#include <stdlib.h>
#include <string.h>

const char *argp_program_version = "dtmf 1.0";

// Program documentation
static char doc[] = "DTMF encoding and decoding program";

// A description of the arguments we accept
static char args_doc[] = "COMMAND INPUT [OUTPUT]";

// The options we understand
static struct argp_option options[] = {{0}};

// Used by main to communicate with parse_opt
struct arguments {
    char *command;
    char *input;
    char *output;
};

// Parse a single option
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;

    switch (key) {
        case ARGP_KEY_ARG:
            if (state->arg_num == 0)
                arguments->command = arg;
            else if (state->arg_num == 1)
                arguments->input = arg;
            else if (state->arg_num == 2)
                arguments->output = arg;
            else
                argp_usage(state);
            break;
        case ARGP_KEY_END:
            if (arguments->command == NULL || arguments->input == NULL ||
                (strcmp(arguments->command, "encode") == 0 && arguments->output == NULL))
                argp_usage(state);
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc, NULL, NULL, NULL};

void parse_arguments(int argc, char **argv, struct arguments *arguments) {
    arguments->command = NULL;
    arguments->input   = NULL;
    arguments->output  = NULL;

    // Parse our arguments; every option seen by parse_opt will be reflected in arguments.
    argp_parse(&argp, argc, argv, 0, 0, arguments);
}
