#include <iostream>
#include <cstdint>
#include <iomanip>
extern "C" {
	void* init_core();
	void* get_process_by_id(void* core, int);
	void* get_process_by_name(void* core, const char* name);
	uint64_t get_main_module_addr(void* process);
	uintptr_t get_main_module_size(void* process);
	uintptr_t get_module_address(void* process, const char* mod);
	void read_mem(void* process, void* offset, int8_t* mem_out, uintptr_t read_length, uint64_t* read_out);
	char* get_process_path(void* process);
	void free_process_path(char* path);
}


void dump(uint8_t* ptr, uint64_t len) {
	for (int i = 0; i < len; ++i)
		std::cout << std::hex << std::setfill('0') << std::setw(2) << (uint64_t)ptr[i] << " ";
	std::cout << std::endl;
}

int main() {
	std::cout << "Test bench initialize\n";

	void* kernel = init_core();

	void* process = get_process_by_id(kernel, 14260);

	std::cout << process << std::endl;

	uint64_t addr = get_main_module_addr(process);
	std::cout << addr << std::endl;

	//base: 7ffdb8830000, size: 438272, path: "C:\\WINDOWS\\System32\\WS2_32.dll", name: "WS2_32.dll"
	uint64_t addr2 = get_module_address(process, "WS2_32.dll");
	std::cout << std::hex << addr2 << std::endl;

	uint8_t* temp = new uint8_t[16];
	uint64_t len_out = 0;
	read_mem(process, (void*)addr2, (int8_t*)temp, 16, &len_out);

	dump(temp, 16);

	void* proc1 = get_process_by_name(kernel, "modernwarfare.exe");

	std::cout << proc1 << std::endl;
}