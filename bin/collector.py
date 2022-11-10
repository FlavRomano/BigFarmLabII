#! /usr/bin/env python3

import socket
import struct
import sys
import threading
from threading import Lock

HOST = "127.0.0.1"
PORT = 65201


class ClientThread(threading.Thread):
    def __init__(self,conn,addr,res, files):
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
            s = "".join([chr(struct.unpack("!i", recv_all(conn,4)[:4])[0]) for i in range(dim)])
            if ":" in s:
                arr = s.split(":")
                name = arr[0]
                l = arr[1]
                mutex.acquire()
                if l not in dic:
                    files.append(arr[0])
                    dic[arr[1]] = arr[0]
                if l in dic and name not in files:
                    files.append(arr[0])
                    dic[arr[1]] += f" {arr[0]}" 
                mutex.release()
            else:
                cerca_somma(s, conn, dic, mutex, 1)
        else:
            print_coppie(conn, dic, mutex)
    

def cerca_somma(s, conn, dic, mutex, flag):
    mutex.acquire()
    f = 0
    if len(dic) == 0 or flag == 0:
        f = 1
        mess = f"{'Nessun file' : >12}\n"
        conn.sendall(struct.pack("!i", len(mess)))
        for c in mess:
            conn.sendall(struct.pack("!i", ord(c)))
    else:
        for k in dic:
            if k == s:
                res = ""
                res += (f"{k : >12} {dic.get(k)}\n")
                f = 1
                conn.sendall(struct.pack("!i", len(res)))
                for c in res:
                    conn.sendall(struct.pack("!i", ord(c)))
    mutex.release()
    if f == 0:
        cerca_somma(s, conn, dic, mutex, f)


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

    
def recv_all(conn,n): 
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
    if len(sys.argv) == 1:
        main()
    elif len(sys.argv) == 2:
        main(sys.argv[1])
    elif len(sys.argv) == 3:
        main(sys.argv[1], int(sys.argv[2]))
    else:
        print("Uso:\n\t %s [host] [port]" % sys.argv[0])