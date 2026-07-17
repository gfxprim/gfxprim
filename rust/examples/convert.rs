use gfxprim::*;
use std::env;
use std::process;

fn main()
{
    let args: Vec<String> = env::args().collect();

    if args.len() != 3 {
        println!("usage: ./{} input_image output_image\n", args[0]);
        process::exit(1);
    }

    let img = Pixmap::load(&args[1], |progress| {eprint!("\rLoading {} {:6.2}%", &args[1], progress); true});

    println!();

    if let Err(e) = img {
        println!("Failed to load {}: {}", args[1], e);
        process::exit(1);
    }

    let err = img.unwrap().save(&args[2], |progress| {eprint!("\rSaving {:6.2}%", progress); true});

    println!();

    if let Err(e) = err {
        println!("Failed to save {}: {}", args[2], e);
        process::exit(1);
    }
}
