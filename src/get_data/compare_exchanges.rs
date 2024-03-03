use binance::api::*;
use binance::model::*;
use binance::market::*;

use bitfinex::api::*;
use bitfinex::pairs::*;
use bitfinex::currency::*;
use bitfinex::precision::*;

pub fn binance_exchange_price(symbol: &str) -> &str{

    let market: Market = Binance::new(None, None);

    match market.get_price("BNBETH") {
        Ok(answer) => println!("{:?}", answer),
        Err(e) => println!("Error: {:?}", e),
    }

    answer

}

pub fn bitfinex_exchange_price(symbol: &str) -> &str {

    let api = Bitfinex::new(None, None);

    let ticker_result = api.ticker.trading_pair(BTCUSD);
    let btc_price = match ticker_result {
        Ok(ticker) => {
            let mid_price = (ticker.bid + ticker.ask) / 2.0;
            mid_price
        }
        Err(e) => return Err(format!("Error getting ticker data: {}", e)),
    };

    mid_price

}