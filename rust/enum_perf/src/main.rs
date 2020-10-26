use std::time::{Instant};

struct Point2D {
  x:f64,
  y:f64
}

enum Transform {
  NoChange,
  RigidChange {cosa:f64, sina:f64, offset:Point2D},
  Scale       {multiplier:f64},
  ExtraVertex {insertion_idx:u64, vertex:Point2D}
}

fn calc_with_match(transform: &Transform) -> f64
{
  return match transform {
    Transform::NoChange                            => 1.0,
    Transform::RigidChange{cosa,sina:_,offset:_}   => *cosa,
    Transform::Scale{multiplier}                   => *multiplier,
    Transform::ExtraVertex{insertion_idx:_,vertex} => vertex.x,
  }
}

fn create_test_data(size:u64) -> Vec<Transform> {
  let mut data = Vec::new();

  for i in 0..size {
    if i % 4 != 0 {
      data.push(Transform::NoChange);      
    }

    let cosa = (i as f64) / (size as f64);
    let sina = (1.0 - cosa * cosa).sqrt();
    let x = 10.0 * cosa;
    let y = i as f64;

    if i % 4 != 1 {
        data.push(Transform::RigidChange{cosa, sina, offset:Point2D{x,y}});
    }

    if i % 4 != 2 {
        data.push(Transform::Scale{multiplier:(2.0*cosa)});
    }

    if i % 4 != 3 {
        data.push(Transform::ExtraVertex{insertion_idx:i, vertex:Point2D{x:y,y:x}});
    }
  }

  return data;
}

fn time_calc(num_runs: u64, data: &[Transform]) -> (f64,std::time::Duration)
{
  let start = Instant::now();
  let mut total:f64 = 0.0;

  for _ in 0..num_runs {
    for transform in data {
      total += calc_with_match(&transform);
    }
  }

  let duration = Instant::now().duration_since(start);
  return (total, duration);
}

fn main() {
    println!("Rust enum calc");

    let data_size:u64 = 10000;
    let data = create_test_data(data_size);

    let num_runs = 200000;

    println!("match: {:?}", time_calc(num_runs, &data));
}
