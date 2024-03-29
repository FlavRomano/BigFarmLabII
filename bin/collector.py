#! /usr/bin/env python3

import socket
import struct
import threading
from threading import Lock

HOST = "127.0.0.1"
PORT = 65201


class ClientThread(threading.Thread):
    def __init__(self, conn, addr, res, files):
        threading.Thread.__init__(self)
        self.conn = conn
        self.addr = addr
        self.res = res
        self.files = files
        
    def run(self):
        mutex = Lock()
        gestisci_connessione(self.conn, self.res, self.files, mutex)


def main(host=HOST, port=PORT):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        try:
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1) # per evitare [Errno 98] Address already in use 
            s.bind((host, port))
            s.listen()
            print("\t\t== Server attivo ==")
            res = dict()
            files = list()
            while True:
                conn, addr = s.accept()
                t = ClientThread(conn, addr, res, files)
                t.start()
        except KeyboardInterrupt:
            pass
        s.shutdown(socket.SHUT_RDWR)

            
def gestisci_connessione(conn, dic, files, mutex):
    with conn:
        data = recv_all(conn, 4)
        dim = struct.unpack("!i", data[:4])[0]
        copy_dic = dic.copy() # per evitare errori a Runtime
        # quando il client fa una richiesta al server prima che abbia concluso la totale
        # ricezione dei dati dalla farm (e quindi l'aggiornamento del dizionario)
        if dim > -1:
            farm_mess = ("").join([chr(struct.unpack("!i", recv_all(conn, 4)[:4])[0]) for _ in range(dim)])
            if ":" in farm_mess:
                ricezione_farm(farm_mess, dic, files, mutex)
            else:
                comunica_somma(farm_mess, conn, copy_dic, mutex)
        else:
            comunica_tutto(conn, copy_dic, mutex)
    

def ricezione_farm(s, dic, files, mutex):
    farm_mess = s.split(":")
    file_name = farm_mess[0]
    long = farm_mess[1]
    mutex.acquire()
    if long not in dic:
        files.append(file_name)
        dic[long] = file_name
    elif file_name not in files:
        files.append(file_name)
        dic[long] += f", {file_name}" 
    mutex.release()


def comunica_somma(somma, conn, dic, mutex):
    mutex.acquire()
    if somma not in dic:
        mess = f"{'Nessun file' : >12}\n"
        conn.sendall(struct.pack("!i", len(mess)))
        for char in mess:
            conn.sendall(struct.pack("!i", ord(char)))
    else:
        for long in dic:
            if long == somma:
                mess = ""
                file_name = dic.get(long)
                mess += (f"{long : >12} {file_name}\n")
                conn.sendall(struct.pack("!i", len(mess)))
                for char in mess:
                    conn.sendall(struct.pack("!i", ord(char)))
    mutex.release()


def comunica_tutto(conn, dic, mutex):
    mutex.acquire()
    if len(dic) == 0:
        mess = f"{'Nessun file' : >12}\n"
        conn.sendall(struct.pack("!i", len(mess)))
        for char in mess:
            conn.sendall(struct.pack("!i", ord(char)))
    else:
        mess = ""
        for long in dic:
            file_name = dic.get(long)
            mess += f"{long : >12} {file_name}\n"
        conn.sendall(struct.pack("!i", len(mess.encode('utf-8'))))
        for char in mess:
            conn.sendall(struct.pack("!i", ord(char)))
    mutex.release()

    
def recv_all(conn, n): 
  chunks = b''
  bytes_recd = 0
  while bytes_recd < n: 
    chunk = conn.recv(min(n - bytes_recd, 1024))
    if len(chunk) == 0:
      raise RuntimeError("socket connection broken")
    chunks += chunk
    bytes_recd = bytes_recd + len(chunk)
  return chunks


if __name__ == "__main__":
    main()