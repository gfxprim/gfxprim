use std::ptr::null;
use std::ffi::CString;
use super::gfxprim_ffi::*;
use super::Pixmap;

/// GFX text drawing functions
impl Pixmap {
    pub fn text(&mut self, x: gp_coord, y: gp_coord, flags: u32, fg: gp_pixel, bg: gp_pixel, text: &str) {
        let t = CString::new(text).expect("Invalid string");
        unsafe{ gp_text(self.ptr, std::ptr::null(), x, y, flags, fg, bg, t.as_ptr()) };
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn text_test() {
        let w = 10;
        let h = 10;
        let pixel_type = PIXEL_RGB888;

        let mut p = Pixmap::new(w, h, pixel_type, 0).expect("Malloc failed");

        let white = p.rgb_to_pixel(0xff, 0xff, 0xff);
        let black = p.rgb_to_pixel(0x00, 0x00, 0x00);

        p.fill(black);

        p.text(0, 0, VALIGN_BELOW | ALIGN_RIGHT, white, black, "TEST");

       assert_eq!(black, p.get_pixel(0, 1));
    }
}
