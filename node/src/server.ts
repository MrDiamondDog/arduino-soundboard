import express from "express";
import path from "path";
import WebSocket from "ws";

import { Log, Logs, OnLog } from "./utils";
import fs from 'fs';

const port = 2345;
const wsPort = 2346;

const serverPath = fs.existsSync(path.resolve("./", "node/server")) ? path.resolve("./", "node/server") : path.resolve("../", "node/server");

const app = express();

Log("Server", "Serving static files from " + serverPath);
app.use(express.static(serverPath));

const wss = new WebSocket.Server({ port: wsPort });

wss.on("connection", ws => {
    Log("WS", "Client connected");

    ws.on("message", message => {
        if (message.toString() === "all") {
            Log("WS", "Sending all logs");
            ws.send("all:" + JSON.stringify(Logs));
        }
    });

    function onLog(log: any) {
        ws.send("new:" + JSON.stringify(log));
    }

    OnLog.on("new", onLog);

    ws.on("close", () => {
        Log("WS", "Client disconnected");

        OnLog.off("new", onLog);
    });
});

app.listen(port, () => {
    Log("Server", "Listening on port " + port);
});
