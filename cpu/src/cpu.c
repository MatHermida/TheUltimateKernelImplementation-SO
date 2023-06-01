#include <stdio.h>
#include <stdlib.h>
#include <utils.h>
#include "../include/configuracion_cpu.h"
#include "../include/ejecucion_instrucciones.h"
#include "../include/escuchador_de_memoria.h"

int main(int argc, char** argv) {
    t_config* config = iniciar_config(argv[1]);
    lectura_de_config = leer_cpu_config(config);

    logger = iniciar_logger("cpu.config", "CPU");

    //TODO: DESCOMENTAR
    /*
    socket_memoria = crear_conexion(lectura_de_config.IP_MEMORIA, lectura_de_config.PUERTO_MEMORIA);
    enviar_handshake(socket_memoria, CPU);
    t_handshake respuesta = recibir_handshake(socket_memoria);
    if(respuesta == ERROR_HANDSHAKE){
        log_error(logger, "CPU no se pudo conectar a Memoria");
        exit(EXIT_FAILURE);
    }
	*/

    list_solicitudes_acceso_memoria = list_create();

    pthread_mutex_init(&mutex_list_solicitudes_acceso_memoria, NULL);

	pthread_t escuchador_de_memoria;
    pthread_create(&escuchador_de_memoria, NULL, (void*)log_acceso_memoria, NULL);
    pthread_detach(escuchador_de_memoria);

    int socket_cpu = iniciar_servidor("127.0.0.1", lectura_de_config.PUERTO_ESCUCHA); //TODO: hardcodeado
    log_warning(logger, "CPU lista para recibir al Kernel");
    socket_kernel = esperar_cliente(socket_cpu);

    log_warning(logger, "Kernel se conecto a CPU");

    while (1) {
    	int cod_op = recibir_operacion(socket_kernel);
    	switch (cod_op) {
    		case PCB_A_EJECUTAR:
    			t_pcb* pcb = recibir_pcb(socket_kernel); //deserializar hace el malloc
    			ejecutar_instrucciones(pcb);
    			liberar_pcb(pcb); //TODO: valgrind dice que se está haciendo algún free dos veces
    			break;
        }
    }

	return EXIT_SUCCESS;
}
