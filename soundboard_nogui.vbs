WScript.Sleep 2000
Set objShell = CreateObject("WScript.Shell")
objShell.CurrentDirectory = "/path/to/arduino-soundboard/out"
objShell.Run """arduino-soundboard.exe""", 0 