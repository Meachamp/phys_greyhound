#include "stdafx.h"

// The class we are implementing
#include "ProcessReader.h"

// We require the following WraithX classes
#include "Patterns.h"
#include "FileSystems.h"
#include "PhysicalMem.h"

ProcessReader::ProcessReader()
{
    core = init_core();
    // Init the handle
    ProcessHandle = NULL;
}

ProcessReader::~ProcessReader()
{
    // Detatch if possible
    Detatch();
    //FREE CORE
    free_core(core);
    core = NULL;

}

bool ProcessReader::IsRunning()
{
    //Let's say the process is always running. 
    return true;
}

void ProcessReader::WaitForExit()
{ 
    //No need to wait. 
}

void ProcessReader::Detatch()
{
    //FREE PROCESS
    free_process(ProcessHandle);
    ProcessHandle = NULL;
}

bool ProcessReader::Attach(int ProcessID)
{
    // Attempt to attach to the following process, if we are already attached disconnect first
    if (ProcessHandle != NULL)
    {
        // Detatch first
        Detatch();
    }
    // Attemp to attach
    ProcessHandle = get_process_by_id(core, ProcessID);
    // Check
    if (ProcessHandle != NULL)
    {
        // Worked
        return true;
    }
    // Failed
    return false;
}

bool ProcessReader::Attach(const std::string& ProcessName)
{
    // Attempt to attach to the following process, if we are already attached disconnect first
    if (ProcessHandle != NULL)
    {
        // Detatch first
        Detatch();
    }
    // Attemp to attach
    ProcessHandle = get_process_by_name(core, ProcessName.c_str());
    // Check
    if (ProcessHandle != NULL)
    {
        // Worked
        return true;
    }
    // Failed
    return false;
}

bool ProcessReader::Connect(HANDLE ProcessHandleReference)
{
    // Detatch if we aren't already
    if (ProcessHandle != NULL)
    {
        // Detatch first
        Detatch();
    }
    // Set the new instance
    ProcessHandle = ProcessHandleReference;
    // Check
    if (ProcessHandle != NULL) { return true; }
    // Failed
    return false;
}

uintptr_t ProcessReader::GetMainModuleAddress()
{
    // Get the main module if we are attached
    if (ProcessHandle != NULL)
    {
        return (uintptr_t)get_main_module_addr(ProcessHandle);
    }
    // Failed to perform read
#ifdef _DEBUG
    throw new std::exception("Not attached to any process");
#else
    throw new std::exception("");
#endif
}

uintptr_t ProcessReader::GetMainModuleMemorySize()
{
    // Get the main module if we are attached
    if (ProcessHandle != NULL)
    {
        return (uintptr_t)get_main_module_size(ProcessHandle);
    }
    // Failed to perform read
#ifdef _DEBUG
    throw new std::exception("Not attached to any process");
#else
    throw new std::exception("");
#endif
}

uintptr_t ProcessReader::GetModuleAddress(const std::string & ModuleName)
{
    // Get the main module if we are attached
    if (ProcessHandle != NULL)
    {
        get_module_address(ProcessHandle, ModuleName.c_str());
    }
    // Failed to perform read
#ifdef _DEBUG
    throw new std::exception("Not attached to any process");
#else
    throw new std::exception("");
#endif
}

uintptr_t ProcessReader::GetModuleMemorySize(const std::string & ModuleName)
{
    return uintptr_t();
}

uintptr_t ProcessReader::GetSizeOfCode()
{
    // Get the main module if we are attached
    if (ProcessHandle != NULL)
    {
        // We must read from the main module address
        auto MainModuleAddress = GetMainModuleAddress();

        // Now we must read the PE information, starting with DOS_Header, NT_Header, then finally the optional entry
        auto DOSHeader = Read<IMAGE_DOS_HEADER>(MainModuleAddress);
        auto NTHeader = Read<IMAGE_NT_HEADERS>(MainModuleAddress + DOSHeader.e_lfanew);

        // Return code size
        return (uintptr_t)NTHeader.OptionalHeader.SizeOfCode;
    }
    // Failed to perform read
#ifdef _DEBUG
    throw new std::exception("Not attached to any process");
#else
    throw new std::exception("");
#endif
}

uintptr_t ProcessReader::GetSizeOfCode(uintptr_t Address)
{
    // Get the main module if we are attached
    if (ProcessHandle != NULL)
    {
        // Now we must read the PE information, starting with DOS_Header, NT_Header, then finally the optional entry
        auto DOSHeader = Read<IMAGE_DOS_HEADER>(Address);
        auto NTHeader = Read<IMAGE_NT_HEADERS>(Address + DOSHeader.e_lfanew);

        // Return code size
        return (uintptr_t)NTHeader.OptionalHeader.SizeOfCode;
    }
    // Failed to perform read
#ifdef _DEBUG
    throw new std::exception("Not attached to any process");
#else
    throw new std::exception("");
#endif
}

