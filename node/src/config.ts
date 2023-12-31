import fs from "fs";

const configPath = fs.existsSync("config.json") ? "config.json" : "../config.json";

function read(data: string) {
    return JSON.parse(data);
}

export let config: Config = read(fs.readFileSync(configPath).toString());
fs.watch(configPath, "utf8", (eventType, filename) => {
    if (eventType === "change") {
        config = read(fs.readFileSync(configPath).toString());
    }
});

interface Config {
    blacklistProcesses: string[];
    priorityProcesses: string[];
    heartbeatInterval: number;
    heartbeatExpire: number;
}
