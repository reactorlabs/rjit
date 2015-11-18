import os
import sys
import subprocess

testpaths = "/home/paley/NEU/R/rjit5/gnur/tests"
tp = "/home/paley/NEU/R/rjit5/rjit/timer/"
timername = sys.argv[1]
timerpath = os.path.join(tp, timername)

visited = []

if sys.argv[2] == 'new':
    timerfile = open(timername, "w")
else: 
    if os.path.isfile(timerpath):
        timer = open(timername, "r")
        for line in timer:
            filename = line.split(':')[0]
            visited.append(filename)
        timer.close()
        timerfile = open(timername,"a")
    else: 
        timerfile = open(timername, "w")

try:
    for files in os.listdir(testpaths):
        filepath = os.path.join(testpaths, files)
        if files.endswith(".R") and not(files in visited):
            print(files)
            size = float(os.path.getsize(filepath)) / (1024)
            envs = os.environ.copy()
            envs['R_LIBS_USER'] = "../packages"
            envs['R_ENABLE_JIT'] = "5"        
            time = subprocess.Popen(["../../gnur/bin/R", "-f", filepath], 
                env=envs, stdout=subprocess.PIPE, 
                stderr=sys.stderr)
            count = 0
            for lines in time.stdout:
                print(lines)
                if "cpu sec" in lines:
                    count = count + float(lines.split(' ')[0])
            timerfile.write(files +": " + str(count) + " size: " + str(size) + "\n")
            timerfile.flush()
except Exception:
    pass
finally:
    timerfile.close()

