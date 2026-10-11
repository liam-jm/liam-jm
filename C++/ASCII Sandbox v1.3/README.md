## ASCII Sandbox v1.3

C'est un de mes premiers projets en C++. Un jeu entièrement fonctionnel dans un terminal.

![Capture d'écran de l'écran d'accueil](images/img_001.png)

C'est un jeu très simple. Vous disposez de 3 mondes. Vous pouvez en choisir un en appuyant sur `SPACE`, déplacer le curseur de sélection avec les flèches gauches ou droites du clavier (`<-`/`->`) et supprimer/réinitialiser un monde en appuyant 5 fois sur `ESC`. Ces mondes sont d'ailleurs rangés dans des fichiers en `.txt`, situés dans le dossier `saves`, et `world_0` représente le monde par défaut.

![Capture d'écran d'un monde](images/img_002.png)

Chaque monde est fait de 32x16 caractères, entouré d'un cadre de dièses (`#`) incassables. Les contrôles sont :
- Z : monter
- Q : aller à gauche
- D : aller à droite
- S : reculer
- F : poser un bloc (`#`)
- R : détruire un bloc (`#`)

![Capture d'écran d'un monde avec des blocs posés par le joueur](images/img_003.png)

Chaque action actualise l'écran intégralement, donc si vous essayez de scroller vers le haut, vous verrez toutes vos actions précédentes.

Le jeu s'appelle ASCII Sandbox car il utilise des caractères ASCII dans un monde sandbox, un monde où la limite est la créativité du joueur.

![Capture d'écran d'une maison de blocs, faite par un joueur](images/img_004.png)

Et vous pouvez donc casser des blocs, sauf les barrières de dièses (`#`) vertes.

![Capture d'écran d'une légende, montrant le fonctionnement de la destruction des blocs](images/img_005.png)

Chaque action sauvegarde votre monde entier dans le fichier `.txt` correspondant au monde. Donc votre monde ne se supprimera pas tout seul.
Certaines actions ont étés abandonnées comme l'inventaire ou les ennemis, même si ça aurait été intéressant à voir.
