use std::env;
use std::process;
use std::net::{IpAddr, TcpStream};
use std::str::FromStr;
use std::sync::mpsc::{Sender, channel};
use std::thread;

const DEFAULT_START_PORT: u16 = 0;
const DEFAULT_END_PORT: u16 = 65535;

const NUM_OF_THREADS: u16 = 10;

fn main() {
  let args: Vec<String> = env::args().collect();
  if args.len() < 2 {
    println!("Please specify a target IP.\nUsage: scan_ports [ip_add] [start_port] [end_port]");
    process::exit(1);
  }
  let ip_address: IpAddr = IpAddr::from_str(&args[1]).expect("Target must be a valid IPv4 or IPv6 address");

  let start_port: u16;
  let end_port: u16;

  if args.len() < 4 {
    println!("No start or end port specified, scanning from {} to {}", DEFAULT_START_PORT, DEFAULT_END_PORT);
    start_port = DEFAULT_START_PORT;
    end_port = DEFAULT_END_PORT;
  } else {
    start_port = args[2].parse::<u16>().ok().expect("The start port must be a valid integer");
    end_port = args[3].parse::<u16>().ok().expect("The start port must be a valid integer");
  }

  println!("Scanning ports...");

  let ports_per_thread = (end_port - start_port + 1) / NUM_OF_THREADS;

  let (sender, receiver) = channel();
  for i in 0..NUM_OF_THREADS {
    let m_sender = sender.clone();
    let thread_start_port = start_port + i * ports_per_thread;
    let thread_end_port = thread_start_port + ports_per_thread-1;
    thread::spawn(move || {
      scan_for_open_ports(m_sender, ip_address, thread_start_port, thread_end_port);
    });
  }
  if (end_port - start_port + 1) % NUM_OF_THREADS != 0 {
    let m_sender = sender.clone();
    let thread_start_port = start_port + NUM_OF_THREADS * ports_per_thread;
    let thread_end_port = end_port;
    thread::spawn(move || {
      scan_for_open_ports(m_sender, ip_address, thread_start_port, thread_end_port);
    });
  }
  drop(sender);
  for port in receiver {
    print!("{}", port);
  }
  println!("Scan finished :)");
}

fn scan_for_open_ports(sender: Sender<u16>, ip_address: IpAddr, start_port: u16, end_port: u16) {
  for curr_port in start_port..=end_port {
    match TcpStream::connect((ip_address, curr_port)) {
      Ok(_) => {
        println!("Port open: {}", curr_port);
        sender.send(curr_port).unwrap();
      }
      Err(_) => {
        // println!("Port closed: {}", curr_port);
      }
    }
  }
}