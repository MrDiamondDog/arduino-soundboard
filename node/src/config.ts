import fs from "fs";

function read(data: string) {
    return JSON.parse(data);
}

export let config: Config = read(fs.readFileSync("config.json").toString());
fs.watch("config.json", "utf8", (eventType, filename) => {
    if (eventType === "change") {
        config = read(fs.readFileSync("config.json").toString());
    }
});

interface Config {
    blacklistProcesses: string[];
    priorityProcesses: string[];
    heartbeatInterval: number;
    heartbeatExpire: number;
}
