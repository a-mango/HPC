#import "@local/heig-report:1.0.0": *
#show: conf.with(
  title: [HPC - Lab 06],
  authors: (
    (
      name: "Aubry Mangold",
      affiliation: "ISCS, HEIG-VD",
      email: "aubry.mangold@heig-vd.ch",
    ),
  ),
  date: "2025-05-27",
)

= Introduction

Ce rapport relate l'utilisation des outils Perf et Powercap pour l'analyse de consommation énergétique de programmes informatiques dans le cadre du cours High-Performance Computing de la HEIG-VD.

== Topologie du système

La machine de développement, de test et de banchmark est basée sur un processeur AMD Ryzen 7 Pro 6850U, a 8 coeurs physiques et 16 threads. Chaque coeur dispose de 32 KiB de cache L1 données et 32 KiB de cache L1 instructions, de 512 KiB de cache L2 par cœur (4 MiB au total) et d’un cache L3 global de 512 KiB. Le système possède 32 GB de RAM DDR4. L'OS utilise le kernel Linux 6.14.3.

== Choix des outils

Les outils Perf et Powercap ont étés retenus pour l'analyse de performance et de consommation d'énergie des programmes, l'outil Likwid n'était pas compatible avec la machine de test.

Les tests ont été menés sur une quantité de _bins_ allant de 2 à 64. Le postulat est que l'implémentation SIMD choisie présenterai des économies d'énergie de plus en plus significatives vis à vis de la version originale au fur et à mesure de l'augmentation du nombre de bin.

= Powercap

Powercap a été intégré aux exécutable afin de tester la consommation d'énergie locale de la fonction `k-means`. L'utilisation de la librairie ne semble pas être correcte car les valeurs indiquées par Powercap sont entre 0 et une valeur proche du maximum, c.f. @fig-powercap. L'implémentation semble correcte et les permissions sur les fichiers systèmes ont été vérifiées. Il est possible que l'outil ne soit pas compatible avec le matériel ou le kernel utilisé. A titre anecdotique, la gestion de la consommation d'énergie est notablement difficile pour les processeur AMD avec Linux. Aucune solution au problème n'a été trouvée.

#figure(
  ```txt
  Original,4,LocalPerf,3,41.87
  Original,4,PowerCap,1,0.0
  Original,4,PowerCap,2,18446603413524.695
  Original,4,PowerCap,3,0.0
  Original,8,GlobalPerf,1,84.88
  Original,8,GlobalPerf,2,76.63
  Original,8,GlobalPerf,3,73.58
  Original,8,LocalPerf,1,74.45
  Original,8,LocalPerf,2,77.87
  Original,8,LocalPerf,3,83.47
  ```,
  caption: "Consommation d'énergie mesurée avec Powercap",
) <fig-powercap>

= Perf local

Les résultats de l'utilisation de Perf en mode local sont présentés dans @fig-perf. On observe que la version SIMD du programme présente une consommation d'énergie beaucoup plus basse (approximativement 8 fois moins pour un test avec 64 bins) que la version originale du programme.

#figure(
  image("assets/energy_consumption.png"),
  caption: "Consommation d'énergie mesurée avec Perf",
) <fig-perf>

= Perf global

Il y a une nette correlation entre l'utilisation d'énergie de la fonction `k-means` que d'une programme entier. Cela indique non seulement que la fonction `k-means` est gourmande en ressources, mais aussi que l'implémentation de la fonction est efficace dans les deux cas.

= Conclusion

L'utilisation d'outils de mesure de performance énergétique semble donc adaptée à analyser quelles parties d'un programme sont les plus gourmandes en ressources. Leur utilisation n'est cependant pas facile et peut dépendre de la configuration et de l'architecture du système.

On conclu en observant les graphiques de @fig-perf que l'utilisation de technologies telles que SIMD peut permet une économie très important d'énergie lors de l'exécution d'un programme. Cela s'explique, dans ce cas précis, par l'utilisation de moins d'instruction et par conséquent de moins de cycles d'horloge pour réaliser une même tâche.

Dans le cas d'un programme couramment utilisé tel que le noyau Linux, ces petits gains cumulés peuvent se traduire en des économies d'énergie de grande ampleur.
