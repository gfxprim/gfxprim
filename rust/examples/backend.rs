use gfxprim::*;
use gfxprim::backends::*;
use std::env;
use std::process;

struct Colors {
    fg: gp_pixel,
    bg: gp_pixel,
}

fn repaint(backend: &mut Backend, colors: &Colors)
{
    let w = backend.pixmap.w() as i32;
    let h = backend.pixmap.h() as i32;

    backend.pixmap.fill(colors.bg);
    backend.pixmap.line(0, 0, w-1, h-1, colors.fg);
    backend.pixmap.line(0, h-1, w-1, 0, colors.fg);
    backend.flip();
}

fn init_colors(colors: &mut Colors, backend: &Backend)
{
    colors.bg = backend.pixmap.rgb_to_pixel(0x10, 0x10, 0x10);
    colors.fg = backend.pixmap.rgb_to_pixel(0xee, 0xee, 0xee);
}

fn main()
{
    let args: Vec<String> = env::args().collect();
    let mut colors: Colors = Colors {fg: 0, bg: 0};

    if args.len() > 2 {
        println!("usage: ./{} backend_pararams (pass 'help' for help)\n", args[0]);
        process::exit(1);
    }

    let init_str = if args.len() == 2 { &args[1] } else { "" };

    let mut backend = Backend::init(init_str, 100, 100, "Rust backend example").expect("Backend init failed");

    loop {
        let ev = backend.wait_ev();

        ev.print();

        match ev.ev_type() {
            EV_SYS => match ev.ev_code() {
                EV_SYS_QUIT => break,
                EV_SYS_RENDER_STOP => backend.render_stopped(),
                EV_SYS_RENDER_RESIZE => println!("Backend resized {}x{}", ev.new_w(), ev.new_h()),
                EV_SYS_RENDER_START => repaint(&mut backend, &colors),
                EV_SYS_RENDER_PIXEL_TYPE => init_colors(&mut colors, &backend),
                _ => {},
            }
            _ => {},
        }
    }
}
