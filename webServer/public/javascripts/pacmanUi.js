"use strict";
const socket = io();

const MIN_VOLUME = 0;
const MAX_VOLUME = 100;
const MIN_BPM = 40;
const MAX_BPM = 300;

const NUM_ROW = 16;
const NUM_COLUMN = 32;

/* color mapping from C enum to Javascript
    typedef enum 
    {
        BLACK=0,
        RED,
        GREEN,
        YELLOW,
        BLUE,
        PINK,
        LIGHT_BLUE,
        WHITE,
        END,  // This will display BLACK.
    } Color;

    Not the best way since if the C enum changes, this need to change as well.
*/
const COLOR_ARRAY = [
    'black',
    'red',
    'green',
    'yellow',
    'blue',
    'pink',
    'light_blue',
    'white'
]

// create a new 2D array with all values set to 0 which is black.
let gameMap = new Array(NUM_ROW).fill(0).map(() => new Array(NUM_COLUMN).fill(0));


function initMap(tableElement, gameMap) {
    for (let i = 0; i < NUM_ROW; i++) {
        const row = $('<tr></tr>');
        for (let j = 0; j < NUM_COLUMN; j++) {
            const cell = $(`<td id=${COLOR_ARRAY[gameMap[i][j]]}></td>`);
            row.append(cell);
        }
        tableElement.append(row);
    }
}

function updateMap(tableElement, gameMap) {
    tableElement.find("td").each(function(idx) {
        $(this).attr("id", `${COLOR_ARRAY[gameMap[Math.floor(idx / NUM_COLUMN)][idx%NUM_COLUMN]]}`);
    });
}



$(document).ready(() => {
    // populate map
    const table = $('#game-map');
    initMap(table, gameMap);

    socket.on('updateMapAnswer', function (result) {
        // result is a string of numbers without any delimiters
        // Referred: https://stackoverflow.com/questions/6484670/how-do-i-split-a-string-into-an-array-of-characters
        console.log(result);
     
        // split string into 1d array and convert 1d array to 2d array
        [...result].forEach(function (value, idx) {
            gameMap[Math.floor(idx / NUM_COLUMN)][idx%NUM_COLUMN] = value;
        });
        updateMap(table, gameMap);
    })

    socket.on('updateCurrentScoreAnswer', function (result) {
        console.log("current RESULT: ", result);
        $("#currentScore").text(result);
    });
    socket.on('updateHighScoreAnswer', function (result) {
        console.log("high RESULT: ", result);
        $("#highScore").text(result);
    });
    
    // show error
    const showError = () => {
        $("#error-box").show();
    };

    const hideError = () => {
        $("#error-box").hide();
    };

    let numHealthChecked = 0;
    let numCServerHealthChecked = 0;

    // setInterval(() => {
    //     console.log("node js health num", numHealthChecked)
    //     if(numHealthChecked > 4) {
    //         showError();
    //     } 
    //     socket.emit('healthcheck')
    //     numHealthChecked++;
    // }, 1000);

    socket.on('healthcheck', function (result) {
        // Received every second to check node js health
        if (numHealthChecked > 4) {
            //server must've been down before reset.
            numHealthChecked = 0;
        } else if (numHealthChecked > 0) {
            numHealthChecked--;
        }
        if (numCServerHealthChecked < 4) {
            hideError();
        }
    })


    // setInterval(() => {
    //     console.log("c helath num", numCServerHealthChecked)
    //     if(numCServerHealthChecked > 4) {
    //         showError();
    //     } 
    //     socket.emit('Chealthcheck')
    //     numCServerHealthChecked++;
    // }, 1000);

    socket.on('Chealthcheck', function (result) {
        // Received every second to check c module's health
        if (numCServerHealthChecked > 4) {
            //server must've been down before reset.
            numCServerHealthChecked = 0;
        } else if (numCServerHealthChecked > 0) {
            numCServerHealthChecked--;
        }
        if (numHealthChecked < 4) {
            hideError();
        }
    })

    // volume
    $("#volumeDown").click(function () {
        const currentVal = $("#volumeid").val();
        if (currentVal === MIN_VOLUME) {
            return;
        }
        socket.emit("updateVolume", { increase: false });
    })

    $("#volumeUp").click(function () {
        const currentVal = $("#volumeid").val();
        if (currentVal === MAX_VOLUME) {
            return;
        }
        socket.emit("updateVolume", { increase: true });
    })

    socket.on('updateVolumeAnswer', function (result) {
        $("#volumeid").val(result)
    })

    // terminate
    $("#stop").click(function () {
        socket.emit("terminate");
        terminateErrorTimer = createErrorTimer();
    })

    socket.on('terminateAnswer', function (result) {
        // diplay error if it didn't work
    })

    socket.on("gameOverAnswer", function (result) {
        console.log("RESULT:gameover ",result)
        if(result==0){
            $("#game-over-text").hide();
        } else {
            $("#game-over-text").show();
        }
        
    })


})