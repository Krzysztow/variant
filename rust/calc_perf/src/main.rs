use std::time::{Instant};

fn bifurcation(x_: f64, n: u64) -> f64 {
  let mut x = x_;

  for _ in 0..n {
    x = 4.0 * x * (1.0-x);
  }

  return x;
}

fn main() {
    println!("Rust calc performance");
    let start = Instant::now();

    println!("{:?}", bifurcation(0.61, 3_000_000_000));

    let finish = Instant::now();
    println!("{:?}", finish.duration_since(start));
   
}
