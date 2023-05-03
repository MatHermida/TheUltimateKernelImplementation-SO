#include "../include/consola.h"

int main(int argc, char** argv) {
	// ***** INICIAR CONSOLA ***** //
    t_config* config = iniciar_config(argv[1]);
    t_log* logger    = iniciar_logger("consola.log", "Consola");
    t_consola_config lectura_de_config = leer_consola_config(config);

	log_info(logger, "IP KERNEL: %s", lectura_de_config.IP_KERNEL); //el %s es para que no tire warning para tomarlo como literal cadena
	log_info(logger, "Puerto KERNEL: %s", lectura_de_config.PUERTO_KERNEL);

    // ***** PARSEAR INSTRUCCIONES ***** /
    t_list* instrucciones = parsearPseudocodigo(logger, argv[2]);
    //log_warning(logger, "Cant bytes a mandar: %d", calculo_tamanio_msj(instrucciones));

    // ***** CONECTAR A KERNEL ***** /
	int socket_kernel = crear_conexion(lectura_de_config.IP_KERNEL, lectura_de_config.PUERTO_KERNEL);

	// ***** ENVIAR INSTRUCCIONES A KERNEL ***** //
	enviar_instrucciones(socket_kernel, instrucciones);
	//esperar_confirmacion(socket_kernel);
	esperar_fin_proceso(socket_kernel, logger);

	// ***** LIBERAR MEMORIA Y CERRAR ***** //
    list_destroy_and_destroy_elements(instrucciones, (void*)destruir_instruccion);
    log_destroy(logger);
    config_destroy(config);
    liberar_estructura_config(lectura_de_config);

    close(socket_kernel);
    return 0;
}

void destruir_instruccion(t_instruccion* instruccion) { // No se porqué no me reconoce el de las utils
	free(instruccion->nombre);
	list_destroy_and_destroy_elements(instruccion->parametros, (void*)destruir_parametro);
	free(instruccion);
}

void destruir_parametro(char* parametro) {
	free(parametro);
}

void esperar_fin_proceso(int socket_kernel, t_log* logger) {
	t_msj_kernel_consola respuesta = recibir_fin_proceso(socket_kernel);
	if(respuesta == FINALIZACION_OK) {
		log_info(logger, "Proceso terminado");
	}
	else {
		log_error(logger, "Hubo un error en el proceso"); //falta enviar el handshake en el Kernel cuando se termina el proceso
	}
}
