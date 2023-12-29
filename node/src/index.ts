import "./server";
import "./volume";

import { ReadlineParser } from "@serialport/parser-readline";
import { SerialPort } from "serialport";

import { config } from "./config";
import { OnData, OnProcessesChange } from "./display";
import { getVolumeProcesses } from "./mixer";
import { Log } from "./utils";

export const portPath = "COM3";

export const port = new SerialPort({ path: portPath, baudRate: 9600 });
export const parser = port.pipe(new ReadlineParser());

export let awaitingHeartbeat = false;

export let canStop = false;

export function onWrite(e: Error | null | undefined) {
    if (e) {
        Log("Serial", "Error: " + e.message);
    }
}

export const { version } = require("../../package.json");
Log("Server", "Starting with version " + version);

export const processes = getVolumeProcesses();

// request new processes every 2.5 seconds
setInterval(() => {
    const newProcesses = getVolumeProcesses();

    if (newProcesses.map(p => p.pid).join("") !== processes.map(p => p.pid).join("")) {
        processes.splice(0, processes.length);
        processes.push(...newProcesses);

        OnProcessesChange.emit("change", processes);
    }
}, 2500);

port.on("open", () => {
    Log("Serial", "Port opened " + portPath);
});

port.on("close", () => {
    Log("Serial", "Port closed " + portPath);

    setTimeout(() => {
        Log("Serial", "Reopening port " + portPath);
        port.open();
    }, 1000);
});

parser.on("data", (data: string) => {
    data = data.trim();

    Log("Serial", data);
    OnData.emit("data", data);

    if (data === "Received data: ready") {
        canStop = true;
        Log("Server", "Arduino is ready");
    }

    if (data === "ready") {
        Log("Node", "ready;");
        port.write("ready;", onWrite);
    }

    if (data === "heartbeat" && awaitingHeartbeat) {
        awaitingHeartbeat = false;
        Log("Node", "Heartbeat received");
    }
});

export function sendHeartbeat() {
    Log("Node", "Heartbeat");
    port.write("heartbeat;", onWrite);
    awaitingHeartbeat = true;

    setTimeout(async () => {
        if (awaitingHeartbeat) {
            Log("Node", "Heartbeat timed out");
            port.flush();
            port.close();

            awaitingHeartbeat = false;
        }
    }, config.heartbeatExpire);
}

export let heartbeat = setInterval(sendHeartbeat, config.heartbeatInterval);

export function setHeartbeatInterval(timer: NodeJS.Timeout) {
    clearInterval(heartbeat);
    heartbeat = timer;
}

function handleDisconnect(reason: string, info?: string) {
    port.write(`disconnect ${reason};`, onWrite);
    if (info) {
        port.write(`disconnectinfo ${info};`, onWrite);
    }
    setTimeout(() => {
        process.exit(0);
    }, 1000);
}

process.on("SIGINT", () => handleDisconnect("SIGINT"));
process.on("exit", () => handleDisconnect("Exit"));
process.on("SIGUSR1", () => handleDisconnect("KillPID"));
process.on("SIGUSR2", () => handleDisconnect("KillPID"));
process.on("uncaughtException", (e, origin) => {
    Log("Server", "Uncaught Exception: " + e + " " + origin);
    handleDisconnect("Exception");
});
