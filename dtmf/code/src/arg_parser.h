/**
 * @file arg_parser.h
 * @brief Argument parser for the DTMF encoding and decoding program.
 * @author Aubry Mangold <aubry.mangold@heig-vd.ch>
 * @date 2025-03-24
 */

#ifndef ARG_PARSER_H
#define ARG_PARSER_H

struct arguments {
    char *command;
    char *input;
    char *output;
};

void parse_arguments(int argc, char **argv, struct arguments *arguments);

#endif  // ARG_PARSER_H
