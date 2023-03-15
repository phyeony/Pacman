const socketio = require('socket.io');
let io;
const PORT =12345;

const dgram = require('dgram');

const udpServer = dgram.createSocket('udp4');

udpServer.on('error', (err) => {
  console.log(`udpServer error:\n${err.stack}`);
  udpServer.close();
});

udpServer.on('message', (msg, rinfo) => {
    console.log(`udpServer got message from ${rinfo.address}:${rinfo.port}:\n${msg}`);
    const [commandName, arg, ...rest] = msg.toString().split(/\s+/);
    io.emit(commandName, arg);
});

udpServer.on('listening', () => {
  const address = udpServer.address();
  console.log(`udpServer listening ${address.address}:${address.port}`);
});

udpServer.bind(PORT);

const udpClient = dgram.createSocket('udp4');
const BBG_IP_ADDRESS = "192.168.7.2"

exports.listen = (server) => {
    io = socketio(server);
    io.sockets.on('connection', (browserSocket) => {
            handleCommand(browserSocket)
        }
    )
}

const handleCommand = (browserSocket) => {
    const errorTimer = setTimeout(()=>{
        browserSocket.emit("serverError", "Time out", 5000);
    })

    browserSocket.on('healthcheck', () => browserSocket.emit('healthcheck', true))

    browserSocket.on('Chealthcheck', () => {
        // call C module
        const message = `healthcheck`;
        udpClient.send(message, PORT, BBG_IP_ADDRESS, (err) => {
            if (err) {
                console.error(`Error sending message: ${err}`);
            } else {
                console.log(`Message sent to ${BBG_IP_ADDRESS}:${PORT}: ${message}`);
                clearTimeout(errorTimer);
            }
        })
    })

    browserSocket.on('updateTempo', (data) => {
        const { increase } = data;
        // call C module
        const message = `updateTempo ${increase}`;
        udpClient.send(message, PORT, BBG_IP_ADDRESS, (err) => {
            if (err) {
                console.error(`Error sending message: ${err}`);
                // show error
                browserSocket.emit("updateTempoAnswerError", err);
            } else {
                console.log(`Message sent to ${BBG_IP_ADDRESS}:${PORT}: ${message}`);
                clearTimeout(errorTimer);
            }
        })
    })

    browserSocket.on('updateVolume', (data) => {
        const { increase } = data;
        // call C module
        const message = `updateVolume ${increase}`;
        udpClient.send(message, PORT, BBG_IP_ADDRESS, (err) => {
            if (err) {
                console.error(`Error sending message: ${err}`);
                // show error
            } else {
                console.log(`Message sent to ${BBG_IP_ADDRESS}:${PORT}: ${message}`);
                clearTimeout(errorTimer);
            }
        })
    })

    browserSocket.on('updateDrumBeatMode', (data) => {
        const {newBeatMode} = data;
        // call C module
        const message = `updateDrumBeatMode ${newBeatMode}`;
        udpClient.send(message, PORT, BBG_IP_ADDRESS, (err) => {
            if (err) {
                console.error(`Error sending message: ${err}`);
                // show error
            } else {
                console.log(`Message sent to ${BBG_IP_ADDRESS}:${PORT}: ${message}`);
                clearTimeout(errorTimer);
            }
        })
    })

    browserSocket.on('playDrumSound', (data) => {
        const {newDrumSound} = data;
        // call C module
        const message = `playDrumSound ${newDrumSound}`;
        console.log("SENDING ", message)
        udpClient.send(message, PORT, BBG_IP_ADDRESS, (err) => {
            if (err) {
                console.error(`Error sending message: ${err}`);
                // show error
            } else {
                console.log(`Message sent to ${BBG_IP_ADDRESS}:${PORT}: ${message}`);
                clearTimeout(errorTimer);
            }
        })
    })

    browserSocket.on('terminate', (data) => {
        // call C module 
        const message = `terminate`;
        udpClient.send(message, PORT, BBG_IP_ADDRESS, (err) => {
            if (err) {
                console.error(`Error sending message: ${err}`);
                // show error
            } else {
                console.log(`Message sent to ${BBG_IP_ADDRESS}:${PORT}: ${message}`);
                clearTimeout(errorTimer);
            }
        })
    })

    const requestDataUpdate = () => {
        // call C module 
        const message = `updateData`;
        udpClient.send(message, PORT, BBG_IP_ADDRESS, (err) => {
            if (err) {
                console.error(`Error sending message: ${err}`);
                // show error
            } else {
                console.log(`Message sent to ${BBG_IP_ADDRESS}:${PORT}: ${message}`);
                clearTimeout(errorTimer);
            }
        })
    }
    requestDataUpdate();
    //setInterval(requestDataUpdate, 900);

}

