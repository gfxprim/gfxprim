#![allow(dead_code)]
#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]

mod gfxprim_ffi;
mod gfx;
mod loaders;

use std::rc::Rc;
use std::ffi::c_void;
use gfxprim_ffi::*;

pub struct Pixmap {
    ptr: *mut gp_pixmap,
    /* Reference counter on the shared pixels pointer for subpixmaps */
    pixels: Rc<*mut u8>,
}

/// An in-memory buffer for pixel data.
///
/// A pixmap is a buffer for pixels along with a metadata describing the pixel format, image
/// size, gamma etc.
impl Pixmap {
    /// Constructs pixmap from a C pointer
    fn from_ptr(c_ptr: *mut gp_pixmap) -> Self {
        Self { ptr: c_ptr, pixels: Rc::new(unsafe { (*c_ptr).pixels })}
    }

    /// Allocates a new pixmap.
    ///
    /// # Arguments
    ///
    /// * `w` New image width.
    /// * `h` New image height.
    /// * `p` A pixel type.
    /// * `stride` Optional stride argument. If passed 0 the stride is calculated automatically.
    ///            Must be larger or equal to minimal stride to store image row with a given width
    ///            and for a given pixel format.
    ///
    /// # Examples
    ///
    /// let mut pic = Pixmap::new(10, 10, GP_PIXEL_RGB888, 0);
    ///
    /// # Errors
    ///
    /// Returns None if underlying allocation has failed or if stride was invalid.
    pub fn new(w: gp_size, h: gp_size, p: gp_pixel_type, stride: u32) -> Option<Self> {
        let raw_ptr = unsafe { gp_pixmap_alloc_ex(w, h, p, stride) };
        if raw_ptr.is_null() {
            None
        } else {
            Some(Self { ptr: raw_ptr, pixels: Rc::new(unsafe { (*raw_ptr).pixels })})
        }
    }
    /// Returns a subpixmap.
    ///
    /// A subpixmap is a rectangular view into a parent pixmap. The pixels buffer is shared between
    /// parent pixmap and subpixmap, but operations on subpixmap are limited to a smaller area.
    ///
    /// # Errors
    ///
    /// Returns None if underlying allocation fails.
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
    /// Returns pixels buffer width after mirror and rotation flags were applied.
    pub fn w(&self) -> gp_size {
        return unsafe { gp_pixmap_w(self.ptr) }
    }
    /// Returns pixels buffer width.
    pub fn raw_w(&self) -> gp_size {
        return unsafe { (*self.ptr).w }
    }
    /// Returns pixels buffer height after mirror and rotation flags were applied.
    pub fn h(&self) -> gp_size {
        return unsafe { gp_pixmap_h(self.ptr) }
    }
    /// Returns pixels buffer height.
    pub fn raw_h(&self) -> gp_size {
        return unsafe { (*self.ptr).h }
    }
    /// Returns pixel type id.
    pub fn pixel_type(&self) -> gp_pixel_type {
        return unsafe { (*self.ptr).pixel_type }
    }
    /// Returns true if pixmap has x and y axes swapped.
    pub fn axes_swap(&self) -> bool {
        return unsafe { (*self.ptr).axes_swap() == 1 }
    }
    /// Sets or resets pixmap x and y axes swap.
    ///
    /// The physical buffer does not change, this on sets a flag that is respected by most pixmap
    /// operations.
    pub fn set_axes_swap(&mut self, swap: bool) {
        return unsafe { (*self.ptr).set_axes_swap(match swap {true => 1, false => 0}) }
    }
    /// Returns true if pixmap x direction is swapped.
    pub fn x_swap(&self) -> bool {
        return unsafe { (*self.ptr).x_swap() == 1 }
    }
    /// Sets or resets pixmap x direction swap.
    ///
    /// The physical buffer does not change, this on sets a flag that is respected by most pixmap
    /// operations.
    pub fn set_x_swap(&mut self, swap: bool) {
        return unsafe { (*self.ptr).set_x_swap(match swap {true => 1, false => 0}) }
    }
    /// Returns true if pixmap y direction is swapped.
    pub fn y_swap(&self) -> bool {
        return unsafe { (*self.ptr).y_swap() == 1 }
    }
    /// Sets or resets pixmap y direction swap.
    ///
    /// The physical buffer does not change, this on sets a flag that is respected by most pixmap
    /// operations.
    pub fn set_y_swap(&mut self, swap: bool) {
        return unsafe { (*self.ptr).set_y_swap(match swap {true => 1, false => 0}) }
    }
    /// Sets image correction gamma function with (gamma) coeficient.
    pub fn set_gamma(&mut self, gamma: f32) -> Result<(), String> {
        unsafe {
            let res = gp_pixmap_gamma_set(self.ptr, gamma);

            if res != 0 {
                return Err("Malloc failed".into());
            }
        }

        Ok(())
    }
    /// Sets image correction to sRGB.
    pub fn set_srgb(&mut self) -> Result<(), String> {
        unsafe {
            let res = gp_pixmap_srgb_set(self.ptr);

            if res != 0 {
                return Err("Malloc failed".into());
            }
        }

        Ok(())
    }
    /// Resizes image pixels buffer.
    ///
    /// Operates on the physical pixmap buffer, i.e. does not take into the account the axes_swap
    /// flag.
    ///
    /// # Errors
    ///
    /// Fails when there are subpixmap references to the buffer or when underlying allocation has
    /// failed.
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
    /// Fills pixmap with a pixel value.
    ///
    /// The (pixel) is an integer that encodes a color value and can be created by the
    /// `rgb_to_pixel()` method.
    pub fn fill(&mut self, pixel: gp_pixel) {
        unsafe {
            gp_fill(self.ptr, pixel);
        }
    }
    /// Creates a pixel value from a RGB representation.
    ///
    /// Returns a color based on a pixmap pixel type and RGB triplett.
    pub fn rgb_to_pixel(&self, r: u8, g: u8, b: u8) -> gp_pixel {
        return unsafe { gp_rgb_to_pixmap_pixel(r, g, b, self.ptr) };
    }
    /// Returns a pixel value at a coordinates x, y
    ///
    /// Returns 0 if coordinates are out of the pixmap.
    pub fn get_pixel(&self, x: gp_coord, y: gp_coord) -> gp_pixel {
        return unsafe { gp_getpixel(self.ptr, x, y) };
    }
    /// Puts a pixel value at a coordinate x, y
    ///
    /// The call is no-op if coordinates are out of the pixmap.
    pub fn put_pixel(&mut self, x: gp_coord, y: gp_coord, pixel: gp_pixel) {
        unsafe { gp_putpixel(self.ptr, x, y, pixel) };
    }
    /// Construct a pixmap from raw data.
    ///
    /// TODO
    pub fn from_data(w: gp_size, h: gp_size, pixel_type: gp_pixel_type, pixels: &mut [u8]) -> Option<Self> {
        let raw_ptr = unsafe { gp_pixmap_from_data(w, h, pixel_type, pixels.as_mut_ptr() as *mut c_void, 0) };

        //TODO check that pixels array is long enough!
        //TODO fix gp_pixmap_from_data() to pass bpr!!!

        if raw_ptr.is_null() {
            return None;
        } else {
            return Some(Self { ptr: raw_ptr, pixels: Rc::new(unsafe { (*raw_ptr).pixels })})
        }
    }
    /// Alias for blit_xywh
    pub fn blit(self, x_src: gp_coord, y_src: gp_coord, w_src: gp_size, h_src: gp_size, dst: &mut Pixmap, x_dst: gp_coord, y_dst: gp_coord) {
        self.blit_xywh(x_src, y_src, w_src, h_src, dst, x_dst, y_dst);
    }
    /// Blits pixmap rectangular region into a destination pixmap at an given offset.
    pub fn blit_xyxy(self, x0_src: gp_coord, y0_src: gp_coord, x1_src: gp_coord, y1_src: gp_coord, dst: &mut Pixmap, x_dst: gp_coord, y_dst: gp_coord) {
        unsafe { gp_blit_xyxy_clipped(self.ptr, x0_src, y0_src, x1_src, y1_src, dst.ptr, x_dst, y_dst) };
    }
    /// Blits pixmap rectangular region into a destination pixmap at an given offset.
    pub fn blit_xywh(self, x_src: gp_coord, y_src: gp_coord, w_src: gp_size, h_src: gp_size, dst: &mut Pixmap, x_dst: gp_coord, y_dst: gp_coord) {
        unsafe { gp_blit_xywh_clipped(self.ptr, x_src, y_src, w_src, h_src, dst.ptr, x_dst, y_dst) };
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
    fn pixmap_test() {
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
    fn sub_pixmap_test() {
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
    fn sub_pixmap_lifetime() {
        let sp : Pixmap;

        {
            let p = Pixmap::new(100, 100, GP_PIXEL_RGB888, 0).expect("Malloc failed");

            sp = p.sub_pixmap(1, 1, 80, 80).expect("Malloc failed");

        }

        assert_eq!(sp.w(), 80);
        assert_eq!(sp.h(), 80);
    }

    #[test]
    fn pixmap_fill()
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
    fn pixmap_from_vec() {
        let mut pixels = vec![0x00, 0x00, 0x00];
        let mut p = Pixmap::from_data(1, 1, GP_PIXEL_RGB888, &mut pixels).expect("Malloc failed");
        let white = p.rgb_to_pixel(0xff, 0xff, 0xff);
        let black = p.rgb_to_pixel(0x00, 0x00, 0x00);

        assert_eq!(black, p.get_pixel(0, 0));
        p.fill(white);
        assert_eq!(white, p.get_pixel(0, 0));
    }

    #[test]
    fn blit_test() {
        let mut src = Pixmap::new(100, 100, GP_PIXEL_RGB888, 0).expect("Malloc failure");
        let mut dst = Pixmap::new(100, 100, GP_PIXEL_RGB888, 0).expect("Malloc failure");

        let white = src.rgb_to_pixel(0xff, 0xff, 0xff);
        let black = dst.rgb_to_pixel(0x00, 0x00, 0x00);

        src.fill(white);
        dst.fill(black);

        src.blit_xywh(0, 0, 10, 10, &mut dst, 0, 0);

        assert_eq!(white, dst.get_pixel(0, 0));
        assert_eq!(black, dst.get_pixel(0, 10));
    }
}
