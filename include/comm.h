#ifndef ROT_TABLE_COMM_H
#define ROT_TABLE_COMM_H

#include <string>
#include "common.h"

const std::string CMD_OK = "OK";
const std::string ERR_UNKNOWN_CMD = "Error: Unknown command";
const std::string ERR_UNKNOWN_AX = "Error: Unknown axis";

void parseSerialCommands();

#endif