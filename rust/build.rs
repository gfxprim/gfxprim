extern crate bindgen;

use std::env;
use std::path::PathBuf;

fn main() {
    println!("cargo:rustc-link-lib=gfxprim");

    let bindings = bindgen::Builder::default()
        .wrap_static_fns(true)
        .wrap_static_fns_path(PathBuf::from("wrapper.c"))
        .prepend_enum_name(false)
        .clang_arg("-I/usr/include/gfxprim")
        .header("wrapper.h")
        .generate()
        .expect("Unable to generate bindings");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("gfxprim_bind.rs"))
        .expect("Couldn't write bindings!");

    cc::Build::new()
        .file("wrapper.c")
        .include("/usr/include/gfxprim/")
        .compile("bound_static_fns");
}