std::string ProcessReader::GetProcessPath()
{
    char* hdl = get_process_path(ProcessHandle);
    std::string path = hdl;
    free_process_path(hdl);

    std::cout << "PATH: " << path << std::endl;
    return path;

    // Failed to perform read
#ifdef _DEBUG
    throw new std::exception("Not attached to any process");
#else
    throw new std::exception("");
#endif
}

HANDLE ProcessReader::GetCurrentProcess() const
{
    // Return it
    return ProcessHandle;
}

std::string ProcessReader::ReadNullTerminatedString(uintptr_t Offset)
{
    // Get the string if we are attached
    if (ProcessHandle != NULL)
    {
        // Keep track of address and preallocate our result
        uintptr_t Address = Offset;
        std::string Result;
        Result.reserve(256);
        // Keep a buffer, and null it, most strings won't be larger than this
        char Buffer[256];
        std::memset(Buffer, 0, sizeof(Buffer));

        while (true)
        {
            // Read a buffer from the memory instead of 1 char at a time
            auto size = Read((uint8_t*)&Buffer, Address, sizeof(Buffer));
            // Check if we got anything back (invalid address, so just return what we have
            if (size == 0)
                break;
            // Loop over buffer
            for (size_t i = 0; i < size; i++)
            {
                // Validate string
                if (Buffer[i] == 0)
                    return Result;
                // Add to string
                Result += Buffer[i];
            }
            // Add address
            Address += size;
        }
        // Return
        return Result;
    }
    // Failed to perform read
#ifdef _DEBUG
    throw new std::exception("Not attached to any process");
#else
    throw new std::exception("");
#endif
}

int8_t* ProcessReader::Read(uintptr_t Offset, uintptr_t Length, uintptr_t& Result)
{
    // Make sure we're attached
    if (ProcessHandle != NULL)
    {
        // We can read the block
        int8_t* ResultBlock = new int8_t[Length];
        // Zero out the memory
        std::memset(ResultBlock, 0, Length);
        // Length read
        SIZE_T LengthRead = 0;
        // Read it
        read_mem(ProcessHandle, (void*)Offset, ResultBlock, Length, &LengthRead);
        // Set result
        Result = LengthRead;
        // Return result
        return ResultBlock;
    }
    // Failed to read it
    return nullptr;
}

size_t ProcessReader::Read(uint8_t * Buffer, uintptr_t Offset, uintptr_t Length)
{
    size_t Result = 0;

    if (ProcessHandle != NULL)
    {
        read_mem(ProcessHandle, (void*)Offset, (int8_t*)Buffer, Length, &Result);
    }

    return Result;
}

intptr_t ProcessReader::Scan(const std::string& Pattern, bool UseExtendedMemoryScan)
{
    // Make sure we're attached
    if (ProcessHandle != NULL)
    {
        // Get the main module and code segment size, depending on flags
        auto MainModuleAttr = GetMainModuleAddress();
        auto MainModuleSize = (UseExtendedMemoryScan) ? GetMainModuleMemorySize() : GetSizeOfCode();
        // Scan
        return Scan(Pattern, MainModuleAttr, MainModuleSize);
    }
    // Failed
    return -1;
}

intptr_t ProcessReader::Scan(const std::string& Pattern, uintptr_t Offset, uintptr_t Length)
{
    // Make sure we're attached
    if (ProcessHandle != NULL)
    {
        // The resulting offset
        intptr_t ResultOffset = 0;

        // Pattern data
        std::string PatternBytes;
        std::string PatternMask;

        // Process the input patterm
        Patterns::ProcessPattern(Pattern, PatternBytes, PatternMask);

        // Total amount of data read
        uintptr_t DataRead = 0;

        // Result
        intptr_t Result = -1;

        // Set base position
        auto Position = Offset;

        // Read chunks, it seems ReadProcessMemory doesn't return the entire module/buffer (we get 0 on some addresses)
        while (true)
        {
            // Scan the memory for it, read a chunk to scan
            int8_t* ChunkToScan = Read(Position, 65536, DataRead);

            // Scan the chunk
            Result = (intptr_t)Patterns::ScanBlock(PatternBytes, PatternMask, (uintptr_t)ChunkToScan, DataRead);

            // Clean up the memory block
            delete[] ChunkToScan;

            // Add the chunk offset itself (Since we're a process scanner)
            if (Result > -1) 
            { 
                Result += Position;
                break; 
            }

            // Increment chunk
            Position += 65536;

            // Check are we at the end of our region to scan
            if (Position > Offset + Length)
                break;
        }
        // Return it
        return Result;
    }
    // Failed to find it
    return -1;
}