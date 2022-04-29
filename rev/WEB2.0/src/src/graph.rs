use std::{cmp::Ordering, collections::BinaryHeap, error::Error, fmt::Display};

#[derive(Default, Clone, Copy)]
struct Node {
    value: char,
    cost: u8,
    visit_status: u8,
    len: u32,
    neighbors: u32,
}

#[derive(Copy, Clone, Eq, PartialEq)]
struct SearchState {
    len: u32,
    position: usize,
}

impl Ord for SearchState {
    fn cmp(&self, other: &Self) -> Ordering {
        other
            .len
            .cmp(&self.len)
            .then_with(|| self.position.cmp(&other.position))
    }
}

impl PartialOrd for SearchState {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

pub struct Graph {
    graph: [Node; 30],
}

pub const KEY: [u32; 31] = [
    0x3b, 0x19, 0x50, 0x48, 0x73, 0x3c, 0x58, 0x1e, 0x1, 0x2d, 0x64, 0x55, 0x4e, 0x64, 0x55, 0x38,
    0x50, 0x17, 0x75, 0x56, 0x5c, 0x7a, 0x3c, 0x64, 0x50, 0x64, 0x2a, 0x48, 0x1f, 0x2f, 0x12,
];

#[derive(Debug)]
pub struct NotFoundError;

impl Display for NotFoundError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "NotFoundError")
    }
}

#[derive(Debug)]
pub struct InvalidKeyError;

impl Display for InvalidKeyError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "InvalidKeyError")
    }
}

impl Error for NotFoundError {}

impl Error for InvalidKeyError {}

