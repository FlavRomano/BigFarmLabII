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
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1) # for avoiding [Errno 98] Address already in use 
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
        if dim > -1:
            farm_mess = ("").join([chr(struct.unpack("!i", recv_all(conn, 4)[:4])[0]) for _ in range(dim)])
            if ":" in farm_mess:
                ricezione_farm(farm_mess, dic, files, mutex)
            else:
                cerca_somma(farm_mess, conn, dic, mutex)
        else:
            print_coppie(conn, dic, mutex)
    

def ricezione_farm(s, dic, files, mutex):
    farm_mess = s.split(":")
    file_name = farm_mess[0]
    long = farm_mess[1]
    mutex.acquire()
    if long not in dic:
        files.append(file_name)
        dic[long] = file_name
    if long in dic and file_name not in files:
        files.append(file_name)
        dic[long] += f", {file_name}" 
    mutex.release()


def cerca_somma(s, conn, dic, mutex):
    mutex.acquire()
    if s not in dic:
        mess = f"{'Nessun file' : >12}\n"
        conn.sendall(struct.pack("!i", len(mess)))
        for c in mess:
            conn.sendall(struct.pack("!i", ord(c)))
    else:
        for k in dic:
            if k == s:
                res = ""
                res += (f"{k : >12} {dic.get(k)}\n")
                conn.sendall(struct.pack("!i", len(res)))
                for c in res:
                    conn.sendall(struct.pack("!i", ord(c)))
    mutex.release()


def print_coppie(conn, dic, mutex):
    mutex.acquire()
    if len(dic) == 0:
        mess = f"{'Nessun file' : >12}\n"
        conn.sendall(struct.pack("!i", len(mess)))
        for c in mess:
            conn.sendall(struct.pack("!i", ord(c)))
    else:
        s = ""
        for k in dic:
            s += f"{k:>12} {dic.get(k)}\n"
        conn.sendall(struct.pack("!i", len(s.encode('utf-8'))))
        for c in s:
            conn.sendall(struct.pack("!i", ord(c)))
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