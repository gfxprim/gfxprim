use super::gfxprim_ffi::*;
use super::Pixmap;

/* callback trampoline */
use std::os::raw::{c_int, c_void};

pub fn progress_trampoline<F, R>(progress_cb: F, op: impl FnOnce(*mut gp_progress_cb) -> R) -> R
where F: FnMut(f32) -> bool + 'static,
{
    let callback_ptr = Box::into_raw(Box::new(progress_cb)) as *mut c_void;

    unsafe extern "C" fn trampoline<F: FnMut(f32) -> bool>(cb: *mut gp_progress_cb) -> c_int
    {
        let callback = unsafe { &mut *(((*cb).priv_) as *mut F) };

        if callback(unsafe { (*cb).percentage }) { 0 } else { 1 }
    }

    let mut cb_struct = gp_progress_cb {
        callback: Some(trampoline::<F>),
        priv_: callback_ptr,
        flags: 0,
        threads: 1,
        percentage: 0.0,
    };

    op(&mut cb_struct)
}

/// Image loaders and savers
impl Pixmap {
    /// Loads image from a file.
    pub fn load(s: String, on_progress: impl Fn(f32) -> bool + 'static) -> Option<Pixmap> {
        progress_trampoline(on_progress, |cb_ptr| {
                unsafe {
                    let img_ptr = gp_load_image(s.as_ptr() as *const i8, cb_ptr);

                    if img_ptr.is_null() {
                        None
                    } else {
                        Some(Pixmap::from_ptr(img_ptr))
                    }
                }
            }
        )
    }

    pub fn save(self, s: String, on_progress: impl Fn(f32) -> bool + 'static) {
        progress_trampoline(on_progress, |cb_ptr| {
                unsafe {
                    gp_save_image(self.ptr, s.as_ptr() as *const i8, cb_ptr);
                }
            }
        )
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn callback_test() {
        let mut p = Pixmap::new(100, 100, GP_PIXEL_RGB888, 0).expect("Malloc failed");

        p.fill(0);

        p.save("test.png".to_string(), | progress | { println!("progress {:.1}%", progress); false });
    }
}
