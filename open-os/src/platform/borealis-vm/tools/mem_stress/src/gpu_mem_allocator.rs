// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

use log::{info, warn};
use sdl2::event::Event;
use sdl2::image::{InitFlag, LoadTexture};
use sdl2::keyboard::Keycode;
use sdl2::pixels::{Color, PixelFormatEnum};
use sdl2::rect::Rect;
use sdl2::render::{Texture, TextureQuery};
use sdl2::surface::Surface;
use std::path::Path;
use std::time::Duration;

// handle the annoying Rect i32
macro_rules! rect(
    ($x:expr, $y:expr, $w:expr, $h:expr) => (
        Rect::new($x as i32, $y as i32, $w as u32, $h as u32)
    )
);

// Scale fonts to a reasonable size when they're too big (though they might look less smooth)
fn get_centered_rect(rect_width: u32, rect_height: u32, cons_width: u32, cons_height: u32) -> Rect {
    let wr = rect_width as f32 / cons_width as f32;
    let hr = rect_height as f32 / cons_height as f32;

    let (w, h) = if wr > 1f32 || hr > 1f32 {
        info!("Scaling down text.");
        if wr > hr {
            let h = (rect_height as f32 / wr) as i32;
            (cons_width as i32, h)
        } else {
            let w = (rect_width as f32 / hr) as i32;
            (w, cons_height as i32)
        }
    } else {
        (rect_width as i32, rect_height as i32)
    };

    let cx = (800 as i32 - w) / 2;
    let cy = (600 as i32 - h) / 2;
    rect!(cx, cy, w, h)
}

const CROS_LOGO_PNG: &str = "assets/win-icon-chrome-stable-600.png";
const TEXT_FONT_UNIX: &str = "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf";
const TEXT_FONT_CROS: &str = "/usr/share/fonts/noto/NotoSans-Regular.ttf";

pub fn allocate_gpu_mem(num_textures: usize) -> Result<(), String> {
    info!("Allocating GPU mem!");
    let sdl_context = sdl2::init()?;
    let video_subsystem = sdl_context.video()?;
    let _image_context = sdl2::image::init(InitFlag::PNG | InitFlag::JPG)?;
    let ttf_context = sdl2::ttf::init().map_err(|e| e.to_string())?;

    let window = video_subsystem
        .window("CrOS memory stress test", 800, 600)
        .position_centered()
        .opengl()
        .build()
        .map_err(|e| e.to_string())?;

    let mut canvas = window.into_canvas().build().map_err(|e| e.to_string())?;
    let texture_creator = canvas.texture_creator();
    let mut event_pump = sdl_context.event_pump()?;

    // There's a crate for this, but its optional/cosmetic anyway.
    let font = if Path::new(TEXT_FONT_CROS).exists() {
        Some(ttf_context.load_font(TEXT_FONT_CROS, 12).unwrap())
    } else if Path::new(TEXT_FONT_UNIX).exists() {
        Some(ttf_context.load_font(TEXT_FONT_UNIX, 12).unwrap())
    } else {
        warn!("No font files found");
        None
    };

    // Loop iterator, only used for modulo'ing the buffer index.
    let mut iter_num = 0;

    // Large hideous gfx memory buffer.
    let mut tx_buffer: Vec<Texture<'_>> = vec![];

    // Smaller hideous logo buffer.
    let mut tx_logo_buffer: Vec<Texture<'_>> = vec![];

    info!("Loading 360 logo textures");
    for _ in 0..360 {
        tx_logo_buffer.push(texture_creator.load_texture(CROS_LOGO_PNG)?);
    }

    // 15 minimum to properly blend 4k gradient (purely aesthtic).
    let n_textures: usize = if num_textures < 15 { 15 } else { num_textures };

    info!("Loading {n_textures} 4k textures");
    // Really inefficient way of doing gradients, but we only care about large gfx mem allocations.
    for idx in 1..=n_textures {
        let mut tx_bloat = texture_creator
            .create_texture_streaming(PixelFormatEnum::RGB24, 3840, 2160)
            .map_err(|e| e.to_string())?;

        // Create a red-green gradient
        tx_bloat.with_lock(None, |buffer: &mut [u8], pitch: usize| {
            for y in 0..2160 {
                for x in 0..3840 {
                    let offset = y * pitch + x * 3;
                    buffer[offset] = (x / idx) as u8;
                    buffer[offset + 1] = (y / idx) as u8;
                    buffer[offset + 2] = 0;
                }
            }
        })?;

        tx_buffer.push(tx_bloat);
    }

    // This falls behind the gradient, but contrasts if the gradient image is ever smaller (?)
    canvas.set_draw_color(Color::RGB(200, 0, 200));
    info!("Textures all loaded.  Starting rendering loop");

    'running: loop {
        for event in event_pump.poll_iter() {
            match event {
                Event::Quit { .. }
                | Event::KeyDown {
                    keycode: Some(Keycode::Escape),
                    ..
                } => break 'running,
                _ => {}
            }
        }
        canvas.clear();

        let surface = match font {
            Some(ref f) => f
                .render("Eating some GFX memory...")
                .blended(Color::RGBA(255, 0, 0, 255))
                .map_err(|e| e.to_string())?,

            // TODO: This will leave a dot in the middle, make a blended surface instead.  Reference a font to avoid this path.
            None => Surface::new(1, 1, PixelFormatEnum::RGB24)?,
        };

        let texture = texture_creator
            .create_texture_from_surface(&surface)
            .map_err(|e| e.to_string())?;

        // If the example text is too big for the screen, downscale it (and center irregardless)
        let TextureQuery { width, height, .. } = texture.query();
        let padding = 64;
        let target = get_centered_rect(width, height, 800 - padding, 600 - padding);

        // Render gradient boxes
        canvas.copy(
            &tx_buffer[iter_num % n_textures],
            None,
            Some(Rect::new(0, 0, 800, 600)),
        )?;

        // Render an (intentionally) horribly inefficient rotating cros logo.
        canvas.copy_ex(
            &tx_logo_buffer[iter_num % 360],
            None,
            Some(Rect::new(0, 0, 100, 100)),
            (iter_num % 360) as f64,
            None,
            false,
            false,
        )?;
        canvas.copy(&texture, None, Some(target)).unwrap();
        canvas.set_draw_color(Color::RGBA(195, 217, 255, 255));

        canvas.present();
        ::std::thread::sleep(Duration::new(0, 1_000_000_000u32 / 30));
        iter_num += 1;
    }

    Ok(())
}
