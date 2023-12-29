import { EventEmitter } from "node:events";

import { onWrite, port, processes } from ".";
import { getVolumeProcesses } from "./mixer";
import { Log } from "./utils";

export const OnData = new EventEmitter();
export const OnProcessesChange = new EventEmitter();

OnProcessesChange.on("change", () => {
    const data = processes.map(process => process.name).join("|");

    Log("Serial", "Sending processes: " + data);
    port.write(`response ${data} /${processes.length};`, onWrite);
});

OnData.on("data", data => {
    if (!data.startsWith("req processes")) return;

    Log("Display", "Processes requested");
    OnProcessesChange.emit("change", getVolumeProcesses());
});
