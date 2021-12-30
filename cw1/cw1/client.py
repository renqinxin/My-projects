import hashlib
import json
import math
import socket
import struct
import threading
import os
import zipfile

from tqdm import tqdm

from Crypto.Cipher import AES
import clientMessage
from os.path import isfile, join
from globalPara import *

file_dir = 'share'
detect_Zip = True
lock = threading.Lock()

class Client:
    def __init__(self, host, port, encryption):
        self.host = host  # server ip
        self.port = port  # server port
        self.encryption = encryption
        self.sockeClient = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # client socket
        self.filesTobeReq = []
        print(f"client process init {self.host}")



    def connectServer(self):
        # try to connect to server until server response
        print(f"client process start to connect {self.host}")
        while True:
            try:
                self.sockeClient.connect((self.host, self.port))
                message = self.sockeClient.recv(1024).decode("utf-8")
                if message == "Successfuly Connected to Server":
                    print(f"[CLIENT] {message}")
                    checkFile = threading.Thread(target=self.filelistRec)
                    checkFile.start()
                    #checkFile.join()
            except:  # server is not running currently
                continue

    def filelistRec(self):
        while True:
            try:
                filelist = self.sockeClient.recv(1024).decode("utf-8")
                if filelist != [''] and len(filelist)!=0:
                    filelist = filelist.split(',')
                    print(f"[ClIENT] recvie [FILE-LIST] from host {self.host} {self.port}--------- ")
                    self.filecheck(filelist)
                    unZipThre = threading.Thread(target=self.unzipFile)
                    unZipThre.start()
                    unZipThre.join()
            except Exception as e:
                print(f"[Client] error is [{e}]")
                self.sockeClient.close()
                break
        self.reconnect()


    def filecheck(self,filelist):
        for file in filelist:
            if isfile(file): # if file already exits in own sharefolder
                # check file information from server, check whether needs to be updated
                print(f"[CLIENT] has had this file [{file}]")
                self.requestFileInformation(file)
            elif isfile(file + '.temp'):
                print(f"[CLIENT] this file isn't finished [{file}]")
                self.requestFileInformationTemp(file)
            elif isfile(file) == False: # if the file not exit in own sharefolder
                # direct request to download the whole file
                print(f"[CLIENT] does not have this file [{file}]")
                self.requestFileDownload(file)

    #unzip file when detect a zip file
    def unzipFile(self):
        global detect_Zip, lock
        lock.acquire()
        try:
            if isfile('share/1.zip'):
                with zipfile.ZipFile('share/1.zip', 'r') as z:
                    for name in z.namelist():
                        z.extract(name)
                lock.release()
            os.remove('share/1.zip')
        except:
            pass
        try:
            if isfile('share/2.zip'):
                with zipfile.ZipFile('share/2.zip', 'r') as z:
                    for name in z.namelist():
                        z.extract(name)
                lock.release()
            os.remove('share/2.zip')
        except:
            pass

    def _move_leter(self, letter, n):
        return chr((ord(letter) - ord('a') + n) % 26 + ord('a'))

    def Dncrypt(self, k, c):
        letter_list = list(c.lower())
        p = ''.join([self._move_leter(x, -k) for x in letter_list])
        return p

    # make header for basic infromation
    def make_get_file_information_header(self, file_name):
        client_operation_code = 0
        file_name = file_name
        headerBuilder = clientMessage.ClientMessage(client_operation_code = client_operation_code, file_name = file_name)
        return headerBuilder.requestInformationHeader()

    #make header for download
    def requestFileInformation(self,file_name):
        _, file_name = file_name.split('/')
        print(file_name)
        print(f"[CLIENT-REQUEST]: [{file_name}] exits in current share folder, looking for more information from sever ...")
        headerRequestFileInformation = self.make_get_file_information_header(file_name)
        self.sockeClient.send(headerRequestFileInformation)
        header = self.recvInformation()
        self.parseMoreInformation(header, file_name)

    #make header for temp file
    def requestFileInformationTemp(self,file_name):
        _, file_name = file_name.split('/')
        print(file_name)
        print(f"[CLIENT-REQUEST]: [{file_name}] exits in current share folder, looking for more information from sever ...")
        headerRequestFileInformation = self.make_get_file_information_header(file_name)
        self.sockeClient.send(headerRequestFileInformation)
        header = self.recvInformation()
        self.parseMoreInformationTemp(header, file_name)

    # make header for block
    def make_get_fil_block_header(self, file_name, block_index):
        client_operation_code = 1
        file_name = file_name
        block_index = block_index
        headerBuilder = clientMessage.ClientMessage(client_operation_code=client_operation_code, file_name=file_name, block_index = block_index)
        return headerBuilder.requestFileBlockHeader()

    #request for download
    def requestFileDownload(self,file_name):
        _, file_name = file_name.split('/')
        print(f"[CLIENT-REQUEST]: request [TRANSFER] [{file_name}] from sever")
        headerRequestFileInformation = self.make_get_file_information_header(file_name)
        self.sockeClient.send(headerRequestFileInformation)
        header = self.recvInformation()
        self.parseInformation(header, file_name)

    #reauest for information
    def recvInformation(self):
        headerlength = self.sockeClient.recv(4)
        headerlength = struct.unpack("I", headerlength)
        jsonfile = self.sockeClient.recv(headerlength[0])
        jsonfile = jsonfile.decode('utf-8')
        header = json.loads(jsonfile)  # load json file to dict
        return header

    #receive file
    def recvFile(self):
        file_block_length_b = self.sockeClient.recv(4)
        file_block_length_t = struct.unpack("I", file_block_length_b)
        file_block_length = file_block_length_t[0]

        header_length_b = self.sockeClient.recv(4)
        header_length_t = struct.unpack("I", header_length_b)
        header_length = header_length_t[0]

        jsonfile = self.sockeClient.recv(header_length)
        jsonfile = jsonfile.decode('utf-8')
        header = json.loads(jsonfile)  # load json file to dict

        file_block = self.sockeClient.recv(file_block_length, socket.MSG_WAITALL)


        return file_block, header

    #parse the file block
    def parse_file_block(self, header, file_block):
        client_operation_code = header["client operation code"]
        server_operation_code = header["server operation code"]
        if server_operation_code == 0:  # get right block
            block_index = header["block index"]
            file_block = file_block
        elif server_operation_code == 1:
            block_index, block_length, file_block = -1, -1, None
        elif server_operation_code == 2:
            block_index, block_length, file_block = -2, -2, None
        else:
            block_index, block_length, file_block = -3, -3, None

        return block_index, file_block

    #get the file md5
    def get_file_md5(self, filename):
        global file_dir
        f = open(join(file_dir, filename), 'rb')
        contents = f.read()
        f.close()
        return hashlib.md5(contents).hexdigest()

    #parse information
    def parseInformation(self, header, file_name):
        file_size = header["file size"]
        block_size = header["block size"]
        total_block_number = header["total block number"]
        md5 = header['md5']
        if file_size > 0:
            print('Filename:', file_name)
            print('File size:', file_size)
            print('Block size:', block_size)
            print('Total block:', total_block_number)
            print('MD5:', md5)

            f = open(os.path.join(file_dir, file_name+'.temp'), 'wb')
            for block_index in tqdm(range(total_block_number)):
                header_send =  self.make_get_fil_block_header(file_name, block_index)
                self.sockeClient.send(header_send)
                print(f"[CLIENT] send header [{header_send}]")
                file_block, message = self.recvFile()
                block_index_from_server, file_block= self.parse_file_block(message, file_block)
                if self.encryption == 'yes':
                    file_block = self.Dncrypt(1, file_block)
                f.write(file_block)
            f.close()

            # Check the MD5
            md5_download = self.get_file_md5(file_name)
            if md5_download == md5:
                print('Downloaded file is completed.')
                os.rename(os.path.join(file_dir, file_name+'.temp'), os.path.join(file_dir, file_name))

            else:
                print('Downloaded file is unfinished.')

        else:
            print('No such file.')

    #parse more information
    def parseMoreInformation(self, header, file_name):
        file_size = header["file size"]
        block_size = header["block size"]
        total_block_number = header["total block number"]
        md5 = header['md5']
        my_md5 = self.get_file_md5(file_name)
        file_new_name = os.path.join(file_dir, file_name)
        my_size = os.path.getsize(file_new_name)
        my_block = math.ceil(my_size / block_size)
        if my_md5!=md5:
            print("md5 is not same, request new download")
            print('Filename:', file_name)
            print('File size:', file_size)
            print('Block size:', block_size)
            print('Total block:', total_block_number)
            print('MD5:', md5)

            f = open(file_new_name, 'rb+')
            for block_index in tqdm(range(total_block_number)):
                self.sockeClient.send(self.make_get_fil_block_header(file_name, block_index))
                file_block, message = self.recvFile()
                block_index_from_server, file_block = self.parse_file_block(message, file_block)
                if self.encryption == 'yes':
                    file_block = self.Dncrypt(1, file_block)
                    f.write(file_block)
                f.close()

                # Check the MD5
            md5_download = self.get_file_md5(file_name)
            if md5_download == md5:
                print('Downloaded file is completed.')

            else:
                print('Downloaded file is broken.')
        if my_md5 == md5:
            print('This file has been downloaded')
        '''else:
            print('this file has broken')'''

    #parse more temp information
    def parseMoreInformationTemp(self, header, file_name):
        file_size = header["file size"]
        block_size = header["block size"]
        total_block_number = header["total block number"]
        md5 = header['md5']
        my_md5 = self.get_file_md5(file_name + '.temp')
        file_new_name = os.path.join(file_dir, file_name+'.temp')
        my_size = os.path.getsize(file_new_name)
        my_block = math.ceil(my_size / block_size)
        if my_md5!=md5:
            if my_block < total_block_number:
                print("block number is not the same", my_block)
                print('Filename:', file_name)
                print('File size:', file_size)
                print('Block size:', block_size)
                print('Total block:', total_block_number)
                print('MD5:', md5)
                f = open(file_new_name, 'ab')
                for block_index in tqdm(range(my_block, total_block_number)):
                    self.sockeClient.send(self.make_get_fil_block_header(file_name, block_index))
                    file_block, message = self.recvFile()
                    block_index_from_server, file_block= self.parse_file_block(message, file_block)
                    if self.encryption == 'yes':
                        file_block = self.Dncrypt(1, file_block)
                    f.write(file_block)
                f.close()

                # Check the MD5
                md5_download = self.get_file_md5(file_new_name)
                if md5_download == md5:
                    print('Downloaded file is completed.')
                    os.rename(file_new_name, file_name)

                else:
                    print('Downloaded file is broken.')

        if my_md5 == md5:
            print('This file has been downloaded')

    #reconnect
    def reconnect(self):
        print("[ClIENT] lost connection with server")
        print("[ClIENT] trying to reconnect to server...")
        self.sockeClient = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        self.connectServer()

