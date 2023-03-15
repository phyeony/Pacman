const fs = require('fs');
const http = require('http');
const path = require('path');
const mime = require('mime');

const server = http.createServer(function(request, response){
    let filePath;
    if(request.url === "/") {
        filePath = 'public/index.html'
    } else {
        filePath = `public/${request.url}`
    }
    const absPath = `./${filePath}`
    serveStatic(response, absPath);
})

const beatBoxServer = require("./lib/beatBoxServer");
// const udpSocket = require("./udpSocket");
beatBoxServer.listen(server);

const serveStatic = (response, absPath) => {
    fs.access(absPath, fs.R_OK, function (err) {
        if(err){
            send404(response);
        } else {
            fs.readFile(absPath, function (err, data) {
                if (err) {
                    send404(response);
                } else {
                    sendFile(response, absPath, data);
                }
            });
        }
    });
}

const sendFile = (response, absPath, fileContent) => {
    response.writeHead(200, {"content-type": mime.getType(path.basename(absPath))});
    response.end(fileContent)
}

const send404 = (response) => {
    response.writeHead(404, {'Content-Type': 'text/plain'});
    response.write('Error 404: resource not found.');
    response.end();
}

const PORT = 8080;

server.listen(PORT, function() {
    console.log(`Server listening on port ${PORT}`)
})