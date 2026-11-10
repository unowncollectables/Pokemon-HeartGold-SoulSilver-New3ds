#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool SS3DS_PwalkerIrOpen(void);
void SS3DS_PwalkerIrClose(void);
void SS3DS_PwalkerIrRestoreInput(void);
bool SS3DS_PwalkerIrListen(void);
// Nonblocking FIFO read. Returns a byte count or -1 on I/O failure.
int SS3DS_PwalkerIrRead(uint8_t *data, size_t capacity);
// On success, the transceiver is already listening for the reply.
bool SS3DS_PwalkerIrSend(const uint8_t *data, size_t length);
