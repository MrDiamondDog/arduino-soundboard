WScript.Sleep 2000
Set objShell = CreateObject("WScript.Shell")
objShell.CurrentDirectory = "C:/Users/drewr/OneDrive/Documents/Workspace/arduino-soundboard/out/"
objShell.Run """arduino-soundboard.exe""", 0 