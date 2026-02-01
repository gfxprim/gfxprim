use std::ffi::CString;
use super::gfxprim_ffi::*;
use super::Pixmap;

pub struct Backend {
    ptr: *mut gp_backend,
    pub pixmap: Pixmap,
}

pub struct Event {
    ptr: *const gp_event,
    ev: gp_event,
}

impl Event {
    pub fn from_ptr(ptr: *const gp_event) -> Self
    {
        Self { ptr: ptr, ev: unsafe { std::ptr::read_unaligned(ptr) } }
    }

    pub fn ev_type(&self) -> u32 {
        unsafe { gp_ev_get_type(self.ptr) }
    }

    pub fn ev_code(&self) -> u32 {
        unsafe { gp_ev_get_code(self.ptr) }
    }

    pub fn print(&self) {
       unsafe { gp_ev_dump(self.ptr) };
    }

    pub fn new_w(&self) -> u32 {
        unsafe { gp_ev_new_w(self.ptr) }
    }

    pub fn new_h(&self) -> u32 {
        unsafe { gp_ev_new_h(self.ptr) }
    }

    pub fn pixel_type(&self) -> gp_pixel_type {
        unsafe { (*self.ptr).__bindgen_anon_1.pixel_type as gp_pixel_type }
    }
}

impl Backend {
    /// Initialize a backend.
    pub fn init(params: &str, pref_w: gp_size, prev_h: gp_size, caption: &str) -> Option<Self>
    {
        let c_params = CString::new(params).expect("Invalid string");
        let c_caption = CString::new(caption).expect("Invalid string");
        let raw_params = if params.is_empty() { std::ptr::null() } else { c_params.as_ptr() };

        let raw_ptr = unsafe {
            gp_backend_init(raw_params, pref_w, prev_h, c_caption.as_ptr())
        };


        if raw_ptr.is_null() {
            None
        } else {
            Some( Self { ptr: raw_ptr, pixmap: Pixmap::from_ptr(unsafe {(*raw_ptr).pixmap}) } )
        }
    }

    /// Updates screen so that it matches the pixmap backend buffer.
    ///
    /// The content of the pixmap buffer is guaranteed to contain the same data as before the
    /// update. The buffer pointer is not necessarily the same.
    pub fn update(&mut self) {
        unsafe { gp_backend_update(self.ptr) };

        //self.pixmap = Pixmap::from_ptr(unsafe {(*self.ptr).pixmap});
    }

    /// Updates a rectangular area from the buffer so that it matches the pixmap buffer.
    pub fn update_rect(&mut self, x0: gp_coord, y0: gp_coord, x1: gp_coord, y1: gp_coord) {
        unsafe { gp_backend_update_rect(self.ptr, x0, y0, x1, y1) };

        //self.pixmap = Pixmap::from_ptr(unsafe {(*self.ptr).pixmap});
    }

    /// Flips the backend pixmap buffer with the screen buffer.
    ///
    /// This is faster than Backend::update() but does not guarantee the pixmap data to be
    /// preserved.
    pub fn flip(&mut self) {
        unsafe { gp_backend_flip(self.ptr) };

        //self.pixmap = Pixmap::from_ptr(unsafe {(*self.ptr).pixmap});
    }

    pub fn wait_ev(&mut self) -> Event {
        let raw_ev = unsafe { gp_backend_ev_wait(self.ptr) };

        Event::from_ptr(raw_ev)
    }

    /// Signals that rendering into backend::pixmap was stopped.
    ///
    /// This must be called after EV_SYS_RENDER_STOP event was received by the application.
    pub fn render_stopped(&mut self) {
        unsafe { gp_backend_render_stopped(self.ptr) };
    }
}

impl Drop for Backend {
    fn drop(&mut self) {
        unsafe {
            gp_backend_exit(self.ptr);
        }
    }
}
