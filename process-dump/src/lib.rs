use log::Level;

use memflow::connector::ConnectorArgs;
use memflow_win32::*;
use std::path::PathBuf;
use memflow_coredump::create_connector;
use std::ffi::CStr;
use std::ptr;
use libc::c_char;
use std::ffi::CString;
use memflow::mem::VirtualMemory;

type CoreType<'a> = memflow_win32::Kernel<memflow::MappedPhysicalMemory<&'a [u8], memflow::MMAPInfo<'a>>, memflow::DirectTranslate>;
type ProcessType = *mut memflow_win32::Win32Process<memflow::VirtualDMA<&'static mut memflow::MappedPhysicalMemory<&'static [u8], memflow::MMAPInfo<'static>>, &'static mut memflow::DirectTranslate, memflow_win32::Win32VirtualTranslate>>;
#[no_mangle]
pub extern "C" fn init_core() -> *mut CoreType<'static> {
    let _ = simple_logger::SimpleLogger::new()
        .with_level(Level::Debug.to_level_filter())
        .init();


    let path = PathBuf::from("./phys.raw");
    println!("LOADING FROM PATH: {}", path.to_str().unwrap());

    let connector = create_connector(&ConnectorArgs::with_default(path.to_str().unwrap())).unwrap();

    let mut kernel = Kernel::builder(connector).build().unwrap();

    let eprocess_list = kernel.eprocess_list().unwrap();
    //println!("eprocess_list.len() = {:?}", eprocess_list);

    for addr in eprocess_list {
        let info = kernel.process_info_from_eprocess(addr);

        if let Ok(info) = info {
            println!("PROCESS: {} - {}", info.name, info.pid);
        }
    }

    //let mut res = kernel.process("DemoMemDump.exe").unwrap();
    //println!("{:?}", res);

    //let mut mem = res.virt_mem;
    //mem.virt_read_addr(memflow::Address::from(0xff));
    Box::into_raw(Box::new(kernel))
}

#[no_mangle]
pub unsafe extern "C" fn get_process_by_id(kernel: *mut CoreType<'static>, id: u32) -> ProcessType {
    if kernel.is_null() {
        panic!("NULL Pointer passed for kernel!");
    }

    let res = (*kernel).process_pid(id);

    match res {
        Ok(res) => Box::into_raw(Box::new(res)),
        Err(_) => ptr::null_mut()
    }
}

#[no_mangle]
pub unsafe extern "C" fn get_process_by_name(kernel: *mut CoreType<'static>, id: *const c_char) -> ProcessType {
    if kernel.is_null() {
        panic!("NULL Pointer passed for kernel!");
    }

    if id.is_null() {
        panic!("NULL Pointer passed for process name!");
    }

    let raw = CStr::from_ptr(id);
    let raw_str = raw.to_str().expect("Process conversion failed");

    let res = (*kernel).process(raw_str);

    match res {
        Ok(res) => Box::into_raw(Box::new(res)),
        Err(_) => ptr::null_mut()
    }

    //println!("PROC: {:?}", res.module_list());
}

#[no_mangle]
pub unsafe extern "C" fn get_main_module_addr(process: ProcessType) -> u64 {
    if process.is_null() {
        panic!("NULL Pointer passed for process!");
    }

    let mlist = (*process).module_list().unwrap();

    let main_mod = &mlist[0];
    let b = main_mod.base.as_u64();

    //for m in mlist {
        //println!("NAME: {}, BASE: {}", m.name, m.base);
    //}

    b
}

#[no_mangle]
pub unsafe extern "C" fn get_main_module_size(process: ProcessType) -> u64 {
    if process.is_null() {
        panic!("NULL Pointer passed for process!");
    }

    let mlist = (*process).module_list().unwrap();
    let main_mod = &mlist[0];

    //println!("SZ: {}", main_mod.size);
    main_mod.size as u64
}

#[no_mangle]
pub unsafe extern "C" fn get_module_address(process: ProcessType, id: *const c_char) -> u64 {
    if process.is_null() {
        panic!("NULL Pointer passed for process!");
    }

    let raw = CStr::from_ptr(id);
    let raw_str = raw.to_str().expect("Module conversion failed");

    let minfo = (*process).module_info(raw_str).unwrap();
    minfo.base.as_u64()
}

#[no_mangle]
pub unsafe extern "C" fn read_mem(process: ProcessType, offset: u64, mem_out: *mut u8, len: u64, read_out: *mut u64) {
    if process.is_null() {
        panic!("NULL Pointer passed for process!");
    }

    let proc = &mut *process;

    let mem = &mut proc.virt_mem;
    //mem.virt_read_addr(memflow::Address::from(offset));
    let res = mem.virt_read_raw(memflow::Address::from(offset), len as usize);

    if let Ok(res) = res {
        //println!("{:?}", res);
        std::ptr::copy_nonoverlapping(res.as_ptr(), mem_out, len as usize);

        if !read_out.is_null() {
            *read_out = len;
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn get_process_path(process: ProcessType) -> *mut c_char {
    if process.is_null() {
        panic!("NULL Pointer passed for process!");
    }

    let mlist = (*process).module_list().expect("MODULE LIST COULD NOT BE RETRIEVED");
    let main_mod = &mlist[0];
    let c_str = CString::new(main_mod.path.clone()).unwrap();
    c_str.into_raw()
}

#[no_mangle]
pub unsafe extern "C" fn free_process_path(c_str: *mut c_char) {
    if c_str.is_null() {
        return;
    }
    let s = CString::from_raw(c_str);
    drop(s);
}

#[no_mangle]
pub unsafe extern "C" fn free_core(core: *mut CoreType) {
    if core.is_null() {
        return;
    }
    drop(Box::from_raw(core));
}

#[no_mangle]
pub unsafe extern "C" fn free_process(proc: ProcessType) {
    if proc.is_null() {
        return;
    }
    drop(Box::from_raw(proc));
}
