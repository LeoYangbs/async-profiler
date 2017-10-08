/*
 * Copyright 2017 Andrei Pangin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <unistd.h>
#include <iostream>
#include "vmStructs.h"
#include "codeCache.h"
#include <string.h>


int VMStructs::_klass_name_offset = -1;
int VMStructs::_symbol_length_offset = -1;
int VMStructs::_symbol_body_offset = -1;

uintptr_t dereferenceSymbol(NativeCodeCache* lib, const char* symbol_name) {
    const void* symbol = lib->findSymbol(symbol_name);
    if (symbol == NULL || symbol < 0) {
        // Fallback to avoid jvm crash in case of missing symbols
        return 0;
    }

    return *((uintptr_t*)symbol);
}

void VMStructs::init(NativeCodeCache* libjvm) {
    uintptr_t entry = dereferenceSymbol(libjvm, "gHotSpotVMStructs");
    uintptr_t stride = dereferenceSymbol(libjvm, "gHotSpotVMStructEntryArrayStride");
    uintptr_t type_offset = dereferenceSymbol(libjvm, "gHotSpotVMStructEntryTypeNameOffset");
    uintptr_t field_offset = dereferenceSymbol(libjvm, "gHotSpotVMStructEntryFieldNameOffset");
    uintptr_t offset_offset = dereferenceSymbol(libjvm, "gHotSpotVMStructEntryOffsetOffset");

    if (entry == 0 || stride == 0) {
        return;
    }

    while (true) {
        const char* type = *(const char**)(entry + type_offset);
        const char* field = *(const char**)(entry + field_offset);
        if (type == NULL || field == NULL) {
            break;
        }

        if (strcmp(type, "Klass") == 0) {
            if (strcmp(field, "_name") == 0) {
                _klass_name_offset = *(int*)(entry + offset_offset);
            }
        } else if (strcmp(type, "Symbol") == 0) {
            if (strcmp(field, "_length") == 0) {
                _symbol_length_offset = *(int*)(entry + offset_offset);
            } else if (strcmp(field, "_body") == 0) {
                _symbol_body_offset = *(int*)(entry + offset_offset);
            }
        }

        entry += stride;
    }
}