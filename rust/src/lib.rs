#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

include!(concat!(env!("OUT_DIR"), "/gfxprim_bind.rs"));

use std::rc::Rc;
use std::ffi::c_void;

pub struct Pixmap {
    ptr: *mut gp_pixmap,
    /* Reference counter on the shared pixels pointer for subpixmaps */
    pixels: Rc<*mut u8>,
}

impl Pixmap {
    pub fn new(w: gp_size, h: gp_size, p: gp_pixel_type, stride: u32) -> Option<Self> {
        let raw_ptr = unsafe { gp_pixmap_alloc_ex(w, h, p, stride) };
        if raw_ptr.is_null() {
            None
        } else {
            Some(Self { ptr: raw_ptr, pixels: Rc::new(unsafe { (*raw_ptr).pixels })})
        }
    }
    pub fn sub_pixmap(&self, x: gp_coord, y: gp_coord, w: gp_size, h: gp_size) -> Option<Self> {
        unsafe {
            let sp = gp_sub_pixmap_alloc(self.ptr, x, y, w, h);

            if sp.is_null() {
                None
            } else {
                Some(Self {ptr: sp, pixels: Rc::clone(&self.pixels)})
            }
        }
    }
    pub fn w(&self) -> gp_size {
        return unsafe { gp_pixmap_w(self.ptr) }
    }
    pub fn raw_w(&self) -> gp_size {
        return unsafe { (*self.ptr).w }
    }
    pub fn h(&self) -> gp_size {
        return unsafe { gp_pixmap_h(self.ptr) }
    }
    pub fn raw_h(&self) -> gp_size {
        return unsafe { (*self.ptr).h }
    }
    pub fn pixel_type(&self) -> gp_pixel_type {
        return unsafe { (*self.ptr).pixel_type }
    }
    pub fn axes_swap(&self) -> bool {
        return unsafe { (*self.ptr).axes_swap() == 1 }
    }
    pub fn set_axes_swap(&mut self, swap: bool) {
        return unsafe { (*self.ptr).set_axes_swap(match swap {true => 1, false => 0}) }
    }
    pub fn x_swap(&self) -> bool {
        return unsafe { (*self.ptr).x_swap() == 1 }
    }
    pub fn set_x_swap(&mut self, swap: bool) {
        return unsafe { (*self.ptr).set_x_swap(match swap {true => 1, false => 0}) }
    }
    pub fn y_swap(&self) -> bool {
        return unsafe { (*self.ptr).y_swap() == 1 }
    }
    pub fn set_y_swap(&mut self, swap: bool) {
        return unsafe { (*self.ptr).set_y_swap(match swap {true => 1, false => 0}) }
    }
    pub fn set_gamma(&mut self, gamma: f32) -> Result<(), String> {
        unsafe {
            let res = gp_pixmap_gamma_set(self.ptr, gamma);

            if res != 0 {
                return Err("Malloc failed".into());
            }
        }

        Ok(())
    }
    pub fn set_srgb(&mut self) -> Result<(), String> {
        unsafe {
            let res = gp_pixmap_srgb_set(self.ptr);

            if res != 0 {
                return Err("Malloc failed".into());
            }
        }

        Ok(())
    }
    pub fn resize(&mut self, new_w: gp_size, new_h: gp_size) -> Result<(), String> {
        if Rc::get_mut(&mut self.pixels).is_none() {
            return Err("Cannot resize, subpixmap references exist".into());
        }

        unsafe {
            let res = gp_pixmap_resize(self.ptr, new_w, new_h);

            if res != 0 {
                return Err("Malloc failed".into());
            }
        }

        Ok(())
    }
    pub fn fill(&mut self, pixel: gp_pixel) {
        unsafe {
            gp_fill(self.ptr, pixel);
        }
    }
    pub fn rgb_to_pixel(&self, r: u8, g: u8, b: u8) -> gp_pixel {
        return unsafe { gp_rgb_to_pixmap_pixel(r, g, b, self.ptr) };
    }
    pub fn get_pixel(&self, x: gp_coord, y: gp_coord) -> gp_pixel {
        return unsafe { gp_getpixel(self.ptr, x, y) };
    }
    pub fn put_pixel(&mut self, x: gp_coord, y: gp_coord, pixel: gp_pixel) {
        unsafe { gp_putpixel(self.ptr, x, y, pixel) };
    }
    pub fn from_data(w: gp_size, h: gp_size, pixel_type: gp_pixel_type, pixels: &mut [u8]) -> Option<Self> {
        let raw_ptr = unsafe { gp_pixmap_from_data(w, h, pixel_type, pixels.as_mut_ptr() as *mut c_void, 0) };

        //TODO check that pixels array is long enough!

        if raw_ptr.is_null() {
            return None;
        } else {
            return Some(Self { ptr: raw_ptr, pixels: Rc::new(unsafe { (*raw_ptr).pixels })})
        }
    }
}

