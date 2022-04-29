from platform import architecture
from qiling import Qiling
from typing import Union

from asm_practice.coding.challenge import ArchSpec, Challenge, TestCase

challenges = []

amd64 = ArchSpec(
    pwntools_arch="amd64",
    pwntools_os="linux",
    qiling_rootfs="qiling/examples/rootfs/x86_linux/",
    qiling_ostype="linux",
    qiling_archtype="x8664",
)

"""
lea rax, [rdi+rsi] ;
"""

challenges.append(
    Challenge(
        archspec=amd64,
        instructions="Add rdi to rsi and store the result in rax using only one instruction.",
        testcases=[
            TestCase(
                codeconditions=[
                    (lambda c: len(c.splitlines()) == 1, "too many instructions!"),
                    (lambda c: c.count(";") <= 1, "too many instructions!"),
                ],
                preconditions=[
                    lambda ql: setattr(ql.reg, "rdi", 0x1000),
                    lambda ql: setattr(ql.reg, "rsi", 0x2000),
                ],
                postconditions=[(lambda ql: ql.reg.rax == 0x3000, "rax != rdi + rsi!")],
            )
        ],
        secret="code{some_instructions_have_many_uses}",
    )
)


class IOBuffer:
    def __init__(self) -> None:
        self.buffer = b""

    def write(self, ql, fd, buf, count, *args, **kw) -> int:
        self.buffer += ql.mem.read(buf, count)
        ql.os.set_syscall_return(count)


"""
mov rax, 0x616d72656b636168 ;
push rax ;
mov rsi, rsp ;
mov rdi, 0x1 ;
mov rdx, 0x8 ;
mov rax, 0x1 ;
syscall ;
mov rax, 0x6e ;
push rax ;
mov rsi, rsp ;
mov rdi, 0x1 ;
mov rdx, 0x1 ;
mov rax, 0x1 ;
syscall ;
"""

IOBuffer_0 = IOBuffer()
challenges.append(
    Challenge(
        archspec=amd64,
        instructions="Print the string 'hackerman' to stdout.",
        testcases=[
            TestCase(
                preconditions=[lambda ql: ql.set_syscall("write", IOBuffer_0.write)],
                postconditions=[
                    (
                        lambda ql: IOBuffer_0.buffer == b"hackerman" or print(f'stdout was {IOBuffer_0.buffer}, not \'hackerman\'!'),
                        f"",
                    )
                ],
            )
        ],
        secret="code{asm_c4n_d0_anything_java_can_do}",
    )
)

"""
mov rax, 0x0 ;
comp:
cmp rax, 0x20 ;
jg end;
mov bl, [rsi+rax] ;
xor bl, 0x82 ;
mov byte ptr [rsi+rax], bl ;
inc rax ;
jmp comp ;
end:
mov rax, 0x1 ;
mov rdi, 0x1 ;
mov rdx, 0x20 ;
syscall ;
"""

ADDR_0 =  0x10000000//0x1000*0x1000
IOBuffer_1 = IOBuffer()
challenges.append(
    Challenge(
        archspec=amd64,
        instructions="""
            Register rsi contains a pointer to the flag, but the 
            flag has been encoded by XOR-ing each byte with 0x82. 
            Decode it and print it out to complete the challenge!
            Hint: the flag is 32 characters long...
        """,
        testcases=[
            TestCase(
                preconditions=[
                    lambda ql: ql.mem.map(ADDR_0, 0x1000),
                    lambda ql: ql.mem.write(
                        ADDR_0,
                        b"\xe0\xe1\xf6\xe4\xf9\xe1\xb2\xec\xe5\xf0\xe3\xf6\xf1\xdd\xfb\xb2\xf7\xdd\xe3\xf0\xe7\xdd\xe3\xdd\xf2\xf0\xb2\xa3\xa3\xa3\xa3\xff",
                        # b"bctf{c0ngrats_y0u_are_a_pr0!!!!}"
                    ),
                    lambda ql: setattr(ql.reg, "rsi", ADDR_0),
                    lambda ql: ql.set_syscall("write", IOBuffer_1.write)
                ],
                postconditions=[(lambda ql: IOBuffer_1.buffer == b'bctf{c0ngrats_y0u_are_a_pr0!!!!}', 'didn\'t print out the encrypted flag!')],
            )
        ],
        secret="code{did_you_get_the_flag?}",
    )
)
