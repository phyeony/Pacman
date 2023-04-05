const socketio = require('socket.io');
let io;
const PORT =12345;
const UDP_SERVER_PORT = 3000; 

const dgram = require('dgram');

const udpServer = dgram.createSocket('udp4');

udpServer.on('error', (err) => {
  console.log(`udpServer error:\n${err.stack}`);
  udpServer.close();
});

udpServer.on('message', (msg, rinfo) => {
    console.log(`udpServer got message from ${rinfo.address}:${rinfo.port}:\n${msg}`);
    const [commandName, arg, ...rest] = msg.toString().split(/\s+/);
    console.log("commandName: ",commandName);
    console.log("args: ",arg);
    console.log("rest: ",rest);
    io.emit(commandName, arg, rest);
});

udpServer.on('listening', () => {
  const address = udpServer.address();
  console.log(`udpServer listening ${address.address}:${address.port}`);
});

udpServer.bind(UDP_SERVER_PORT);

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

    // update game map and current score
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
    setInterval(requestDataUpdate, 350);

}

