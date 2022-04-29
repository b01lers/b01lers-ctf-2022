template = """state = malloc(sizeof(struct Node));
set_all_actions(dec_tape);
set_all_gotos(5);
state->gotos[{0}] = {1};
states[{2}] = state;"""

enc = "8v=t\x1a\x06\x1a{(\x1a\"=t90;>5;&)"
offset = 7
for i in range(len(enc)):
	print(template.format(ord(enc[i]), i+1+offset, i+offset))