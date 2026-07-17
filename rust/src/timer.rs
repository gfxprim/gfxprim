use std::ffi::CString;
use super::gfxprim_ffi::*;

const TIMER_STOP: u32 = u32::MAX;

pub struct Timer {
    raw_ptr: *mut gp_timer,
}

impl Drop for Timer {
    fn drop(&mut self) {
        unsafe { gp_timer_free(self.raw_ptr) };
    }
}

impl Timer {
    /// Allocates and initializes a new timer
    pub fn new(expires_ms: u32, period_ms: u32, id: &str) -> Option<Self> {
        let cid = CString::new(id).expect("Invalid string");

        let raw_ptr = unsafe { gp_timer_alloc(expires_ms, period_ms, cid.as_ptr(), None, std::ptr::null_mut()) };

        if raw_ptr.is_null() {
            None
        } else {
            Some(Self {raw_ptr: raw_ptr})
        }
    }

    /// Returns timer period
    pub fn period(self) -> u32 {
        unsafe { (*self.raw_ptr).period }
    }

    /// Returns true if timer is running
    pub fn is_running(&self) -> bool {
        if unsafe { gp_timer_is_running(self.raw_ptr)} == 0 {
            return false;
        }
        return true;
    }

    pub fn as_ptr(&self) -> *mut gp_timer {
        return self.raw_ptr;
    }
}
