use binance::api::*;
use binance::market::*;

use reqwest;

use std::fs::File;
use std::io::{BufWriter, Write};
use crate::get_data::error::error_handler;
use serde;

mod error;

#[derive(serde::Deserialize)]
struct CoinInfo {
    name: String,
    symbol: String,
}

pub fn get_symbol_data(symbol: &str) -> std::io::Result<()> {
    // Open file in write mode (truncate) and handle errors
    let file = File::create("result_data/data.txt")?;
    let mut writer = BufWriter::new(file);

    // Create Binance client
    let market: Market = Binance::new(None, None);

    // Handle errors using `?` operator
    if let Ok(answer) = market.get_price(symbol) {
        println!("{:?}", answer);
        // Write result to file
        writeln!(writer, "{:?}", answer)?;
    } else if let Err(e) = market.get_price(symbol) {
        error_handler(e)?;
    }

    if let Ok(answer) = market.get_average_price(symbol) {
        println!("{:?}", answer);
        writeln!(writer, "{:?}", answer)?;
    } else if let Err(e) = market.get_average_price(symbol) {
        error_handler(e)?;
    }

    if let Ok(answer) = market.get_book_ticker(symbol) {
        println!(
            "Bid Price: {}, Ask Price: {}",
            answer.bid_price, answer.ask_price
        );
        writeln!(writer, "Bid Price: {}, Ask Price: {}", answer.bid_price, answer.ask_price)?;
    } else if let Err(e) = market.get_book_ticker(symbol) {
        error_handler(e)?;
    }

    if let Ok(answer) = market.get_24h_price_stats(symbol) {
        println!(
            "Open Price: {}, Higher Price: {}, Lower Price: {:?}",
            answer.open_price, answer.high_price, answer.low_price
        );
        writeln!(writer, "Open Price: {}, Higher Price: {}, Lower Price: {:?}", answer.open_price, answer.high_price, answer.low_price)?;
    } else if let Err(e) = market.get_24h_price_stats(symbol) {
        error_handler(e)?;
    }

    Ok(())
}

pub fn get_symbol_name(symbol: &str) -> Option<String> {
    
    let url = format!("https://api.coingecko.com/api/v3/coins/list");
    let response = reqwest::blocking::get(&url).ok()?;
    let coins: Vec<CoinInfo> = response.json().ok()?;

    for coin in coins {
        if coin.symbol == symbol {
            return Some(coin.name);
        }
    }

    None
}
