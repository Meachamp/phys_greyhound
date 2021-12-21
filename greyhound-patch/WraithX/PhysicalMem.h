#pragma once
#include "stdafx.h"

extern "C" {
	void* init_core();
	void* get_process_by_id(void* core, int);
	void* get_process_by_name(void* core, const char* name);
	void* get_main_module_addr(void* process);
	uintptr_t get_main_module_size(void* process);
	uintptr_t get_module_address(void* process, const char* mod);
	void read_mem(void* process, void* offset, int8_t* mem_out, uintptr_t read_length, SIZE_T* read_out);
	char* get_process_path(void* process);
	void free_process_path(char* path);
	void free_process(void* process);
	void free_core(void* core);
}