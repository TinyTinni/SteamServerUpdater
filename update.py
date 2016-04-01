import urllib.request
import time
import shlex, subprocess, os
from xml.dom.minidom import parse

def checkupdate(version):
    appid = "322330"
    response = urllib.request.urlopen('https://api.steampowered.com/ISteamApps/UpToDateCheck/v1?appid='+appid+'&version='+version+'&format=xml')
    doc = parse(response)
    elements = doc.getElementsByTagName('up_to_date')
    if (len(elements) > 0):
        return elements[0].firstChild.nodeValue == "true"
    return False


# workDir: path to the DST installation. Seperator has to be at the end
# binary: path from the workDir to the binary + binary name
# params: params
# returns: subprocess of DST
def startDSTServer(workDir, binary, params):
    command_line = binary + " " + params
    args = shlex.split(command_line)
    os.chdir(workDir+"\\bin")
    p = subprocess.Popen(args)
    os.chdir(workDir)
    return p

def stopDSTServer():
    c_shutdown()

def getInstalledVersion(workDir):
    f = open(workDir+"version.txt")
    return f.read()

def update(installedVersion, steamCMD):
    if (len(steamCMD) > 0):
        #todo
        pass
    else:
        time.sleep(30)
        while(checkUpdate(installedVersion)): #wait for the steam ui to update
            time.sleep(30)
    return

def main ():
    wrkDir = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Don't Starve Together Dedicated Server\\"
    binary = "dontstarve_dedicated_server_nullrenderer.exe"
    params = "-conf_dir DSTServer -console"
    steamCMD = "" #leave empty, if updates is done via steam ui

    #check if update is required
    installedVersion = getInstalledVersion(wrkDir)
    if (not checkupdate(installedVersion)):
        update()
        installedVersion = getInstalledVersion(wrkDir)

    #initial start
    process = startDSTServer(wrkDir, binary, params)

    while(process.poll() == None):
        if (not checkupdate(installedVersion)):            
            #check, if server is empty            
            listPlayers = c_listPlayers()
            while(len(listPlayers) != 0):
                listPlayers = c_listPlayers()
                time.sleep(10)

            #shutdown and restart
            stopDSTServer()
            update(installedVersion, steamCMD)
            instlledVersion = getInstalledVersion(wrkDir)
            startDSTServer(wrkDir, binary, params)
            print ("updated to version: " + installedVersion)
        print("current Version: " + str(installedVersion) + " is current: " + str(checkupdate(installedVersion)))
        time.sleep(10)

if __name__ == "__main__":
    main()