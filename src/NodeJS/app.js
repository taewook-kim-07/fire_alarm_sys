const fs = require('fs');
const http = require('http');
const https = require('https');
const express = require('express');
const WebSocket = require('ws');

const app = express();

app.all('*', (req, res, next) =>
{
	let protocol = req.headers['x-forwarded-proto'] || req.protocol;
	if (protocol == 'https') {
		next(); 
	} else { 
		let from = `${protocol}://${req.hostname}${req.url}`;
		let to = `https://${req.hostname}${req.url}`;
		// log and redirect
		console.log(`[${req.method}]: ${from} -> ${to}`);
		res.redirect(to);
	} 
});

app.get('/', function (req, res) {
  res.sendFile(__dirname + '/index.html');
});

const options = {
ca: fs.readFileSync('/etc/letsencrypt/live/donga.eglu.top/fullchain.pem'),
key: fs.readFileSync('/etc/letsencrypt/live/donga.eglu.top/privkey.pem'),
cert: fs.readFileSync('/etc/letsencrypt/live/donga.eglu.top/cert.pem')
};
http.createServer(app).listen(8080);
var httpsServer = https.createServer(options, app).listen(443);

/* 웹 소켓 포트번호 TCP */
const LISTENPORT = 443;
const webSocketServer = new WebSocket.Server({ server: httpsServer, autoAcceptConnections: false }); //port: LISTENPORT, clientTracking: true });

var ControlNets = [ ];

var temp = 0, co2 = 0,
	maxtemp = 80, maxco2 = 1500, alarmstatus = 0;


function isControlNets(ws){
	var result = false;
	for(var i=0; i<ControlNets.length; i++)
	{
		if(ControlNets[i] == ws)
		{
			result = true;
			break;
		}
	}
    return result;
}

webSocketServer.SendToTarget = function(ws, message) {
	webSocketServer.clients.forEach(function(client) {
		if(ws == client) {
			client.send(message);
			return false;
		}
	})
}

webSocketServer.SendToBrowserWithoutMe = function(ws, message) {
	webSocketServer.clients.forEach(function(client) {
		if(!isControlNets(client) && ws != client)
			client.send(message);
	})
}

webSocketServer.SendToBrowser = function(message) {
	webSocketServer.clients.forEach(function(client) {
		if(!isControlNets(client))
			client.send(message);
	})
}

webSocketServer.SendToControlNets = function(message) {
	webSocketServer.clients.forEach(function(client) {
		if(isControlNets(client))
		{
			client.send(message);
			return false;
		}
	})
}

webSocketServer.on('connection', function connection(ws, req) {
	let ip = req.headers['x-forwarded-for'] || req.connection.remoteAddress;
	console.log("Client Connected (" + ip + ")");

	ws.on('close', function() {
		try {
			for(var i = 0; i < ControlNets.length; i++) {
				if(ControlNets[i] == ws) {
					ControlNets.splice(i, 1);
					console.log("ControlNet Disconnected (" + ip + ")");
					break;
				}
			}
			
			console.log("Client Disconnected (" + ip + ")");
		}catch{}
	});
	
		//webSocketServer.SendToAll("test메세지");
	ws.on('message', function incoming(message) {
		try {
			if(message == "controlnet_init") {
				if(!isControlNets(ws)) {
					ControlNets.push(ws);
					webSocketServer.SendToControlNets("OK");
					console.log("ControlNet Connected (" + ip + ")");
				}
			} else if(message == "request_data") {				
				webSocketServer.SendToTarget(ws, "temp|" + temp);
				webSocketServer.SendToTarget(ws, "co2|" + co2);
				webSocketServer.SendToTarget(ws, "mtemp|" + maxtemp);
				webSocketServer.SendToTarget(ws, "mco2|" + maxco2);
				webSocketServer.SendToTarget(ws, "alarm|" + alarmstatus);
				webSocketServer.SendToTarget(ws, "server|" + ControlNets.length);
			} else {
				if(isControlNets(ws))
				{
					var splitedMSG = message.split('|');
					if(splitedMSG.length == 2)
					{
						var isValidData = true;
						switch(splitedMSG[0])
						{
							case 'temp': temp = splitedMSG[1]; break;
							case 'co2': co2 = splitedMSG[1]; break;
							case 'alarm': alarmstatus = splitedMSG[1]; break;
							default: isValidData = false;
						}
						if(isValidData)
							webSocketServer.SendToBrowser(message);
					}else
						console.log("(" + ip + ") : "+ message);
				}else{	// 웹브라우저에서 ATMEGA로
					var splitedMSG = message.split('|');
					if(splitedMSG.length == 2)
					{
						var isValidData = true;
						switch(splitedMSG[0])
						{
							case 'mtemp': maxtemp = splitedMSG[1]; webSocketServer.SendToBrowserWithoutMe(ws, "mtemp|" + maxtemp); break;
							case 'mco2': maxco2 = splitedMSG[1]; webSocketServer.SendToBrowserWithoutMe(ws, "mco2|" + maxco2); break;
							case 'salarm': break;
							default: isValidData = false;
						}
						if(isValidData)
							webSocketServer.SendToControlNets(message);
						console.log(message);
					}else
						console.log("(" + ip + ") : "+ message);
				}
			}
		}catch{}
	});
});

console.log("Listen... (PORT: 80, 443)");