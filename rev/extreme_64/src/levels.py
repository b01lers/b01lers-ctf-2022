from platform import architecture
from qiling import Qiling

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
mov rdi,0x1337
"""

challenges.append(
    Challenge(
        archspec=amd64,
        instructions="Set rdi to 0x1337 using only one instruction.",
        testcases=[
            TestCase(
                codeconditions=[(lambda c: len(c.splitlines()) == 1, "too many instructions!"), (lambda c: c.count(";") <= 1, "too many instructions!")],
                preconditions=[lambda ql: setattr(ql.reg, "rdi", 0x0)],
                postconditions=[(lambda ql: ql.reg.rdi == 0x1337, "rdi != 0x1337")],
            )
        ],
        secret="code{very_1337}",
    )
)
"""
add rdi, rsi
mov rax, rdi
"""

challenges.append(
    Challenge(
        archspec=amd64,
        instructions="Add rdi to rsi and store the result in rax using two or less instructions.",
        testcases=[
            TestCase(
                codeconditions=[(lambda c: len(c.splitlines()) <= 2, "too many instructions!"), (lambda c: c.count(";") <= 2, "too many instructions!")],
                preconditions=[lambda ql: setattr(ql.reg, "rdi", 0x1123), lambda ql: setattr(ql.reg, "rsi", 0x5321)],
                postconditions=[(lambda ql: ql.reg.rax == 0x1123 + 0x5321, "rax != rdi + rsi!")],
            )
        ],
        secret="code{math_time}"
    )
)
"""
cmp rax, 0x1000
jne label1
mov rsi, 0x10
label1:
nop
"""

challenges.append(
    Challenge(
        archspec=amd64,
        instructions="Translate the following C-Like pseudocode to x86_64 assembly:",
        pseudocode="""
        if (rax == 0x1000) {
            rsi = 0x10;
        }""",
        testcases=[
            TestCase(
                preconditions=[lambda ql: setattr(ql.reg, "rax", 0x1000)],
                postconditions=[(lambda ql: ql.reg.rsi == 0x10, "rsi != 0x10")],
            ),
            TestCase(
                preconditions=[lambda ql: setattr(ql.reg, "rax", 0x2000)],
                postconditions=[(lambda ql: ql.reg.rsi == 0x0, "rsi != 0x0")],
            ),
        ],
        secret="code{control_flow_is_the_best}"
    )
)
"""
cmp rax, 0x1000
jne label1
mov rsi, 0x10
jmp label2
label1:
cmp rax, 0x3000
jne label2
mov rsi, 0x20
label2:
nop
"""

challenges.append(
    Challenge(
        archspec=amd64,
        instructions="Translate the following C-Like pseudocode to x86_64 assembly:",
        pseudocode="""
        if (rax == 0x1000) {
            rsi = 0x10;
        } else if (rax == 0x3000) {
            rsi = 0x20;
        }""",
        testcases=[
            TestCase(
                preconditions=[lambda ql: setattr(ql.reg, "rax", 0x1000)],
                postconditions=[(lambda ql: ql.reg.rsi == 0x10, "rsi != 0x10")],
            ),
            TestCase(
                preconditions=[lambda ql: setattr(ql.reg, "rax", 0x2000)],
                postconditions=[(lambda ql: ql.reg.rsi == 0x0, "rsi != 0x0")],
            ),
            TestCase(
                preconditions=[lambda ql: setattr(ql.reg, "rax", 0x3000)],
                postconditions=[(lambda ql: ql.reg.rsi == 0x20, "rsi != 0x20")],
            ),
        ],
        secret="code{we_c4n_d0_th1s_all_d4y}"
    )
)

"""
whileStart:
cmp rax, 0x0
jle whileExit
add rsi, rax
sub rax, 1
jmp whileStart
whileExit:
nop
"""

challenges.append(
    Challenge(
        archspec=amd64,
        instructions="Translate the following C-Like pseudocode to x86_64 assembly:",
        pseudocode="""
        while (rax > 0x0) {
            rsi += rax;
            rax--;
        }
        """,
        testcases=[
            TestCase(
                preconditions=[lambda ql: setattr(ql.reg, "rax", 0x42), lambda ql: setattr(ql.reg, "rsi", 0x13371337)],
                postconditions=[(lambda ql: ql.reg.rsi == sum(range(0x42,0x0,-0x1)) + 0x13371337, "the value of rsi isn't correct!")],
            ),
            TestCase(
                preconditions=[lambda ql: setattr(ql.reg, "rax", -0x1), lambda ql: setattr(ql.reg, "rsi", 0x13371337)],
                postconditions=[(lambda ql: ql.reg.rsi == 0x13371337, "while loop should not have executed!")],
            ),
        ],
        secret="code{l00p_the_l00p}"
    )
)


# challenges.append(
#     Challenge(
#         archspec=amd64,
#         instructions="Add rdi to rsi and store the result in rax using only one instruction.",
#         testcases=[
#             TestCase(
#                 codeconditions=[(lambda c: len(c.splitlines()) == 1, "too many instructions!"), (lambda c: c.count(";") <= 1, "too many instructions!")],
#                 preconditions=[lambda ql: setattr(ql.reg, "rdi", 0x1000), lambda ql: setattr(ql.reg, "rsi", 0x2000)],
#                 postconditions=[(lambda ql: ql.reg.rax == 0x3000, "rax != rdi + rsi!")],
#             )
#         ],
#         secret="code{some_instructions_have_many_uses}"
#     )
# )
