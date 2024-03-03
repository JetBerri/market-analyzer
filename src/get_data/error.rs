use binance::errors::Error as BinanceError;
use binance::errors::ErrorKind as BinanceErrorKind;
use std::io;

pub fn error_handler(error: BinanceError) -> io::Result<()> {
    println!("Can't put an order!");

    match error.kind() {
        BinanceErrorKind::BinanceError(response) => match response.code {
            -1013_i16 => println!("Filter failure: LOT_SIZE!"),
            -2010_i16 => println!("Funds insufficient! {}", response.msg),
            _ => println!("Non-catched code {}: {}", response.code, response.msg),
        },
        BinanceErrorKind::Msg(msg) => {
            println!("Binancelib error msg: {}", msg)
        }
        _ => println!("Other errors: {}.", error),
    };

    Err(io::Error::new(io::ErrorKind::Other, "There has been an error while processing. Please restart.")) // Or create a specific error message
}
