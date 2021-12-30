import argparse
from globalPara import *
from threading import Thread
import client, server
import multiprocessing as mp
from multiprocessing import Process
from multiprocessing.managers import BaseManager


class Chatroom():
    def __init__(self):
        """
        初始化服务端
        """
        ips, encryption = self.parser_parameters()

        params = {}#mp.Manager().dict({})
        params['ips'] = ips
        params['encryption'] = encryption
        params['server_port'] = SERVER_PORT
        t1 = Process(target=self.connectServer, args=(params,))
        t1.daemon = True
        t1.start()

        t2 = Process(target=self.connectClient, args=(params,0))
        t2.daemon = True
        t2.start()

        t3 = Process(target=self.connectClient, args=(params,1))
        t3.daemon = True
        t3.start()
        print("server start, waiting for connection")
        while True:
            pass


    def parser_parameters(self):
        parser = argparse.ArgumentParser()
        parser.add_argument('--ip', help='ip addresses for two vms')
        parser.add_argument('--encryption', default='no', help=('yes or no'))
        args = parser.parse_args()
        print(args.ip.split(','))
        return args.ip.split(','), args.encryption

    def connectServer(self, params: dict):
        s = server.Server("", params['server_port'], params['encryption'])
        s.listen()
        try:
            s.handleCon()
        except Exception as e:
            print(e)

    def connectClient(self, params: dict,i):
        c = client.Client(params['ips'][i], params['server_port'], params['encryption'])
        try:
            c.connectServer()
        except Exception as e:
            print(e)


if __name__ == '__main__':
    demo = Chatroom()
