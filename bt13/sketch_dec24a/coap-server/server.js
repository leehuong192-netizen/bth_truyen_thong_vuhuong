const coap = require('coap');
const server = coap.createServer();

server.on('request', (req, res) => {
  console.log('Received:', req.payload.toString());

  // KHÔNG phản hồi ACK trong lần test đầu
  // res.end('OK');
});

server.listen(() => {
  console.log('CoAP server started');
});