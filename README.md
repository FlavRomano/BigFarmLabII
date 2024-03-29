# MasterWorker (farm.c)
```c
void gen_params(int argc, char *argv[], int *nthread, int *qlen, int *delay)
```

Il programma prende da linea di comando:
- Argomenti opzionali:
	- n = specifica il numero di thread WORKER del processo. Di default è 4.
	- q = specifica la lunghezza del buffer prod/cons. Di default è 8.
	- t = specifica il delay che intercorre tra due richieste successive al thread master.
- Nomi file.

Questa funzione usa `getopt` per aggiornare i riferimenti ai parametri della farm secondo quanto immesso da terminale.

## Segnali
La funzione `sigaction` è chiamata per specificare cosa fare quando arriva il segnale `SIGINT`, visto che non devo mascherare particolari segnali o fare gestioni complicate di essi, posso limitarmi ad abbinargli un handler che ho definito in modo tale da far terminare anticipatamente il processo però prima completando i task nel buffer. In particolare ho dichiarato una variabile globale `volatile sig_atomic_t sign = 0` che funge da flag all'interno del *for* dove il thread master invia i nomi dei file, quando viene inviato un `SIGINT` la guardia del *for* viene violata e si passa alla terminazione dei thread.
### Perché `volatile`?
Viene utilizzato per assicurarsi che il contenuto della variabile venga letto dalla sua posizione effettiva in memoria piuttosto che da un registro della CPU. In modo tale da evitare che il compilatore applichi ottimizzazioni su variabili che possono cambiare in modi non determinabili dal compilatore stesso e.g un segnale lanciato dall'utente.
## Semafori
Utilizzo 3 semafori:
1. `sem_data_items` il cui valore viene inizializzato a 0, conterà il numero di elementi inseriti nel buffer.
2. `sem_free_slots` il cui valore viene inizializzato a `q_len`, conterrà il numero di slot liberi all'interno del buffer.
3. `mutex` semaforo mutex (è un semaforo normale inizializzato a 0) viene utilizzato per serializzare l'accesso all'interno della sezione critica della funzione invocata dai thread worker.
## Struct
Ovviamente devo passare una struct (`t_args`) al thread worker che contenga tutto ciò che gli serve per operare, in particolare:
- `int *cindex` puntatore all'indice del buffer che consumerà il generico worker. Condiviso fra tutti i worker.
- `char **buffer` puntatore all'inizio del buffer. Condiviso fra tutti i worker.
- `int *buf_len` puntatore alla lunghezza del buffer. Condiviso fra tutti i worker.
- `mutex, sem_free_slots, sem_data_items` puntatori ai semafori. Condivisi fra tutti i worker.
## Master
È il thread che ha il compito di copiare all'interno del buffer i nomi dei file passati da linea di comando. Prima di copiare la stringa nel buffer viene chiamata una `wait` su `sem_free_slots` cosicché il thread master possa sospendersi se il buffer risulti pieno e svegliarsi solo quando viene decrementato il semaforo tramite una `post`. 
Dopo la copia all'interno del buffer, viene chiamata una post su `sem_data_items` così da svegliare eventuali thread worker (consumatori) sospesi dopo aver trovato il buffer vuoto.
## Worker
Sono un insieme di thread che "consumano" i file scritti dal Master nel buffer. Il loro compito è leggere il contenuto dei file i quali nomi stanno nel buffer e calcolare la somma
$$\sum_{i=0}^{N-1}(i\cdot\text{file}[i]))$$ 
`N` è il numero dei `long` nel file, `file[i]` è l'i-esimo `long` del file. Questa somma viene inviata al processo collector insieme al nome del file nel formato `file_name:long_sum`.
### Gestione concorrenza
Viene utilizzato il classico pattern produttore-consumatore dove:
1. Viene chiamata una `wait` su `sem_data_items` per sospendere un worker in caso di buffer vuoto (non ha elementi da consumare).
2. Viene acquisita la lock tramite il semaforo `mutex` così da poter entrare in sezione critica garantendo mutua esclusione.
3. Rilascio la lock.
4. Viene chiamata una `post` su `sem_free_slots` per svegliare l'eventuale thread Master sospeso perché il buffer era pieno.
### Invio al collector
Utilizzo la funzione `void send_to_collector(char *mess, size_t mess_len)` per inviare un risultato prodotto da un worker al processo Collector. 
Uso i socket ma non posso inviare la stringa così com'è.

Invio la lunghezza della stringa `mess_len` tramite socket al collector (cioè il numero di byte che deve leggere il server). Invio carattere per carattere al server la mia stringa.

Visto che un'operazione di scrittura può restituire meno di quanto specificato (e.g buffer pieno del kernel); si dovrebbe continuare a scrivere il resto dei dati (ma con una `write` ciò non accade). Per questo motivo utilizziamo la funzione `writen`, definita *ad hoc* per invocare `write` un numero di volte necessario a scrivere tutti gli `N` byte di dati.
### Terminazione thread
Invio il carattere `"_"` dal master ai worker per far capire che devono fermarsi. Successivamente chiamo una `pthread_join` per tutti i thread così da:
	
1. Aspettare che i thread finiscano.
2. Liberare le risorse associate al thread.

# Client (.c)
Il client comunica con il collector. Può prendere in input da linea di comando dei long oppure nulla:
- Per ogni long passato da linea di comando restituisce il risultato comunica al collector la dimensione del long convertita in *network byte order* (grazie a `htonl`). Con una `readn` legge la dimensione in byte della stringa che il collector vuole restituirgli che viene convertita in *network byte order* (grazie a `ntohl`) così da poter leggere carattere per carattere tutta la stringa poi inviata dal collector.
- Se non passo nulla allora riceve tutti i risultati ottenuti dalla farm, legge la dimensione della stringa che vuole inviare il collector e, carattere per carattere, viene scritta dal client per poi essere restituita all'utente.
# Collector (.py)
È il server in ascolto sulla porta 65201 della macchina.  Nel `main` usando `with [...] as` alla fine del blocco il socket viene chiuso automaticamente. Dentro il while, con `accept` il server si blocca fino a quando arriva un client; all'arrivo di un client vengono inizializzate le variabili `conn` e `addr`:
- `conn` è un oggetto connessione usato per gestire la connessione.
- `addr` è l'indirizzo del client.
Se riceve un'interruzione `SIGINT` (chiamata `KeyboardInterrupt` in python) allora il server chiude il socket e si spegne.

La classe `ClientThread` estende la classe `Thread`, in particolare aggiungo `res` che contiene il dizionario con associazioni `(long: file_name)` ricevuti dal processo `farm`.  Ho assunto che il nome di un file possa comparire solo una volta all'interno del valore di una data chiave (se la `farm` invia due volte gli stessi risultati, il contenuto di `res` rimarrà invariato dopo il primo inoltro). La classe contiene un metodo `run` che viene invocato non appena viene chiamato il metodo `start()` del thread.

Dentro il metodo `run()` inizializzo un semaforo `mutex` (usato per garantire mutua esclusione dentro la sezione critica dove si accede al dizionario di risultati) e chiamo `gestisci_connessione()`. 

Questa funzione permette lo scambio di dati tra client e server, dal client può ricevere due tipi di richieste. 
- Se il server riceve $-1$ allora il client sta richiedendo tutte le coppie `long: file_name`.
- Altrimenti il server riceve la lunghezza del long, che servirà per ricevere, cifra dopo cifra, l'intero long; dopodiché invierà al client le varie coppie
che corrispondono a quel long.
