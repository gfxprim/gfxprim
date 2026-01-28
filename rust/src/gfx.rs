use super::gfxprim_ffi::*;
use super::Pixmap;

/// GFX drawing functions
impl Pixmap {
    /// Draws a line between points [x0, y0] and [x1, y1]
    pub fn line(&mut self, x0: gp_coord, y0: gp_coord, x1: gp_coord, y1: gp_coord, pixel: gp_pixel) {
        unsafe { gp_line(self.ptr, x0, y0, x1, y1, pixel) };
    }
    /// Alias for vline_xyy
    pub fn vline(&mut self, x: gp_coord, y0: gp_coord, y1: gp_coord, pixel: gp_pixel) {
        self.vline_xyy(x, y0, y1, pixel);
    }
    /// Draws a vertical line between points y0 and y1 at x
    pub fn vline_xyy(&mut self, x: gp_coord, y0: gp_coord, y1: gp_coord, pixel: gp_pixel) {
        unsafe { gp_vline_xyy(self.ptr, x, y0, y1, pixel) };
    }
    /// Draws a vertical line starting at point x, y with lenght h
    pub fn vline_xyh(&mut self, x: gp_coord, y: gp_coord, h: gp_size, pixel: gp_pixel) {
        unsafe { gp_vline_xyh(self.ptr, x, y, h, pixel) };
    }
    /// Alias for hline_xxy
    pub fn hline(&mut self, x0: gp_coord, x1: gp_coord, y: gp_coord, pixel: gp_pixel) {
        self.hline_xxy(x0, x1, y, pixel);
    }
    /// Draws a horizontal line between points x0 and x1 at y
    pub fn hline_xxy(&mut self, x0: gp_coord, x1: gp_coord, y: gp_coord, pixel: gp_pixel) {
        unsafe { gp_hline_xxy(self.ptr, x0, x1, y, pixel) };
    }
    /// Draws a horizontal line starting at point x, y with length w
    pub fn hline_xyw(&mut self, x: gp_coord, y: gp_coord, w: gp_size, pixel: gp_pixel) {
        unsafe { gp_hline_xyw(self.ptr, x, y, w, pixel) };
    }
    /// Draws a circle at point [x_center, y_center] with radius r
    pub fn circle(&mut self, x_center: gp_coord, y_center: gp_coord, r: gp_size, pixel: gp_pixel) {
        unsafe { gp_circle(self.ptr, x_center, y_center, r, pixel) };
    }
    /// Draws a filled circle at point [x_center, y_center] with radius r
    pub fn fill_circle(&mut self, x_center: gp_coord, y_center: gp_coord, r: gp_size, pixel: gp_pixel) {
        unsafe { gp_fill_circle(self.ptr, x_center, y_center, r, pixel) };
    }
    /// Draws a ring at point [x_center, y_center] with radiuses r1 and r2
    pub fn ring(&mut self, x_center: gp_coord, y_center: gp_coord, r1: gp_size, r2: gp_size, pixel: gp_pixel) {
        unsafe { gp_ring(self.ptr, x_center, y_center, r1, r2, pixel) };
    }
    /// Draws a filled ring at point [x_center, y_center] with radiuses r1 and r2
    pub fn fill_ring(&mut self, x_center: gp_coord, y_center: gp_coord, r1: gp_size, r2: gp_size, pixel: gp_pixel) {
        unsafe { gp_fill_ring(self.ptr, x_center, y_center, r1, r2, pixel) };
    }
    pub fn polygon(&mut self, x_off: gp_coord, y_off: gp_coord, xy: &[gp_coord], pixel: gp_pixel) {
        if xy.len() % 2 == 1 {
            panic!("Odd number of coordinates");
        }
        unsafe{ gp_polygon(self.ptr, x_off, y_off, (xy.len()/2).try_into().unwrap(), xy.as_ptr(), pixel) };
    }
    pub fn polygon_th(&mut self, x_off: gp_coord, y_off: gp_coord, xy: &[gp_coord], r:gp_size, pixel: gp_pixel) {
        if xy.len() % 2 == 1 {
            panic!("Odd number of coordinates");
        }
        unsafe{ gp_polygon_th(self.ptr, x_off, y_off, (xy.len()/2).try_into().unwrap(), xy.as_ptr(), r, pixel) };
    }
    pub fn fill_polygon(&mut self, x_off: gp_coord, y_off: gp_coord, xy: &[gp_coord], pixel: gp_pixel) {
        if xy.len() % 2 == 1 {
            panic!("Odd number of coordinates");
        }
        unsafe{ gp_fill_polygon(self.ptr, x_off, y_off, (xy.len()/2).try_into().unwrap(), xy.as_ptr(), pixel) };
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn line_test() {
        let w = 10;
        let h = 10;
        let pixel_type = GP_PIXEL_RGB888;

        let mut p = Pixmap::new(w, h, pixel_type, 0).expect("Malloc failed");

        let white = p.rgb_to_pixel(0xff, 0xff, 0xff);
        let black = p.rgb_to_pixel(0x00, 0x00, 0x00);

        p.fill(white);
        p.line(0, 0, 9, 9, black);

        assert_eq!(black, p.get_pixel(0, 0));
    }
}
