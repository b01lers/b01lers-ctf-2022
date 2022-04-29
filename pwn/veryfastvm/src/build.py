#!/usr/bin/env python

import io
import tokenize
import os
import stat


#https://stackoverflow.com/questions/1769332/script-to-remove-python-comments-docstrings
def remove_comments_and_docstrings(source):
    """
    Returns 'source' minus comments and docstrings.
    """
    io_obj = io.StringIO(source)
    out = ""
    prev_toktype = tokenize.INDENT
    last_lineno = -1
    last_col = 0
    for tok in tokenize.generate_tokens(io_obj.readline):
        token_type = tok[0]
        token_string = tok[1]
        start_line, start_col = tok[2]
        end_line, end_col = tok[3]
        ltext = tok[4]
        # The following two conditionals preserve indentation.
        # This is necessary because we're not using tokenize.untokenize()
        # (because it spits out code with copious amounts of oddly-placed
        # whitespace).
        if start_line > last_lineno:
            last_col = 0
        if start_col > last_col:
            out += (" " * (start_col - last_col))
        # Remove comments:
        if token_type == tokenize.COMMENT:
            pass
        # This series of conditionals removes docstrings:
        elif token_type == tokenize.STRING:
            if prev_toktype != tokenize.INDENT:
        # This is likely a docstring; double-check we're not inside an operator:
                if prev_toktype != tokenize.NEWLINE:
                    # Note regarding NEWLINE vs NL: The tokenize module
                    # differentiates between newlines that start a new statement
                    # and newlines inside of operators such as parens, brackes,
                    # and curly braces.  Newlines inside of operators are
                    # NEWLINE and newlines that start new code are NL.
                    # Catch whole-module docstrings:
                    if start_col > 0:
                        # Unlabelled indentation means we're inside an operator
                        out += token_string
                    # Note regarding the INDENT token: The tokenize module does
                    # not label indentation inside of an operator (parens,
                    # brackets, and curly braces) as actual indentation.
                    # For example:
                    # def foo():
                    #     "The spaces before this docstring are tokenize.INDENT"
                    #     test = [
                    #         "The spaces before this string do not get a token"
                    #     ]
        else:
            out += token_string
        prev_toktype = token_type
        last_col = end_col
        last_lineno = end_line
    return out


def delf(fname):
    try:
        os.unlink(fname)
    except IOError:
        pass


with open("cpu_original.py", "r") as fp:
    cc = fp.read()
delf("cpu.py")


tt = remove_comments_and_docstrings(cc)
tt = "\n".join([l for l in tt.split("\n") if not l.strip().startswith("if debug>0: print")])
tt = "\n".join([l for l in tt.split("\n") if l.strip() != ""])
print(tt)

remove_tokens = ["execution_level", "registers", "clock", "cache", "secret_values", "ipointer", "parse_immediate","parse_register", "target", "mem_location", "memory_address", "o1", "o2", "MEMSIZE", "NREGS", "MAXTEXT", "MAXINST", "parse", "wlc"]
assert len(remove_tokens) < 100
remove_dictionary = {k:"X"+"%02d" % i for i,k in enumerate(remove_tokens)}
print(remove_dictionary)
nlines = []
for l in tt.split("\n"):
    for k,v in remove_dictionary.items():
        l = l.replace(k, v)
    nlines.append(l)
tt2 = "\n".join(nlines)

print(tt2)
outfname = "cpu.py"
with open(outfname, "w") as fp:
    fp.write(tt2)
st = os.stat(outfname)
os.chmod(outfname, st.st_mode | stat.S_IEXEC)
print(st)

