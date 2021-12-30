import struct
import json

class ServerMessage:

    def __init__(self, client_operation_code, server_operation_code, file_name = None, file_size = None, block_size = None, total_block_number = None, md5 = None, block_index = None):
        self.client_operation_code = client_operation_code
        self.server_operation_code = server_operation_code
        self.file_name = file_name
        self.file_size = file_size
        self.block_size = block_size
        self.total_block_number = total_block_number
        self.md5 = md5
        self.block_index = block_index

    def infromationHeader(self):
        jsonheader = {
            "client operation code": self.client_operation_code,
            "server operation code": self.server_operation_code,
            "file name": self.file_name,
            "file size": self.file_size,
            "block size":self.block_size,
            "total block number":self.total_block_number,
            "md5":self.md5
        }
        informationheader = self.json_encode(jsonheader)
        headerlength = struct.pack("I",len(informationheader))
        header = headerlength + informationheader
        return header

    def nofileHeader(self):
        jsonheader = {
            "client operation code": self.client_operation_code,
            "server operation code": self.server_operation_code
        }
        nofileheader = self.json_encode(jsonheader)
        headerlength = struct.pack("I",len(nofileheader))
        header = headerlength + nofileheader
        return header


    def fileHeader(self):
        jsonheader = {
            "client operation code": self.client_operation_code,
            "server operation code": self.server_operation_code,
            "block index": self.block_index
        }
        fileheader = self.json_encode(jsonheader)
        headerlength = struct.pack("I", len(fileheader))
        header = headerlength + fileheader
        return header


    def json_encode(self,jsonheader):
        return json.dumps(jsonheader,ensure_ascii=False).encode("utf-8")
