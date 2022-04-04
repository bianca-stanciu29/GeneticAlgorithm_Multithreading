STANCIU BIANCA-ANDREEA, 335CC
                                 Algoritmi paraleli si distribuiti
                             Tema #1 Paralelizarea unui algoritm genetic

      In cadrul acestei teme am pornit de la skeletul dat, in versiunea secventiala, si de la laboratorul 3.
      Pentru a optimiza si a-l face mai rapid am creat mai multe threaduri, numarul de threaduri extragandu-l din linia
de comanda. Pentru ca fiecare thread sa lucreze pe aceleasi argumente, am creat o structura care contine: objects,
object_count, generation_count, sack_capacity, id, numThreads, vectorii de current_generation, next_generation si
un vector necesar in cadrul sortarii, si o bariera la care sa aiba acces fiecare thread.
      Dupa citirea datelor se parcurge fiecare thread si i se atribuie fiecarui element din structura valorile necesare.
Tot in tema1_par am creat si distrus bariera dupa terminarea folosirii acesteia.
      Pentru executia in paralel, am paralelizat functia de run_genetic_algoritm, functia de compute_fitness_function si
am folosit mergeSort-ul paralelizat in locul qsort-ului initial. Celelalte operatii neparalelizate se vor executa doar
pentru un thread(threadul 0 care va exista intotdeauna). Dupa fiecare secventa de cod paralelizata cu ajutorul
formulelor de start si end, am pus o bariera pentru ca toate thread-urile sa se execute simultan, pentru a se asigura
executia corecta.



