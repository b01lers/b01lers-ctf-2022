import random
from string import Template
import sys
from typing import Optional
from os import path,mkdir
from shutil import rmtree
import subprocess
ALPHA = "".join(
    [chr(v) for v in range(ord("a"), ord("z") + 1)]
    + [chr(v) for v in range(ord("A"), ord("Z") + 1)]
    + [chr(v) for v in range(ord("0"), ord("9") + 1)]
    + ["{", "}", "_"]
)


class Node:
    def __init__(self, value) -> None:
        self.left: Optional[Node] = None
        self.right: Optional[Node] = None
        self.value = value
        self.freq = 0


def insert_in_order(head: Optional[Node], val: Node) -> Node:
    new_node = Node(val)
    new_node.freq = val.freq
    if head is None:
        return new_node

    cur = head
    while cur.right is not None and cur.right.freq < new_node.freq:
        cur = cur.right
    new_node.right = cur.right
    cur.right = new_node
    return head


def build_tree(freqs) -> Node:
    list_head: Optional[Node] = None
    for i, f in enumerate(freqs):
        if f != 0:
            n = Node(i)
            n.freq = f
            list_head = insert_in_order(list_head, n)
    while list_head is not None and list_head.right is not None:
        left = list_head.value
        list_head = list_head.right
        right = list_head.value
        list_head = list_head.right
        internal = Node(0)
        internal.freq = left.freq + right.freq
        internal.left = left
        internal.right = right
        list_head = insert_in_order(list_head, internal)

    if list_head is not None:
        return list_head.value
    else:
        return None


def create_sample(output_dir: str = "."):
    freqs = [0] * 256
    for _ in range(1000):
        sel = random.choice(ALPHA)
        freqs[ord(sel)] += 1

    root = build_tree(freqs)
    key = "".join([random.choice(ALPHA) for _ in range(random.randrange(10, 25))])
    cur = root
    output = []
    for c in key:
        cur_char = ord(c)
        for _ in range(8):
            if cur.left is None and cur.right is None:
                output.append(cur.value)
                cur = root
            if (cur_char & 1) == 1:
                cur = cur.right
            else:
                cur = cur.left
            cur_char = cur_char >> 1
    d = {
        "number_freqs": freqs[ord("0") : ord("9") + 1],
        "lowercase_freqs": freqs[ord("a") : ord("z") + 1],
        "uppercase_freqs": freqs[ord("A") : ord("Z") + 1],
        "open_brack_freq": freqs[ord("{")],
        "close_brack_freq": freqs[ord("}")],
        "underscore_freq": freqs[ord("_")],
        "KEY_STR": "".join(map(chr, output)),
    }
    fmt =""
    with open("./snr.pact.template", "r") as f:
        src = Template(f.read())
        fmt = src.substitute(d)
    with open(path.join(output_dir, "snr_fmt.pact"), "w") as f:
        f.write(fmt)

def run(output_dir):
    subprocess.run(f"./pactc {path.join(output_dir, 'snr_fmt.pact')} {path.join(output_dir, 'snr_fmt.pactb')}".split())
    subprocess.run(f"xxd {path.join(output_dir, 'snr_fmt.pactb')}".split())
    rv = subprocess.run(f"./pactvm {path.join(output_dir, 'snr_fmt.pactb')}".split())
    return rv.returncode == 0

if __name__ == "__main__":
    root_dir = path.join("/tmp", str(random.randint(1000, 100000)))
    mkdir(root_dir)
    for _ in range(5):
        create_sample(root_dir)
        rv = run(root_dir)
        if not rv:
            print("Better luck next time!")
            rmtree(root_dir)
            sys.exit(1)
    with open("./flag.txt", "r") as f:
        print(f.read())
    rmtree(root_dir)
