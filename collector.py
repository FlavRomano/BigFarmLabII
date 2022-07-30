#! /usr/bin/env python3

import sys, struct, socket, threading
from threading import Lock

HOST = "127.0.0.1"
PORT = 65201


class ClientThread(threading.Thread):
    def __init__(self,conn,addr,res):
        threading.Thread.__init__(self)
        self.conn = conn
        self.addr = addr
        self.res = res
    def run(self):
        print("====", self.ident, "mi occupo di", self.addr)
        mutex = Lock()
        gestisci_connessione(self.conn, self.addr, self.res, mutex)
        print("====", self.ident, "ho finito")

def main(host=HOST, port=PORT):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        try:
            s.bind((host, port))
            s.listen()
            print("\t\t==Server attivo==\n\n")
            res = dict()
            while True:
                conn, addr = s.accept()
                t = ClientThread(conn, addr, res)
                t.start()
        except KeyboardInterrupt:
            pass
        print(f"\nDati ricevuti:",res)
        s.shutdown(socket.SHUT_RDWR)
        print("\nTermino")
            
            
def gestisci_connessione(conn, addr, dic, mutex):
    with conn:
        data = recv_all(conn, 4)
        dim = struct.unpack("!i", data[:4])[0]
        
        if dim != -1:
            s = "".join([chr(struct.unpack("!i", recv_all(conn,4)[:4])[0]) for i in range(dim)])
            print("\tHo ricevuto ", s)
            if ":" in s:
                arr = s.split(":")
                if arr[1] not in dic:
                    mutex.acquire()
                    dic[arr[1]] = arr[0]
                    mutex.release()
                else:
                    mutex.acquire()
                    dic[arr[1]] += f";{arr[0]}" 
                    mutex.release()
            else:
                print("RICHIESTA DEL CLIENT DI STAMPARE LE COPPIE")
                cerca_somma(s, conn, dic, mutex)
        else:
            print("RICHIESTA DEL CLIENT CERCARE UNA SOMMA")
            # print_coppie()
        print(f"Terminato con {addr}")
    

def cerca_somma(s, conn, dic, mutex):
    mutex.acquire()
    if len(dic) == 0:
        print("== Nessun file ==")
        s = "Nessun file"
        conn.sendall(struct.pack("!i", 11))
        for c in s:
            conn.sendall(struct.pack("!i", ord(c)))
    mutex.release()


def print_coppie():
    pass

    
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
