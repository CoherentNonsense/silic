# The Sil Programming Language

A low-level programming language with an emphasis on simplicity for embedded systems.

`Note: I am using this project to learn compiler development and most of this README isn't implemented.`


`hello_world.sil`
```
extern const puts = (message: *u8) -> i32;

const main = fn () -> i32 {
    puts("Hello, world!\n");

    return 0;
};
```

`blink_led.sil`
```
const stm32 = import("stm32-hal");

fn wait() {
    for i in [0..2000000] volatile {
        asm nop;
    }
};

fn main() -> ! { 
  stm32.enable_port_clock(stm32.PORT_C);
  stm32.configure_port(stm32.PORT_C, stm32.PUSH_PULL);

  loop {
    stm32.toggle_led();
    wait();
  }
};
```
