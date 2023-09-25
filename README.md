# Silic Programming Language

A systems language with an emphasis on simplicity and expressivity.

`Note: I am using this project to learn compiler development so stuff might change around a lot.`


`hello_world.sil`
```zig
extern fn puts(message: *u8) -> i32;

fn main() -> i32 {
    puts("Hello, world!\n");

    return 0;
}
```

`math.sil`
```zig
fn add(a: i32, b: i32) -> i32 { a + b }
fn sub(a: i32, b: i32) -> i32 { a - b }
fn mul(a: i32, b: i32) -> i32 { a * b }
fn div(a: i32, b: i32) -> i32 { a / b }
```

`blink_led.sil`
```zig
const stm32 = import("stm32-hal");

fn wait() {
    for i in [0..2000000] volatile {
        asm nop;
    }
}

fn main() -> ! { 
  stm32.enable_port_clock(stm32.PORT_C);
  stm32.configure_port(stm32.PORT_C, stm32.PUSH_PULL);

  loop {
    stm32.toggle_led();
    wait();
  }
}
```
