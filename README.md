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

`structs_and_enums.sil`
```
const Country = enum {
    Canada,
    Japan,
    Usa,
};

const Person = struct {
    pub age: i32,
    pub country: Country,

    pub new: fn(age: i32, country: Country) -> Self {
        Self { age, country }
    }
};
```

`blink_led.sil`
```
const stm32 = import("stm32-hal");

const wait = fn() {
    for i in [0..2000000] volatile {
        asm nop;
    }
};

const main = fn() -> ! { 
  stm32.enable_port_clock(stm32.PORT_C);
  stm32.configure_port(stm32.PORT_C, stm32.PUSH_PULL);

  loop {
    stm32.toggle_led();
    wait();
  }
};
```
