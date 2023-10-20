# BMP280 Driver

## Description
Ce répertoire contient un driver pour le capteur de pression et de température BMP280. Le driver a été développé pour fonctionner avec une interface I2C, et il permet de lire la température et la pression à partir du capteur. Le code est écrit en langage C.

## Architecture
Le driver BMP280 est structuré en plusieurs fonctions pour initialiser le capteur, récupérer les valeurs d'étalonnage, lire la température et la pression, et effectuer des opérations de compensation. Voici un aperçu des principales fonctions :

- `BMP280_init`: Cette fonction initialise le capteur BMP280 en configurant le mode de fonctionnement et les échantillonnages. Elle vérifie également l'ID du composant pour s'assurer qu'il s'agit bien d'un BMP280.

- `BMP280_etalonnage`: Cette fonction récupère les valeurs d'étalonnage à partir du capteur BMP280. Ces valeurs d'étalonnage sont essentielles pour la compensation des données de température et de pression.

- `BMP280_get_temp`: Cette fonction lit la température brute à partir du capteur, puis la compense en utilisant les valeurs d'étalonnage pour obtenir la température en degrés Celsius.

- `BMP280_get_pressure`: Cette fonction lit la pression brute à partir du capteur, puis la compense en utilisant les valeurs d'étalonnage pour obtenir la pression en Pascals.

## Utilisation
Pour utiliser le driver BMP280, suivez ces étapes :

1. Incluez le fichier d'en-tête `BMP280.h` dans votre code.

2. Initialisez une structure `h_BMP280_t` pour stocker les données du capteur et configurez la structure selon vos besoins.

3. Appelez la fonction `BMP280_init` pour initialiser le capteur.

4. Appelez la fonction `BMP280_etalonnage` pour récupérer les valeurs d'étalonnage du capteur.

5. Utilisez les fonctions `BMP280_get_temp` et `BMP280_get_pressure` pour lire la température et la pression.

## Exemple
Voici un exemple d'utilisation du driver BMP280 :

```c
#include "BMP280.h"

h_BMP280_t bmp280_data;

int main() {
    // Initialisation du capteur
    BMP280_init(&bmp280_data);

    // Récupération des valeurs d'étalonnage
    BMP280_etalonnage(&bmp280_data);

    // Lecture de la température
    BMP280_get_temp(&bmp280_data);
    
    // Lecture de la pression
    BMP280_get_pressure(&bmp280_data);

    // Utilisation des données de température et de pression
    // ...

    return 0;
}
