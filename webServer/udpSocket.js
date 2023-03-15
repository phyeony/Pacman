const dgram = require('dgram');

const udpSocket = dgram.createSocket('udp4');

udpSocket.on('error', (err) => {
  console.log(`udpSocket error:\n${err.stack}`);
  udpSocket.close();
});

udpSocket.on('message', (msg, rinfo) => {
  console.log(`udpSocket got message from ${rinfo.address}:${rinfo.port}:\n${msg}`);
});

udpSocket.on('listening', () => {
  const address = udpSocket.address();
  console.log(`udpSocket listening ${address.address}:${address.port}`);
});

udpSocket.bind(12345);

module.exports = udpSocket;