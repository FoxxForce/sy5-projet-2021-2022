Le programme comporte 2 modules principaux saturnd.c et cassini.c.
 
# Saturnd
 
## Création
Au démarrage, saturnd va créer une arborescence tenant les tubes des requêtes et des réponses, et le répertoire qui contiendra les données des tâches. Le fichier path.h contient les chemins qui définissent les emplacements où vont être créées les structures.
Dans le répertoire `/tmp` sera donc créés (s'ils n'existent pas) les répertoire `saturnd/pipes`, `saturnd/task` et les tubes `saturnd/pipes/saturnd-request-pipe`, `saturnd/pipes/saturnd-reply-pipe`.
 
## Traitement des requêtes
La boucle principale `while(1)` de saturnd exécute les tâches et traite les requêtes envoyées par cassini. Pour cela, saturnd va ouvrir le tube des requêtes dont l'emplacement est donné comme constante dans le fichier path.h (qui est aussi utilisé par cassini), en lecture non bloquante puis va attendre une écriture sur le tube en utilisant la fonction `poll`. Quand une requête est envoyée, saturnd appelle une fonction du fichier request-saturnd.c `read_request` pour la traiter. Selon la requête, la fonction read_request va utiliser l'une des fonctions du fichier write-pipe.c pour envoyer la réponse sur le tube des réponses donnés dans path.h.
 
## L'arborescence des tâches
Les données concernant les tâches sont stockées dans le répertoire `/tmp/saturnd/task`. A la création d'une tâche, un sous répertoire qui aura comme nom l'id de la tâche, sera créé dans `/tmp/saturnd/task`. Ce répertoire contient 4 fichiers:
- `command` : contient la commande de la tâche.
- `time` : contient les dates auxquelles la tâche doit être exécutée de la tâche (qui peut être convertie en timing).
- `exitcodes` : est vide (contiendra les dates et les codes de retour de l'exécution de la tâche).
- `id` : contient l'id de la tâche.
 
Les fonctions manipulant l'arborescence des tâches sont définies dans task.c.
 
## Traitement des requêtes
La boucle principale `while(1)` traite les requêtes envoyées par cassini. Pour cela, saturnd va ouvrir le tube des requêtes dont l'emplacement est donné comme constante dans le fichier path.h (qui est aussi utilisé par cassini), en lecture non bloquante puis va attendre une écriture sur le tube en utilisant la fonction `poll`. Quand une requête est envoyée, saturnd appelle une fonction du fichier request-saturnd.c `read_request` pour la traiter. Selon la requête, la fonction read_request va utiliser l'une des fonctions du fichier write-pipe.c pour envoyer la réponse sur le tube des réponses donnés dans path.h.
 
## Exécution des tâche
L'exécution des tâches est aussi faite dans la boucle principale `while(1)`. A chaque minute, saturnd va vérifier pour chaque tâche, si elle doit être exécutée en comparant le timing contenant dans le fichier time de la tâche à la date actuelle. Si la tâche doit être exécutée, alors on crée un processus fils qui va créer 3 fichiers dans le répertoire de la tâche :
- `pid`: contient le pid du processus fils.
- `stdout` : dans lequel on va rediriger la sortie standard.
- `stderr` : dans lequel on va rediriger la sortie erreur.
 
Il va ensuite écrire la date actuelle (le nombre de secondes depuis le premier janvier 1970) à la fin du fichier `exitcodes` de la tâche puis exécuter la commande contenu dans le fichier `command` de la tâche avec la fonction `execvp`.
 
Le poll au début du `while(1)` surveille aussi le côté écriture d'un tube anonyme. Quand un processus fils qui a exécuté une tâche à fini, il envoie un signal à son père (le processus principal qui exécute saturnd) ce qui provoque une écriture sur le tube anonyme (grâce à un handler définit sur le signal SIGCHLD avec la fonction `sigaction`) et réveille l'appel à `poll`. Ensuite avec la fonction `waitpid`, pour chaque exécution terminée, on récupère le pid et on écrit dans la la fin du fichier `exitcodes` de la tâche ayant le fichier `pid`(créé par les fils) contenant le pid récupéré par l'appel à waitpid puis le fichier `pid` est supprimé. 
 
 
# Cassini
 
Si l'option `-p` n'est pas utilisée, cassini utilise les chemins des tubes définies dans path.h.
 
## Envoi des requêtes
En premier lieu, il ouvre le tube des requêtes en écriture non bloquant et selon l'argument donné, il va écrire la requête dans le tube.
Pour la requête CR, cassini va créer un struct commandline qui représente la commande à envoyer. Il va utiliser la fonction `commandline_from_arguments` du fichier `commandline` qui va extraire la commande du tableau argv et la mettre dans la struct commandline qui va ensuite être envoyée à saturnd par le tube des requêtes avec le timing de la tâche en utilisant la fonction `write_request_cr` de `write-pipe.c`.
 
## Réception des réponses
Après avoir envoyé la requête, cassini ouvre le tube des réponses en lecture bloquante et utilise les fonctions définies dans read-pipe pour lire et afficher la réponse.
 
# Les modules auxiliaires
 
## commandline.c
Contient les fonctions créant et manipulant les struct commandline.
 
## write-pipe.c
Contient les fonctions pour lire les requêtes et les réponses dans les tubes.
 
## read-pipe.c
Contient les fonctions pour écrire les requêtes et les réponses dans les tubes.
 
## request-saturnd.c
Contient les fonctions qui traitent des requêtes et l'envoi des réponses par saturnd.
 
## task.c
Contient les fonctions manipulant les arborescences des tâches.

