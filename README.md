# 🎸 Amplificador Digital de Guitarra (VST3 / Standalone)

![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![JUCE](https://img.shields.io/badge/JUCE-5A6986?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![SQLite](https://img.shields.io/badge/SQLite-07405E?style=for-the-badge&logo=sqlite&logoColor=white)

## 📝 Descripción
Este proyecto es un emulador de amplificador de guitarra digital desarrollado en **C++** utilizando el framework **JUCE**. Está diseñado para procesar audio en tiempo real con latencia ultrabaja, ofreciendo a los músicos un control total sobre su tono mediante una interfaz gráfica interactiva.

Este sistema fue diseñado y desarrollado por **Sergio Butzmann**, **Gustavo Guzman**, **Diego Alonso** y su equipo como parte del proyecto de la materia de **Ingeniería de Software**, dentro del programa de **Ingeniería en Inteligencia Artificial**. 

El software puede ser ejecutado como una aplicación de escritorio independiente (Standalone) o como un plugin (VST3/AU) dentro de una Estación de Trabajo de Audio Digital (DAW).

## ✨ Características Principales
* **Procesamiento de Audio en Tiempo Real:** Algoritmos DSP optimizados para emular la respuesta de un amplificador físico.
* **Gestión de Presets:** Sistema integrado de base de datos local mediante **SQLite** para guardar, cargar y organizar configuraciones de usuario personalizadas.
* **Interfaz de Usuario (GUI) Reactiva:** Controles visuales intuitivos (perillas, faders y botones) vinculados directamente al motor de audio.
* **Multiplataforma:** Código base exportable para Windows y macOS.

## 🛠️ Arquitectura y Tecnologías
* **Lenguaje Core:** C++
* **Audio Framework:** JUCE
* **Base de Datos:** SQLite (Embebido)
* **Control de Versiones y Colaboración:** Git / GitHub

El repositorio contiene tanto la implementación del código fuente como la documentación arquitectónica (incluyendo el modelo Entidad-Relación de la base de datos).

## 🚀 Instrucciones de Construcción (Build)
Para compilar este proyecto en tu máquina local:

1. Clona este repositorio:
   ```bash
   git clone [https://github.com/TuUsuario/Nombre-Repo-Amp.git](https://github.com/TuUsuario/Nombre-Repo-Amp.git)
