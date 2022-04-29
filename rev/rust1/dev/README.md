### Rust1

* This challenge creates a 'monster' that the player must fight using
fire, ice, or wind. Monster starts out with a major health advantage
over the player, so arbitrary 'powerups' are supported. The menu
says that the monster must be beaten the 'correct' way, which hints
that beating the monster (attacks are generated via RNG) is not enough.

* In order to win, the player has to find a powerup that satisfies the
following:
	* The string length must be 22
	* Only uses ascii printable characters
	* First four chars sum up to 0x115
	* Next four sum up to 0x1337.
	* Remaining chars. have constraints on them, see main.rs
	for more details.
		
