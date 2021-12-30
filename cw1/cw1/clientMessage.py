import struct
import json


class ClientMessage: # a message class for client and server to communicate

    def __init__(self, client_operation_code, file_name = None, block_index = None):
        self.client_operation_code = client_operation_code # operation code sent by client
        self.file_name = file_name # filename
        self.block_index = block_index # requested block requested by client, should be None if client only wants the file information



    def requestInformationHeader(self):

        jsonHeader = {
            "client operation code": self.client_operation_code,
            "file name": self.file_name
        }
        jsonHeaderBytes = self.json_enocde(jsonHeader)
        headerlength = struct.pack("I", len(jsonHeaderBytes)) # this int is the length of jsonheader in bytes
        header = headerlength + jsonHeaderBytes
        return header

    def requestFileBlockHeader(self):

        jsonHeader = {
            "client operation code": self.client_operation_code,
            "file name": self.file_name,
            "block index": self.block_index
        }
        jsonHeaderBytes = self.json_enocde(jsonHeader)
        headerlength = struct.pack("I", len(jsonHeaderBytes)) # this int is the length of jsonheader in bytes
        header = headerlength + jsonHeaderBytes
        return header


    def json_enocde(self,jsonheader):
        return json.dumps(jsonheader, ensure_ascii=False).encode("utf-8")
