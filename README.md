# FILE SYSTEM FAT
#### Progetto d'esame per il corso di Sistemi Operativi a.a. 2023/2024

Questo progetto consiste nella simulazione di un File System di tipo **FAT** attraverso codice scritto in linguaggio C.

Un File System di tipo **FAT** (**F**ile **A**llocation **T**able) è caratterizzato da una divisione in blocchi del disco e da una tabella che mantiene traccia delle aree del disco disponibili e di quelle già in uso da file e cartelle. In questo caso il disco denominato **DISK** ha capienza 1.024.000 byte ed è suddiviso in blocchi denominati **BLOCK** da 1.024 byte ciscuno.

#### Avvio:
Per avviare il progetto è sufficiente eseguire il comando `make` e successivamente mandare in esecuzione il programma `exec` seguito da una stringa che sarà utilizzata come nome del disco. Se il nome è già stato utilizzato verranno ricaricati i dati precedentemente salvati altrimenti si creerà un nuovo disco. Il salvataggio dei dati avviene in automatico nel momento in cui viene utilizzata la funzione `quit`.

#### Funzionalità: 
L'interazione con la struttura dati avverrà unicamente tramite la simulazione di un terminale semplificata dalla libreria `linenoise`. Grazie al comando `help` sarà possibile leggere tutte le funzioni scritte per questa interazione:

- `quit` : fa terminare il FAT Project eliminando il disco 
- `clear` : pulisce il terminale
- `mk` : crea un file, ha bisogno di una stringa da assegnare come nome del file
- `rm` : elimina il file, ha bisgno di una stringa per identificare il file da eliminare
- `cat` : stampa il contenuto del file nello stdout
- `mkdir` : crea una cartella, ha bisogno di una stringa da assegnare come nome della cartella
- `rmdir` : elimina la cartella, ha bisgno di una stringa per identificare la cartella da eliminare
- `cd` : cambia la cartella di corrente
- `ls` : stampa a schermo la lista delle cartelle contenute in quella corrente
- `edit` : apre il file passato come argomento per la scrittura in fondo a questo
- `seek` : apre il file passato come argomento per la scrittura in un punto scelto di questo
- `rename` : permette di rinominare un file o una cartella che viene passato come argomento 

#### Nota a margine 

All'interno del file `TestoPerFile.txt` è presente una porzione del primo capitolo del romanzo di C. Collodi *Pinocchio*, per testare la scrittura su file e la stampa di quest'ultimo.