impl Drop for Pixmap {
    fn drop(&mut self) {
        unsafe {
            gp_pixmap_free(self.ptr);
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn PixmapTest() {
        let w = 100;
        let h = 200;
        let pixel_type = GP_PIXEL_RGB888;

        let mut p = Pixmap::new(w, h, pixel_type, 0).expect("Malloc failed");

        assert_eq!(p.w(), w);
        assert_eq!(p.h(), h);
        assert_eq!(p.pixel_type(), pixel_type);

        assert_eq!(p.x_swap(), false);
        p.set_x_swap(true);
        assert_eq!(p.x_swap(), true);

        assert_eq!(p.y_swap(), false);
        p.set_y_swap(true);
        assert_eq!(p.y_swap(), true);

        assert_eq!(p.axes_swap(), false);
        p.set_axes_swap(true);
        assert_eq!(p.axes_swap(), true);
    }

    #[test]
    fn SubPixmapTest() {
        let w = 100;
        let h = 200;
        let pixel_type = GP_PIXEL_RGB888;

        let mut p = Pixmap::new(w, h, pixel_type, 0).expect("Malloc failed");
        let mut sp = p.sub_pixmap(1, 1, w - 2, h - 2).expect("Malloc failed");
        let ssp = sp.sub_pixmap(1, 1, sp.w() - 2, sp.h() - 2).expect("Malloc failed");

        assert_eq!(sp.w(), w-2);
        assert_eq!(sp.h(), h-2);
        assert_eq!(sp.pixel_type(), pixel_type);

        assert_eq!(ssp.w(), w-4);
        assert_eq!(ssp.h(), h-4);
        assert_eq!(ssp.pixel_type(), pixel_type);

        assert_eq!(sp.x_swap(), false);
        sp.set_x_swap(true);
        assert_eq!(sp.x_swap(), true);

        assert_eq!(sp.y_swap(), false);
        sp.set_y_swap(true);
        assert_eq!(sp.y_swap(), true);

        assert_eq!(sp.axes_swap(), false);
        sp.set_axes_swap(true);
        assert_eq!(sp.axes_swap(), true);

        let res = sp.resize(10, 10);

        assert!(res.is_err(), "Subpixmap resize should have failed");

        let res = p.resize(10, 10);

        assert!(res.is_err(), "Pixmap with subpixmap cannot be resized");
    }

    #[test]
    fn SubPixmapLifetime() {
        let sp : Pixmap;

        {
            let p = Pixmap::new(100, 100, GP_PIXEL_RGB888, 0).expect("Malloc failed");

            sp = p.sub_pixmap(1, 1, 80, 80).expect("Malloc failed");

        }

        assert_eq!(sp.w(), 80);
        assert_eq!(sp.h(), 80);
    }

    #[test]
    fn PixmapFill()
    {
        let mut pixmap = Pixmap::new(100, 100, GP_PIXEL_RGB888, 0).expect("Malloc failure");
        let red = pixmap.rgb_to_pixel(0xff, 0x00, 0x00);
        let green = pixmap.rgb_to_pixel(0x00, 0xff, 0x00);

        pixmap.fill(red);
        assert_eq!(red, pixmap.get_pixel(0, 0));

        pixmap.fill(green);
        assert_eq!(green, pixmap.get_pixel(0, 0));
    }

    #[test]
    fn PixmapFromVec() {
        let mut pixels = vec![0x00, 0x00, 0x00];
        let mut p = Pixmap::from_data(1, 1, GP_PIXEL_RGB888, &mut pixels).expect("Malloc failed");
        let white = p.rgb_to_pixel(0xff, 0xff, 0xff);
        let black = p.rgb_to_pixel(0x00, 0x00, 0x00);

        assert_eq!(black, p.get_pixel(0, 0));
        p.fill(white);
        assert_eq!(white, p.get_pixel(0, 0));
    }
}
