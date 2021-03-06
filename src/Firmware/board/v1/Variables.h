/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2017 Igor Petrovic

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "constants/Constants.h"

extern bool                 dmBufferCopied;
extern volatile uint8_t     activeLEDcolumn;
extern volatile uint64_t    inputBuffer[DIGITAL_BUFFER_SIZE];
extern volatile uint8_t     digital_buffer_head;
extern volatile uint8_t     digital_buffer_tail;
extern volatile uint8_t     activeButtonColumn;
extern volatile bool        _analogDataAvailable;
extern bool                 encodersProcessed;
extern uint64_t             inputMatrixBufferCopy;
extern bool                 buttonsProcessed;
