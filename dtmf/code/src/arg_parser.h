#ifndef ARG_PARSER_H
#define ARG_PARSER_H

struct arguments {
    char *command;
    char *input;
    char *output;
};

void parse_arguments(int argc, char **argv, struct arguments *arguments);

#endif  // ARG_PARSER_H

