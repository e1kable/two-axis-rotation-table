#ifndef ROT_TABLE_COMM_H
#define ROT_TABLE_COMM_H

#include "common.h"

const string CMD_OK = "OK";
const string ERR_UNKNOWN_CMD = "Error: Unknown command";
const string ERR_UNKNOWN_AX = "Error: Unknown axis";

void parseSerialCommands();

#endif