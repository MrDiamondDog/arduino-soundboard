import { NodeAudioVolumeMixer } from "node-audio-volume-mixer";

import { processes } from ".";
import { OnData } from "./display";

OnData.on("data", data => {
    if (!data.startsWith("changevolume")) return;

    const [_, name, volume] = data.split(" ");

    const process = processes.find(p => p.name === name);

    if (!process) return;

    process.volume = Number(volume) / 100;
    NodeAudioVolumeMixer.setAudioSessionVolumeLevelScalar(process.pid, process.volume);
});
