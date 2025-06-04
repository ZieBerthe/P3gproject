# Projet de Scène 3D Infinie avec Sol Dynamique

## Auteurs
[Nom des membres du groupe]

## Description du Projet
Ce projet consiste en la création d'une scène 3D interactive où le sol se déplace de manière continue, créant l'illusion d'un terrain infini. Le projet utilise OpenGL et la bibliothèque CGP pour la gestion graphique.

## Fonctionnalités Techniques

### 1. Génération de Terrain
- Utilisation d'un système de chunks pour optimiser le rendu
- 10 chunks dans chaque direction (total de 100 chunks)
- Longueur de chunk : 10.0 unités
- Résolution : 17x17 vertices par chunk
- Distance de rendu ajustable

### 2. Génération de Hauteur
- Algorithme de bruit procédural pour la génération du terrain
- Paramètres configurables :
  - Amplitude : 10.0
  - Octaves : 3
  - Rugosité : 0.03
- Système de lissage pour des transitions naturelles

### 3. Animation du Sol
- Système de points de déformation dynamiques
- Paramètres d'animation :
  - Amplitude maximale : 2.0
  - Facteur d'aplatissement : 4.0
  - Vitesse variable : 0.05 à 0.6
- Génération procédurale des mouvements

### 4. Système de Caméra
- Contrôle de caméra en 2D
- Position initiale : (1.0, 10.0, 10.0)
- Hauteur de caméra : 1.0
- Mode flottant disponible

### 5. Éléments Visuels
- Skybox pour l'environnement
- Textures pour :
  - Le terrain
  - L'eau
  - La terre
- Support du wireframe et des frames de référence

### 6. Physique
- Système de gravité implémenté
- Détection de chute
- Temps de chute suivi

## Aspects Techniques Notables
1. Optimisation du rendu avec le système de chunks
2. Génération procédurale du terrain
3. Animation fluide du sol
4. Système de caméra interactif
5. Gestion des textures et des matériaux
6. Implémentation de la physique

## Démarche de Développement
Le projet a été développé en suivant une approche modulaire :
1. Mise en place de la structure de base avec CGP
2. Implémentation du système de génération de terrain
3. Développement du système d'animation
4. Intégration des éléments visuels
5. Ajout des fonctionnalités interactives

## Conclusion
Ce projet démontre l'implémentation réussie d'une scène 3D interactive avec un terrain dynamique et infini. Les techniques utilisées permettent une expérience utilisateur fluide tout en maintenant des performances optimales.

## Vidéo de Démonstration
[Lien vers la vidéo de démonstration]
