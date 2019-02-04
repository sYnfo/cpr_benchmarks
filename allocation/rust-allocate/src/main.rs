extern crate csv;
use csv::Reader;

use std::time::Instant;


fn main() {
    let mut rdr = Reader::from_path("../data/data.csv")
        .expect("Failed to read csv file");

    type Record = (f64, f64, f64, f64, f64);

    let now = Instant::now();
    let result: Vec<Record> = rdr.deserialize()
        .filter_map(Result::ok)
        .collect();
    println!("Loaded CSV in {}ms", now.elapsed().as_millis());
    println!("Found {} records", result.len());
}
