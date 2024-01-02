import { NodeAudioVolumeMixer } from "node-audio-volume-mixer";

import { config } from "./config";

export interface VolumeProcess {
    pid: number;
    aliases: number[];
    name: string;
    volume: number;
}

export function getVolumeProcesses(): VolumeProcess[] {
    const processes: VolumeProcess[] = [];
    const foundProcesses: string[] = [];

    const audioSessions = NodeAudioVolumeMixer.getAudioSessionProcesses();

    audioSessions.forEach(session => session.name = session.name.replace(".exe", "").replace(".EXE", ""));

    audioSessions.forEach(session => {
        if (foundProcesses.includes(session.name))
            return;

        const aliases = processes.filter(process => process.name === session.name);

        foundProcesses.push(session.name);
        foundProcesses.push(...aliases.map(alias => alias.name));

        processes.push({
            pid: session.pid,
            aliases: aliases.map(alias => alias.pid),
            name: session.name,
            volume: NodeAudioVolumeMixer.getAudioSessionVolumeLevelScalar(session.pid)
        });
    });

    config.blacklistProcesses.forEach(process => {
        const foundProcess = processes.find(p => p.name === process);

        if (foundProcess) {
            processes.splice(processes.indexOf(foundProcess), 1);
        }
    });

    config.priorityProcesses.forEach(process => {
        const foundProcess = processes.find(p => p.name === process);

        if (foundProcess) {
            processes.splice(processes.indexOf(foundProcess), 1);
            processes.unshift(foundProcess);
        }
    });

    return processes;
}
