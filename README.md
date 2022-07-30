# BigFarm
Si chiede di realizzare un progetto che implementa lo schema di comunicazione tra processi e thread mostrato in figura:
<a href="https://ibb.co/9ZTYMNM"><img src="https://i.ibb.co/41TNQ7Q/farm.png" alt="farm" border="0"></a>
Il progetto è composto da tre processi, il primo in C denominato _MasterWorker_ (eseguibile `farm`); il secondo in Python denominato _Collector_ (eseguibile `collector.py`) il terzo in C denominato _Client_ (eseguibile `client`).

- _MasterWorker_ è un processo multi-threaded composto da un thread Master e da `n` thread Worker (il numero di thread Worker può essere variato utilizzando l’argomento opzionale `-n`, vedere nel seguito). Il programma prende come argomenti sulla linea di comando una lista di file binari contenenti interi lunghi (`long`) ed un certo numero di argomenti opzionali (opzioni `-n`, `-q` e `-t` discusse nelle note).
- Il processo _Collector_ deve essere lanciato indipendentemente dal processo _MasterWorker_ e non prende argomenti sulla linea di comando. I due processi comunicano attraverso una connessione socket `INET/STREAM` (TCP) usando per semplicità l’indirizzo `localhost`. Il processo _Collector_ deve svolgere il ruolo di server e deve essere in grado di gestire più client contemporaneamente, eventualmente usando più thread.
- Il processo _Client_ effettua delle interrogazioni al _Collector_ sui dati che ha ricevuto dal _MasterWorker_.

## MasterWorker
1. Il processo _MasterWorker_ legge i nomi dei file passati sulla linea di comando e li passa uno alla volta (con eventuali altri parametri) ai thread Worker mediante il meccanismo produttori/consumatori. Il generico thread Worker si occupa di leggere il contenuto del file ricevuto in input e di calcolare la somma:
![formula](https://render.githubusercontent.com/render/math?math=somma=\displaystyle\sum_{i=0}^{N-1}(i\cdot\text{file}[i]))
	dove `N` è il numero dei `long` nel file, `file[i]` è l'i-esimo `long` del file. Questo valore deve essere inviato, unitamente al nome del file, al processo _Collector_.
2. Deve gestire il segnale `SIGINT`. Alla ricezione di tale segnale il processo deve completare i task eventualmente presenti nel buffer/produttori consumatori e terminare dopo aver deallocato la memoria utilizzata e cancellato ogni eventuale file temporaneo. Se non viene inviato il segnale SIGINT, la procedura qui sopra deve essere seguita quando sono stati processati tutti i file passati sulla linea di comando.
## Client
Il processo _Client_ prende come input sulla linea di comando una sequenza di `long` e per ognuno di essi chiede al _Collector_ se ha ricevuto dal _MasterWorker_ nomi di file con associato quella somma (il _Client_ deve fare una richiesta distinta al server per ogni intero). 

Se invece il _Client_ viene invocato senza argomenti sulla linea di comando, deve inviare una richiesta speciale al _Collector_ di elencare tutte le coppie `somma`, `nomeFile` che lui ha ricevuto. 

In ogni caso, per ogni richiesta il _Client_ deve visualizzare su `stdout` le coppie `somma`, `nome` ricevute dal server, o la stringa `Nessun file` nel caso il server non contenga coppie che soddifano i requisiti. Per convertire i valori `long` passati sulla linea di comandi il _Client_ deve usare la funzione `atol(3)`.

## Collector
Il processo _Collector_ svolge il ruolo di un server e riceve tre tipi di richieste: 
- (1) Dai Worker la richiesta di memorizzare una data coppia `somma`, `nomefile`.
- (2) Dal _Client_ o la richiesta di elencare tutte le coppie, oppure (3) la richiesta di elencare le coppie con una data somma. 
Il _Collector_ non deve rispondere nulla al primo tipo di richiesta (quella dei Worker) mentre per le richieste di tipo 2 e 3 deve restituire l'elenco delle coppie `somma`, `nomefile` ordinate per somma crescente. Il server non termina spontaneamente ma rimane sempre in attesa di nuove interrogazioni.

==Fa parte dell'esercizio stabilire un protocollo per le interrogazioni al server, in quanto esso non può conoscere in anticipo quale tipo di richiesta riceverà di volta in volta (suggerimento: la richiesta dovrebbe inizare con un codice che ne indica il tipo).==

# Note
Gli argomenti che opzionalmente possono essere passati al processo MasterWorker sono i seguenti:
-   `-n nthread` specifica il numero di thread Worker del processo MasterWorker (valore di default 4).
-   `-q qlen` specifica la lunghezza del buffer produttori/consumatori (valore di default 8).
-   `-t delay` specifica un tempo in millisecondi che intercorre tra l’invio di due richieste successive ai thread Worker da parte del thread Master (serve per il debugging, valore di default 0).
Per leggere le opzioni sulla riga di comando utilizzare la funzione `getopt(3)`.
La dimensione dei file in input non è limitata ad un valore specifico. Si supponga che la lunghezza del nome dei file sia non superiore a 255 caratteri.

# Consegna
l repository deve contenere tutti i file del progetto oltre ai file menzionati sopra. Il makefile deve essere scritto in modo che la sequenza di istruzioni sulla linea di comando:
```
git clone git@github.com:user/progetto.git
make
./collector.py &      # parte il server in background
./client -1           # chiede coppie con somma -1
./farm z?.dat         # invia i file z0.dat e z1.dat
./client 9876543210 1 # chiede coppie con somma data
./client              # chiede tutte le coppie 
pkill collector.py    # termina il server 
```
non restituisca errori e generi l'output
```
Nessun file        (risposta alla richiesta di somma -1)
9876543210 z0.dat  (risposta alla richiesta di somma 9876543210)
Nessun file        (risposta alla richiesta di somma 1)

        -1 z1.dat  (risposta alla richiesta di tutte le coppie)
9876543210 z9.dat
```
Verificate su `laboratorio2.di.unipi.it`, _partendo da una directory vuota_, che non ci siano errori e che l'output corrisponda. Questo è un requisito _minimo_ per la sufficienza; altri test saranno fatti durante la correzione ma sempre su `laboratorio2`, e ovviamente sarà valutato anche il codice. Il programma deve gestire in maniera pulita evenutali errori (file non esistenti, server che non risponde etc.).

Il repository deve contenere anche un file README.md contenente una breve relazione che descrive le principali scelte implementative.
