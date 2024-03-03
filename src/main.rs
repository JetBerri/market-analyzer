use std::env;

mod get_data;


fn main() {
    let args: Vec<String> = env::args().collect();
    
    if args.len() < 2 {
        println!("Please provide a symbol as argument.");
        return;
    }

    let symbol = &args[1];

    println!("Coin symbol : {}",symbol);

    if let Some(coin_name) = get_data::get_symbol_name(symbol){
        println!("Coin name : {}", coin_name);
    } else {
        println!("Coin name not found for symbol {}", symbol);
    }

    if let Err(e) = get_data::get_symbol_data(symbol) {
        eprintln!("Error: {}", e);
    }
}