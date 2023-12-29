import { EventEmitter } from "node:events";

import fs from "fs";

export const Logs: Log[] = [];
export const OnLog = new EventEmitter();

fs.writeFileSync("logs.txt", "");

export function DateStr(date: Date): string {
    const hours = date.getHours() > 12 ? date.getHours() - 12 : date.getHours();
    let minutes: string | number = date.getMinutes();
    minutes = minutes < 10 ? `0${minutes}` : minutes;

    let seconds: string | number = date.getSeconds();
    seconds = seconds < 10 ? `0${seconds}` : seconds;

    const ampm = date.getHours() >= 12 ? "PM" : "AM";

    return `${hours}:${minutes}:${seconds} ${ampm}`;
}

export const Log = (ctx: string, ...str: string[]) => {
    const log = {
        ctx,
        message: str,
        time: new Date()
    };

    Logs.push(log);

    OnLog.emit("new", log);

    fs.appendFileSync("logs.txt", `[${DateStr(new Date())}] [${ctx}] ` + str.join(" ") + "\n");

    console.log(`[${ctx}] ` + str.join(" "));
};

export interface Log {
    ctx: string;
    time: Date;
    message: string[];
}
