extern crate bindgen;

use std::env;
use std::path::PathBuf;

use bindgen::callbacks::{ParseCallbacks};

#[derive(Debug)]
struct CustomTrimmer;

impl ParseCallbacks for CustomTrimmer {
     fn enum_variant_name(
        &self,
        _enum_name: Option<&str>,
        variant_name: &str,
        _variant_value: bindgen::callbacks::EnumVariantValue,
    ) -> Option<String> {
        variant_name.strip_prefix("GP_").map(String::from)
    }
}

fn main() {
    println!("cargo:rustc-link-lib=gfxprim");
    println!("cargo:rustc-link-lib=gfxprim-loaders");
    println!("cargo:rustc-link-lib=gfxprim-backends");

    let bindings = bindgen::Builder::default()
        .wrap_static_fns(true)
        .wrap_static_fns_path(PathBuf::from("wrapper.c"))
        .allowlist_function("gp_.*")
        .allowlist_type("gp_.*")
        .allowlist_var("gp_.*")
        .prepend_enum_name(false)
        .parse_callbacks(Box::new(CustomTrimmer))
        .generate_comments(false)
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
