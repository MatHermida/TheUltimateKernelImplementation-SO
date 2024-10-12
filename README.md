# T.U.K.I - The Ultimate Kernel Implementation
Este proyecto es el trabajo práctico cuatrimestral de la cátedra de Sistemas Operativos, cuyo objetivo es simular un sistema distribuido que gestione procesos, memoria y archivos. El sistema está compuesto por diferentes módulos que interactúan entre sí para simular un kernel que coordina los recursos y tareas en un entorno simulado.

## Descripción del proyecto
El proyecto consiste en desarrollar una implementación que simule un Kernel que coordine la ejecución de procesos en un sistema distribuido, gestionando peticiones a la CPU, memoria y sistema de archivos. Cada proceso será generado a partir del módulo Consola, enviado al Kernel, ejecutado por la CPU, y almacenará o recuperará datos del sistema de Memoria y el File System.

### Módulos:
- **Consola**: Permite crear procesos desde pseudocódigo que luego son gestionados por el Kernel. Inicia los procesos a ejecutar en el sistema y los envía al kernel.
- **Kernel**: El núcleo del sistema encargado de coordinar la ejecución de los procesos. Administra la planificación de procesos, la gestión de memoria y coordina la interacción con la CPU y el sistema de archivos.
- **CPU**: Ejecuta las instrucciones de los procesos y realiza operaciones con la memoria y el sistema de archivos.
- **Memoria**: Gestiona la memoria de los procesos bajo un esquema de segmentación. Asegura que la CPU pueda leer y escribir en los segmentos correspondientes.
- **File System**: Maneja las operaciones de archivos, como la creación, lectura y escritura, utilizando un esquema simplificado inspirado en un sistema de archivos tipo UNIX.

### Tecnologías utilizadas
- **Lenguaje**: **C**, Lenguaje principal para la implementación de los módulos.
- **Sistema Operativo**: **Linux**, utilizando directamente este sistema operativo o con el uso de Maquinas Virtuales (VM VirtualBox)
- **Sockets**: Para la comunicación entre los diferentes módulos.
- **Makefiles**: Para la compilación y gestión del proyecto.
- **Loggers**: Para registrar los eventos importantes durante la ejecución de los módulos.
- **Commons**: Librerías proporcionadas por la cátedra para facilitar la serialización, manejo de archivos de configuración, logs, etc.

### Requisitos
- **Linux**: El sistema está diseñado para ser ejecutado en un entorno Linux.
- **gcc**: Para compilar los módulos.
- **Commons**: Librerías comunes utilizadas en el proyecto (deberás clonarlas desde el repositorio oficial).