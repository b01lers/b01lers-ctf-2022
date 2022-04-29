mod graph;
use std::{
    error::Error,
    io::{Read, Write},
    net::TcpListener,
};

use crate::graph::{Graph, InvalidKeyError, KEY};

fn validate_input(input: &str) -> bool {
    let mut graph = Graph::new();
    let input = input.chars().collect::<Vec<char>>();
    let test = input
        .windows(2)
        .map(|window| graph.traverse_web(window[0], window[1]));
    if test.len() + 1 != KEY.len() {
        false
    } else {
        test.zip(KEY.iter())
            .map(|(check_len, key_len)| match check_len {
                Ok(check_len) => check_len == *key_len,
                _ => false,
            })
            .all(|v| v)
    }
}

#[derive(PartialEq)]
enum RequestParseState {
    Start,
    Type,
    End,
    Invalid,
    Done,
}

fn get_input() -> Result<String, Box<dyn Error>> {
    let listener = TcpListener::bind("127.0.0.1:7878")?;
    let mut input = String::new();
    for stream in listener.incoming() {
        let resp = b"HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
        let mut stream = stream?;
        let mut buffer = [0; 1024];
        stream.read(&mut buffer)?;

        let mut splits = buffer
            .split(|&v| v == ' ' as u8)
            .rev()
            .collect::<Vec<&[u8]>>();
        let mut state = RequestParseState::Start;
        while let Some(el) = splits.pop() {
            match state {
                RequestParseState::Start => {
                    if el == b"GET" || el == b"get" {
                        state = RequestParseState::Type;
                    } else {
                        state = RequestParseState::Invalid;
                    }
                }
                RequestParseState::Type => {
                    let param = el.split(|&v| v == '/' as u8).nth(1).ok_or("No element")?;
                    if param.len() == 1 {
                        input.push(param[0] as char);
                        state = RequestParseState::End;
                        stream.write(resp)?;
                        stream.flush()?;
                    } else if param[0] == '%' as u8 {
                        let without_prefix = &param[1..=2];
                        let c =
                            u8::from_str_radix(std::str::from_utf8(without_prefix)?, 16)? as char;
                        input.push(c);
                        state = RequestParseState::End;
                        stream.write(resp)?;
                        stream.flush()?;
                    } else if param == b"crawl" {
                        state = RequestParseState::Done;
                        stream.write(resp)?;
                        stream.flush()?;
                    }
                }
                RequestParseState::Done => {
                    return Ok(input);
                }
                _ => {}
            }
            if state == RequestParseState::End {
                break;
            }
        }
    }

    //std::io::stdin().read_line(&mut input)?;
    //input = input.replace('\n', "\0");
    //input.insert(0, '\0');
    //
    Ok(input)
}

fn main() -> Result<(), Box<dyn Error>> {
    println!("Welcome to the b01lers flag verification service!");
    let input = match get_input() {
        Ok(input) => input,
        Err(e) => {
            println!("Input collection failed :(");
            return Err(e);
        }
    };

    let new_input = "\0".to_owned() + &input.replace("\n", "\0");

    //bctf{sorry_no_nfts_only_flags}
    if validate_input(&new_input) {
        println!("Congrats, your flag {} is valid!", input);
        Ok(())
    } else {
        println!("Sorry {} is not a valid flag.", input);
        Err(Box::new(InvalidKeyError {}))
    }
}
