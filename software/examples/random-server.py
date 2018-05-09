#!/usr/bin/python

import shlex
import subprocess
from threading import Thread
import time
import web

urls = (
    '/', 'help',
    '/get', 'GetRandomData',
#    '/reg', 'Register',
    '/status', 'GetPoolStatus'
)

RESPONSE_SIZE=8192

MIN_POOL_SIZE=1  # in MB
MAX_POOL_SIZE=32 # in MB

class help:
    def GET(self):
        message= "<html><head>"
        message+= "<title>Infinite Noise webservice</title></head>"
        message+= "<body><h1>Usage:</h1><table border=1>"
        message+= "<tr><td><b>request</b></td><td><b>help</b></td></tr>"
        message+= "<tr><td>/status</td><td>show current buffer status</td></tr>"
        message+= "<tr><td>/get</td><td>get " + str(RESPONSE_SIZE) + "bytes (binary format)</td></tr>"
#        print "<td><tr>/reg</tr><tr>todo</tr></td>"
        message+= "</table>"
        message+= "</body></html>"
	return message

class GetRandomData:
    def GET(self):
        return POOL.getData()

class GetPoolStatus:
    def GET(self):
        return POOL.getStatus()

class RandPool(object):
    minPoolSize = (MIN_POOL_SIZE * 1024 * 1024) / RESPONSE_SIZE
    maxPoolSize = (MAX_POOL_SIZE * 1024 * 1024) / RESPONSE_SIZE
    poolFillmark = -1
    randomPool = list()
    process = None
    do_run = True

    def __init__(self):
        command = "sudo /usr/sbin/infnoise --multiplier 10"
        self.process = subprocess.Popen(shlex.split(command), stdout=subprocess.PIPE)
        for i in range(0, self.minPoolSize):
            self.randomPool.append(self.process.stdout.read(RESPONSE_SIZE))
            self.poolFillmark+= 1
        thread2 = Thread(target=self.monitorPool)
        thread2.start()

    def monitorPool(self):
        while getattr(self, "do_run", True):
            if self.poolFillmark < self.maxPoolSize: # keep pool at 100%
                print(str(self.poolFillmark)+ "/" + str(self.maxPoolSize))
                self.__refill_pool__()
            time.sleep(1)
        print("Stopping as you wish.")
        self.process.kill()

    def __refill_pool__(self):
        print("refilling pool - current level: " + str(self.poolFillmark))
        for i in range(self.poolFillmark, self.maxPoolSize):
            self.randomPool.append(self.process.stdout.read(RESPONSE_SIZE))
            self.poolFillmark += 1
        self.randomPool.reverse()
        print("refilled pool - new level: " + str(self.poolFillmark))

    def getData(self):
        if self.poolFillmark <= 0:
            web.ctx.status = '503 Service currently unavailable - Out of entropy error'
            return "0"
        value = self.randomPool[self.poolFillmark]
        self.randomPool[self.poolFillmark] = None
        self.poolFillmark -= 1
        return value

    def getStatus(self):
        return str(self.poolFillmark) + "/" + str(self.maxPoolSize)

POOL = RandPool()

if __name__ == '__main__':
    app = web.application(urls, globals())
    app.run()
    POOL.do_run = False