impl Graph {
    fn calc_idx(c: char) -> usize {
        match c {
            'a'..='z' => c as usize - 'a' as usize,
            '{' => 'z' as usize - 'a' as usize + 1,
            '_' => 'z' as usize - 'a' as usize + 2,
            '}' => 'z' as usize - 'a' as usize + 3,
            _ => 'z' as usize - 'a' as usize + 4,
        }
    }
    fn create_state(&self, c: char) -> SearchState {
        let position = Graph::calc_idx(c);
        let len = self.graph[position].len;
        SearchState { len, position }
    }
    pub fn traverse_web(&mut self, start: char, end: char) -> Result<u32, NotFoundError> {
        let mut heap = BinaryHeap::<SearchState>::new();
        let start_idx = Graph::calc_idx(start);
        for node in self.graph.iter_mut() {
            node.len = u32::MAX;
            node.visit_status = 0;
        }

        self.graph[start_idx].len = 1;
        self.graph[start_idx].visit_status = 1;
        heap.push(self.create_state(start));
        while let Some(SearchState { len: _, position }) = heap.pop() {
            let mut cur = self.graph[position];
            if cur.value == end {
                return Ok(cur.len);
            }
            cur.visit_status = 0;
            let mut neighbor_mapping = cur.neighbors;
            for i in 0..30 {
                if neighbor_mapping & 1 == 1 {
                    let alt_len = cur.len + self.graph[i].cost as u32;
                    if self.graph[i].len > alt_len {
                        self.graph[i].len = alt_len;
                        self.graph[i].visit_status = 1;
                        heap.push(self.create_state(self.graph[i].value));
                    }
                }
                neighbor_mapping >>= 1;
            }
        }
        Err(NotFoundError {})
    }
    pub fn new() -> Self {
        Graph {
            graph: [
                Node {
                    value: 'a',
                    cost: 17,
                    visit_status: 0,
                    len: 0,
                    neighbors: 450582092,
                },
                Node {
                    value: 'b',
                    cost: 58,
                    visit_status: 0,
                    len: 0,
                    neighbors: 8124552,
                },
                Node {
                    value: 'c',
                    cost: 16,
                    visit_status: 0,
                    len: 0,
                    neighbors: 21580451,
                },
                Node {
                    value: 'd',
                    cost: 22,
                    visit_status: 0,
                    len: 0,
                    neighbors: 326179780,
                },
                Node {
                    value: 'e',
                    cost: 46,
                    visit_status: 0,
                    len: 0,
                    neighbors: 1040138438,
                },
                Node {
                    value: 'f',
                    cost: 55,
                    visit_status: 0,
                    len: 0,
                    neighbors: 188312774,
                },
                Node {
                    value: 'g',
                    cost: 71,
                    visit_status: 0,
                    len: 0,
                    neighbors: 163642271,
                },
                Node {
                    value: 'h',
                    cost: 61,
                    visit_status: 0,
                    len: 0,
                    neighbors: 962627953,
                },
                Node {
                    value: 'i',
                    cost: 88,
                    visit_status: 0,
                    len: 0,
                    neighbors: 905065215,
                },
                Node {
                    value: 'j',
                    cost: 2,
                    visit_status: 0,
                    len: 0,
                    neighbors: 810681675,
                },
                Node {
                    value: 'k',
                    cost: 90,
                    visit_status: 0,
                    len: 0,
                    neighbors: 432607642,
                },
                Node {
                    value: 'l',
                    cost: 99,
                    visit_status: 0,
                    len: 0,
                    neighbors: 740675736,
                },
                Node {
                    value: 'm',
                    cost: 43,
                    visit_status: 0,
                    len: 0,
                    neighbors: 954321907,
                },
                Node {
                    value: 'n',
                    cost: 69,
                    visit_status: 0,
                    len: 0,
                    neighbors: 409780577,
                },
                Node {
                    value: 'o',
                    cost: 77,
                    visit_status: 0,
                    len: 0,
                    neighbors: 152539387,
                },
                Node {
                    value: 'p',
                    cost: 78,
                    visit_status: 0,
                    len: 0,
                    neighbors: 640379245,
                },
                Node {
                    value: 'q',
                    cost: 79,
                    visit_status: 0,
                    len: 0,
                    neighbors: 1069957269,
                },
                Node {
                    value: 'r',
                    cost: 29,
                    visit_status: 0,
                    len: 0,
                    neighbors: 532776632,
                },
                Node {
                    value: 's',
                    cost: 22,
                    visit_status: 0,
                    len: 0,
                    neighbors: 296762204,
                },
                Node {
                    value: 't',
                    cost: 79,
                    visit_status: 0,
                    len: 0,
                    neighbors: 1010044829,
                },
                Node {
                    value: 'u',
                    cost: 66,
                    visit_status: 0,
                    len: 0,
                    neighbors: 744992188,
                },
                Node {
                    value: 'v',
                    cost: 66,
                    visit_status: 0,
                    len: 0,
                    neighbors: 50499116,
                },
                Node {
                    value: 'w',
                    cost: 8,
                    visit_status: 0,
                    len: 0,
                    neighbors: 95180613,
                },
                Node {
                    value: 'x',
                    cost: 93,
                    visit_status: 0,
                    len: 0,
                    neighbors: 941883966,
                },
                Node {
                    value: 'y',
                    cost: 44,
                    visit_status: 0,
                    len: 0,
                    neighbors: 483363992,
                },
                Node {
                    value: 'z',
                    cost: 79,
                    visit_status: 0,
                    len: 0,
                    neighbors: 760504126,
                },
                Node {
                    value: '{',
                    cost: 90,
                    visit_status: 0,
                    len: 0,
                    neighbors: 824924578,
                },
                Node {
                    value: '_',
                    cost: 99,
                    visit_status: 0,
                    len: 0,
                    neighbors: 862325321,
                },
                Node {
                    value: '}',
                    cost: 46,
                    visit_status: 0,
                    len: 0,
                    neighbors: 172900081,
                },
                Node {
                    value: '\0',
                    cost: 15,
                    visit_status: 0,
                    len: 0,
                    neighbors: 531216514,
                },
            ],
        }
    }
}
