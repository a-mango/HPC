/**
 * @file args.h
 * @brief Argument parser for the DTMF encoding and decoding program.
 * @author Aubry Mangold <aubry.mangold@heig-vd.ch>
 * @date 2025-03-24
 */

#ifndef ARGS_H
#define ARGS_H

struct arguments {
    char *command;
    char *input;
    char *output;
};

void parse_arguments(int argc, char **argv, struct arguments *arguments);

#endif  // ARGS_H
