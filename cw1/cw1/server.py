import math
import socket
import threading
from queue import Queue
import os
import struct
import json
import hashlib
import zipfile
import time

import serverMessage
from globalPara import *
from os import listdir
from os.path import isfile, join
updateOccurs = False
block_size = 10240
lock = threading.Lock()
# server will be listening for connection
class Server:

    def __init__(self, host, port, encryption):
        self.host = host
        self.port = port
        self.encrytion = encryption
        self.severSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.clients = []
        self.currentfiles = []
        self.files = {}
        self.filelists = {}
        self.diff_file = {}
        self.client1Queue = Queue()
        self.client2Queue = Queue()


    #listen for connection
    def listen(self):
        self.severSock.bind((self.host, self.port)) # bind the server socket
        self.severSock.listen(10) # listen for connection
        print(f"[SERVER] listening on {self.host} | {self.port}")

    #handle connection
    def handleCon(self):
        while True:
            conn, addr = self.severSock.accept() # accept connection from other clients, also blocking
            print(f"[SERVER] connected from [{addr[0]} | {addr[1]}]")
            conn.send(b"Successfuly Connected to Server") # send welcome message to client
            handleThread = threading.Thread(target=self.handle, args=(conn, ))
            handleThread.start()

    #handle task
    def handle(self,conn):
        listenUpdateThre = threading.Thread(target=self.updateFileList, args=(params, ))
        listenUpdateThre.start() # thread for updating the file list in current share folder
        while True:
            global updateOccurs
            if updateOccurs == True:
                self.sendFileList(conn)
                print(f"[SERVER] sent the newest file list to [CLIENT]")
            try: # try to recevie a request header from client
                self.recvRequest(conn)
            except:
                continue

    #check whether file list is updated
    def updateFileList(self):  # keep listening on share folder
        while True:
            try:
                self.traverse("share")
                if self.filelists != self.files:
                    self.filelists = self.files.copy()
                    self.currentfiles = list(self.filelists.keys())
                    global updateOccurs
                    updateOccurs = True
                    print("[SERVER-UPDATE] Share Folder modified ..")
            except:
                continue

    #send the newest file list
    def sendFileList(self, client_conn):  # send the file list to client
        global updateOccurs
        if len(self.currentfiles) != 0:
            files = ",".join(self.currentfiles)
            message = files.encode("utf-8")  # may need to construct a message factory class
            client_conn.send(message)
            #time.sleep(0.1)
            updateOccurs = False
        return

    #traverse share folder
    def traverse(self, dir_path):
        time.sleep(0.1)
        file_folder_list = listdir(dir_path)
        for file_folder_name in file_folder_list:
            file_path = join(dir_path, file_folder_name)
            if isfile(file_path) and file_path != 'share/test1.zip' and file_path != 'share/test2.zip' and os.path.getsize(file_path)!=0 and os.path.getsize(file_path) < 900000000 and not file_path.endswith('.temp'):
                self.files[file_path] = os.path.getmtime(file_path)
            if isfile(file_path) and os.path.getsize(file_path) > 900000000:
                zipThre1 = threading.Thread(target=self.makeZip1, args=(file_path,))
                zipThre1.start()
                zipThre1.join()
            if os.path.isdir(file_path):
                zipThre2 = threading.Thread(target=self.makeZip2, args=(file_path,))
                zipThre2.start()
                zipThre2.join()

        return

    #zip folder
    def makeZip1(self, file_path):
        lock.acquire()
        with zipfile.ZipFile(os.path.join('share', '1.zip'), 'w') as z:
            z.write(file_path)
        self.files['share/1.zip'] = 1
        lock.release()
        return

    #zip big file
    def makeZip2(self, file_path):
        lock.acquire()
        file_list = listdir(file_path)
        with zipfile.ZipFile(os.path.join('share', '2.zip'), 'w') as z:
            for f in file_list:
                if f != '.DS_Store':
                    z.write(join(file_path, f))
        self.files['share/2.zip'] = 1
        lock.release()
        return

    #receive request
    def recvRequest(self,conn):
        conn.setblocking(False)
        while True:
            headerlength = conn.recv(4)
            headerlength = struct.unpack("I", headerlength)
            jsonfile = conn.recv(headerlength[0])
            jsonfile = jsonfile.decode('utf-8')
            requestHeader = json.loads(jsonfile) # load json file to dict
            responsetHeader = self.responseHeader(requestHeader, conn)
            print(f"[SERVER] receive the header [{requestHeader}]")
            print(f"[SERVER] send the header [{responsetHeader}]")
            conn.send(responsetHeader)
        return

        # generater a response header back to client

    #receive response header
    def responseHeader(self, requestHeader, conn):
        client_opreation_code = requestHeader["client operation code"]
        #应该是client传过来的
        file_name = requestHeader["file name"]
        if client_opreation_code == 0: # request file information
            header = self.make_return_file_information_header(file_name)
            return header

        if client_opreation_code == 1: # request file block
            #应该是client传过来的
            block_index = requestHeader["block index"]
            header = self.make_file_block(file_name, block_index, conn)
            return header

    #check md5
    def md5(self, file):
        hash_md5 = hashlib.md5()
        with open(file,"rb") as f:
            for chunk in iter(lambda: f.read(4096), b""):
                hash_md5.update(chunk)
        return hash_md5.digest()

    #encrypt
    def _move_leter(self, letter, n):
        return chr((ord(letter) - ord('a') + n) % 26 + ord('a'))

    def Encrypt(self, k, p):
        letter_list = list(p.lower())
        c = ''.join([self._move_leter(x, k) for x in letter_list])
        return c

    #make information header
    def make_return_file_information_header(self, file_name):
        global block_size
        if os.path.exists(join('share', file_name)):  # find file and return information
            client_operation_code = 0
            server_operation_code = 0
            file_name = file_name
            file_size = self.get_file_size(file_name)
            total_block_number = math.ceil(file_size / block_size)
            md5 = self.get_file_md5(file_name)
            headerBuilder = serverMessage.ServerMessage(client_operation_code = client_operation_code,
                                          server_operation_code = server_operation_code,
                                          file_name = file_name,
                                          file_size = file_size,
                                          block_size =block_size,
                                          total_block_number = total_block_number,
                                          md5 = md5)
            return headerBuilder.infromationHeader()

        else:  # no such file
            client_operation_code = 0
            server_operation_code = 1
            headerBuilder = serverMessage.ServerMessage(client_operation_code=client_operation_code,
                                          server_operation_code=server_operation_code)
            return headerBuilder.nofileHeader()

    #make file block
    def make_file_block(self, file_name, block_index, conn):
        file_size = self.get_file_size(file_name)
        total_block_number = math.ceil(file_size / block_size)

        if os.path.exists(join('share', file_name)) is False:  # Check the file existence
            client_operation_code = 1
            server_operation_code = 1
            headerBuilder = serverMessage.ServerMessage(client_operation_code=client_operation_code,
                                          server_operation_code=server_operation_code)
            file_block_length = struct.pack('!I', 0)
            return file_block_length + headerBuilder.nofileHeader()

        if block_index < total_block_number:
            file_block, file_block_length = self.get_file_block(file_name, block_index)
            if self.encrytion == 'yes':
                file_block = self.Encrypt(1, file_block)
            client_operation_code = 1
            server_operation_code = 0
            headerBuilder = serverMessage.ServerMessage(client_operation_code=client_operation_code,
                                          server_operation_code=server_operation_code,
                                          block_index = block_index)
            file_block_length = struct.pack("I", file_block_length)
            return file_block_length + headerBuilder.fileHeader() + file_block
        else:
            client_operation_code = 1
            server_operation_code = 2
            headerBuilder = serverMessage.ServerMessage(client_operation_code=client_operation_code,
                                          server_operation_code=server_operation_code)
            file_block_length = struct.pack('!I', 0)
            return file_block_length + headerBuilder.nofileHeader()

    #get file block
    def get_file_block(self, filename, block_index):
        global block_size
        f = open(join('share', filename), 'rb')
        f.seek(block_index * block_size)
        file_block = f.read(block_size)
        f.close()
        return file_block, len(file_block)

    def get_file_size(self, filename):
        return os.path.getsize(join('share', filename))

    # get md5
    def get_file_md5(self, filename):
        f = open(join('share', filename), 'rb')
        contents = f.read()
        f.close()
        return hashlib.md5(contents).hexdigest()

