from typing import List, Optional, Union
import claripy
import random
flag = "bctf{are_you_satisfied_with_this_vm}"

def create_level() -> Optional[List[str]]:
    s = claripy.Solver()
    syms = [claripy.BVS("syms", 8) for _ in range(len(flag))]
    random_op = lambda: random.choice(["+", "-", "^"])

    for i in range(len("bctf{")):
        s.add(syms[i] == ord(flag[i]))
    s.add(syms[-1] == ord("}"))

    for sym in syms:
        s.add(sym >= ord(" "))
        s.add(sym <= 0x7e)

    checks = []
    for i, e in enumerate(flag):
        other_idx = random.choice([v for v in range(len(flag)) if v != i])
        op = random_op()
        equ_val = eval(f"ord(e) {op} ord(flag[other_idx])")
        s.add(eval(f"syms[i] {op} syms[other_idx] == equ_val"))
        checks.append(f"  if ((num_input[{i}] {op} num_input[{other_idx}]) != {equ_val}) {{ return 1; }}")

    reconst_flag = ""
    for sym in syms:
        soln = list(map(chr, s.eval(sym, 10)))
        if len(soln) != 1:
            return None
        reconst_flag += soln[0]
    if flag != reconst_flag:
        return None
    random.shuffle(checks)
    return checks

checks = None
while (checks := create_level()) is None:
    pass

check_str = "\n".join(checks)
with open("level1.pact", "w") as f:
    f.write(f"""//{flag}
fun main() {{
    var user_input = split(input("Flag Validator: "));
    if (len(user_input) != {len(flag)}) {{ return 1; }}
    var num_input = range(len(user_input));
    for (var i = 0; i < len(user_input); i = i + 1) {{
        num_input[i] = int(user_input[i]);
    }}
{check_str}
    return 0;
}}
if (main() == 0) {{
    print "Congrats!";
}} else {{
    print "Try again :(";
}}
""")
