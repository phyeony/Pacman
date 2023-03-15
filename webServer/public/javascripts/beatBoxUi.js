"use strict";
const socket = io();

const MIN_VOLUME = 0;
const MAX_VOLUME = 100;
const MIN_BPM = 40;
const MAX_BPM = 300;

const NUM_ROW = 16;
const NUM_COLUMN = 32;

function populateMap(tableElement) {
    for (let i = 0; i < NUM_ROW; i++) {
        const row = $('<tr></tr>');
        for (let j = 0; j < NUM_COLUMN; j++) {
            const cell = $('<td></td>');
            row.append(cell);
        }
        tableElement.append(row);
    }
}


$(document).ready(() => {
    // populate map
    const table = $('#game-map');
    populateMap(table);


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

    // mode
    $("#modeNone").click(function () {
        socket.emit("updateDrumBeatMode", { newBeatMode: "NONE" });
    })

    $("#modeRock1").click(function () {
        socket.emit("updateDrumBeatMode", { newBeatMode: "ROCK_1" });
    })

    $("#modeRock2").click(function () {
        socket.emit("updateDrumBeatMode", { newBeatMode: "ROCK_2" });
    })


    socket.on('updateDrumBeatModeAnswer', function (result) {
        // expected result is one of "none", "rock 1", "rock 2"
        $("#modeid").text(result)
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

    // tempo
    $("#tempoDown").click(function () {
        const currentVal = $("#tempoid").val();
        if (currentVal === MIN_BPM) {
            return;
        }
        socket.emit("updateTempo", { increase: false });
    })

    $("#tempoUp").click(function () {
        const currentVal = $("#tempoid").val();
        if (currentVal === MAX_BPM) {
            return;
        }
        socket.emit("updateTempo", { increase: true });
    })

    socket.on('updateTempoAnswer', function (result) {
        $("#tempoid").val(result)
    })

    // drum sounds
    $("#hiHatSound").click(function () {
        socket.emit("playDrumSound", { newDrumSound: "HI-HAT" });
    })
    $("#snareSound").click(function () {
        socket.emit("playDrumSound", { newDrumSound: "SNARE" });
    })
    $("#baseSound").click(function () {
        socket.emit("playDrumSound", { newDrumSound: "BASE" });
    })

    socket.on('playDrumSoundAnswer', function (result) {
        // display error if it didn't work.
    })

    // terminate
    $("#stop").click(function () {
        socket.emit("terminate");
        terminateErrorTimer = createErrorTimer();
    })

    socket.on('terminateAnswer', function (result) {
        // diplay error if it didn't work
    })

    // beatbox up time

    socket.on('updateDeviceUpTimeAnswer', function (result) {
        // diplay error if it didn't work
        // result is a double value in seconds.
        const hours = Math.floor(result / 3600);
        const minutes = Math.floor((result % 3600) / 60);
        const secondsRemainder = Math.floor(result % 60);
        $('#uptime').text(`${hours}:${minutes}:${secondsRemainder}`)
    })
})