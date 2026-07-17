use std::ffi::CString;
use super::gfxprim_ffi::*;
use super::Pixmap;
use super::timer::Timer;

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

    pub fn ev_key(&self) -> u32 {
        unsafe {
            if gp_ev_get_type(self.ptr) == EV_KEY {
                gp_ev_get_key(self.ptr)
            } else {
                0
            }
        }
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

    pub fn is_rendering(&self) {
        return unsafe { self.pixmap.
    }

    /// Updates screen so that it matches the pixmap backend buffer.
    ///
    /// The content of the pixmap buffer is guaranteed to contain the same data as before the
    /// update. The buffer pointer is not necessarily the same.
    pub fn update(&mut self) {
        unsafe { gp_backend_update(self.ptr) };
    }

    /// Updates a rectangular area from the buffer so that it matches the pixmap buffer.
    pub fn update_rect(&mut self, x0: gp_coord, y0: gp_coord, x1: gp_coord, y1: gp_coord) {
        unsafe { gp_backend_update_rect(self.ptr, x0, y0, x1, y1) };
    }

    /// Flips the backend pixmap buffer with the screen buffer.
    ///
    /// This is faster than Backend::update() but does not guarantee the pixmap data to be
    /// preserved.
    pub fn flip(&mut self) {
        unsafe { gp_backend_flip(self.ptr) };

        self.pixmap = Pixmap::from_ptr(unsafe {(*self.ptr).pixmap});
    }

    pub fn wait_ev(&mut self) -> Event {
        let raw_ev = unsafe { gp_backend_ev_wait(self.ptr) };

        let ev = Event::from_ptr(raw_ev);

        if ev.ev_type() == EV_SYS && ev.ev_code() == EV_SYS_RENDER_START {
            self.pixmap = Pixmap::from_ptr(unsafe {(*self.ptr).pixmap});
        }

        return ev;
    }

    /// Signals that rendering into backend::pixmap was stopped.
    ///
    /// This must be called after EV_SYS_RENDER_STOP event was received by the application.
    pub fn render_stopped(&mut self) {
        unsafe {
            gp_backend_render_stopped(self.ptr);
            self.pixmap = Pixmap::from_ptr(unsafe {(*self.ptr).pixmap});
        };
    }

    /// Starts a timer
    ///
    /// The timer is inserted into the backend timer queue and starts ticking. When timer expires
    /// an event is pushed into the backend event queue.
    pub fn timer_start(&mut self, timer: &mut Timer) {
        unsafe { gp_backend_timer_start(self.ptr, timer.as_ptr()) };
    }

    /// Stops a timer
    ///
    /// The timer is removed from the backend timer queue.
    pub fn timer_stop(&mut self, timer: &mut Timer) {
        unsafe { gp_backend_timer_stop(self.ptr, timer.as_ptr()) };
    }
}

impl Drop for Backend {
    fn drop(&mut self) {
        unsafe {
            gp_backend_exit(self.ptr);
        }
    }
}
